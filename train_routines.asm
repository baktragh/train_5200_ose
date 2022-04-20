;===============================================================================
; Train unified engine for Atari 5200
; Open source edition (OSE)
; Assembler routines
;===============================================================================
.include "atari5200.inc"

;Import symbols
.import _TRAIN_DATA_CHARSET2_PAGE
.import _TRAIN_DATA_CHARSET1_PAGE
.import _TRAIN_DATA_STATUSBARSCREEN
.import _p0x
.import _menuCycleTrainFlag
.import _menuGameAudio

;Variables
.segment "DATA"
;VBI
_vbistorel:             .byte 0                 ;Original VBI routine LO
_vbistoreh:             .byte 0                 ;Original VBI routine HI

;Audio
_sfxRequested:          .byte 0                 ;SFX requested (high level)
_songLineRequested:     .byte 0                 ;Song line requested
_llsfx:                 .byte 0                 ;Low level SFX (instrument nr)
_inGameAudioFlag:       .byte 0                 ;In-game audio indication
_kpdLastKey:            .byte $FF               ;Last keypad key


;Visuals
_statusBarForeground:   .byte 0                 ;Status bar foreground for DLI
_statusBarBackground:   .byte 0                 ;Status bar background for DLI
_menuDliCount:          .byte 0                 ;Counter to control DLI


.segment "ZEROPAGE"
_zp_ptr1:   .word 0
_zp_ptr2:   .word 0
_zp_ptr3:   .word 0
_zp_x1:     .byte 0
_zp_z1:     .byte 0
_zp_jmp:    .word 0


;==============================================================================
; DLI Handlers
;==============================================================================

;------------------------------------------------------------------------------
;DLI handler 1 - Colors and character set for the status bar
;------------------------------------------------------------------------------
.segment "CODE"
_dliHandler:
        pha
        lda _menuDliCount
        bne @status
@field:
        sta WSYNC
        lda _TRAIN_DATA_CHARSET1_PAGE
        sta CHBASE
        inc _menuDliCount
        pla
        rti
@status:
        sta WSYNC               ;Horizontal retrace
        lda _TRAIN_DATA_CHARSET2_PAGE
        sta CHBASE
        lda _statusBarBackground ;Status bar background
        sta COLPF2
        lda _statusBarForeground ;
        sta COLPF1               ;Status bar foreground
        pla
        rti
;------------------------------------------------------------------------------
;DLI handler 2- Handle DLI for menus
; set colors for separator
; set colors for main title
; set colors for subtitle
; set colors for separator
; set colors for menu items
; set colors for the scroller
; set colors for the status bar
;------------------------------------------------------------------------------

_dliMenuHandler:
        pha

        lda _menuDliCount         ;Check where we are
        beq dli_menu_sep

        cmp #1                   ;For the main title
        beq dli_menu_title

        cmp #2                   ;For the main title, 2
        beq dli_menu_title2

        cmp #3
        beq dli_menu_subtitle       ;For the subtitle (darker gray)

        cmp #4                   ;For the second separator
        beq dli_menu_sep

        cmp #5                   ;For the menu items
        beq dli_menu_items

        cmp #6                   ;For the scroller
        beq dli_menu_scroll

        cmp #7                   ;For the status bar
        jmp dli_menu_sbar


.ifdef GAME_TRAIN1
        ctitle_color1a = $0A
        ctitle_color1b = $0C
        ctitle_color2a = $0C
        ctitle_color2b = $0E
.endif

.ifdef GAME_TRAIN2
        ctitle_color1a = $DA
        ctitle_color1b = $DC
        ctitle_color2a = $3C
        ctitle_color2b = $3A
.endif

.ifdef GAME_TRAIN3
        ctitle_color1a = $FA
        ctitle_color1b = $FC
        ctitle_color2a = $1C
        ctitle_color2b = $1A
.endif



        

