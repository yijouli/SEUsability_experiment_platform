import json
import platform
import random
import shutil
import time
from pathlib import Path
import hashlib
import subprocess
import signal
from multiprocessing import Process
from threading import Thread

from libbs.api import DecompilerInterface
from libbs.artifacts import (
    Artifact, ArtifactFormat, Context, FunctionHeader, StackVariable, GlobalVariable, Struct, Enum, Comment
)
from dailalib.llm_chat import get_llm_chat_creator

from . import OAI_API_KEY

WATCHED_ARTIFACTS = [FunctionHeader, StackVariable, GlobalVariable, Struct, Enum, Comment]
METADATA_PATH = Path("~/").expanduser().absolute() / "reallm_data" / "metadata.json"
WARMUP_CHALL_NAME = "warmup"
SECRETS_CHALL_NAME = "secrets"
RPC_CHALL_NAME = "rpc"
IS_MACOS = 'macOS' in platform.platform()

deci: DecompilerInterface = None
litellm_api: "ReallmAIAPI" = None
llm_chat = None
log_dir: Path = None
start_time = None
started = False
stopped = False
screen_recording_proc = None
writeup_saver_proc = None
keylogger_proc = None


def get_logger():
    import logging
    return logging.getLogger("reallm")

#
# Action logs
#


def log_llm_prompt_style_change(prompt_style):
    _l = get_logger()
    data = {"type": "prompt-style-changed", "data": {"prompt_style": prompt_style}, "time": time.time()}
    _l.info(data)


def log_llm_model_change(model):
    _l = get_logger()
    data = {"type": "model-changed", "data": {"model": model}, "time": time.time()}
    _l.info(data)


def log_undo(*args, **kwargs):
    _l = get_logger()
    data = {"type": "undo", "data": kwargs, "time": time.time()}
    _l.info(data)


def log_llm_query(query_name, model, prompt_style, function, decompilation: str, resp: str, total_time=None, cost=None, success=False, **kwargs):
    global log_dir
    if log_dir is None:
        return

    _l = get_logger()
    dec_hash = hashlib.md5(decompilation.encode()).hexdigest()
    data = {
        "type": "query",
        "data": {
            "query_name": query_name,
            "prompt_style": prompt_style,
            "model": model,
            "function": hex(function.addr) if function and function.addr is not None else None,
            "dec_hash": dec_hash,
            "response_time": total_time,
            "response": resp,
            "cost": cost,
            "success": success
        },
        "time": time.time()
    }

    _l.info(data)
    with open(log_dir / f"{dec_hash}.c", "w") as f:
        f.write(decompilation)


def log_ctx_change(ctx: Context, *args, **kwargs):
    _l = get_logger()
    ctx_json = ctx.dumps(fmt=ArtifactFormat.JSON)
    data = {"type": Context.__name__, "data": ctx_json, "time": time.time()}
    _l.info(data)


def log_artifact_change(artifact: Artifact, *args, **kwargs):
    _l = get_logger()
    artifact_json = artifact.dumps(fmt=ArtifactFormat.JSON)
    data = {"type": str(artifact.__class__.__name__), "data": artifact_json, "time": time.time()}
    _l.info(data)


def log_llm_chat_recv(msg, model=None):
    _l = get_logger()
    data = {"type": "llm_chat_recv", "data": msg, "time": time.time(), "model": model}
    _l.info(data)


def log_llm_chat_send(msg, model=None, prompt_style=None):
    _l = get_logger()
    data = {"type": "llm_chat_send", "data": msg, "time": time.time(), "model": model}
    _l.info(data)


def log_llm_chat_open(*args, context=None, model=None, prompt_style=None, **kwargs):
    _l = get_logger()
    ctx_data = context.dumps(fmt=ArtifactFormat.JSON) if context else {}
    data = {"type": "llm_chat_open", "context": ctx_data, "time": time.time(), "model": model}
    _l.info(data)

#
# Events
#


def save_writeup_routine(writeup_path: Path, log_dir_path: Path, interval: int = 120):
    while True:
        int_time = time.time()
        shutil.copy(str(writeup_path), str(log_dir_path / f"writeup_{int_time}.md"))
        time.sleep(interval)


