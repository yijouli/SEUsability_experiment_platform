import textwrap
import time

from dailalib.api.litellm.litellm_api import LiteLLMAIAPI
from .logging import log_llm_model_change, log_llm_prompt_style_change


class ReallmAIAPI(LiteLLMAIAPI):
    IDK_OPTION = "I don't know"

    def __init__(self, *args, **kwargs):
        # force overwrite of model and prompt_style
        super().__init__(*args, model=None, prompt_style=None, **kwargs)
        self._whitelist_models = {
            "gpt-4o",
            "gpt-4o-mini",
            "gpt-4-turbo",
            "claude-3-5-sonnet-20240620",
            "gemini/gemini-pro",
        }
        for model in list(self.MODEL_TO_TOKENS.keys()):
            if model not in self._whitelist_models:
                del self.MODEL_TO_TOKENS[model]

    def ask_prompt_style(self, *args, **kwargs):
        if self._dec_interface is not None:
            from dailalib.api.litellm.prompts import ALL_STYLES, DEFAULT_STYLE

            style_choices = ALL_STYLES.copy()
            if self.prompt_style:
                style_choices.remove(self.prompt_style)
                style_choices = [self.prompt_style] + style_choices
            else:
                style_choices = ["", self.IDK_OPTION] + style_choices

            p_style = self._dec_interface.gui_ask_for_choice(
                textwrap.dedent("""
                In some cases, they way you provide data to an LLM can change the quality of its output, known as prompt engineering. 
                Please select from one of the most common prompt engineering styles below. If you do not know what to 
                choose, you may choose the 'I don't know' option."""),
                style_choices,
                title="REaLLM"
            )
            if p_style == self.IDK_OPTION:
                p_style = DEFAULT_STYLE
            elif p_style:
                if p_style not in ALL_STYLES:
                    self._dec_interface.error(f"Prompt style {p_style} is not supported.")
                    return

            if p_style:
                self._set_prompt_style(p_style)
                self._dec_interface.info(f"Prompt style set to {p_style}")
                log_llm_prompt_style_change(p_style)
            else:
                self._dec_interface.error("Prompt style not set!")

    def ask_model(self, *args, **kwargs):
        if self._dec_interface is not None:
            model_choices = list(LiteLLMAIAPI.MODEL_TO_TOKENS.keys())
            if self.model:
                model_choices.remove(self.model)
                model_choices = [self.model] + model_choices
            else:
                model_choices = ["", self.IDK_OPTION] + model_choices

            model = self._dec_interface.gui_ask_for_choice(
                textwrap.dedent("""
                In some cases, the LLM model you use can change the quality of answers you receive. 
                Please select from one of the most common models below. If you do not know what to 
                choose, you may choose the 'I don't know' option."""),
                model_choices,
                title="REaLLM"
            )
            if model == self.IDK_OPTION:
                model = self.DEFAULT_MODEL
            elif model:
                if model not in self.MODEL_TO_TOKENS:
                    self._dec_interface.error(f"Model {model} is not supported.")
                    return

            if model:
                self._set_model(model)
                self._dec_interface.info(f"Model set to {model}")
                log_llm_model_change(model)
            else:
                self._dec_interface.error("Model not set!")
