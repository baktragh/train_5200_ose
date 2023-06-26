;===============================================================================
; Train unified engine for Atari 5200
; Open source edition (OSE)
; Game data
;===============================================================================


;-------------------------------------------------------------------------------
; Export symbols, so the symbols are visible for the main program
;-------------------------------------------------------------------------------
.export  _TRAIN_DATA_CHARSET1
.export  _TRAIN_DATA_CHARSET1_PAGE
.export  _TRAIN_DATA_CHARSET2
.export  _TRAIN_DATA_CHARSET2_PAGE
.export  _TRAIN_DATA_DL_MENU
.export  _TRAIN_DATA_DL_GAME
.export  _TRAIN_DATA_DL_INTERMISSION
.export  _TRAIN_DATA_DL_SCENESELECTION
.export  _TRAIN_DATA_PMGAREA
.export  _TRAIN_DATA_GAMESCREEN
.export  _TRAIN_DATA_STATUSBARSCREEN

.export  _TRAIN_FIGURE_RIGHT
.export  _TRAIN_FIGURE_LEFT
.export  _TRAIN_FIGURE_UP
.export  _TRAIN_FIGURE_DOWN

.export  _MENU_TRAIN_TITLE
.export  _TRAIN_GAME_LEVELS
.export  _TRAIN_GAME_ELEMENTS

.export  _TRAIN_DATA_RMTPLAYER
.export  _TRAIN_DATA_RMTPLAYER_END
.export  _TRAIN_DATA_RMTSONG
.export  _TRAIN_DATA_RMTSONG_END


.import __TRAIN_SEG_CHARSET1_RUN__

;-------------------------------------------------------------------------------
; Character sets 
; Character sets must be aligned at page boudnary
;-------------------------------------------------------------------------------
; Character set 1 - Level plane elements
.segment "TRAIN_SEG_CHARSET1"
_TRAIN_DATA_CHARSET1:
.incbin "data/fonts/trainfont1.fnt"
_TRAIN_DATA_CHARSET1_END:
.segment "CODE"
_TRAIN_DATA_CHARSET1_PAGE: .byte >(__TRAIN_SEG_CHARSET1_RUN__)

;Character set 2 - Status bar
.segment "TRAIN_SEG_CHARSET2"
_TRAIN_DATA_CHARSET2:
.incbin "data/fonts/trainfont2.fnt"
_TRAIN_DATA_CHARSET2_END:
.segment "CODE"
_TRAIN_DATA_CHARSET2_PAGE: .byte >(_TRAIN_DATA_CHARSET2)

;===============================================================================
; Display lists
;===============================================================================

;-------------------------------------------------------------------------------
; Display list for menu
;-------------------------------------------------------------------------------
.segment "TRAIN_SEG_DL"
_TRAIN_DATA_DL_MENU:
.byte 112,112
.byte 112+128                 ; Blank + DLI for separator colors
.byte 4+64                    ; Separator line with LMS                       1      
.word _TRAIN_DATA_GAMESCREEN  ; Screen memory
.byte 32+128                  ; Three blank lines + DLI for main title          
.byte 4,4,4,4,4,0+128,4,4,4,4   ; 8 lines for main title, DLI for more colors 9 
.byte 32+128                  ; Three blank lines + DLI for subtitle colors
.byte 2                       ; 1 line for subtitle                           1
.byte 32+128                  ; Three blank lines + DLI for separator colors
.byte 4                       ; Separator line                                1
.byte 80+128                 ; More blank lines + DLI for menu option colors
.byte 2                       ; Menu line - Start game                        1 
.byte 16                      ; 2 blank lines
.byte 2                       ; Menu line - Game Speed: xxxx                  1 
.byte 16                      ; 2 blank lines
.byte 2                       ; Menu line - In-Game Audio:                    1
.byte 16                      ; 2 blank lines
.byte 2                       ; Menu line - Dead zone                         1      
.byte 16,80+128               ; More blank lines between menu items and bar
.byte 4                       ; The track                                     1
.byte 48
.byte 32+128                  ; Three blank lines + DLI for status bar
.byte 2+64                    ; One line for status bar (scores)              1
.word _TRAIN_DATA_STATUSBARSCREEN
.byte 65                      ; JVB
.word _TRAIN_DATA_DL_MENU     ; DL jump                                      18
_TRAIN_DATA_DL_MENU_END:

_TRAIN_DATA_DL_GAME:
.byte 112 ,112+128            ; Blank+DLI
.byte 068                     ; LMS for GR.12
.word _TRAIN_DATA_GAMESCREEN  ; Screen memory
.byte 004 ,004 ,004 ,004 ,004 ,004 ,004 ,004 ,004 ,004 ,004 ,004 ,004 ,004 ,004
.byte 004 ,004 ,004 ,004 ,004 ,004, 004, 004
.byte 16+128                  ; Blank + DLI

