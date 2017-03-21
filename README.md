Cheemu is a WIP Chip8 emulator( interpreter ) that I wanted to work on to learn about emulation.

I have not tested all ROMs, but there are definitely some that do not work.

Build:
1. Install dependencies, premake4 is required, and then it should tell you about
   any other dependencies needed when you try to build it.
2. premake4 gmake && make
3. cd Cheemu
4. premake4 gmake && make

Run:
1. ./CheemuTest to run tests
2. ./Cheemu path-to-rom to run emulator with rom.
3. Roms are located in roms folder. It's best you try Chip8 ROMs in Chip-8 Games.

Cleanup:
In both root folder and Cheemu folder:
1. premake4 clean
2. rm -rf build

Misc:
Premake4 is the build system I'm using to build the project. I'm not super familiar with how it
works, so to build some external dependencies such as SDL2 and SDL2 mixer, I use regular lua
within the premake4 script to configure, make and install them into the build folders.
I think you may need build-essential library for configure, not sure.
