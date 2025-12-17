# REaLLM Study Plugin
This is the plugin for running the REaLLM Study.
Under the hood, it uses DAILA for AI interaction and uses LibBS for data collection.

## Installation
``` 
pip install -e . && reallm_study --install
``` 

## Usage
The OpenAI API key is hardcoded in the plugin, so you don't need to provide it.
To use the plugin:
1. Start IDA Pro and go to any function
2. Right-click and go to the `REaLLM` submenu
3. Click `REaLLM->Other->Start REaLLM study...`

Then use the AI features and IDA as normally. When you are done, simply close IDA.

## Data Collection
The following things are collected as the user interacts with the plugin:
- any data change (renaming, retyping, commenting, ...)
- any AI interaction (query, response, ...)
- any click (screen, address, ...)

The data is stored in `~/reallm_data/<binary_name>/`. All actions are recorded and stored
in a log file named `actions.log`.

Any time a user interacts with the LLM, a copy of the decompilation at that time is copied and stored in
the folder.
As an example:
```
Querying (summarize, gpt-4o, few-shot) for <Function: int main(args=3); @0x71d vars=3 len=0xb8> on 5e37221271b354415d45e2ec6fc44f61.c
```

This stored the decompilation in `5e37221271b354415d45e2ec6fc44f61.c` at the time of the query.

You can find examples in the [example_data](./example_data) folder.