def start_writeup_saver(writeup_path: Path):
    global writeup_saver_proc
    global log_dir

    # In IDA on MacOS, we can't start a new process
    if not IS_MACOS:
        writeup_saver_proc = Process(target=save_writeup_routine, args=(writeup_path, Path(log_dir)))
    else:
        writeup_saver_proc = Thread(target=save_writeup_routine, args=(writeup_path, Path(log_dir)), daemon=True)

    writeup_saver_proc.start()


def stop_writeup_saver():
    global writeup_saver_proc
    if writeup_saver_proc is None:
        print("Writeup saver not started or already stopped. Exiting...")
        return

    writeup_saver_proc.terminate()
    writeup_saver_proc.join()
    writeup_saver_proc = None


def start_key_logger():
    global keylogger_proc
    print("Starting VM key logger...")
    # "xinput test-xi2 --root > ~/reallm_data/keylog.txt"
    key_logger_proc = subprocess.Popen(
        [
            "xinput",
            "test-xi2",
            "--root",
            ">",
            str(log_dir / "keylog.txt")
        ],
        shell=True
    )
    keylogger_proc = subprocess.Popen(
        "xinput test-xi2 --root | awk '{ print strftime(\"%Y-%m-%d %H:%M:%S\"), $0 }' > " + str(log_dir / "keylog.txt"),
        shell=True
    )

    if key_logger_proc is None:
        raise Exception("Failed to start key logger")


def stop_key_logger():
    global keylogger_proc
    if keylogger_proc is None:
        print("Key logger not started or already stopped")
        return

    keylogger_proc.send_signal(signal.SIGINT)
    time.sleep(1)
    keylogger_proc.terminate()
    keylogger_proc.wait()
    keylogger_proc = None


def start_screen_recording(*args, **kwargs):
    global screen_recording_proc
    print("Starting screen recording...")
    # record in segments of 2 minutes
    ffmpeg_binary = "ffmpeg" if IS_MACOS else "/usr/bin/ffmpeg"
    screen_recording_proc = subprocess.Popen(
        [
            ffmpeg_binary,
            "-f", "x11grab",
            "-framerate", "30",
            "-i", ":0.0+0.0",
            "-vf", "scale=1512:802",
            "-vcodec", "libx264",
            "-preset", "medium",
            "-crf", "23",
            "-pix_fmt", "yuv420p",
            "-f", "segment",
            "-segment_time", "120",
            "-reset_timestamps", "1",
            f"{str(log_dir / 'checkpoint_%05d.mp4')}",
            "-y"
        ]
    )

    if screen_recording_proc is None:
        raise Exception("Failed to start screen recording")


def stop_screen_recording():
    global screen_recording_proc
    if screen_recording_proc is None:
        print("Screen recording not started or already stopped. Exiting...")
        return

    # send SIGINT to stop the recording
    screen_recording_proc.send_signal(signal.SIGINT)
    time.sleep(1)
    screen_recording_proc.terminate()
    screen_recording_proc.wait()

    # Merge segments
    # 1. Generate the list of segment files for FFmpeg concat
    segment_list_path = log_dir / "segment_list.txt"
    with open(segment_list_path, "w") as f:
        for segment_file in sorted(log_dir.glob("checkpoint_*.mp4")):
            f.write(f"file '{segment_file}'\n")

    # 2. Use FFmpeg to concatenate segments into a single output file
    final_output_path = log_dir / "final_output.mp4"
    ffmpeg_binary = "ffmpeg" if 'macOS' in platform.platform() else "/usr/bin/ffmpeg"
    subprocess.run(
        [
            ffmpeg_binary,
            "-f", "concat",
            "-safe", "0",
            "-i", str(segment_list_path),
            "-c", "copy",
            str(final_output_path),
            "-y"
        ]
    )

    screen_recording_proc = None


def start_study_on_click(*args, **kwargs):
    if started:
        return

    start_study(*args, **kwargs)


