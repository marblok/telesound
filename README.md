# TeleSound
TeleSound is an application designed for observing the properties of signals transmitted through a communication channel. Additionally, it serves as a tool for demonstrating the characteristics of various modulators. This application provides a platform for exploring signal processing, modulation techniques, and their impact on the transmitted signals.

## Features
- Signal Analysis: TeleSound allows users to observe the properties of signals. Users can visualize the signals in time and frequency domains.
- Transmission Channel Modeling: The application provides a simulation of different communication channels, allowing users to understand how signals behave in real-world scenarios.
- Modulation Techniques: TeleSound supports various digital modulation techniques, such as ASK, PSK and QAM. Users can observe and compare the effects of different modulation schemes on the transmitted signals.
- User-Friendly Interface: TeleSound features an intuitive and user-friendly interface, making it easy for both students and researchers to navigate and observe different aspects of signal processing.

See the [wiki](https://git.pg.edu.pl/dspe/telesound/-/wikis/home) for more information.

## DLLs required for TeleSound to work:
- libgcc_s_seh-1.dll
- libstdc++-6.dll
- libwinpthread-1.dll

- wxbase32u_gcc1220_x64.dll
- wxms32u_gl_gcc1220_x64.dll
- wxms32u_core_gcc1220_x64.dll

Make sure these files are in PATH, or place them in the same folder as *TeleSound.exe*

Also, ensure that the coefficients for the filters are present in the config folder. Modulators may not function correctly without these coefficients.


## Known issues:
- The program may crash when audio devices become unavailable during runtime.
