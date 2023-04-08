# TeleSound
See the [wiki](https://git.pg.edu.pl/dspe/telesound/-/wikis/home) for more information.

## DLLs required for TeleSound to work:
- wxbase32u_gcc1220_x64.dll
- wxms32u_gl_gcc1220_x64.dll
- wxms32u_core_gcc1220_x64.dll
Place them in the same folder as *TeleSound.exe*

## Known issues:
- The program may crash when running without audio devices or when audio devices become unavailable during runtime.
- Switching modes (client/server) may cause problems (also in earlier versions).

## TODO:
- Verify that devices are available before attempting to use them.
- Fix the "No audio devices found" message box,
- Fix release build (#if DEBUG statements),
- Fix spacing between elements in the main window,
- Fix switching modes.