.byte 066                          ; LMS for GR.0
.word _TRAIN_DATA_STATUSBARSCREEN  ; Status bar screen memory
.byte 065                          ; JVB
.byte <_TRAIN_DATA_DL_GAME,>_TRAIN_DATA_DL_GAME  ;DL Jump
_TRAIN_DATA_DL_GAME_END:

;Intermission screen -  Game over, game end, and well done
;Primary color COLPF0 - Capital letters and numbers
_TRAIN_DATA_DL_INTERMISSION:
.byte 112,112
.byte 112+128                 ;Blank + DLI for GR.2 title line
.byte 64+7                    ;LMS for graphics 2                           (0)
.word _TRAIN_DATA_GAMESCREEN  ;Screen memory
.byte 32+128                  ;Three blank lines + DLI for separator colors
.byte 4                       ;Separator line                               (20)
.byte 32+128                  ;Three blank lines + DLI for main area colors
.byte 7,7,7,7,7               ;Graphics 2, 5 lines                          (60,80,100,120,140)
.byte 0+128                   ;Three blank lines + DLI for track
.byte 0
.byte 0
.byte 4                       ;Track                                        (160)
.byte 16+128                  ;Two blank lines + DLI for separator colors   
.byte 4                       ;Separator                                    (200)
.byte 0+128                   ;1 blank line + DLI for status bar
.byte 0                       ;One more blank line 
.byte 0                       ;One more blank line
.byte 2+64                    ;Status bar line + LMS
.word _TRAIN_DATA_STATUSBARSCREEN
.byte 065                     ;JVB
.word _TRAIN_DATA_DL_INTERMISSION


;Scene selection screen
;Primary color COLPF0 - Capital letters and numbers
_TRAIN_DATA_DL_SCENESELECTION:
.byte 112,112                 ;Blank lines    
.byte 112+128                 ;Blank line + DLI for title line
.byte 64+7                    ;LMS for graphics 2                        (0)
.word _TRAIN_DATA_GAMESCREEN  ;Screen memory
.byte 16+128                  ;Two blanks + DLI for separator 1
.byte 4                       ;Separator                                 (20)
.byte 16+128                  ;Two blanks + DLI for main contents
.byte 6,16,6,16,6,16,6,16,6,16 ;20 lines graphics 1 interleaved          (60)
.byte 6,16,6,16,6,16,6,16,6,16
.byte 0+128                   ;1 blank line for fake DLI for track
.byte 0+128                   ;1 blank line + DLI for separator 2
.byte 4                       ;Separator                                 (260)
.byte 32+128                  ;3 blank lines + DLI for status bar
.byte 64+2                    ;LMS for graphics 0
.word _TRAIN_DATA_STATUSBARSCREEN
.byte 065
.word _TRAIN_DATA_DL_SCENESELECTION

;-------------------------------------------------------------------------------
; Raster music tracker song - The song is a binary file
; It will be included with section headers
;-------------------------------------------------------------------------------
.segment "TRAIN_SEG_RMTSONG"
_TRAIN_DATA_RMTSONG:

.ifdef GAME_TRAIN1
.incbin "data/music/train1_stripped.rmt"
.endif

.ifdef GAME_TRAIN2
.incbin "data/music/train2_stripped.rmt"
.endif

.ifdef GAME_TRAIN3
.incbin "data/music/train3_stripped.rmt"
.endif

_TRAIN_DATA_RMTSONG_END:

;-------------------------------------------------------------------------------
; Raster music tracker player - The player is a binary file
; It will be included with section headers
;-------------------------------------------------------------------------------
.segment "TRAIN_SEG_RMTPLAYER"
_TRAIN_DATA_RMTPLAYER:

.ifdef GAME_TRAIN1
.incbin "data/music/rmtplayr1.obx"
.endif


.ifdef GAME_TRAIN2
.incbin "data/music/rmtplayr2.obx"
.endif


.ifdef GAME_TRAIN3
.incbin "data/music/rmtplayr3.obx"
.endif


_TRAIN_DATA_RMTPLAYER_END:

;-------------------------------------------------------------------------------
; PMG Area (dummy segment, not written to the binary file)
;-------------------------------------------------------------------------------
.segment "TRAIN_SEG_PMGAREA"
_TRAIN_DATA_PMGAREA:

;-------------------------------------------------------------------------------
; Cave display memory and status bar (dummy segment, not written to the binary)
;-------------------------------------------------------------------------------
.segment "TRAIN_SEG_GAMESCREEN"
_TRAIN_DATA_GAMESCREEN:

