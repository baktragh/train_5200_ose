/*
  
  Train unified engine for Atari 5200
  Open source edition (OSE)
  
  Copyright (c) 2020, Michael Kalouš, Petr Postava (BAHA Software).
  All rights reserved.
 
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
  3. All advertising materials mentioning features or use of this software must
     display the following acknowledgement: This product includes software
     developed by the BAHA Software
  4. Neither the name of the BAHA Software nor the names of its contributors may
     be used to endorse or promote products derived from this software without
     specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY BAHA Software ''AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
  IN NO EVENT SHALL BAHA Software BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

/* Main program */
#include <stdio.h>
#include <peekpoke.h>
#include <stdlib.h>
#include <string.h>
#include <atari5200.h>

/*Cheat defines*/
//#define CFG_CHEAT_UNLOCK
//#define CFG_CHEAT_SINGLE_TREASURE

/*Texts*/
#ifdef GAME_TRAIN1
#include "build/train1_text.h"
#define CFG_MAX_LEVEL_INDEX (49)
#define CFG_STATUSBAR_BG (0xB2)
#endif

#ifdef GAME_TRAIN2
#include "build/train2_text.h"
#define CFG_MAX_LEVEL_INDEX (49)
#define CFG_STATUSBAR_BG (0x30)
#endif

#ifdef GAME_TRAIN3
#include "build/train3_text.h"
#define CFG_MAX_LEVEL_INDEX (24)
#define CFG_STATUSBAR_BG (0x62)
#endif


/*External data*/
extern unsigned char TRAIN_DATA_CHARSET1;
extern unsigned char TRAIN_DATA_CHARSET1_PAGE;
extern unsigned char TRAIN_DATA_CHARSET2;
extern unsigned char TRAIN_DATA_CHARSET2_PAGE;
extern unsigned char TRAIN_DATA_DL_MENU;
extern unsigned char TRAIN_DATA_DL_GAME;
extern unsigned char TRAIN_DATA_DL_INTERMISSION;
extern unsigned char TRAIN_DATA_DL_SCENESELECTION;
extern unsigned char TRAIN_DATA_PMGAREA;
extern unsigned char TRAIN_DATA_GAMESCREEN;
extern unsigned char TRAIN_DATA_STATUSBARSCREEN;
extern unsigned char TRAIN_DATA_RMTPLAYER;
extern unsigned char TRAIN_DATA_RMTPLAYER_END;
extern unsigned char TRAIN_DATA_RMTSONG;
extern unsigned char TRAIN_DATA_RMTSONG_END; 

extern unsigned char TRAIN_FIGURE_RIGHT;
extern unsigned char TRAIN_FIGURE_LEFT;
extern unsigned char TRAIN_FIGURE_UP;
extern unsigned char TRAIN_FIGURE_DOWN;

extern unsigned char MENU_TRAIN_TITLE;
extern unsigned char TRAIN_GAME_LEVELS;
extern unsigned char TRAIN_GAME_ELEMENTS;

extern unsigned char menuScrollTextFlag;

extern unsigned int _TRAIN_SEG_CHARSET1_LOAD__;
extern unsigned int _TRAIN_SEG_CHARSET1_RUN__;


/*Screen related*/
#define GAMESCREEN_SIZE (960)
#define PLAYFIELD_OFFSET (82)
#define STATUSBARSCREEN_SIZE (40)

/*Joystick*/
#define JS_LEFT (11)
#define JS_RIGHT (7)
#define JS_UP (14)
#define JS_DOWN (13)
#define JS_C (15)
#define JS_RESET (128)
#define JS_START (64)
#define JS_FLAG_START (1)
#define JS_FLAG_RESET (2)


/*Game speed*/
#define GAME_SPEED_NORMAL (0)
#define GAME_SPEED_SLOW (1)


/*PMG*/
#define PMG_P0_AREA (1024)
#define PMG_P1_AREA (1280)
#define PMG_P2_AREA (1536)
#define PMG_P3_AREA (1792)

/*Level and level elements*/
#define LEVEL_DATA_SIZE (144)
#define ELEMENT_DATA_SIZE (33)

#define EL_BLANK (0)
#define EL_TR1 (4)
#define EL_TR2 (8)
#define EL_TR3 (12)
#define EL_TR4 (16)
#define EL_WALL (20)
#define EL_GATE_CLOSED (24)
#define EL_GATE_OPEN (28)
#define EL_POISONED_WALL2 (32)

/*Direction element masks*/
#define EL_DIR_LEFT (0)
#define EL_DIR_RIGHT (1)
#define EL_DIR_UP (2)
#define EL_DIR_DOWN (3)

/*Characters for separator and track*/
#define CHAR_SEPARATOR (89)
#define CHAR_TRACK (94)

/*Index of the first car character*/
#define EL_CARS_FIRST (64)

/*Quick Vertical SYNC*/
#define VERTICAL_SYNC(ID) __asm__ (" lda $02"); \
                      __asm__ ("@vsync_%s: cmp $02",ID); \
                      __asm__ (" beq @vsync_%s",ID);

/*Game status*/
#define ADVANCE_NEXT_CONTINUE (0)
#define ADVANCE_NEXT_CONGRATS (1)
#define LOSE_LIFE_CONTINUE (0)
#define LOSE_LIFE_GAMEOVER (1)
#define NEW_HISCORE_YES (1)
#define NEW_HISCORE_NO (0)
#define LEVEL_NONE (254)
#define DEATH_TYPE_CRASH (0)
#define DEATH_TYPE_POISON (1)

#define LEVEL_FLAG_POISONED_TREASURE (0x80)

/*Quickly erase element at loco position*/
#define ERASE_AT_LOCO(PTR_ID,PROBE_PTR_ID) \
        PTR_ID = PROBE_PTR_ID; \
        *(PTR_ID) = 0; \
        ++PTR_ID; \
        *(PTR_ID) = 0; \
        PTR_ID += 39;  \
        *(PTR_ID) = 0; \
        ++PTR_ID;      \
        *(PTR_ID) = 0; \


/*Display and handle main game menu*/
void handleMenu();
void clearScreen();
void clearStatusBar();
void paintTrainTitle();
void paintMenuItems();
unsigned char screenSceneSelection();
void clearPlayField();
void hideLoco();

/*Game toggles*/
void setGameSpeed(unsigned char speed);
void setJoystickZones(unsigned char zoneSize);
void toggleGameInit(unsigned char gameInitType);


/*Level and element expansion*/
void expandAndPaintLevel(unsigned char levelIndex);
void transposeElement(unsigned char* ptr);
void eraseElement(unsigned char* ptr);
void expandMask(unsigned char* elementDataPtr, unsigned char elementCharIndex);
void paintGameStatusBar();

/*Decimal displays*/
void decimalDisplaysInit();
void resetZonedScore();
extern void incrementScore();
extern void decrementScore();
unsigned char updateMenuScores();
void incrementNextLifeScore();

/*PMG*/
void pmgInit();
void setLocoPos(unsigned char x, unsigned char y, unsigned char xoffset, unsigned char yoffset);
void locoMove(unsigned char direction);
void repaintLoco();
void pmgSetForLoco();
void pmgSetForSceneSelection();
void pmgSetDefaultLocoColors();

/*Time and timing*/
void delay(unsigned char w);

/*Graphics mode displays*/
void setScreen(void* dlPtr,
        unsigned char statusBarFg,
        unsigned char statusBarBg,
        void* dliHndl,
        unsigned char chsetPage,
        unsigned char c0,
        unsigned char c1,
        unsigned char c2,
        unsigned char c3,
        unsigned char c4
        );

/*Display settings and visual effects*/
void enableDisplay();
void disableDisplay();
void fadeInAndOut();
void levelFadeOut();
void levelFadeIn();
void verticalSync(unsigned char count);


/*Music numbers*/
#ifdef GAME_TRAIN1
#define MUSIC_INGAME (0x12)
#define MUSIC_MENU (0x00)
#define MUSIC_NEW_HISCORE (0x08)
#define MUSIC_GAME_OVER (0x04)
#define MUSIC_SCENE_SELECTION (0x12)
#define MUSIC_CONGRATULATIONS (0x0B)
#define MUSIC_EMPTY (0x12)
#endif

#ifdef GAME_TRAIN2
#define MUSIC_INGAME (0x12)
#define MUSIC_MENU (0x00)
#define MUSIC_NEW_HISCORE (0x02)
#define MUSIC_GAME_OVER (0x05)
#define MUSIC_SCENE_SELECTION (0x08)
#define MUSIC_CONGRATULATIONS (0x0B)
#define MUSIC_EMPTY (0x0F)
#endif

#ifdef GAME_TRAIN3
#define MUSIC_INGAME (0x00)
#define MUSIC_MENU (0x02)
#define MUSIC_NEW_HISCORE (0x06)
#define MUSIC_GAME_OVER (0x09)
#define MUSIC_SCENE_SELECTION (0x0D)
#define MUSIC_CONGRATULATIONS (0x10)
#define MUSIC_EMPTY (0x00)
#endif

/*SFX Numbers*/
#define SFX_TRAIN (1)
#define SFX_TREASURE (2)
#define SFX_CRASH (4)
#define SFX_SCENE_COMPLETE (8)
#define SFX_POISONED_CARGO (16)


/* Audio-related imports*/
extern unsigned char _sfxRequested;
extern unsigned char _songLineRequested;
extern unsigned char _inGameAudioFlag;

/*Scrolling text imports*/
extern unsigned char hscroll;
extern unsigned char scrollCount;

/* Music routines - low level*/
extern void rmtSetUniversalVBI();

/* Music routines - high level*/
void audioRequestSongLine(unsigned char songLine);
void audioRequestSFX(unsigned char sfxNumber);
void audioSetInGameFlag(unsigned char inGameFlag);
void audioSetForSilence();
void audioStopInGameAudio();

/*Train movement*/
void updateTrainArray(unsigned char dirCode);
void drawTrainCars(unsigned char doErase);

/*Level outcomes and continuations*/
unsigned char gameToNextLevel();
unsigned char gameLoseLife(unsigned char deathElement);
void screenGameOver();
void screenGratulation();
void screenNewHighScore();

/*DLI handlers*/
extern unsigned char dliHandler;
extern unsigned char dliMenuHandler;
extern unsigned char dliInterMission;
/*DLI colors*/
extern unsigned char statusBarForeground;
extern unsigned char statusBarBackground;

