# Welcome to the TeleSound Wiki!

This wiki serves as a central hub for all documentation and tips related to the TeleSound app.

## About TeleSound
**TeleSound** is an application that models phenomena occurring in telecommunications systems, using telegraphy and telephony as examples. The software allows for the adjustment of parameters in the transmission channel, and provides the ability to display temporal waveforms and spectrograms. It demonstrates the properties of information sources, including text and speech.


### Libraries in use:

|Name| Version | Link | 
|----------|----------|---|
|DSPElib|  0.20.036  | [git.pg.edu.pl/dspe/DSPElib/](https://git.pg.edu.pl/dspe/DSPElib/)|
| wxWidgets| 3.2.2 | [wxwidgets.org](https://www.wxwidgets.org)|




## How to build on MS Windows

1. Prepare the environment -  for reference, see [VSCode DSPElib readme](https://git.pg.edu.pl/dspe/DSPElib/-/wikis/VSCode-DSPElib-readme) .
2. Clone the repository.
3. Download wxWidgets [header files](https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.2.1/wxWidgets-3.2.2.1-headers.7z) and [DLLs](https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.2.1/wxMSW-3.2.2_gcc1220_x64_ReleaseDLL.7z) for MSYS2 MinGW-w64 12.2.
4. Extract the downloaded archives. Rename "lib" folder to "lib_dev". Copy the "include" and "lib_dev" folders to "wxWidgets-3.2.2".
5. Open the project in VSCode:
    - in the top bar, select _Terminal > Run Build Task..._
6. ✅ Done!
