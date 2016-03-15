; Useful macros for PIC16s
; This code is PUBLIC DOMAIN created by Shaos
; 15-Oct-2009 : _bank0, _bank1, _bank2, _bank3
; 16-Oct-2009 : _delay1, _delay2, _movlr, _movrr

_bank0	MACRO
	bcf STATUS,RP0
	bcf STATUS,RP1
	ENDM

_bank1	MACRO
	bsf STATUS,RP0
	bcf STATUS,RP1
	ENDM

_bank3	MACRO
	bcf STATUS,RP0
	bsf STATUS,RP1
	ENDM

_bank4	MACRO
	bsf STATUS,RP0
	bsf STATUS,RP1
	ENDM

; Delay 3*N+3 cycles using one register
; _delay1 byte-value,reg-name
_delay1	MACRO	N,R1 ; 3*N+3 (N=0 means 256)
	LOCAL	D1
	movlw	N
	movwf	R1
D1:	decfsz	R1,1
	goto	D1
	ENDM

; Delay	774*N+3 cycles using two registers
; _delay2 byte-value,reg-name-1,reg-name-2
_delay2	MACRO	N,R1,R2 ; 774*N+3 (N=0 means N=256)
	LOCAL	D2
        movlw	N
	movwf	R1
D2:	delay1	0,R2	
	decfsz	R1,1
	goto	D2	
	endm
	
; Move byte to register through accumulator W
; _movlr byte-value,reg-name
_movlr	MACRO	B,R ; 2 cycles
	movlw	B
	movwf	R
	ENDM

; Move register value to register through accumulator W
; _movrr reg-name-src,reg-name-dst
_movrr	MACRO	R1,R2 ; 2 cycles
	movf	R1,0
	movwf	R2
	ENDM