/*IRQ handlers*/
extern unsigned char brkIRQHandler;
extern unsigned char kpdHandler;

/*Last key pressed on keypad*/
extern unsigned char kpdLastKey;

/*Controls*/
void ctrlSwapAll();
unsigned char jsGet(unsigned char flags);
void jsSwap();
void jsSwapFire();
unsigned char jsGetFire();
void jsInit();
unsigned char kpdGet();
void kpdSwap();

/*System routines*/
void binaryLoad(unsigned char* beginPtr,unsigned char* pastPtr);


/* Train */
/* Position of the train and eraser in the screen memory*/
unsigned char locoX, locoY;
unsigned char* locoPtr;
/* PMG coordinates*/
unsigned char p0x, p0y;
/* PMG Player areas*/
unsigned char* p0Area, *p1Area, *p2Area, *p3Area;
/* Direction */
unsigned char locoJoystickDirection;
unsigned char locoOldJoystickDirection;
unsigned char locoDirectionCode;
/* Element in the train path*/
unsigned char* locoProbePtr;
unsigned char probedElement;

/*Array of train cars and their directions*/
unsigned char trainCarArray[180];
unsigned char trainDirectionArray[180];

/*Zero page variables for repainting of the train*/
extern unsigned char *zp_ptr1, *zp_ptr2, *zp_ptr3;
extern unsigned char zp_z1, zp_x1;

#pragma zpsym ("zp_ptr1")
#pragma zpsym ("zp_ptr2")
#pragma zpsym ("zp_ptr3")
#pragma zpsym ("zp_z1")
#pragma zpsym ("zp_x1")


/* Door pointer*/
unsigned char* doorPtr;

/*Element masks*/
unsigned char inverseMasks[128];

/*Direction updates*/
unsigned int directionReverseUpdate[4] = {2, -2, 80, -80};

/*Zoned decimals*/
#define ZN_0 16
#define ZN_2 18
#define ZN_3 19
#define ZN_5 21
#define ZN_9 25
#define ZN_6 22
#define ZN_7 23
#define ZN_8 24

#define ZN_SCORE_LENGTH (5)

unsigned char dScore[5];
unsigned char dLastScore[5];
unsigned char dHighScore[5];
unsigned char dNextLifeScore[5];
unsigned char dLevelInitialScore[5];
unsigned char d300[5] = {ZN_0, ZN_0, ZN_0, ZN_3, ZN_0};
unsigned char d5000[5] = {ZN_0, ZN_0, ZN_5, ZN_0, ZN_0};



/*Movement control*/
unsigned char normalMoveDelay;
unsigned char fastMoveDelay;
unsigned char realMoveDelay;
unsigned char lastKey;
unsigned char lastJS;

unsigned char jsZoneLeft;
unsigned char jsZoneRight;
unsigned char jsZoneUp;
unsigned char jsZoneDown;
unsigned char jsZoneSizes[3] = {25,45,65};

unsigned char NTSCSpeeds[2] = {20, 24};

/*Colors changed in DLI*/
extern unsigned char colorStore1;
extern unsigned char colorStore2;

/*NTSC color correction*/
unsigned char PAL2NTSC[16] = {0, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 240, 224, 16, 32};
/*Pointer to current train figure*/
unsigned char* trainData = &TRAIN_FIGURE_RIGHT;

/*Game initial parameters*/
#define GAME_INIT_NORMAL (0)
unsigned char gameInitLives;
unsigned char dGameInitScore[5];
unsigned char dGameInitNextLifeScore[5];
unsigned char gameInitType;

/*Game status*/
unsigned char gameLevelIndex;
unsigned char gameMaxLevelIndex;
unsigned char gameLives;
unsigned char lastLevelExpanded;

/*Level status*/
unsigned char levelTreasure;
unsigned char levelTrainLength;
unsigned char* levelInitialLocoFigure;
unsigned char levelFlags;


/*Score, scene, and lives display*/
unsigned char* gameScorePtr;
unsigned char* gameLevelPtr;
unsigned char* gameLivesPtr;

unsigned char* lastScorePtr;
unsigned char* highScorePtr;

/*Menu*/
#define MENU_ITEM_START_GAME (0)
#define MENU_ITEM_SPEED (1)
#define MENU_ITEM_DEAD_ZONE (2)
#define MENU_ITEM_MAX (2)

#define GAME_AUDIO_SFX (0)

#define MENU_DZ_SMALL (0)
#define MENU_DZ_MEDIUM (1)
#define MENU_DZ_LARGE (2)

unsigned char menuCurrentItem;
unsigned char menuGameSpeed;
unsigned char menuDeadZone;
unsigned char menuCycleTrainFlag;
unsigned char menuScrollTextFlag;

/*Sounds and music*/
extern unsigned char songLineRequested;
extern unsigned char sfxRequested;
extern unsigned char inGameAudioFlag;

