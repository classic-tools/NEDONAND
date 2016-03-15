	processor pic16f870
	radix dec
	include "p16f870.inc"
	__CONFIG _CP_OFF & _DEBUG_OFF & _WRT_ENABLE_OFF & _CPD_OFF & _LVP_OFF & _PWRTE_ON & _WDT_OFF & _HS_OSC
	include "shaos-p16.inc"
        include "PDBLv1-2A2.inc"

; NN4 (bad one) -> nedoCPU-16 
;-----------------------------
; 1) GND
; 2) D0 ~~~ 
; 3) D1 ~~~ 
; 4) D2 ~~~ 
; 5) D3 ~~~ 
; 6) COUT-> RA0 (flag C output)
; 7) VOUT-> RA1 (flag V output)
; 8) /O0    RA2 (wire to D0 output)
; 9) /O1    RA3 (wire to D1 output)
;10) /O2    RA4 (wire to D2 output)
;11) NC*    RA5 (wire to D3 output)
;12) VCC -> VCC
;13) --- -> GND
;14)     
;15) O0  -> RB0 (operation 0 input)
;16) O1  -> RB1 (operation 1 input)
;17) O2  -> RB2 (operation 2 input)
;18) C   -> RB3 (flag C input)
;19) A0  -> RB4 (argument A bit 0 input)
;20) A1  -> RB5 (argument A bit 1 input)
;21) A2  -> RB6 (argument A bit 2 input)
;22) A3  -> RB7 (argument A bit 3 input)
;23) B0 ~~~ 
;24) B1  -> RC0 (argument B bit 1 input)
;25) B2  -> RC1 (argument B bit 2 input)
;26) B3  -> RC2 (argument B bit 3 input)
;27)        RC3 (wire to B0 input)
;28)        RC4 (not connected)
;29)        RC5 (not connected)
;30)        RC6 (not connected - used for RS232)
;           RC7 (not connected - used for RS232)

; Variables

byte	equ	0x3C
prefix	equ	0x3D
store0	equ	0x40
store1	equ	0x41
store2	equ	0x42
store3	equ	0x43
store4	equ	0x44

; Reset vector
	ORG 00h
	goto Start

; Interrupt vector
	ORG 04h
	retfie

Start:

; Configure all I/O pins as digital
	_bank1
	_movlr 0x06,ADCON1

; Set direction of ports (A-inputs, B/C-outputs)
	_bank1
	_movlr b'11111111',TRISA
	_movlr b'00000000',TRISB
	_movlr b'11110000',TRISC

; Initialize output ports
	_bank0
	_movlr b'00000000',PORTA
	_movlr b'00000000',PORTB
	_movlr b'00000000',PORTC

; Setup interrupts
	_bank1
	clrf	INTCON ; disable all interrupts and clear all flags
	bcf	OPTION_REG,NOT_RBPU ; enable pull-ups
	bsf	OPTION_REG,INTEDG ; interrupt on rising edge
;	bsf	INTCON,INTE ; enable RB0 port change interrupt
;	bsf	INTCON,GIE ; enable interrupts

; Clear watch dog
	_bank0
	clrwdt

	goto	Main
go256:
        clrf    byte
go256loop:
	comf	PORTC,w
	movlw	255
	movwf	PORTB
	comf	PORTC,w
	nop

        movf    byte,w
        movwf   PORTB   ; 0.0us
;        nop ; to add 0.2us to everything below
        movf    PORTA,w ; 0.2us -> 0x40
        movwf   store0  ; 0.4us
        movf    PORTA,w ; 0.6us -> 0x41
        movwf   store1  ; 0.8us
        movf    PORTA,w ; 1.0us -> 0x42
        movwf   store2  ; 1.2us
        movf    PORTA,w ; 1.4us -> 0x43
        movwf   store3  ; 1.6us
        movf    PORTA,w ; 1.8us -> 0x44
        movwf   store4  ; 2.0us

        movf    prefix,w
        call serial_print_hex
        _serial_print_byte byte
        _serial_send_ ':'
        _serial_print_byte store0
        _serial_send_ ','
        _serial_print_byte store1
        _serial_send_ ','
        _serial_print_byte store2
        _serial_send_ ','
        _serial_print_byte store3
        _serial_send_ ','
        _serial_print_byte store4
        _serial_print_nl

        incfsz  byte,f
        goto    go256loop
        return

Main:
        _serial_print_nl
        _serial_send_ 'H'
        _serial_send_ 'E'
        _serial_send_ 'L'
        _serial_send_ 'L'
        _serial_send_ 'O'
        _serial_print_nl
MainLoop:
	_movlr	0,prefix
	_movlr	0,PORTC
	call	go256
	_movlr	1,prefix
	_movlr	8,PORTC
	call	go256
	_movlr	2,prefix
	_movlr	1,PORTC
	call	go256
	_movlr	3,prefix
	_movlr	9,PORTC
	call	go256
	_movlr	4,prefix
	_movlr	2,PORTC
	call	go256
	_movlr	5,prefix
	_movlr	10,PORTC
	call	go256
	_movlr	6,prefix
	_movlr	3,PORTC
	call	go256
	_movlr	7,prefix
	_movlr	11,PORTC
	call	go256
	_movlr	8,prefix
	_movlr	4,PORTC
	call	go256
	_movlr	9,prefix
	_movlr	12,PORTC
	call	go256
	_movlr	10,prefix
	_movlr	5,PORTC
	call	go256
	_movlr	11,prefix
	_movlr	13,PORTC
	call	go256
	_movlr	12,prefix
	_movlr	6,PORTC
	call	go256
	_movlr	13,prefix
	_movlr	14,PORTC
	call	go256
	_movlr	14,prefix
	_movlr	7,PORTC
	call	go256
	_movlr	15,prefix
	_movlr	15,PORTC
	call	go256

loop:
	goto	loop

	END