dli_menu_title:
        lda #(ctitle_color1a)          ;Set colors for main title part 1
        sta WSYNC
        sta COLPF1
        lda #(ctitle_color1b)
        sta COLPF3
        inc _menuDliCount
        pla
        rti

dli_menu_title2:
        lda #(ctitle_color2a)          ;Set colors for main title part 2
        sta WSYNC
        sta COLPF1
        lda #(ctitle_color2b)
        sta COLPF3
        inc _menuDliCount
        pla
        rti

dli_menu_sep:                           ;Set colors for separator (blue)
        lda #$72
        sta WSYNC
        sta COLPF0
        lda #$74
        sta COLPF1
        lda #$76
        sta COLPF2
        lda #$78
        sta COLPF3
        inc _menuDliCount
        pla
        rti

dli_menu_subtitle:                       ;Set colors for subtitle (gray)
        lda #8
        sta WSYNC
        sta COLPF1
        lda #0
        sta COLPF2
        inc _menuDliCount
        pla
        rti


dli_menu_items:                          ;Set colors for menu title
        lda #14
        sta WSYNC
        sta COLPF1
        lda #0
        sta COLPF2
        inc _menuDliCount
        pla
        rti

dli_menu_scroll:                         ;Set colors for the scroller
        lda #0
        sta WSYNC
        sta COLPF2
        lda #8
        sta COLPF1
        lda #12
        sta WSYNC
        sta WSYNC
        sta WSYNC
        sta COLPF1
        inc _menuDliCount
        pla
        rti

dli_menu_sbar:                           ;Set colors for the status bar
        lda _statusBarBackground ;Status bar background
        sta WSYNC                ;Synchronize
        sta COLPF2
        lda _statusBarForeground ;
        sta COLPF1               ;Status bar foreground
        inc _menuDliCount
        pla
        rti


;===============================================================================
; DLI handler for intermission screen
; We will reuse some code for the menu
; 0. Set colors for screen title in graphics 2
; 1. Set colors for separator
; 2. Set colors for main contents in graphics 2
; 3. Set colors for train track
; 4. Set colors for second separator
; 5. Set colors for status bar
;===============================================================================
_dliInterMission:
        pha
        txa
        pha

        lda _menuDliCount        ;Check where we are
        asl                      ;Double
        tax                      ;Transfer to x
        lda dli_jump_table,x
        sta _zp_jmp
        lda dli_jump_table+1,x
        sta _zp_jmp+1

@djmp:  jmp (_zp_jmp)

dli_inter_title:
        sta WSYNC
        lda #180
        sta COLPF0
        inc _menuDliCount
        pla
        tax
        pla
        rti

dli_inter_main:
        sta WSYNC
        lda #$CA
        sta COLPF0
        lda #$EA
        sta COLPF1
        lda #$0F
        sta COLPF2
        sta COLPF3
        inc _menuDliCount
        pla
        tax
        pla
        rti

dli_inter_subtitle:
        lda #8
        sta WSYNC
        sta COLPF1
        lda #0
        sta COLPF2
        inc _menuDliCount
        pla
        tax
        pla
        rti

dli_inter_sep:                            ;Set colors for separator (blue)
        lda #$72
        sta WSYNC
        sta COLPF0
        lda #$74
        sta COLPF1
        lda #$76
        sta COLPF2
        lda #$78
        sta COLPF3
        inc _menuDliCount
        pla
        tax
        pla
        rti

dli_inter_sbar:                  ;Set colors for the status bar
        lda #0
        sta WSYNC                ;Synchronize
        sta COLPF2
        lda #14
        sta COLPF1               ;Status bar foreground
        inc _menuDliCount
        pla
        tax
        pla
        rti

;-------------------------------------------------------------------------------
; DLI jump table
;-------------------------------------------------------------------------------
dli_jump_table:
       .word dli_inter_title
       .word dli_inter_sep
       .word dli_inter_main
       .word dli_inter_subtitle
       .word dli_inter_sep
       .word dli_inter_sbar