int main() {

    unsigned char* ptr1;
    unsigned char deathType;

    /*Initialization for a diagnostic cartridge*/
    memset(0x0000, 0, 0x1D); //Clear zero PAGE
    memset((void*) &ANTIC, 0, sizeof (ANTIC)); //Clear ANTIC
    memset((void*) &GTIA_WRITE, 0, sizeof (GTIA_WRITE)); //Clear GTIA
    memset((void*) &POKEY_WRITE, 0, sizeof (POKEY_WRITE)); //Clear POKEY

    /*Set interrupt handler vectors - fixed locations in OS ROM. These
      values work with all three OS ROMs that concern us:
      - Original BIOS for 4-port model
      - New BIOS for 2-port model
      - Altirra's BIOS for 5200 
      */
    OS.vinter=(void*)0xFC03;
    OS.vvblki=(void*)0xFCB8;
    OS.vvblkd=(void*)0xFCB2;
    OS.vkeybd=(void*)0xFD02;

    /*Set custom interrupt handlers*/
    OS.vbrkky=(void*)&brkIRQHandler;
    OS.vbreak=(void*)&brkIRQHandler;
    OS.vkeypd=(void*)&kpdHandler;
    

    /*Set DMA and interrupts*/
    OS.sdmctl = 0x3E; //Normal screen + 1-line PMG
    ANTIC.nmien = 0x40; //Just VBI for now
    ANTIC.chactl = 0x02; //Standard character display
    
    

    POKEY_WRITE.skctl = 0x02; //Standard serial I/O
    OS.pokmsk = 0xC0; //Standard mask
    POKEY_WRITE.irqen = 0xC0; //Standard interrupts
    
    
    /*Initialize joystick*/
    jsInit();

    disableDisplay();
    verticalSync(2);

    /*Copy data from ROM to RAM*/
    memcpy((unsigned char*) &_TRAIN_SEG_CHARSET1_RUN__, (unsigned char*) &_TRAIN_SEG_CHARSET1_LOAD__, 1024);
    binaryLoad((unsigned char*)&TRAIN_DATA_RMTPLAYER,(unsigned char*)&TRAIN_DATA_RMTPLAYER_END);
    binaryLoad((unsigned char*)&TRAIN_DATA_RMTSONG,(unsigned char*)&TRAIN_DATA_RMTSONG_END);

    /*Housekeeping*/
    gameInitType = GAME_INIT_NORMAL;
    setGameSpeed(GAME_SPEED_NORMAL);
    menuGameSpeed = GAME_SPEED_NORMAL;
    menuDeadZone = MENU_DZ_MEDIUM;

    toggleGameInit(gameInitType);

    /*Initialize the PMG*/
    pmgInit();
    pmgSetForLoco();
    decimalDisplaysInit();

    /*Game-relate variables independent on a single game*/
    #ifdef CFG_CHEAT_UNLOCK
    gameMaxLevelIndex = CFG_MAX_LEVEL_INDEX;
    #else
    gameMaxLevelIndex = 0;
    #endif
    

    memset(dHighScore, ZN_0, sizeof (dHighScore));
    memset(dLastScore, ZN_0, sizeof (dLastScore));

    /*Set audio for silence*/
    audioSetInGameFlag(0);
    audioSetForSilence();
    rmtSetUniversalVBI();

    /*Main menu ==================================================================*/
menu:
    /*Handle the main menu*/
    handleMenu();

    disableDisplay();
    verticalSync(1);

    /*Reset single-game variables*/
    gameLevelIndex = 0;
    lastLevelExpanded = LEVEL_NONE;

    /*Prepare normal or head start - score, lives, next life score*/
    gameLives = gameInitLives;
    memcpy(dNextLifeScore, dGameInitNextLifeScore, sizeof (dNextLifeScore));
    memcpy(dScore, dGameInitScore, sizeof (dScore));

    /*Set game speed to the speed selected in menu*/
    setGameSpeed(menuGameSpeed);
    setJoystickZones(menuDeadZone);

    /*Select scene*/
    if (gameMaxLevelIndex > 0) {
        disableDisplay();
        fadeInAndOut();
        gameLevelIndex = screenSceneSelection();
    }

    /*Play either empty music or in-game music*/
    audioRequestSongLine(MUSIC_EMPTY);


    /*Level loop =================================================================*/
level:

    /*Clear everything*/
    setScreen(&TRAIN_DATA_DL_GAME,
            12, CFG_STATUSBAR_BG,
            &dliHandler,
            TRAIN_DATA_CHARSET1_PAGE,
            0x24, 12, 0x32, 14, 0);

    pmgSetForLoco();
    hideLoco();
    clearStatusBar();
    clearPlayField();

    /*Keep initial level score*/
    memcpy(dLevelInitialScore, dScore, sizeof (dLevelInitialScore));

    /*Expand and paint level and update status bar*/
    expandAndPaintLevel(gameLevelIndex);
    paintGameStatusBar();

    /*Prepare loco for the level*/
    setLocoPos(locoX, locoY, 0, 0);
    trainData = levelInitialLocoFigure;
    repaintLoco();

    /*Re-enable display with fading in*/
    levelFadeIn();
    ctrlSwapAll();

    /*Reset train status*/
    levelTrainLength = 0;
    memset(trainCarArray, 0, sizeof (trainCarArray));
    memset(trainCarArray, 0, sizeof (trainDirectionArray));

    /*Wait for joystick movement*/
    locoOldJoystickDirection = JS_C;
    locoJoystickDirection=JS_C;
    lastKey = 0xFF;
    
    /*Now wait until 
      - A joystick is moved to a direction or a direction is 
        selected on the keypad
      - RESET is pressed on keypad
    */
      
    while(1) {
        locoJoystickDirection=jsGet(JS_FLAG_RESET);
        if (locoJoystickDirection!=JS_C) break;
    }
    
    /*Check for premature exit*/
    if (locoJoystickDirection == JS_RESET) {
        audioStopInGameAudio();
        goto menu;
    }

    /*Clear time and keyboard*/
    OS.rtclok[1] = 0;
    OS.atract = 0;

    /*Initial movement delay*/
    realMoveDelay = normalMoveDelay;
    fastMoveDelay = normalMoveDelay >> 1;

    /*Game loop ================================================================= */
gameLoop:

    /*If fire is pressed, then move faster*/
    if (jsGetFire() == 0) realMoveDelay = fastMoveDelay;

    /*Determine what is the direction*/
    lastJS = jsGet(JS_FLAG_RESET);
    if (lastJS != JS_C) {
        locoJoystickDirection = lastJS;
        OS.atract=0;
    }

    /*Equalized movement of the loco*/
    if (OS.rtclok[1] < realMoveDelay) goto gameLoop;

    /*Reset timer and movement delay*/
    OS.rtclok[1] = 0;
    realMoveDelay = normalMoveDelay;

    /*Point to the element in train's path*/
    if (locoJoystickDirection == JS_LEFT) {
        locoProbePtr = (locoPtr - 2);
        locoDirectionCode = EL_DIR_LEFT;
        trainData = &TRAIN_FIGURE_LEFT;
    } else if (locoJoystickDirection == JS_RIGHT) {
        locoProbePtr = (locoPtr + 2);
        locoDirectionCode = EL_DIR_RIGHT;
        trainData = &TRAIN_FIGURE_RIGHT;
    } else if (locoJoystickDirection == JS_UP) {
        locoProbePtr = locoPtr - 80;
        locoDirectionCode = EL_DIR_UP;
        trainData = &TRAIN_FIGURE_UP;
    } else if (locoJoystickDirection == JS_DOWN) {
        locoProbePtr = locoPtr + 80;
        locoDirectionCode = EL_DIR_DOWN;
        trainData = &TRAIN_FIGURE_DOWN;
    }
    /*Reset, exit game*/
    else if (locoJoystickDirection == JS_RESET) {
        audioStopInGameAudio();
        goto menu;
    }
    /*No known direction. White screen of death*/
    else {
        OS.color4=0x0F;
        while(1);
    }


    /*Repaint train if direction has changed*/
    if (locoOldJoystickDirection != locoJoystickDirection) {
        repaintLoco();
        locoOldJoystickDirection = locoJoystickDirection;
    }

    /*Check what the element is*/
    probedElement = ((*locoProbePtr) & 0x7C);

    /*Open door - move train, paint train and terminate level*/
    if (probedElement == EL_GATE_OPEN) {
        ERASE_AT_LOCO(ptr1, locoProbePtr)
        locoMove(locoJoystickDirection);
        if (levelTrainLength != 0) {
            updateTrainArray(locoDirectionCode);
            drawTrainCars(1);
        }
        if (gameToNextLevel() == ADVANCE_NEXT_CONTINUE) {
            levelFadeOut();
            goto level;
        } else {
            audioStopInGameAudio();
            levelFadeOut();
            screenGratulation();
            if (updateMenuScores() == NEW_HISCORE_YES) {
                screenNewHighScore();
            }
            goto menu;
        }
    }

    /*Something solid - death*/
    if (probedElement > EL_TR4) {

        /*If poisoned treasure, move the train before death*/
        if (probedElement == EL_POISONED_WALL2 && (levelFlags & LEVEL_FLAG_POISONED_TREASURE)) {
            ERASE_AT_LOCO(ptr1, locoProbePtr)
            locoMove(locoJoystickDirection);
            if (levelTrainLength != 0) {
                updateTrainArray(locoDirectionCode);
                drawTrainCars(1);
            }
            deathType = DEATH_TYPE_POISON;
        } else {
            deathType = DEATH_TYPE_CRASH;
        }


        if (gameLoseLife(deathType) == LOSE_LIFE_CONTINUE) {
            levelFadeOut();
            goto level;
        } else {
            levelFadeOut();
            audioStopInGameAudio();
            screenGameOver();
            if (updateMenuScores() == NEW_HISCORE_YES) {
                screenNewHighScore();
            }
            goto menu;
        }

    }

    /*Some treasure - pick it and grow the train*/
    if (probedElement >= EL_TR1 && probedElement <= EL_TR4) {

        /*Sound*/
        audioRequestSFX(SFX_TREASURE);

        /*Erase treasure*/
        ERASE_AT_LOCO(ptr1, locoProbePtr)

        /*Move the loco into the treasure*/
        locoMove(locoJoystickDirection);

        /*Prepare for new train*/
        if (levelTrainLength == 0) {
            trainDirectionArray[0] = locoDirectionCode;
        }
        /*Update train array*/
        trainCarArray[levelTrainLength] = EL_CARS_FIRST + ((probedElement - EL_TR1) << 2);
        ++levelTrainLength;
        updateTrainArray(locoDirectionCode);
        incrementScore();

        /*Check for new life*/
        if (memcmp(gameScorePtr, dNextLifeScore, sizeof (dNextLifeScore)) > 0) {

            if (gameLives < 9) {
                ++gameLives; //Increment internal counter
                ++(*gameLivesPtr); //Increment counter on the screen  
            }
            incrementNextLifeScore(); //Next life in next 5000
        }

        /*Draw the train*/
        drawTrainCars(0);

        locoPtr = locoProbePtr;

        /*Count treasure*/
        #ifdef CFG_CHEAT_SINGLE_TREASURE
        levelTreasure=0;
        #else
        levelTreasure--;
        #endif
        if (levelTreasure == 0) {
            *(doorPtr) = EL_GATE_OPEN + inverseMasks[EL_GATE_OPEN];
            *(doorPtr + 1) = EL_GATE_OPEN + 1 + inverseMasks[EL_GATE_OPEN + 1];
            *(doorPtr + 40) = EL_GATE_OPEN + 2 + inverseMasks[EL_GATE_OPEN + 2];
            *(doorPtr + 41) = EL_GATE_OPEN + 3 + inverseMasks[EL_GATE_OPEN + 3];
        }

    }/*Otherwise just move the train*/
    else {
        /*Sound*/
        audioRequestSFX(SFX_TRAIN);

        /*Move the loco*/
        locoMove(locoJoystickDirection);

        updateTrainArray(locoDirectionCode);
        drawTrainCars(1);

        locoPtr = locoProbePtr;

    }

    goto gameLoop;

    return 0;
}

/*Move all train cars in proper direction*/
void updateTrainArray(unsigned char dirCode) {

    memmove(trainDirectionArray + 1, trainDirectionArray, levelTrainLength);
    trainDirectionArray[0] = dirCode;

}

/*Draw train cars. Making these fast by usage of the zp*/
void drawTrainCars(unsigned char doErase) {

    /*Point to the first car*/
    zp_ptr1 = locoPtr;
    /*Point to the train array and direction array*/
    zp_ptr2 = trainCarArray;
    zp_ptr3 = trainDirectionArray;


    for (zp_z1 = 0; zp_z1 < levelTrainLength; ++zp_z1) {

        /*Combine car type and direction*/
        zp_x1 = *(zp_ptr2)+((*zp_ptr3) << 2);

        /*Paint the car*/
        *zp_ptr1 = zp_x1 | inverseMasks[zp_x1];
        ++zp_x1;
        *(zp_ptr1 + 1) = zp_x1 | inverseMasks[zp_x1];
        ++zp_x1;
        *(zp_ptr1 + 40) = zp_x1 | inverseMasks[zp_x1];
        ++zp_x1;
        *(zp_ptr1 + 41) = zp_x1 | inverseMasks[zp_x1];

        zp_ptr3++;
        zp_ptr1 += directionReverseUpdate[(*zp_ptr3)];
        zp_ptr2++;
    }

    if (!doErase) return;

    zp_x1 = 0;
    /*Erase last car*/
    *zp_ptr1 = zp_x1;
    ++zp_ptr1;
    *zp_ptr1 = zp_x1;
    zp_ptr1 += 39;
    *zp_ptr1 = zp_x1;
    ++zp_ptr1;
    *zp_ptr1 = zp_x1;

}

