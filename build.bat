@ECHO OFF
REM Train unified engine for Atari 5200 build 
REM Open source edition (OSE)
REM -----------------------------------
SET CC65_HOME=C:\UTILS\A8\CC65
SET PATH=%PATH%;C:\UTILS\A8\CC65\BIN
REM -----------------------------------

ECHO Bulding Train trilogy
ECHO CC65 HOME DIR: %CC65_HOME%

ECHO Toolkit versions:
cc65 -V
ld65 -V

REM Process text
ECHO [1/5] Processing texts...
java -jar tools\CTexter.jar data\train3_text.txt build\train3_text.h
IF %ERRORLEVEL% NEQ 0 GOTO :XIT
java -jar tools\CTexter.jar data\train2_text.txt build\train2_text.h
IF %ERRORLEVEL% NEQ 0 GOTO :XIT
java -jar tools\CTexter.jar data\train1_text.txt build\train1_text.h
IF %ERRORLEVEL% NEQ 0 GOTO :XIT


REM Compile and assemble main program
ECHO [2/5] Compiling and assembling main program...
cc65 -t atari5200 -O --all-cdecl --static-locals -DGAME_TRAIN3 -o build\train3.s train.c 
IF %ERRORLEVEL% NEQ 0 GOTO :XIT
cc65 -t atari5200 -O --all-cdecl --static-locals -DGAME_TRAIN2 -o build\train2.s train.c 
IF %ERRORLEVEL% NEQ 0 GOTO :XIT
cc65 -t atari5200 -O --all-cdecl --static-locals -DGAME_TRAIN1 -o build\train1.s train.c 
IF %ERRORLEVEL% NEQ 0 GOTO :XIT

ca65 -t atari5200 -o build\train3.o build\train3.s
IF %ERRORLEVEL% NEQ 0 GOTO :XIT
ca65 -t atari5200 -o build\train2.o build\train2.s
IF %ERRORLEVEL% NEQ 0 GOTO :XIT
ca65 -t atari5200 -o build\train1.o build\train1.s
IF %ERRORLEVEL% NEQ 0 GOTO :XIT

REM Assemble native routines in assembler
ECHO [3/5] Assembling assembler routines...
ca65 -t atari5200 -D GAME_TRAIN3=1 -o build\train3_routines.o train_routines.asm
IF %ERRORLEVEL% NEQ 0 GOTO :XIT
ca65 -t atari5200 -D GAME_TRAIN2=1 -o build\train2_routines.o train_routines.asm
IF %ERRORLEVEL% NEQ 0 GOTO :XIT
ca65 -t atari5200 -D GAME_TRAIN1=1 -o build\train1_routines.o train_routines.asm
IF %ERRORLEVEL% NEQ 0 GOTO :XIT


REM Assemble game data
ECHO [4/5] Assembling game data...
ca65 -t atari5200 -D GAME_TRAIN3=1 -o build\train3_data.o train_data.asm
IF %ERRORLEVEL% NEQ 0 GOTO :XIT
ca65 -t atari5200 -D GAME_TRAIN2=1 -o build\train2_data.o train_data.asm
IF %ERRORLEVEL% NEQ 0 GOTO :XIT
ca65 -t atari5200 -D GAME_TRAIN1=1 -o build\train1_data.o train_data.asm
IF %ERRORLEVEL% NEQ 0 GOTO :XIT


REM Link cartridge images
ECHO [5/5] Linking cartridge images...
ld65 -C linker.cfg --mapfile build\map3.txt -o train3_5200.bin build\train3.o build\train3_data.o build\train3_routines.o atari5200.lib
IF %ERRORLEVEL% NEQ 0 GOTO :XIT
ld65 -C linker.cfg --mapfile build\map2.txt -o train2_5200.bin build\train2.o build\train2_data.o build\train2_routines.o atari5200.lib
IF %ERRORLEVEL% NEQ 0 GOTO :XIT
ld65 -C linker.cfg --mapfile build\map1.txt -o train1_5200.bin build\train1.o build\train1_data.o build\train1_routines.o atari5200.lib
IF %ERRORLEVEL% NEQ 0 GOTO :XIT


Echo Build OK
REM PAUSE
EXIT /B 0

:XIT
ECHO An error occured during build
PAUSE
EXIT /B -1