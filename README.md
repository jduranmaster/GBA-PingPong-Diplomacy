# GBA-PingPong-Diplomacy
A repository to storage the source code of the homebrew game "Ping-Pong Diplomacy" programmed for the Game Boy Advance.
The Pong Game Engine has been developed in C programming language and DEVKITADV develpment kit.

Ping-Pong Diplomacy for GBA.

Author: Ryoga a.k.a. JDURANMASTER

First step, on GBA programming. This module is a sample tetris-clone.

- Testing GBA mode 4, mode 3, double buffering, keyboard controlling.
- No sound implemented yet

Change the following compilation script if you need to do it in order to compile the source code in your local system.


path=D:\devkitadv\bin

gcc -c -O3 -mthumb -mthumb-interwork pingpong.c
gcc -mthumb -mthumb-interwork -o pingpong.elf pingpong.o

objcopy -O binary pingpong.elf PingPongDiplomacy.gba

del *.elf
del *.o

pause