/*Get level data and expand them to arrays*/
void expandAndPaintLevel(unsigned char level) {

    unsigned char *ptr1, *ptr2, *ptr3;
    unsigned char i1;
    unsigned char x1, y1, z1;
    unsigned char* elPtr;

    /*Point to the beginning of a level and reset counters*/
    ptr1 = &TRAIN_GAME_LEVELS + (level * LEVEL_DATA_SIZE);
    ptr3 = &TRAIN_GAME_ELEMENTS;

    levelTreasure = 0;

    /*Level data has the following structure  
     * 5 bytes - Color pallette 712,708,709,710,711
     * 1 byte  - Gate closed element number
     * 1 byte  - Gate open element number
     * 1 byte  - Wall element number
     * 1 byte  - Poisoned/Wall2 element number
     * 4 bytes - Numbers of treasure elements
     * 8 bytes - Numbers of car elements, two cars per element
     * 
     * 1 byte - Train Y and X coordinates
     * 1 byte - Gate Y and X coordinates 
     * 1 byte - Flags (train orientation and poisoned treasure flag)
     * 120 bytes - Element indexes follow, one element per nibble
     */

    /*Perform what is needed only once for level*/
    if (lastLevelExpanded != level) {

        /*Process the color pallette*/
        OS.color4 = *ptr1;
        ptr1++;

        memcpy((unsigned char*) &OS.color0, ptr1, 4);
        ptr1 += 4;
        
        /*Adjust the color palette for NTSC. Atari 5200 is always NTSC*/
        for (i1 = 0; i1 < 4; i1++) {
                /*Color shift*/
                z1 = *(&OS.color0+i1);
                z1 = PAL2NTSC[(z1 & 0xF0) >> 4];
                POKE(((unsigned int)&OS.color0) + i1, (PEEK(((unsigned int)&OS.color0) + i1)&0x0F) | z1);

                /*Not too dark*/
                z1 = PEEK(((unsigned int)&OS.color0) + i1);
                if ((z1 & 0x0F) == 0) z1 |= 2;
                POKE(((unsigned int)&OS.color0) + i1, z1);
            }

        /*Clear the inversion masks for everything except train cars*/
        memset(inverseMasks, 0, 36);

        /*Set characters for the gate closed element*/
        ptr2 = &TRAIN_DATA_CHARSET1 + (EL_GATE_CLOSED << 3);
        memcpy(ptr2, ptr3 + ((*ptr1) * ELEMENT_DATA_SIZE), 32);
        expandMask(ptr3 + ((*ptr1) * ELEMENT_DATA_SIZE), EL_GATE_CLOSED);
        ++ptr1;

        /*Set characters for the gate open element*/
        ptr2 = &TRAIN_DATA_CHARSET1 + (EL_GATE_OPEN << 3);
        memcpy(ptr2, ptr3 + ((*ptr1) * ELEMENT_DATA_SIZE), 32);
        expandMask(ptr3 + ((*ptr1) * ELEMENT_DATA_SIZE), EL_GATE_OPEN);
        ++ptr1;

        /*Set characters for the wall element*/
        ptr2 = &TRAIN_DATA_CHARSET1 + (EL_WALL << 3);
        memcpy(ptr2, ptr3 + ((*ptr1) * ELEMENT_DATA_SIZE), 32);
        expandMask(ptr3 + ((*ptr1) * ELEMENT_DATA_SIZE), EL_WALL);
        ++ptr1;

        /*Set characters for the poisoned treasure/wall2 element*/
        ptr2 = &TRAIN_DATA_CHARSET1 + (EL_POISONED_WALL2 << 3);
        memcpy(ptr2, ptr3 + ((*ptr1) * ELEMENT_DATA_SIZE), 32);
        expandMask(ptr3 + ((*ptr1) * ELEMENT_DATA_SIZE), EL_POISONED_WALL2);
        ++ptr1;

        /*Set characters for the treasure elements - 4 treasure elements*/
        ptr2 = &TRAIN_DATA_CHARSET1 + (EL_TR1 << 3);

        for (i1 = 0; i1 < 4; i1++) {
            memcpy(ptr2, ptr3 + ((*ptr1) * ELEMENT_DATA_SIZE), 32);
            expandMask(ptr3 + ((*ptr1) * ELEMENT_DATA_SIZE), EL_TR1 + (i1 << 2));
            ptr1++;
            ptr2 += 32;
        }

        /*Car elements. For each treasure, we have 4 cars. But only two are
         * defined in the elements (right and up), because the other figures are
         * just mirrors of the two. 
         * 
         * Each treasure requires 4 figures of car (16 characters, 128 character bytes)
         */

        /*For each treasure*/
        for (i1 = 0; i1 < 4; i1++) {

            elPtr = (ptr3 + ((*ptr1) * ELEMENT_DATA_SIZE));

            ptr2 = &TRAIN_DATA_CHARSET1 + (EL_CARS_FIRST << 3)+(i1 << 7);


            /*Expand right car - Primary Image*/
            memcpy(ptr2 + 32, elPtr, 32);
            expandMask(elPtr, EL_CARS_FIRST + 4 + (i1 << 4));

            /*Copy to the left car*/
            memcpy(ptr2, elPtr, 32);
            expandMask(elPtr, EL_CARS_FIRST + (i1 << 4));

            /*Transpose the left car*/
            transposeElement(ptr2);

            ++ptr1;
            elPtr += ELEMENT_DATA_SIZE;

            /*Expand top car*/
            memcpy(ptr2 + 64, elPtr, 32);
            expandMask(elPtr, EL_CARS_FIRST + 8 + (i1 << 4));
            /*Copy to the bottom car*/
            memcpy(ptr2 + 96, elPtr, 32);
            expandMask(elPtr, EL_CARS_FIRST + 12 + (i1 << 4));
            /*Transpose the bottom car*/
            transposeElement(ptr2 + 96);

            ++ptr1;
            elPtr += ELEMENT_DATA_SIZE;
        }


    } else {
        ptr1 += 21;
    }

    /*Now process the rest that must be refreshed always*/

    /*Set initial train position*/
    /*Train X,Y - for PMG display*/
    locoY = *ptr1 / 20;
    locoX = *ptr1 % 20;
    ++ptr1;

    /*Train pointer - screen memory (top left corner)*/
    locoPtr = &TRAIN_DATA_GAMESCREEN + (locoX << 1)+(locoY * 80);

    /*Door position and pointer*/
    y1 = *ptr1 / 20;
    x1 = *ptr1 % 20;
    ptr1++;

    /*Paint door*/
    doorPtr = &TRAIN_DATA_GAMESCREEN + (x1 << 1)+(y1 * 80);
    *(doorPtr) = EL_GATE_CLOSED + inverseMasks[EL_GATE_CLOSED];
    *(doorPtr + 1) = EL_GATE_CLOSED + 1 + inverseMasks[EL_GATE_CLOSED + 1];
    *(doorPtr + 40) = EL_GATE_CLOSED + 2 + inverseMasks[EL_GATE_CLOSED + 2];
    *(doorPtr + 41) = EL_GATE_CLOSED + 3 + inverseMasks[EL_GATE_CLOSED + 3];

    /*Process the level flags*/
    z1 = (*(ptr1)&0x0F);
    switch (z1) {
        case JS_LEFT: levelInitialLocoFigure = &TRAIN_FIGURE_LEFT;
            break;
        case JS_RIGHT: levelInitialLocoFigure = &TRAIN_FIGURE_RIGHT;
            break;
        case JS_UP: levelInitialLocoFigure = &TRAIN_FIGURE_UP;
            break;
        case JS_DOWN: levelInitialLocoFigure = &TRAIN_FIGURE_DOWN;
            break;
    }
    z1 = (*(ptr1)&0xF0);
    levelFlags = z1;
    ++ptr1;

    /*Now paint the level elements */
    ptr2 = &TRAIN_DATA_GAMESCREEN;

    /*12 Rows*/
    for (y1 = 0; y1 < 12; ++y1) {
        /*And 20 columns*/
        for (x1 = 0; x1 < 20; ++x1) {

            /*Get element*/
            if (x1 & 0x01) {
                z1 = (*ptr1)&0xF;
                ++ptr1;
            } else {
                z1 = ((*ptr1) >> 4)&0xF;
            }

            /*Multiply by 4 to get screen code*/
            z1 = z1 << 2;

            /*Count treasure*/
            if (z1 >= EL_TR1 && z1 <= EL_TR4) {
                levelTreasure++;
            }

            /*Top left*/
            *ptr2 = z1 + inverseMasks[z1];
            ++z1;
            /*Top right*/
            *(ptr2 + 1) = z1 + inverseMasks[z1];
            ++z1;
            /*Bottom left*/
            *(ptr2 + 40) = z1 + inverseMasks[z1];
            ++z1;
            /*Bottom right*/
            *(ptr2 + 41) = z1 + inverseMasks[z1];
            /*Move two columns in screen memory*/
            ptr2 += 2;

        }
        /*Move two rows in screen memory*/
        ptr2 += 40;

    }

#ifdef CFG_SINGLE_TREASURE
    levelTreasure = 1;
#endif

}

/*Transpose element. An element consists of 4 characters, pointer points
  to the first of the four. Each character is 8 bytes*/
void transposeElement(unsigned char* elFirstCharPtr) {

    unsigned char cLeft, cRight, cCur;
    unsigned char i1;

    /*Transpose pairs of characters*/
    for (i1 = 0; i1 < 8; i1++) {

        cLeft = *(elFirstCharPtr + i1);
        cRight = *(elFirstCharPtr + 8 + i1);

        cCur = (cRight & 0x03) << 6;
        cCur |= (cRight & 0x0C) << 2;
        cCur |= (cRight & 0x30) >> 2;
        cCur |= (cRight & 0xC0) >> 6;
        *(elFirstCharPtr + i1) = cCur;

        cCur = (cLeft & 0xC0) >> 6;
        cCur |= (cLeft & 0x30) >> 2;
        cCur |= (cLeft & 0x0C) << 2;
        cCur |= (cLeft & 0x03) << 6;
        *(elFirstCharPtr + 8 + i1) = cCur;

        cLeft = *(elFirstCharPtr + i1 + 16);
        cRight = *(elFirstCharPtr + 24 + i1);

        cCur = (cRight & 0x03) << 6;
        cCur |= (cRight & 0x0C) << 2;
        cCur |= (cRight & 0x30) >> 2;
        cCur |= (cRight & 0xC0) >> 6;
        *(elFirstCharPtr + i1 + 16) = cCur;

        cCur = (cLeft & 0xC0) >> 6;
        cCur |= (cLeft & 0x30) >> 2;
        cCur |= (cLeft & 0x0C) << 2;
        cCur |= (cLeft & 0x03) << 6;
        *(elFirstCharPtr + 24 + i1) = cCur;
    }

    /*Transpose also the inverse masks*/
    cLeft = (elFirstCharPtr - &TRAIN_DATA_CHARSET1) >> 3;
    cRight = cLeft + 1;
    cCur = inverseMasks[cLeft];
    inverseMasks[cLeft] = inverseMasks[cRight];
    inverseMasks[cRight] = cCur;
    cLeft += 2;
    cRight += 2;
    cCur = inverseMasks[cLeft];
    inverseMasks[cLeft] = inverseMasks[cRight];
    inverseMasks[cRight] = cCur;


}

void expandMask(unsigned char* elementDataPtr, unsigned char elementCharIndex) {

    unsigned char* maskArrayPtr;
    unsigned char bitMask = 128;
    unsigned char k;

    /*Skip over element characters*/
    elementDataPtr += 32;
    /*Point to the mask array*/
    maskArrayPtr = inverseMasks + elementCharIndex;

    /*Expand mask bits to bytes of 0 and 128*/
    for (k = 0; k < 4; k++) {
        if ((*elementDataPtr) & bitMask) {
            *maskArrayPtr = 128;
        } else {
            *maskArrayPtr = 0;
        }
        /*Advance to next mask bit and mask byte */
        ++maskArrayPtr;
        bitMask = bitMask >> 1;
    }
}

/*Erase element, ptr points to the top left char*/
void eraseElement(unsigned char* ptr) {

    *ptr = 0;
    ++ptr;
    *ptr = 0;
    ptr += 39;
    *ptr = 0;
    ++ptr;
    *ptr = 0;
}