;===============================================================================
; Universal VBI Routine
; Play menu music
; Play ingame music (when enabled)
; Play ingame SFX (when enabled)
; Allow switching between music and SFX
; Train cycling (when enabled)
; Resetting DLI counter
;===============================================================================

; RMT addresses
vRMT_BASE = $2000
vRMT_SFX  = vRMT_BASE+15
vRMT_UPDATE = vRMT_BASE+3
vRMT_STOP = vRMT_BASE+9
vRMT_SETPOKEY = vRMT_BASE+12

; RMT Music address
aRMT_MUSIC = $2400

; SFX equates
;High level requests
cSFX_TRAIN=1
cSFX_TREASURE=2
cSFX_CRASH=4
cSFX_SCENE_COMPLETE=8
cSFX_POISONED_TREASURE=16

;Instrument numbers (doubled)
rSFX_TRAIN=$15*2
rSFX_TREASURE=$16*2
rSFX_CRASH=$17*2
rSFX_SCENE_COMPLETE=$18*2
rSFX_POISONED_TREASURE=$19*2


.segment "CODE"
_vbiUniversalRoutine:
;-------------------------------------------------------------------------------
; Clear display list counter
;-------------------------------------------------------------------------------
        lda #0                  ;Clear DLI counter for all screens
        sta _menuDliCount
;-------------------------------------------------------------------------------
;Handle SFX if enabled
;-------------------------------------------------------------------------------
        ;Check if an SFX is requested
        lda _sfxRequested
        beq @nosfx

;Determine what SFX
        ;Check if train sound
        cmp #cSFX_TRAIN
        beq @train

        cmp #cSFX_TREASURE
        beq @treasure

        cmp #cSFX_CRASH
        beq @crash

        cmp #cSFX_POISONED_TREASURE
        beq @poisoned

        cmp #cSFX_SCENE_COMPLETE
        beq @scene

;Handle crash
@crash: ldy #rSFX_CRASH
        bne @t2

@poisoned: ldy #rSFX_POISONED_TREASURE
        bne @t2

;Handle scene complete
@scene: ldy #rSFX_SCENE_COMPLETE
        ldx #1
        lda #30
        sty _llsfx
        jsr vRMT_SFX
        jmp @ready

;Handle treasure (combine with train sound)
@treasure:
        ldx #1          ;Channel 0+1
        lda #53         ;Note 30
        ldy #rSFX_TREASURE   ;SFX number
        sty _llsfx
        jsr vRMT_SFX    ;Tell RMT

;Handle train sound
@train: ldy #rSFX_TRAIN  ;SFX number
@t2:    sty _llsfx
        ldx #3          ;Channel 0+3
        lda #30         ;Note 30
        jsr vRMT_SFX    ;Tell RMT

;Reset requested SFX
@ready: lda #0
        sta _sfxRequested

;-------------------------------------------------------------------------------
; Handle any updates in music
;-------------------------------------------------------------------------------
@nosfx: lda _songLineRequested      ;Check request for song line
        cmp #255                    ;$FF Indicates no request
        beq @update                 ;Skip if no request

        ldx #<aRMT_MUSIC            ;Request playing a songline
        ldy #>aRMT_MUSIC
        ;Initialize the tracker
        jsr vRMT_BASE
        lda #255
        sta _songLineRequested      ;Clear the request
;-------------------------------------------------------------------------------
; Handle RMT player update
;-------------------------------------------------------------------------------
@update:
         ;Music update - Call RMT
         jsr vRMT_UPDATE
;-------------------------------------------------------------------------------
; Cycle the train when requested
;-------------------------------------------------------------------------------
        ;Cycle the train if requested
        lda _menuCycleTrainFlag
        bne @cycledo
        jmp @callOriginal
@cycledo:
        ;Check current position of the train
        lda _p0x
        cmp #208
        bne @cycleInc   ;If not 200, then move to the right
        lda #32         ;Otherwise reset
        sta _p0x        ;Store new coordinate
        bne @cycleDone  ;And set GTIA registers (always jumps)
