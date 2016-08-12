path=D:\devkitadv\bin



gcc -c -O3 -mthumb -mthumb-interwork pingpong.c


gcc -mthumb -mthumb-interwork -o pingpong.elf pingpong.o



objcopy -O binary pingpong.elf PingPongDiplomacy-GBA-Edition.gba


gbafix PingPongDiplomacy-GBA-Edition.gba -p -tPingPongDiplomacyGBA -c7777 -m86 -r1


del *.elf

del *.o

pause