def start_study(*args, **kwargs):
    global start_time
    global log_dir
    global started
    global deci

    if started:
        print("Study already started.")
        return

    import logging
    logging.getLogger("reallm").addHandler(logging.NullHandler())
    from reallm.logger_conf import setup_logger
    base_dir = Path("~/").expanduser().absolute() / "reallm_data"
    binary_name = Path(deci.binary_path).name
    log_dir = base_dir / binary_name
    Loggers = setup_logger(binary_name, base_dir)
    loggers = Loggers()
    del Loggers
    del logging

    _l = get_logger()
    litellm_api.logger = _l
    #litellm_api.ask_prompt_style()
    deci.info(f"Starting REaLLM Study with: model={litellm_api.model} | prompting_style={litellm_api.prompt_style}")
    deci.info(f"To change the above, right-click, go to REaLLM->Settings->Change model or Change prompt style.")
    deci.info(f"Your screen and clicks will now be recorded! Good luck!")
    deci.start_artifact_watchers()

    # start recording the screen
    try:
        start_screen_recording()
    except Exception as e:
        _l.error(f"Failed to start screen recording: {e}, you are likely testing locally.")

    # start the key logger
    try:
        start_key_logger()
    except Exception as e:
        _l.error(f"Failed to start key logger: {e}, you are likely testing locally.")

    start_time = time.time()
    started = True

    # start saving the writeup
    writeup = Path(f"/home/hacker/{Path(deci.binary_path).with_suffix('').name}-writeup.md")
    start_writeup_saver(writeup)


def print_flag(_deci, no_popup=False):
    flag_loc = Path("/flag")
    flag_data = ""
    if flag_loc.exists():
        flag_data = flag_loc.read_text()

    # inform the user that the study has ended
    if not no_popup:
        try:
            _deci.gui_popup_text(
                "Thank you for participating in the REaLLM study. Please make sure you have saved your writeup before "
                f"closing this window. After saving your writeup you can submit this flag: {flag_data} . To submit the "
                "flag, first highlight it. Then go to the far left of your screen. You should see a little tab to pull out."
                "Click on it. Then click on the clipboard icon. Your flag (the highlighted text in the VM) should be in"
                "the cilp board. Simply copy it to your normal clipboard. Now click on the flag on the top tab of your"
                "screen and paste and submit it there."
                f"If this is the last challenge you are doing, please fill out the post-study survey found on the "
                f"challenges page."
            )
        except Exception:
            print("Error, study likely stopped once already!")

    print(f"Your flag is: {flag_data}")


def save_writeup(_deci):
    # copy the writeup to the log directory
    writeup = Path(f"/home/hacker/{Path(_deci.binary_path).with_suffix('').name}-writeup.md")
    if writeup.exists():
        shutil.copy(writeup, log_dir / "writeup.md")


def end_on_close(*args, **kwargs):
    global stopped
    if not stopped:
        end_study(*args, no_popup=True, **kwargs)


def end_study(*args, no_popup=False, **kwargs):
    global screen_recording_proc
    global started
    global stopped
    total_time = time.time() - start_time
    litellm_api.logger.info({"type": "study_end", "time": total_time})

    if not started:
        print("Study has not started yet!")
        return
    if stopped:
        print("Study has already ended, but we will print the flag in case you missed it earlier.")
        print_flag(deci, no_popup=no_popup)
        return

    # stop the screen recording
    stop_screen_recording()
    stop_key_logger()

    # gather all the sites visted using firefox
    moz_files = Path("/home/hacker/.mozilla/firefox")
    if moz_files.exists():
        user_moz_files = list(moz_files.glob("*.default-release"))
        if user_moz_files:
            user_moz_files = user_moz_files[0]
            moz_history = user_moz_files / "places.sqlite"
            shutil.copy(moz_history, log_dir / "places.sqlite")

    # stop the artifact watchers
    deci.stop_artifact_watchers()

    # print flag and popup, also, save the writeup twice because the user may just close the
    # entire screen without saving the writeup
    save_writeup(deci)
    print_flag(deci, no_popup=no_popup)
    save_writeup(deci)

    print(f"Study has ended! Thank you for participating. You took {total_time} seconds to complete the study. Please close the VM window now")
    stopped = True


def get_hooked_llm_chat_creator(litellm_api):
    chat = get_llm_chat_creator(litellm_api)

    def hooked_llm_chat_creator(*args, **kwargs):
        log_llm_chat_open(*args, context=kwargs.get("context", None))
        litellm_api.info("Opening LLM Chat! Note this model has no context of your current function. You must provide it. Please close when moving to a new function.")
        return chat(*args, **kwargs)

    return hooked_llm_chat_creator


#
# Plugin Creation
#

def discover_challenge_name():
    challenge_binary_names = ["warmup", "secrets.out", "rpc.out"]
    challenge_dir = Path("/challenge")
    for name in challenge_binary_names:
        if (challenge_dir / name).exists():
            if name == "warmup":
                return WARMUP_CHALL_NAME
            elif name == "secrets.out":
                return SECRETS_CHALL_NAME
            elif name == "rpc.out":
                return RPC_CHALL_NAME

    print("Could not determine the challenge name! Likely running locally.")
    return None


