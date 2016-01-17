path=D:\devkitadv\bin

gcc -c -O3 -mthumb -mthumb-interwork pingpong.c
gcc -mthumb -mthumb-interwork -o pingpong.elf pingpong.o

objcopy -O binary pingpong.elf PingPongDiplomacy.gba

del *.elf
del *.o

pause