void paintGameStatusBar() {
    memcpy(&TRAIN_DATA_STATUSBARSCREEN, T_SCORE_BAR, T_SCORE_BAR_L);
    memcpy(gameScorePtr, dScore, sizeof (dScore));
    *(gameLivesPtr) = ZN_0 + gameLives;
    *(gameLevelPtr) = ZN_0 + (gameLevelIndex + 1) / 10;
    *(gameLevelPtr + 1) = ZN_0 + (gameLevelIndex + 1) % 10;
}

void saveZonedScore() {
    memcpy(dScore, gameScorePtr, sizeof (dScore));
}

void resetZonedScore() {
    memset(dScore, ZN_0, sizeof (dScore));
}

void audioStopInGameAudio() {
    inGameAudioFlag = 0;
    sfxRequested = 0;
    songLineRequested = MUSIC_EMPTY;
    verticalSync(2);
}

void clearPlayField() {

    unsigned char* ptr1;
    unsigned char y1;

    ptr1 = &TRAIN_DATA_GAMESCREEN + PLAYFIELD_OFFSET;

    /*Clear 20 lines*/
    for (y1 = 0; y1 < 20; ++y1) {
        memset(ptr1, 0, 36);
        ptr1 += 40;
    }

}

void hideLoco() {
    GTIA_WRITE.hposp0 = 0;
    GTIA_WRITE.hposp1 = 0;
    GTIA_WRITE.hposp2 = 0;
    GTIA_WRITE.hposp3 = 0;

}

void handleMenu() {

    /*First set the screen*/
    setScreen(&TRAIN_DATA_DL_MENU,
            12, CFG_STATUSBAR_BG,
            &dliMenuHandler,
            TRAIN_DATA_CHARSET2_PAGE,
            0x24, 12, 0x32, 14, 0);

    /*Clear the screen*/
    clearScreen();
    clearStatusBar();
    pmgSetForLoco();
    pmgSetDefaultLocoColors();

    /*Set menu items*/
    menuCurrentItem = MENU_ITEM_START_GAME;


    /*Paint title*/
    paintTrainTitle();
    /*Paint menu items*/
    paintMenuItems();

    /*Cycle the train and scroll the text*/
    menuCycleTrainFlag = 1;
    menuScrollTextFlag = 2;
    audioRequestSongLine(MUSIC_MENU);
    verticalSync(2);


    /*Last and top score*/
    memcpy(&TRAIN_DATA_STATUSBARSCREEN, T_TOPSCORE_BAR, T_TOPSCORE_BAR_L);
    memcpy(lastScorePtr, dLastScore, sizeof (dLastScore));
    memcpy(highScorePtr, dHighScore, sizeof (dHighScore));

    /*Show the loco*/
    locoX = 0;
    locoY = 9;
    trainData = &TRAIN_FIGURE_RIGHT;
    setLocoPos(locoX, locoY, 4, 6);

    /*Enable display*/
    enableDisplay();

    /*Swap keyboard and joystick*/
    ctrlSwapAll();

    /*Reset clock and atract mode*/
    OS.rtclok[1] = 0;
    OS.atract=0;

    /*Almost endless loop - terminated by either RESET or game start*/

    while (1) {

        /*Fire will either start game or toggle game speed*/
        if (jsGetFire() == 0) {

            /*Reset ATRACT mode*/
            OS.atract = 0;
            
            /*Start game?*/
            if (menuCurrentItem == MENU_ITEM_START_GAME) break;
            
            /*If not, then wait until trigger released*/
            while (jsGetFire() == 0);
            if (menuCurrentItem == MENU_ITEM_SPEED) {
                menuGameSpeed = !menuGameSpeed;
            }
            else if (menuCurrentItem == MENU_ITEM_DEAD_ZONE) {
                if (menuDeadZone==MENU_DZ_LARGE) {
                    menuDeadZone=MENU_DZ_SMALL;
                    }
                else {
                    ++menuDeadZone;
                }
            }
            paintMenuItems();
            delay(10);
        }
        
        /*Allow menu item selection*/
        lastJS=jsGet(JS_FLAG_START);

        if (lastJS == JS_DOWN && menuCurrentItem < MENU_ITEM_MAX) {
            menuCurrentItem++;
            paintMenuItems();
            OS.atract = 0;
            delay(10);
            kpdSwap();
        }
        
        if (lastJS == JS_UP && menuCurrentItem > MENU_ITEM_START_GAME) {
            menuCurrentItem--;
            paintMenuItems();
            OS.atract = 0;
            delay(10);
            kpdSwap();
        }
        
        if (lastJS == JS_START) {
            break;
        }
        

    }

    /*Stop cycling the train*/
    disableDisplay();
    menuCycleTrainFlag = 0;
    menuScrollTextFlag = 0;

    /*And stop music*/
    audioRequestSongLine(MUSIC_EMPTY);

}

void clearScreen() {
    memset(&TRAIN_DATA_GAMESCREEN, 0, GAMESCREEN_SIZE);
}

void clearStatusBar() {
    memset(&TRAIN_DATA_STATUSBARSCREEN, 0, STATUSBARSCREEN_SIZE);
}

void paintTrainTitle() {

    unsigned char* ptr1, *ptr2;
    unsigned char x1;

    /*Title*/
    ptr1 = &TRAIN_DATA_GAMESCREEN + (1 * 40);
    ptr2 = &MENU_TRAIN_TITLE;

    for (x1 = 0; x1 < 9; ++x1) {
        memcpy(ptr1, ptr2, 39);
        ptr1 += 40;
        ptr2 += 39;
    }

    /*Subtitle*/
    memcpy(&TRAIN_DATA_GAMESCREEN + (10 * 40) + T_TRAIN_TITLE_C, T_TRAIN_TITLE, T_TRAIN_TITLE_L);

    /*Separators*/
    memset(&TRAIN_DATA_GAMESCREEN + (0 * 40), CHAR_SEPARATOR, 40);
    memset(&TRAIN_DATA_GAMESCREEN + (11 * 40), CHAR_SEPARATOR, 40);

    /*Track*/
    memset(&TRAIN_DATA_GAMESCREEN + (16 * 40), CHAR_TRACK, 40);

}

void paintMenuItems() {

    unsigned char i;
    unsigned char l;
    unsigned char* ptr;

    verticalSync(1);

    /*Clear everything*/
    memset(&TRAIN_DATA_GAMESCREEN + (12 * 40), 0, 120);

    /*Paint "START GAME"*/
    memcpy(&TRAIN_DATA_GAMESCREEN + (12 * 40) + T_MENU_ITEM_1_C, T_MENU_ITEM_1, T_MENU_ITEM_1_L);

    /*Paint "Game Speed" */
    if (menuGameSpeed == GAME_SPEED_NORMAL) {
        memcpy(&TRAIN_DATA_GAMESCREEN + (13 * 40) + T_MENU_ITEM_SPD_1_C, T_MENU_ITEM_SPD_1, T_MENU_ITEM_SPD_1_L);
    } else {
        memcpy(&TRAIN_DATA_GAMESCREEN + (13 * 40) + T_MENU_ITEM_SPD_2_C, T_MENU_ITEM_SPD_2, T_MENU_ITEM_SPD_2_L);
    }

    /*Paint "Dead zone"*/
    switch (menuDeadZone) {
        case MENU_DZ_SMALL: {
            memcpy(&TRAIN_DATA_GAMESCREEN + (14*40)+T_MENU_ITEM_DZ_0_C,T_MENU_ITEM_DZ_0,T_MENU_ITEM_DZ_0_L);
            break;
        }
        case MENU_DZ_MEDIUM: {
            memcpy(&TRAIN_DATA_GAMESCREEN + (14*40)+T_MENU_ITEM_DZ_1_C,T_MENU_ITEM_DZ_1,T_MENU_ITEM_DZ_1_L);
            break;
        }
        case MENU_DZ_LARGE: {
            memcpy(&TRAIN_DATA_GAMESCREEN + (14*40)+T_MENU_ITEM_DZ_2_C,T_MENU_ITEM_DZ_2,T_MENU_ITEM_DZ_2_L);
            break;
        }
    }

    /*Inverse the current menu item*/
    switch (menuCurrentItem) {
        case(MENU_ITEM_START_GAME):
        {
            ptr = &TRAIN_DATA_GAMESCREEN + (12 * 40) + T_MENU_ITEM_1_C - 1;
            l = T_MENU_ITEM_1_L + 2;
            break;
        }
        case (MENU_ITEM_SPEED):
        {
            ptr = &TRAIN_DATA_GAMESCREEN + (13 * 40) + T_MENU_ITEM_SPD_1_C - 1;
            l = T_MENU_ITEM_SPD_1_L + 2;
            break;
        }
        case (MENU_ITEM_DEAD_ZONE): {
            ptr = &TRAIN_DATA_GAMESCREEN + (14 * 40) + T_MENU_ITEM_DZ_1_C - 1;
            l = T_MENU_ITEM_DZ_1_L +2 ;
            break;
        }
    }

    for (i = 0; i < l; i++) {
        *(ptr) = (*ptr) | 128;
        ++ptr;
    }


}

/*Player missile graphics*/
void pmgInit() {

    /*Set PMG memory start*/
    ANTIC.pmbase = (((unsigned) (&TRAIN_DATA_PMGAREA)) >> 8);

    /*Clear all PMG memory*/
    memset((void*) &TRAIN_DATA_PMGAREA, 0U, 2048);

    /*Enable PMG*/
    GTIA_WRITE.gractl = 2;


    /*Convenience pointers*/
    p0Area = &TRAIN_DATA_PMGAREA + PMG_P0_AREA;
    p1Area = &TRAIN_DATA_PMGAREA + PMG_P1_AREA;
    p2Area = &TRAIN_DATA_PMGAREA + PMG_P2_AREA;
    p3Area = &TRAIN_DATA_PMGAREA + PMG_P3_AREA;
}

void pmgSetForLoco() {


    /*Clear*/
    memset((void*) &TRAIN_DATA_PMGAREA, 0U, 2048);
    p0x = 0;
    p0y = 0;

    pmgSetDefaultLocoColors();

    /*Players width - default*/
    memset(&(GTIA_WRITE.sizep0), 0, 4);

    /*Priority*/
    GTIA_WRITE.prior = 0x01;


}