def create_plugin(*args, **kwargs):
    global deci
    global litellm_api

    # create/check the metadata file
    if not METADATA_PATH.exists():
        METADATA_PATH.parent.mkdir(parents=True, exist_ok=True)
        METADATA_PATH.write_text("{}")
    metadata = json.loads(METADATA_PATH.read_text())

    # decide if the user will h

    from .reallm_aiapi import ReallmAIAPI
    litellm_api = ReallmAIAPI(
        delay_init=True, fit_to_tokens=False, chat_use_ctx=False,
        chat_event_callbacks={"send": log_llm_chat_send, "receive": log_llm_chat_recv}
    )
    # hardcode the API key
    litellm_api.api_key = OAI_API_KEY

    # create context menus for prompts
    prompt_ctx_menus = {
        f"REaLLM/{prompt_name}": (prompt.desc, getattr(litellm_api, prompt_name))
        for prompt_name, prompt in litellm_api.prompts_by_name.items()
    }
    # create context menu for llm chat
    prompt_ctx_menus["REaLLM/chat"] = ("Open free prompt...", get_hooked_llm_chat_creator(litellm_api))

    # create context menus for study actions
    study_ctx_menus = {
        #"REaLLM/Study/start_study": ("Start REaLLM study...", start_study),
        "REaLLM/Study/end_study": ("End REaLLM study...", end_study)
    }
    # create context menus for llm settings
    settings_ctx_menus = {
        #"REaLLM/Settings/update_api_key": ("Update API key...", litellm_api.ask_api_key),
        "REaLLM/Settings/update_pmpt_style": ("Change prompt style...", litellm_api.ask_prompt_style),
        "REaLLM/Settings/update_model": ("Change model...", litellm_api.ask_model)
    }

    #
    # Decompiler Plugin Registration
    #

    force_decompiler = kwargs.pop("force_decompiler", None)
    deci = DecompilerInterface.discover(
        force_decompiler=force_decompiler,
        # decompiler-creation args
        plugin_name="REaLLM",
        init_plugin=True,
        # try to save the study on the close of IDA, regardless of if they ended the study!
        decompiler_closed_callbacks=[end_on_close],
        # start record clicks on the very first click
        force_click_recording=True,
        # record mouse movement
        track_mouse_moves=True,
        # we would normally initalize the gui_ctx_menu_actions here, but we need to do a check first
        gui_init_args=args,
        gui_init_kwargs=kwargs
    )
    litellm_api.init_decompiler_interface(decompiler_interface=deci)

    # add menus based on the challenge
    challenge_name = discover_challenge_name()
    if challenge_name == WARMUP_CHALL_NAME:
        use_llm = True
    elif challenge_name is None:
        # for local debugging
        use_llm = True
    else:
        llm_used_on = metadata.get("llm_used_on", None)
        if llm_used_on is None:
            # this is the first time we are deciding if the user will use LLM
            # we need to pick a challenge to allow LLMs o
            challs = [SECRETS_CHALL_NAME, RPC_CHALL_NAME]
            llm_chall = random.choice(challs)
            metadata["llm_used_on"] = llm_chall
            llm_used_on = llm_chall
            # save the metadata
            METADATA_PATH.write_text(json.dumps(metadata))

        use_llm = challenge_name == llm_used_on

    if use_llm:
        deci.gui_register_ctx_menu_many(prompt_ctx_menus)
        deci.gui_register_ctx_menu_many(study_ctx_menus)
        deci.gui_register_ctx_menu_many(settings_ctx_menus)
        print_msg = "You will be using an LLM for this challenge!"
    else:
        deci.gui_register_ctx_menu_many(study_ctx_menus)
        print_msg = "You will not be using an LLM for this challenge!"
    deci.info(print_msg)

    #
    # Setup Logging
    #

    litellm_api.query_callbacks.append(log_llm_query)
    deci.artifact_change_callbacks[Context].append(log_ctx_change)
    deci.artifact_change_callbacks[Context].append(start_study_on_click)
    deci.undo_event_callbacks.append(log_undo)
    for artifact_cls in WATCHED_ARTIFACTS:
        deci.artifact_change_callbacks[artifact_cls].append(log_artifact_change)

    return deci.gui_plugin