.segment "TRAIN_SEG_STATUSBARSCREEN"
_TRAIN_DATA_STATUSBARSCREEN:

;-------------------------------------------------------------------------------
; Train figures
;-------------------------------------------------------------------------------
.segment "CODE"
_TRAIN_FIGURE_RIGHT:
.byte  240,000,000,000,000,000,000,000,000,000,000,000,090,153,231,090  ;Red
.byte  000,000,000,000,000,000,000,000,000,000,000,000,000,102,024,000  ;Yellow
.byte  006,240,144,150,150,254,255,255,255,254,000,000,000,000,000,000  ;LightB
.byte  000,006,006,000,001,001,000,000,000,001,255,254,000,000,000,000  ;DarkB
_TRAIN_FIGURE_LEFT:
.byte  015,000,000,000,000,000,000,000,000,000,000,000,090,153,231,090  ;Red
.byte  000,000,000,000,000,000,000,000,000,000,000,000,000,102,024,000  ;Yellow
.byte  096,015,009,105,105,127,255,255,255,127,000,000,000,000,000,000  ;LightB
.byte  000,096,096,000,128,128,000,000,000,128,255,127,000,000,000,000  ;DarkB
_TRAIN_FIGURE_UP:
.byte  002,003,001,000,000,000,000,000,001,001,128,128,128,129,129,130  ;Red
.byte  000,000,002,002,002,002,002,002,002,002,002,002,002,002,002,000  ;Yellow
.byte  016,056,184,184,184,184,024,024,024,120,024,024,024,024,024,120  ;LightB
.byte  040,004,068,068,068,068,004,004,004,004,004,004,004,004,004,004  ;DarkB
_TRAIN_FIGURE_DOWN:
.byte  065,129,129,001,001,001,001,128,128,000,000,000,000,128,192,064  ;Red
.byte  000,064,064,064,064,064,064,064,064,064,064,064,064,064,000,000  ;Yellow
.byte  030,024,024,024,024,024,030,024,024,024,029,029,029,029,028,008  ;LightB
.byte  032,032,032,032,032,032,032,032,032,032,034,034,034,034,032,020  ;DarkB

;-------------------------------------------------------------------------------
; TRAIN TITLE SCREEN
;-------------------------------------------------------------------------------
_MENU_TRAIN_TITLE:

.ifdef GAME_TRAIN1
.byte 000,000,000,000,216,216,216,216,216,216,000,216,216,216,216,216,000,000,000,216,216,216,216,000,000,216,216,000,000,216,216,216,216,000,000,000,000,000,000
.byte 000,000,000,000,216,216,216,216,216,216,000,216,216,216,216,216,216,000,216,216,216,216,216,216,000,216,216,000,216,216,216,216,216,216,000,000,000,000,000
.byte 000,000,000,000,000,000,216,216,000,000,000,216,216,000,000,216,216,000,216,216,000,000,216,216,000,216,216,000,216,216,000,000,216,216,000,000,000,000,000
.byte 000,000,000,000,000,000,216,216,000,000,000,216,216,000,000,216,216,000,216,216,000,000,216,216,000,216,216,000,216,216,000,000,216,216,000,000,000,000,000
.byte 000,000,000,000,000,000,216,216,000,000,000,216,216,216,216,216,000,000,216,216,216,216,216,216,000,216,216,000,216,216,000,000,216,216,000,000,000,000,000
.byte 000,000,000,000,000,000,216,216,000,000,000,216,216,216,216,000,000,000,216,216,216,216,216,216,000,216,216,000,216,216,000,000,216,216,000,000,000,000,000
.byte 000,000,000,000,000,000,216,216,000,000,000,216,216,000,216,000,000,000,216,216,000,000,216,216,000,216,216,000,216,216,000,000,216,216,000,000,000,000,000
.byte 000,000,000,000,000,000,216,216,000,000,000,216,216,000,216,216,000,000,216,216,000,000,216,216,000,216,216,000,216,216,000,000,216,216,000,000,000,000,000
.byte 000,000,000,000,000,000,216,216,000,000,000,216,216,000,000,216,216,000,216,216,000,000,216,216,000,216,216,000,216,216,000,000,216,216,000,000,000,000,000
.endif