void pmgSetDefaultLocoColors() {
    /*Player colors*/
    OS.pcolr0 = 0x26; //Red
    OS.pcolr1 = 0xEC; //Yellow
    OS.pcolr2 = 0x74; //Light Blue
    OS.pcolr3 = 0x72; //Dark Blue
}

void pmgSetForSceneSelection() {

    /*Clear*/
    memset((void*) &TRAIN_DATA_PMGAREA, 0U, 2048);

    /*Player P0 width*/
    GTIA_WRITE.sizep0=3;

    /*Priority*/
    GTIA_WRITE.prior=4;

    /*Colors - just player 0*/
    OS.pcolr0=0x26;

}

/*Repaint the loco*/
void repaintLoco() {

    memcpy(((unsigned char*) (p0y + p0Area)), trainData, 16);
    memcpy(((unsigned char*) (p0y + p1Area)), trainData + 16, 16);
    memcpy(((unsigned char*) (p0y + p2Area)), trainData + 32, 16);
    memcpy(((unsigned char*) (p0y + p3Area)), trainData + 48, 16);
}

/*Set position of the loco*/
void setLocoPos(unsigned char x, unsigned char y, unsigned char xoffset, unsigned char yoffset) {

    p0x = 48 + xoffset + (x << 3);
    memset(p0y + p0Area, 0, 16);
    memset(p0y + p1Area, 0, 16);
    memset(p0y + p2Area, 0, 16);
    memset(p0y + p3Area, 0, 16);
    p0y = 24 + yoffset + (y << 4);

    GTIA_WRITE.hposp0 = p0x;
    GTIA_WRITE.hposp1 = p0x;
    GTIA_WRITE.hposp2 = p0x;
    GTIA_WRITE.hposp3 = p0x;
    repaintLoco();

}

void locoMove(unsigned char direction) {

    switch (direction) {
        case JS_LEFT:
        {
            p0x -= 8;
            GTIA_WRITE.hposp0 = p0x;
            GTIA_WRITE.hposp1 = p0x;
            GTIA_WRITE.hposp2 = p0x;
            GTIA_WRITE.hposp3 = p0x;
            --locoX;
            break;
        }
        case JS_RIGHT:
        {
            p0x += 8;
            GTIA_WRITE.hposp0 = p0x;
            GTIA_WRITE.hposp1 = p0x;
            GTIA_WRITE.hposp2 = p0x;
            GTIA_WRITE.hposp3 = p0x;
            ++locoX;
            break;
        }
        case JS_UP:
        {
            memset(p0y + p0Area, 0, 16);
            memset(p0y + p1Area, 0, 16);
            memset(p0y + p2Area, 0, 16);
            memset(p0y + p3Area, 0, 16);

            p0y -= 16;
            --locoY;
            repaintLoco();
            break;
        }
        case JS_DOWN:
        {
            memset(p0y + p0Area, 0, 16);
            memset(p0y + p1Area, 0, 16);
            memset(p0y + p2Area, 0, 16);
            memset(p0y + p3Area, 0, 16);
            p0y += 16;
            ++locoY;
            repaintLoco();
            break;
        }
    }

}

/*Wait for some time*/
void delay(unsigned char w) {
    unsigned char i = 0;
    for (i = 0; i < w; i++) {
        unsigned char a = OS.rtclok[1];
        while (OS.rtclok[1] == a) {
        }
    }
}

/* Audio routines - high level*/
void audioRequestSongLine(unsigned char songLine) {
    songLineRequested = songLine;
}

void audioRequestSFX(unsigned char sfxNumber) {
    sfxRequested = sfxNumber;
}

void audioSetInGameFlag(unsigned char inGameFlag) {
    inGameAudioFlag = inGameFlag;
    verticalSync(2);
}

void audioSetForSilence() {
    songLineRequested = MUSIC_EMPTY;
    sfxRequested = 0;
    verticalSync(2);
}

void setScreen(void* dlPtr,
        unsigned char statusBarFg,
        unsigned char statusBarBg,
        void* dliHndl,
        unsigned char chsetPage,
        unsigned char c0,
        unsigned char c1,
        unsigned char c2,
        unsigned char c3,
        unsigned char c4) {

    /*Disable screen*/
    OS.sdmctl=0;
    verticalSync(1);

    /*Set colors*/
    OS.color0= c0;
    OS.color1= c1;
    OS.color2= c2;
    OS.color3= c3;
    OS.color4= c4;
    
    /*Set character set*/
    ANTIC.chbase = chsetPage;

    /*Wait for vsync before setting-up DL*/
    __asm__( " SEI ");
    OS.vdslst = dliHndl;
    OS.sdlst = dlPtr;
    __asm__( " CLI ");

    /*Wait until screen stabilizes*/
    verticalSync(1);

    statusBarForeground = statusBarFg;
    statusBarBackground = statusBarBg;

    /*Wait for vsync before enabling DLI*/
    verticalSync(1);
    ANTIC.nmien = 0xC0;
}

void setIntermissionScreen() {

    setScreen(&TRAIN_DATA_DL_INTERMISSION,
            12, 0,
            &dliInterMission,
            TRAIN_DATA_CHARSET2_PAGE,
            0xC8, 12, 0, 0, 0);
}

void setSceneSelectionScreen() {

    setScreen(&TRAIN_DATA_DL_SCENESELECTION,
            12, 0,
            &dliInterMission,
            TRAIN_DATA_CHARSET2_PAGE,
            0xC8, 12, 0, 14, 0);
}

/*Scene selection repaint selection cursor*/
void sceneSelectionMoveHighlight(unsigned char** ptr, signed char delta) {
    unsigned char* curNumberPtr = *ptr;

    *(curNumberPtr) &= 0x7F;
    *(curNumberPtr + 1) &= 0x7F;
    curNumberPtr += delta;
    *(curNumberPtr) |= 0x80;
    *(curNumberPtr + 1) |= 0x80;
    *ptr += delta;

}

/*Scene selection*/
unsigned char screenSceneSelection() {

    unsigned char *ptr1, *ptr2;
    unsigned char maxCurY;
    unsigned char maxCurX;
    unsigned char z1, cursorX, cursorY;
    unsigned char *curNumberPtr;

    /*Set screen*/
    setSceneSelectionScreen();

    /*Set PMG for scene selection*/
    pmgSetForSceneSelection();

    /*Clear everything*/
    clearScreen();
    clearStatusBar();

    audioRequestSongLine(MUSIC_SCENE_SELECTION);

    /*Draw title line*/
    ptr1 = &TRAIN_DATA_GAMESCREEN;
    memcpy(ptr1 + T_SCENE_SEL_C, T_SCENE_SEL, T_SCENE_SEL_L);
    ptr1 += 20;

    /*Draw separators*/
    memset(&TRAIN_DATA_GAMESCREEN + 20, CHAR_SEPARATOR, 40);
    memset(&TRAIN_DATA_GAMESCREEN + 260, CHAR_SEPARATOR, 40);

    ptr1 = &TRAIN_DATA_GAMESCREEN + 60;

    /*Draw main contents - lines with numbers*/
    if (gameMaxLevelIndex > 0) {
        memcpy(ptr1, T_SCENE_ROW1, T_SCENE_ROW1_L);
        ptr1 += 20;
        maxCurY = 0;
    }
    if (gameMaxLevelIndex > 4) {
        memcpy(ptr1, T_SCENE_ROW2, T_SCENE_ROW2_L);
        ptr1 += 20;
        maxCurY = 1;
    }
    if (gameMaxLevelIndex > 9) {
        memcpy(ptr1, T_SCENE_ROW3, T_SCENE_ROW3_L);
        ptr1 += 20;
        maxCurY = 2;
    }
    if (gameMaxLevelIndex > 14) {
        memcpy(ptr1, T_SCENE_ROW4, T_SCENE_ROW4_L);
        ptr1 += 20;
        maxCurY = 3;
    }
    if (gameMaxLevelIndex > 19) {
        memcpy(ptr1, T_SCENE_ROW5, T_SCENE_ROW5_L);
        ptr1 += 20;
        maxCurY = 4;
    }
    if (gameMaxLevelIndex > 24) {
        memcpy(ptr1, T_SCENE_ROW6, T_SCENE_ROW6_L);
        ptr1 += 20;
        maxCurY = 5;
    }
    if (gameMaxLevelIndex > 29) {
        memcpy(ptr1, T_SCENE_ROW7, T_SCENE_ROW7_L);
        ptr1 += 20;
        maxCurY = 6;
    }
    if (gameMaxLevelIndex > 34) {
        memcpy(ptr1, T_SCENE_ROW8, T_SCENE_ROW8_L);
        ptr1 += 20;
        maxCurY = 7;
    }
    if (gameMaxLevelIndex > 39) {
        memcpy(ptr1, T_SCENE_ROW9, T_SCENE_ROW9_L);
        ptr1 += 20;
        maxCurY = 8;
    }
    if (gameMaxLevelIndex > 44) {
        memcpy(ptr1, T_SCENE_ROW10, T_SCENE_ROW10_L);
        ptr1 += 20;
        maxCurY = 9;
    }

    /*Every other line is highlighted*/
    for (ptr2 = &TRAIN_DATA_GAMESCREEN + 60; ptr2<&TRAIN_DATA_GAMESCREEN + 260; ptr2 += 40) {
        for (curNumberPtr = ptr2; curNumberPtr < ptr2 + 20; curNumberPtr++) {
            *(curNumberPtr) |= 64;
        }
    }

    /*Show what to do in the status bar*/
    memcpy(&TRAIN_DATA_STATUSBARSCREEN + T_SCENE_BEGIN_C, T_SCENE_BEGIN, T_SCENE_BEGIN_L);

    /*Determine maximum X cursor coordinate*/
    maxCurX = gameMaxLevelIndex % 5;

    /*Determine cursor coordinates*/
    cursorY = gameMaxLevelIndex / 5;
    cursorX = gameMaxLevelIndex % 5;

    /*Now delete extraneous scene numbers*/
    ptr2 = ptr1;
    ptr1 -= 20; //Point to the last line
    ptr1 += 3; //Position to the first triplet
    ptr1 += 3 * ((cursorX) + 1); //Position for deletion
    curNumberPtr = ptr1 - 3; //Position in screen memory for current
    memset(ptr1, 0, ptr2 - ptr1); //Delete rest of the line

    /*Position the player so that the maximum reached level is selected*/
    p0x = 66 + (cursorX * 24);
    p0y = 59 + (cursorY * 10);
    GTIA_WRITE.hposp0 = p0x;
    memset(p0Area + p0y, 254, 10);

    /*Highlight the selected level*/
    *(curNumberPtr) |= 0x80;
    *(curNumberPtr + 1) |= 0x80;

    enableDisplay();

    /*Swap all controls*/
    ctrlSwapAll();

    while (jsGetFire() == 1) {

        z1 = jsGet(0);

        switch (z1) {
            case JS_LEFT:
            {
                OS.atract=0;
                if (cursorX == 0) break;
                cursorX--;
                p0x -= 24;
                GTIA_WRITE.hposp0= p0x;
                sceneSelectionMoveHighlight(&curNumberPtr, -3);
                delay(10);
                kpdSwap();
                break;
            }
            case JS_RIGHT:
            {
                OS.atract=0;
                if ((cursorX == 4) || (cursorY == maxCurY && cursorX == maxCurX)) break;
                cursorX++;
                p0x += 24;
                GTIA_WRITE.hposp0=p0x;
                sceneSelectionMoveHighlight(&curNumberPtr, +3);
                delay(10);
                kpdSwap();
                break;
            }
            case JS_UP:
            {
                OS.atract=0;
                if (cursorY == 0) break;
                memset(p0Area + p0y, 0, 10);
                cursorY--;
                p0y -= 10;
                memset(p0Area + p0y, 254, 10);
                sceneSelectionMoveHighlight(&curNumberPtr, -20);
                delay(9);
                kpdSwap();
                break;
            }
            case JS_DOWN:
            {
                OS.atract=0;
                if ((cursorY == maxCurY) || (cursorY == maxCurY - 1 && cursorX > maxCurX)) break;
                memset(p0Area + p0y, 0, 10);
                cursorY++;
                p0y += 10;
                memset(p0Area + p0y, 254, 10);
                sceneSelectionMoveHighlight(&curNumberPtr, +20);
                delay(9);
                kpdSwap();
                break;
            }
        }

    }

    audioRequestSongLine(MUSIC_EMPTY);

    return (cursorY * 5)+cursorX;
}