@cycleInc:
        inc _p0x        ;Just increment the position
        lda _p0x        ;And keep it

@cycleDone:             ;Set GTIA registers
        sta HPOSP0
        sta HPOSP1
        sta HPOSP2
        sta HPOSP3

;-------------------------------------------------------------------------------
;Call the original VBI routine
;-------------------------------------------------------------------------------
@callOriginal:
        jmp (_vbistorel)

;===============================================================================
; Set-up the universal VBI routone
; Do it via proper OS call
;===============================================================================
.segment "CODE"

.proc _rmtSetUniversalVBI: near
.segment "CODE"
      lda #124
 @l:  cmp VCOUNT
      bne @l
      ;store original vbi address
      lda VVBLKD
      sta _vbistorel
      lda VVBLKD+1
      sta _vbistoreh
      ;Set new
      lda  #<_vbiUniversalRoutine
      sta VVBLKD
      lda  #>_vbiUniversalRoutine
      sta VVBLKD+1
      rts
.endproc





;===============================================================================
; Score increment.
; This one is written in assembler, because the routine
; must be fast. Simple zoned decimal increment.
;===============================================================================

.segment "CODE"
.proc _incrementScore: near
        scoreOffset = 6
        scoreMaxIndex = 4
        pha
        ldx #scoreMaxIndex
@loop:  lda _TRAIN_DATA_STATUSBARSCREEN+scoreOffset,X      ;Get zoned digit
        cmp #(16+9)                                     ;Is that nine?
        bne @not_9                                      ;No, just increment
        lda #16                                         ;Set to 0
        sta _TRAIN_DATA_STATUSBARSCREEN+scoreOffset,X
        cpx #0                                          ;All digits?
        beq @quit                                       ;Yes, what to do?
        dex                                             ;Next digit
        jmp @loop                                       ;Do it again

@not_9: inc _TRAIN_DATA_STATUSBARSCREEN+scoreOffset,X
@quit:  pla
        rts
.endproc

;===============================================================================
; Score decrement.
; This one is written in assembler, because the routine
; must be fast. Simple zoned decimal increment.
;===============================================================================

.segment "CODE"
.proc _decrementScore: near
        scoreOffset = 6
        scoreMaxIndex = 4
        pha
        ldx #scoreMaxIndex
@loop:  lda _TRAIN_DATA_STATUSBARSCREEN+scoreOffset,X      ;Get zoned digit
        cmp #(16)                                       ;Is that zero?
        bne @not_0                                      ;No, just decrement
        lda #(16+9)                                     ;Set to 9
        sta _TRAIN_DATA_STATUSBARSCREEN+scoreOffset,X
        cpx #0                                          ;All digits?
        beq @quit                                       ;Yes, what to do?
        dex                                             ;Next digit
        jmp @loop                                       ;Do it again

@not_0: dec _TRAIN_DATA_STATUSBARSCREEN+scoreOffset,X
@quit:  pla
        rts

.endproc

;-------------------------------------------------------------------------------
; Break IRQ handler - do nothing
;-------------------------------------------------------------------------------
.segment "CODE"
_brkIRQHandler:
        pla
        rti
;-------------------------------------------------------------------------------
; Keypad continuation handler. A holds keypad code.
;-------------------------------------------------------------------------------
.segment "CODE"
_kpdHandler:
        sta _kpdLastKey       ;Store the last key
@kpxit: pla
        tay
        pla
        tax
        pla
        rti
;===============================================================================
; Exports
;===============================================================================
.export _rmtSetUniversalVBI
.export _dliHandler
.export _dliMenuHandler
.export _dliInterMission
.export _songLineRequested
.export _statusBarForeground
.export _statusBarBackground
.export _incrementScore
.export _decrementScore
.export _sfxRequested
.export _inGameAudioFlag
.export _zp_ptr1
.export _zp_ptr2
.export _zp_ptr3
.export _zp_x1
.export _zp_z1
.export _brkIRQHandler
.export _kpdLastKey
.export _kpdHandler