.ifdef GAME_TRAIN2
.byte 000,216,216,216,216,216,216,000,216,216,216,216,216,000,000,000,216,216,216,216,000,000,216,216,000,000,216,216,216,216,000,000,000,216,216,216,216,216,216
.byte 000,216,216,216,216,216,216,000,216,216,216,216,216,216,000,216,216,216,216,216,216,000,216,216,000,216,216,216,216,216,216,000,000,216,216,216,216,216,216
.byte 000,000,000,216,216,000,000,000,216,216,000,000,216,216,000,216,216,000,000,216,216,000,216,216,000,216,216,000,000,216,216,000,000,000,000,000,000,216,216
.byte 000,000,000,216,216,000,000,000,216,216,000,000,216,216,000,216,216,000,000,216,216,000,216,216,000,216,216,000,000,216,216,000,000,000,000,000,000,216,216
.byte 000,000,000,216,216,000,000,000,216,216,216,216,216,000,000,216,216,216,216,216,216,000,216,216,000,216,216,000,000,216,216,000,000,216,216,216,216,216,216 
.byte 000,000,000,216,216,000,000,000,216,216,000,216,000,000,000,216,216,000,000,216,216,000,216,216,000,216,216,000,000,216,216,000,000,216,216,000,000,000,000
.byte 000,000,000,216,216,000,000,000,216,216,000,216,216,000,000,216,216,000,000,216,216,000,216,216,000,216,216,000,000,216,216,000,000,216,216,216,216,216,216
.byte 000,000,000,216,216,000,000,000,216,216,000,000,216,216,000,216,216,000,000,216,216,000,216,216,000,216,216,000,000,216,216,000,000,216,216,216,216,216,216
.byte 000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000
.endif

.ifdef GAME_TRAIN3
.byte 000,216,216,216,216,216,216,000,218,216,216,216,216,219,000,218,216,216,216,216,219,000,216,216,000,218,216,216,216,216,219,000,000,218,216,216,216,216,219
.byte 000,216,216,216,216,216,216,000,216,216,216,216,216,216,000,216,216,216,216,216,216,000,216,216,000,216,216,216,216,216,216,000,000,216,216,216,216,216,216
.byte 000,000,000,216,216,000,000,000,216,216,000,000,216,216,000,216,216,000,000,216,216,000,216,216,000,216,216,000,000,216,216,000,000,000,000,000,000,216,216
.byte 000,000,000,216,216,000,000,000,216,216,000,000,216,216,000,216,216,000,000,216,216,000,216,216,000,216,216,000,000,216,216,000,000,000,000,000,000,216,216
.byte 000,000,000,216,216,000,000,000,216,216,216,216,216,221,000,216,216,216,216,216,216,000,216,216,000,216,216,000,000,216,216,000,000,216,216,216,216,216,216 
.byte 000,000,000,216,216,000,000,000,216,216,216,216,219,000,000,216,216,216,216,216,216,000,216,216,000,216,216,000,000,216,216,000,000,216,216,216,216,216,216
.byte 000,000,000,216,216,000,000,000,216,216,000,216,216,000,000,216,216,000,000,216,216,000,216,216,000,216,216,000,000,216,216,000,000,000,000,000,000,216,216
.byte 000,000,000,216,216,000,000,000,216,216,000,216,216,000,000,216,216,000,000,216,216,000,216,216,000,216,216,000,000,216,216,000,000,216,216,216,216,216,216
.byte 000,000,000,216,216,000,000,000,216,216,000,216,216,219,000,216,216,000,000,216,216,000,216,216,000,216,216,000,000,216,216,000,000,220,216,216,216,216,221
.endif


;-------------------------------------------------------------------------------
; Game levels
;-------------------------------------------------------------------------------
.byte "@LE"
_TRAIN_GAME_LEVELS:

.ifdef GAME_TRAIN1
.incbin "data/levels/levels.dat.1"
.endif

.ifdef GAME_TRAIN2
.incbin "data/levels/levels.dat.2"
.endif

.ifdef GAME_TRAIN3
.incbin "data/levels/levels.dat.3"
.endif

;-------------------------------------------------------------------------------
; Game elements (64*33)
;-------------------------------------------------------------------------------
.byte "@EL"
_TRAIN_GAME_ELEMENTS:

.ifdef GAME_TRAIN1
.incbin "data/levels/elements.dat.1"
.endif

.ifdef GAME_TRAIN2
.incbin "data/levels/elements.dat.2"
.endif

.ifdef GAME_TRAIN3
.incbin "data/levels/elements.dat.3"
.endif

_TRAIN_GAME_ELEMENTS_END:

;Pad to 64 * 33
.repeat (64*33-(_TRAIN_GAME_ELEMENTS_END-_TRAIN_GAME_ELEMENTS))
.byte 000
.endrepeat

.byte "@EE"
.byte "BAHA Software, T35200 OSE *"

;--------------------------------------------------------------------------------
; Diagnostic cartridge flags
;--------------------------------------------------------------------------------
.segment "DCARTYEAR"
.byte $FF,$FF