void screenGameOver() {



    /*Set screen*/
    setIntermissionScreen();

    /*Set PMG for scene selection*/
    hideLoco();

    /*Clear everything*/
    clearScreen();
    clearStatusBar();

    /*Two separator lines*/
    memset(&TRAIN_DATA_GAMESCREEN + 20, CHAR_SEPARATOR, 40);
    memset(&TRAIN_DATA_GAMESCREEN + 200, CHAR_SEPARATOR, 40);

    /*Game Over inscription*/
    memcpy(&TRAIN_DATA_GAMESCREEN + 100 + T_GAME_OVER_C, T_GAME_OVER, T_GAME_OVER_L);

    audioRequestSongLine(MUSIC_GAME_OVER);

    enableDisplay();

    delay(100);
    ctrlSwapAll();
    memcpy(&TRAIN_DATA_STATUSBARSCREEN + T_PRESS_FIRE_C, T_PRESS_FIRE, T_PRESS_FIRE_L);

    /*Wait for FIRE*/
    while (jsGetFire() == 1);

    /*And disable music*/
    audioRequestSongLine(MUSIC_EMPTY);

}

void screenRainbow() {

    unsigned char p20;
    unsigned char vc;
    unsigned char c = 0;

    while (jsGetFire() == 1) {

        p20 = OS.rtclok[1];
        while (p20 == OS.rtclok[1]) {
            vc =ANTIC.vcount;
            ANTIC.wsync=0;
            GTIA_WRITE.colbk= (vc + c) & 0xF2;
        }
        ++c;
    }

}

void screenGratulation() {

    unsigned char *ptr1;

    /*Set screen*/
    setIntermissionScreen();
    pmgSetForLoco();

    fadeInAndOut();

    /*Clear everything*/
    clearScreen();
    clearStatusBar();

    /*Two separator lines*/
    memset(&TRAIN_DATA_GAMESCREEN + 20, CHAR_SEPARATOR, 40);
    memset(&TRAIN_DATA_GAMESCREEN + 200, CHAR_SEPARATOR, 40);
    /*Track*/
    memset(&TRAIN_DATA_GAMESCREEN + 160, CHAR_TRACK, 40);

    /*Title*/
    ptr1 = &TRAIN_DATA_GAMESCREEN;
    memcpy(ptr1 + T_CONGRAT_1_C, T_CONGRAT_1, T_CONGRAT_1_L);

    /*Main text for congratulations*/
    ptr1 = &TRAIN_DATA_GAMESCREEN + 60;
    memcpy(ptr1 + T_CONGRAT_2_C, T_CONGRAT_2, T_CONGRAT_2_L);
    ptr1 += 20;
    memcpy(ptr1 + T_CONGRAT_3_C, T_CONGRAT_3, T_CONGRAT_3_L);
    ptr1 += 40;
    memcpy(ptr1 + T_CONGRAT_4_C, T_CONGRAT_4, T_CONGRAT_4_L);

    /*Set PMG for scene selection*/
    trainData = &TRAIN_FIGURE_RIGHT;
    setLocoPos(0, 6, 0, 14);
    menuCycleTrainFlag = 1;
    audioRequestSongLine(MUSIC_CONGRATULATIONS);
    enableDisplay();

    delay(100);
    ctrlSwapAll();
    memcpy(&TRAIN_DATA_STATUSBARSCREEN + T_PRESS_FIRE_C, T_PRESS_FIRE, T_PRESS_FIRE_L);

    /*Cycle train and wait for FIRE*/
    screenRainbow();
    menuCycleTrainFlag = 0;
    audioRequestSongLine(MUSIC_EMPTY);

}

/*The new high score screen*/
void screenNewHighScore() {

    disableDisplay();
    clearStatusBar();
    clearScreen();

    fadeInAndOut();
    pmgSetForLoco();

    /*Two separator lines*/
    memset(&TRAIN_DATA_GAMESCREEN + 20, CHAR_SEPARATOR, 40);
    memset(&TRAIN_DATA_GAMESCREEN + 200, CHAR_SEPARATOR, 40);
    /*Track*/
    memset(&TRAIN_DATA_GAMESCREEN + 160, CHAR_TRACK, 40);

    /*Display 'Well Done' as screen title*/
    memcpy(&TRAIN_DATA_GAMESCREEN + T_WELL_DONE_C, T_WELL_DONE, T_WELL_DONE_L);

    /*Place new high score*/
    memcpy(&TRAIN_DATA_GAMESCREEN + 80 + T_NEW_HISCORE_C, T_NEW_HISCORE, T_NEW_HISCORE_L);
    memcpy(&TRAIN_DATA_GAMESCREEN + 100 + 7, dHighScore, sizeof (dHighScore));
    *(&TRAIN_DATA_GAMESCREEN + 100 + 7 + sizeof (dHighScore)) = 16;

    /*Prepare train for cycling*/
    trainData = &TRAIN_FIGURE_RIGHT;
    setLocoPos(0, 6, 0, 14);
    menuCycleTrainFlag = 1;

    audioRequestSongLine(MUSIC_NEW_HISCORE);
    enableDisplay();

    delay(100);
    ctrlSwapAll();
    memcpy(&TRAIN_DATA_STATUSBARSCREEN + T_PRESS_FIRE_C, T_PRESS_FIRE, T_PRESS_FIRE_L);

    /*Wait for FIRE*/
    while (jsGetFire()==1);

    menuCycleTrainFlag = 0;
    audioRequestSongLine(MUSIC_EMPTY);

}

unsigned char gameToNextLevel() {

    unsigned char i2;
    unsigned char z1;

    /*Display level complete*/
    memset(&TRAIN_DATA_STATUSBARSCREEN + 14, 0, 26);
    memcpy(&TRAIN_DATA_STATUSBARSCREEN + 14 + T_LEVEL_COMPLETE_C, T_LEVEL_COMPLETE, T_LEVEL_COMPLETE_L);

    audioRequestSFX(SFX_SCENE_COMPLETE);

    /*Increment score*/
    z1 = gameLives * 10;
    for (i2 = 0; i2 < z1; i2++) {
        incrementScore();
        incrementScore();
        VERTICAL_SYNC(GTNL_1);
    }
    /*And save the score*/
    saveZonedScore();

    /*Check for new life*/
    if (memcmp(dScore, dNextLifeScore, sizeof (dScore)) > 0) {
        if (gameLives < 9) {
            gameLives++;
        }
        incrementNextLifeScore();
    }

    /*If all levels complete, show the congratulations screen*/
    if (gameLevelIndex == CFG_MAX_LEVEL_INDEX) {
        delay(150 - z1);
        return ADVANCE_NEXT_CONGRATS;
    }/*Otherwise proceed to the next level*/
    else {
        gameLevelIndex++;
        /*Update maximum level reached*/
        if (gameLevelIndex > gameMaxLevelIndex) {
            gameMaxLevelIndex = gameLevelIndex;
        }
        delay(150 - z1);
        return ADVANCE_NEXT_CONTINUE;
    }

}

