__version__ = "0.0.0"

import os
# stop LiteLLM from querying at all to the remote server
# https://github.com/BerriAI/litellm/blob/4d29c1fb6941e49191280c4fd63961dec1a1e7c5/litellm/__init__.py#L286C20-L286C48
os.environ["LITELLM_LOCAL_MODEL_COST_MAP"] = "True"

# TODO: ADD REAL KEYS HERE
OAI_API_KEY = ""
os.environ["OPENAI_API_KEY"] = OAI_API_KEY
os.environ["ANTHROPIC_API_KEY"] = ""
os.environ["GEMINI_API_KEY"] = ""

from .logging import create_plugin
