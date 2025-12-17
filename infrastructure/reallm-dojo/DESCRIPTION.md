## Welcome

Welcome to the REaLLM study. In this study you will be asked to complete three reversing challenges, one of which is an 
warm-up challenge. All challenges should be completed in the provided VNC with _only_ static analysis in IDA Pro. 
We have provided an IDA Pro instance with our LLM plugin installed. 

## Instructions
**To begin, please watch the following video**:

[https://youtu.be/OqhD11l7CUE](https://youtu.be/OqhD11l7CUE).

It should take around 5 minutes.
You can also reference the instructions below.

During this study, you should only use the following tools in the VNC to complete the challenge:
- IDA Pro
- Text editor (Sublime)
- Web browser (Firefox)

You should **not** execute the program or use any dynamic analysis tools.
When you complete a challenge, you will save a small writeup in the provided text editor.

### IDA Pro Use
As soon as you click on IDA Pro, the REaLLM plugin will be running, starting the study.
When you use IDA Pro please follow the following guidelines:
- Do not use the Python interface to emulate code execution.
- Do not use any dynamic analysis tools.
- Do not use multiple windows at the same time. Please keep your focus on only one window.
- Click often/whenever you are focusing on something (like a variable in the decompilation).

When you are done with a challenge, you right-click on a function and click `REaLLM->Study->End REaLLM study...`.

### LLM Use
In this study, you will only get to use LLMs for one of the two challenges, which we have predetermined for your
session. 
To interact with an LLM, you can right-click on a function and click `REaLLM->...` and select the desired action.
If no LLM options are available, it means this challenge has the LLM disabled.
On your first selection of a prompt, you wil be asked to select a model and prompting style. 
If you don't know what to choose, you can select the default options.

Of all the LLM prompts, two are of note:
1. `summarize library man pages`: requires you to right-click on a function call and select this option.
2. `free prompt...`: is an llm chat window with no context. Use this option as a last resort. 

### Technical Difficulties

In the event of technical difficulties, you can stop the study and contact Adam Doupé and Zion Basque at `doupe@asu.edu` 
and `zbasque@asu.edu` respectively. In most cases, a single exception is _ok_, however, if you see an error involving
the LLM API key or rate limits, please contact us immediately. We will triage the problem and fix it as soon as possible. 

## Post-study Survey
When you have completed the challenges, please fill out the following survey to be eligible for the 
$50 Amazon gift card: 

[https://forms.gle/AvwZSBCYy7ctUakA6](https://forms.gle/AvwZSBCYy7ctUakA6).

Please use the same email address you used to sign up for the study.