# Infrastructure 
Below you will find information for running the infra and building the docker container used in the study.

Note that REaLLM is the study-version of the DAILA plugin.


## REaLLM Plugin
The REaLLM plugin is a plugin for your decompiler that will provide you with various LLM features done in the study.
It is written in Python, and can be installed in IDA Pro, Ghidra, Binary Ninja, or angr.

Note: If you are trying to reproduce functionality from the study, you will need to use IDA Pro.

You can install it with:
```
pip install -e ./reallm-plugin && reallm --install
```

You should now be able to start IDA Pro and see the REaLLM menu.
Right-click on a function and select "REaLLM" to see the options.
You should be able to use all the features of the plugin specified in the study.

### Docker Use
Place your IDA Pro 9 installation in the `./idapro-reallm-9` directory, then build the docker contianer:
```
docker build . -t reallm
```

Then run the docker container.

**NOTE**: you need to update the LLM API keys in the `reallm/__init__.py` file before building the container.


## Dojo
We host our own version of [pwn.college](https://pwn.college), which comes with a VNC that users can use in their
web browser.

### Hosting
It is recommended to run the dojo on a machine with at least 16GB of RAM and 4 CPU cores.
To run the dojo yourself, first install pwncollege dojo and docker:
```bash 
curl -fsSL https://get.docker.com | /bin/sh

DOJO_PATH="./dojo"
DATA_PATH="./dojo/data"

git clone https://github.com/pwncollege/dojo "$DOJO_PATH"
(cd dojo && git checkout 9e4d2f769c8183588759c043ba5344547fc853cd && git apply ../dojo.patch)
docker build -t pwncollege/dojo "$DOJO_PATH"
```

Then run our start script:
```bash
./start_dojo.sh
```

After a few minutes (20min<), this should start the pwncollege instance.
In the admin settings you can add the reallm-dojo.
For more info, please follow the [Dojo](https://github.com/pwncollege/dojo) instructions.