unsigned char gameLoseLife(unsigned char deathType) {

    unsigned char z1;
    unsigned char y1;
    unsigned char i2;

    unsigned char glowColor1;
    unsigned char glowColor2;

    /*Clear portion of the status bar*/
    memset(&TRAIN_DATA_STATUSBARSCREEN + 14, 0, 26);

    if (deathType == DEATH_TYPE_POISON) {
        memcpy(&TRAIN_DATA_STATUSBARSCREEN + 14 + T_TRAIN_POISONED_C, T_TRAIN_POISONED, T_TRAIN_POISONED_L);
    } else {
        /*Random message*/
        z1 = POKEY_READ.random & 0x03;
        switch (z1) {
            case 0:
            {
                memcpy(&TRAIN_DATA_STATUSBARSCREEN + 14 + T_TRAIN_CRASHED0_C, T_TRAIN_CRASHED0, T_TRAIN_CRASHED0_L);
                break;
            }
            case 1:
            {
                memcpy(&TRAIN_DATA_STATUSBARSCREEN + 14 + T_TRAIN_CRASHED1_C, T_TRAIN_CRASHED1, T_TRAIN_CRASHED1_L);
                break;
            }
            case 2:
            {
                memcpy(&TRAIN_DATA_STATUSBARSCREEN + 14 + T_TRAIN_CRASHED2_C, T_TRAIN_CRASHED2, T_TRAIN_CRASHED2_L);
                break;
            }
            case 3:
            {
                memcpy(&TRAIN_DATA_STATUSBARSCREEN + 14 + T_TRAIN_CRASHED3_C, T_TRAIN_CRASHED3, T_TRAIN_CRASHED3_L);
                break;
            }
        }
    }

    /*Determine the glow style*/
    if (deathType == DEATH_TYPE_CRASH) {
        glowColor1 = 0xF4;
        glowColor2 = 0xFF;
        audioRequestSFX(SFX_CRASH);
    }/*Otherwise poisoned cargo*/
    else {
        glowColor1 = 0xB4;
        glowColor2 = 0xBf;
        audioRequestSFX(SFX_POISONED_CARGO);
    }

    /*Glowing loco phase 1*/
    for (y1 = glowColor1; y1 < glowColor2; y1++) {
        memset((void*) &OS.pcolr0, y1, 4);
        delay(3);
    }

    /*Glowing loco - phase 2*/
    for (y1 = glowColor2; y1 > glowColor1; y1--) {
        memset((void*) &OS.pcolr0, y1, 4);
        delay(2);
    }


    /*Hide loco and restore colors*/
    hideLoco();
    pmgSetForLoco();


    /*If crashed, then decrease score by 300 (if possible)*/
    if (deathType == DEATH_TYPE_CRASH) {
        if (memcmp(gameScorePtr, d300, ZN_SCORE_LENGTH) > 0) {
            for (i2 = 0; i2 < 30; i2++) {
                verticalSync(1);
                decrementScore();
            }
        } else {
            memset(gameScorePtr, ZN_0, ZN_SCORE_LENGTH);
        }
    }/*If poisoned, revert the score back*/
    else {
        memcpy(gameScorePtr, dLevelInitialScore, ZN_SCORE_LENGTH);
    }
    delay(50);

    /*Store the score*/
    saveZonedScore();

    /*Lose life when crashed*/
    if (deathType == DEATH_TYPE_CRASH) {
        if (gameLives > 0) {
            gameLives--;
            return LOSE_LIFE_CONTINUE;
        } else {
            return LOSE_LIFE_GAMEOVER;
        }
        /*If poisoned, then just continue*/
    } else {
        return LOSE_LIFE_CONTINUE;
    }

}

void decimalDisplaysInit() {
    gameScorePtr = &TRAIN_DATA_STATUSBARSCREEN + 6;
    gameLivesPtr = &TRAIN_DATA_STATUSBARSCREEN + 39;
    gameLevelPtr = &TRAIN_DATA_STATUSBARSCREEN + 24;
    lastScorePtr = &TRAIN_DATA_STATUSBARSCREEN + 11;
    highScorePtr = &TRAIN_DATA_STATUSBARSCREEN + 34;
    resetZonedScore();
}

unsigned char updateMenuScores() {
    memcpy(dLastScore, dScore, sizeof (dLastScore));
    if (memcmp(dLastScore, dHighScore, sizeof (dLastScore)) > 0) {
        memcpy(dHighScore, dLastScore, sizeof (dLastScore));
        return NEW_HISCORE_YES;
    }
    return NEW_HISCORE_NO;
}

void incrementNextLifeScore() {
    if (dNextLifeScore[2] == ZN_5) {
        dNextLifeScore[2] = ZN_0;
        if (dNextLifeScore[1] == ZN_9) {
            dNextLifeScore[1] = ZN_0;
            ++dNextLifeScore[0];
        } else {
            ++dNextLifeScore[1];
        }
    } else {
        dNextLifeScore[2] = ZN_5;
    }
}

/* Enable text mode display*/
void enableDisplay() {
   OS.sdmctl=62;
}

void disableDisplay() {
    OS.sdmctl=0;
}

void setGameSpeed(unsigned char speed) {
    normalMoveDelay = NTSCSpeeds[speed];
}

void setJoystickZones(unsigned char zoneSize) {
    unsigned char z = jsZoneSizes[zoneSize];
    jsZoneDown = 114+z;
    jsZoneUp = 114-z;
    jsZoneLeft = 114-z;
    jsZoneRight = 114+z;
}

void toggleGameInit(unsigned char gameInitType) {

    if (gameInitType == GAME_INIT_NORMAL) {

        gameInitLives = 3;
        memset(dGameInitScore, ZN_0, sizeof (dGameInitScore));
        memcpy(dGameInitNextLifeScore, d5000, sizeof (dGameInitScore));

    }
}

void fadeInAndOut() {

    unsigned char i1;

    for (i1 = 0; i1 < 16; i1 += 2) {
        OS.color4=i1;
        verticalSync(3);
    }
    for (i1 = 0; i1 < 16; i1 += 2) {
        OS.color4=(14 - i1);
        verticalSync(3);
    }
}

void verticalSync(unsigned char count) {
    int i = 0;
    for (i = 0; i < count; i++) {
        VERTICAL_SYNC(VSYNC_0);
    }

}

/*When a level is over or a life is lost, fade out everything*/
void levelFadeOut() {

    unsigned char i, k;
    unsigned char* ptr;
    unsigned char count;

    verticalSync(1);

    for (i = 0; i < 16; i++) {

        count = 0;
        ptr = (unsigned char*) &OS.pcolr0;

        for (k = 0; k < 9; k++) {
            if (*ptr > ((*ptr)&0xF0)) {
                *ptr -= 2;
            } else {
                count++;
            }
            ptr++;
        }
        if (count == 9) break;
        verticalSync(3);
    }

}

/*When a level is about to start, fade it in 
  We have desired colors in the shadow registers, we will use them*/
void levelFadeIn() {

    unsigned char i, k, c;

    unsigned char desiredColors[9];
    unsigned char initialColors[5];


    /*Create array of desired colors, PMG + Standard graphics*/
    memcpy(desiredColors, (unsigned char*) &OS.pcolr0, 9);

    /*Set initial color array for standard graphics*/
    for (i = 0; i < 5; i++) {
        initialColors[i] = desiredColors[i + 4]&0xF0;
    }

    /*Set registers for the initial colors*/
    memcpy((unsigned char*) &OS.color0, initialColors, 5);
    memset((unsigned char*) &OS.pcolr0, 0, 4);

    /*Synchronize*/
    verticalSync(1);

    /*Show display*/
    enableDisplay();

    /*Start fading in the standard graphics*/
    for (i = 0; i < 16; i++) {

        c = 0;
        for (k = 0; k < 5; k++) {
            if (initialColors[k] < desiredColors[k + 4]) {
                ++initialColors[k];
                POKE(((unsigned char*) &OS.color0) + k, initialColors[k]);
            } else {
                c++;
            }
        }
        if (c == 5) break;
        verticalSync(2);
    }

    /*Then suddenly display the loco, so it is obvious the game can begin*/
    memcpy((unsigned char*) &OS.pcolr0, desiredColors, 4);

}

/*============================================================================*/
/*Controls*/
/*============================================================================*/
void ctrlSwapAll() {
    jsSwap();
    jsSwapFire();
    kpdSwap();
}

/* Initialize joystick */
void jsInit() {
    GTIA_WRITE.consol = 0x04;
    setJoystickZones(MENU_DZ_MEDIUM);
}

/*Get trigger. 0 Pressed, 1 not pressed*/
unsigned char jsGetFire() {
    return GTIA_READ.trig0;
}

/*Get joystick direction*/
unsigned char jsGet(unsigned char flags) {

    unsigned char jx;
    unsigned char jy;
    unsigned char jk;
    
    /*First read keypad*/
    jk=kpdGet();
    if (jk!=0xFF) {
        
        if (jk==0x04) return JS_LEFT;
        if (jk==0x06) return JS_RIGHT;
        if (jk==0x02) return JS_UP;
        if (jk==0x05 || jk==0x08) return JS_DOWN;
        
        /*Start and reset only if requested by flags*/
        if (flags & JS_FLAG_RESET) {
            if (jk==0x0E) return JS_RESET;
        }
        if (flags & JS_FLAG_START) {
            if (jk==0x0C) return JS_START;
        }
        
    }
    
    /*If not input from keypad, then get the stick*/

    /*Determine direction*/
    jx = PEEK(0x11);
    jy = PEEK(0x12);

    /*If diagonal, return nothing*/
    if ((jx < jsZoneLeft || jx > jsZoneRight) && (jy < jsZoneUp || jy > jsZoneDown)) return JS_C;

    /*Return digital direction*/
    if (jx < jsZoneLeft) return JS_LEFT;
    if (jx > jsZoneRight) return JS_RIGHT;
    if (jy < jsZoneUp) return JS_UP;
    if (jy > jsZoneDown) return JS_DOWN;

    /*Otherwise return no direction*/
    return JS_C;
}

/*Swap joystick*/
void jsSwap() {
    while (jsGet(JS_FLAG_START|JS_FLAG_RESET) != JS_C);
    kpdSwap();
}

void jsSwapFire() {
    while (jsGetFire() == 0);
}

unsigned char kpdGet() {
    unsigned char k;
    k=kpdLastKey;
    kpdLastKey=0xFF;
    return k;
}

void kpdSwap() {
    while (kpdGet()!=0xFF);
}

/*============================================================================*/
/*Binary load*/
/*============================================================================*/
void binaryLoad(unsigned char* beginPtr,unsigned char* pastPtr) {
    
    unsigned char* firstBytePtr;
    unsigned char* lastBytePtr;
    unsigned int segLen;
    
    while(beginPtr<pastPtr) {
    
        /*Binary header, just skip*/
        if (*beginPtr==0xFF && *(beginPtr+1)==0xFF) {
            beginPtr+=2;
            continue;
        }
        /*Segment header*/
        firstBytePtr = (unsigned char*)(*((unsigned int*)beginPtr));
        beginPtr+=2;
        lastBytePtr = (unsigned char*)(*((unsigned int*)beginPtr));
        beginPtr+=2;
        /*Segment body*/
        segLen=((unsigned int)lastBytePtr)-((unsigned int)firstBytePtr)+1;
        memcpy(firstBytePtr,beginPtr,segLen);
        beginPtr+=segLen;
    }
    
}

