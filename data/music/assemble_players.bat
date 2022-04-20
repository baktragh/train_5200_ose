@ECHO OFF
REM Assemble all RMT players
..\..\tools\XASM -d GAME=3 -l rmtplayr3.lst -o rmtplayr3.obx rmtplayr.asm
..\..\tools\XASM -d GAME=2 -l rmtplayr2.lst -o rmtplayr2.obx rmtplayr.asm
..\..\tools\XASM -d GAME=1 -l rmtplayr1.lst -o rmtplayr1.obx rmtplayr.asm
PAUSE