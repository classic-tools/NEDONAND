; Constants for PDBLv1.2A2 (November 2011)
; Created by Alexander "Shaos" Shabarshin <me@shaos.net>
; http://nedocon.com/store/ncpu16/

; COMMON #70..#7D(14) free #7E,#7F
; BANK0 #20..#3B(27) free #3C..#6F
; BANK1 (0) free #A0..#BF

bootloader equ 0x02A2

pdbl_entry equ 0x04DC

_PDBL MACRO
	goto	pdbl_entry
	ENDM

; compact_nibbles - compact 4 nibbles to 1 word
; ARG:W,#70,#71,#72 RET:#70,#71 USE:#20..#2B,#70..#73
compact_nibbles		equ	0x0450
compact_nibbles_1	equ	0x70
compact_nibbles_2	equ	0x71
compact_nibbles_3	equ	0x72

_compact_nibbles MACRO A,B,C,D,W0,W1
	movf	B,w
	movwf	compact_nibbles_1
	movf	C,w
	movwf	compact_nibbles_2
	movf	D,w
	movwf	compact_nibbles_3
	movf	A,w
	call	compact_nibbles
	movf	compact_nibbles_1,w
	movwf	W0
	movf	compact_nibbles_2,w
	movwf	W1
	ENDM

data_read		equ	0x02CE
; ARG:a0,a1 RET:W USE:#70..#73
data_read_a0		equ	0x70
data_read_a1		equ	0x71

data_write		equ	0x2FA
; ARG:a0,a1,b RET:no USE:#70..#75
data_write_a0		equ	0x70
data_write_a1		equ	0x71
data_write_b		equ	0x72

; delay - universal delay in ms or us (for 20MHz)
delay			equ	0x03C4
; ARG:#70,#71,#72 RET:no USE:#70..#76
delay_ms0		equ	0x70
delay_ms1		equ	0x71
delay_us4		equ	0x72

_delay_ MACRO H,L,U
	movlw	L
	movwf	delay_ms0
	movlw	H
	movwf	delay_ms1
	movlw	U
	movwf	delay_us4
	call	delay
	ENDM

_delay_ms MACRO H,L
	_delay_ H,L,250
	ENDM

_delay_us MACRO U
	_delay_ 0,1,U/4
	ENDM

do_read 		equ	0x052F
; ARG:no RET:no USE:#2C..#37

do_write		equ	0x0614
; ARG:no RET:no USE:#2C..#37,#3A,#3B
do_write_bootadr0	equ	0x3A
do_write_bootadr1	equ	0x3B

eeprom_read		equ	0x0385
; ARG:W RET:W USE:#70,#71

_eeprom_read MACRO A,B
	movf	A,w
	call	eeprom_read
	bcf STATUS,RP0
	bcf STATUS,RP1
	movwf	B
	ENDM

_eeprom_read_ MACRO A,B
	movlw	A
	call	eeprom_read
	bcf STATUS,RP0
	bcf STATUS,RP1
	movwf	B
	ENDM

eeprom_write		equ	0x041F
; ARG:W,value RET:W USE:#70-#72
eeprom_write_value	equ	0x70

_eeprom_write MACRO A,B
	movf	B,w
	movwf	eeprom_write_value
	movf	A,w
	call eeprom_write
	ENDM

_eeprom_write_ MACRO A,B
	movf	B,w
	movwf	eeprom_write_value
	movlw	A
	call eeprom_write
	ENDM

_eeprom_write_a MACRO A,B
	movlw	B
	movwf	eeprom_write_value
	movf	A,w
	call eeprom_write
	ENDM

_eeprom_write__ MACRO A,B
	movlw	B
	movwf	eeprom_write_value
	movlw	A
	call eeprom_write
	ENDM

hex1			equ	0x0484
; ARG:W RET:W USE:#70..#72

prog_read		equ	0x03A4
; ARG:a0,a1 RET:a0,a1 USE:#70..#74
prog_read_a0		equ	0x70
prog_read_a1		equ	0x71

_prog_read MACRO A0,A1,W0,W1
	movf	A0,w
	movwf	prog_read_a0
	movf	A1,w
	movwf	prog_read_a1
	call	prog_read
	movf	prog_read_a0,w
	movwf	W0
	movf	prog_read_a1,w
	movwf	W1
	ENDM

prog_write		equ	0x034F
; ARG:a0,a1,b0,b1 RET:no USE:#70..#73
prog_write_a0		equ	0x70
prog_write_a1		equ	0x71
prog_write_b0		equ	0x72
prog_write_b1		equ	0x73

_prog_write MACRO A0,A1,W0,W1
	movf	A0,w
	movwf	prog_write_a0
	movf	A1,w
	movwf	prog_write_a1
	movf	W0,w
	movwf	prog_write_b0
	movf	W1,w
	movwf	prog_write_b1
	call	prog_write
	ENDM

serial_check		equ	0x03F1
; ARG:ms0,ms1 RET:no USE:#77..#7C
serial_check_ms0	equ	0x77
serial_check_ms1	equ	0x78

serial_err		equ	0x02DC
; ARG:no RET:no USE:?

serial_init		equ	0x0337
; ARG:no RET:no USE:?

serial_print_byte	equ	0x02EB
; ARG:W RET:no USE:#7A,#7B

_serial_print_byte MACRO B
	movf	B,w
	call	serial_print_byte
	ENDM

_serial_print_byte_ MACRO B
	movlw	B
	call	serial_print_byte
	ENDM

serial_print_err	equ	0x02B6
; ARG:W RET:no USE:#7C

_serial_print_err MACRO B
	movf	B,w
	call	serial_print_err
	ENDM

_serial_print_err_ MACRO B
	movlw	B
	call	serial_print_err
	ENDM

serial_print_hex	equ	0x0368
; ARG:W RET:no USE:#79

serial_print_nl		equ	0x02A6
; ARG:no RET:no USE:?

_serial_print_nl MACRO
	call	serial_print_nl
	ENDM

serial_print_ok		equ	0x02B0
; ARG:no RET:no USE:?

_serial_print_ok MACRO
	call	serial_print_ok
	ENDM

serial_print_word	equ	0x02AB
; ARG:0,1 RET:no USE:#7C,#7D
serial_print_word_0	equ	0x7C
serial_print_word_1	equ	0x7D

_serial_print_word MACRO B0,B1
	movf	B0,w
	movwf	serial_print_word_0
	movf	B1,w
	movwf	serial_print_word_1
	call	serial_print_word
	ENDM

serial_read		equ	0x030A
; ARG:no RET:W USE:#79,#7A

_serial_read MACRO B
	call	serial_read
	movwf	B
	ENDM

serial_recv		equ	0x02C1
; ARG:no RET:W USE:?

_serial_recv MACRO B
	call	serial_recv
	movwf	B
	ENDM
	
serial_send		equ	0x0320
; ARG:W RET:no USE:#77,#78

_serial_send MACRO B
	movf	B,w
	call	serial_send
	ENDM

_serial_send_ MACRO B
	movlw	B
	call	serial_send
	ENDM
