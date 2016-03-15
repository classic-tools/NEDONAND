// PDBLv1 - Public Domain Boot Loader version 1 (16 Nov 2011)
// Created by Alexander A. Shabarshin <ashabarshin@gmail.com>
// For PIC16F870 on frequency 20 MHz and speed 9600
// You are free to use this code in any possible way
// Tested in PICC 9.82.9453 lite under MPLABX IDE beta 7.12

// Usage: Press Enter multiple times immediately after reset (~1sec)
// then use commands below to run (all numbers are hexadecimal):
// !AAAA=BBBB - write word BBBB to program memory in address AAAA
// !AAA=BB - write byte BB to data memory in address AAA
// !AA=BB - write byte BB to EEPROM in address AA
// ?AAAA - read one word from program memory with address AAAA
// ?AAA - read one byte from data memory with address AAA
// ?AA - read one byte from EEPROM in address AA
// . - exit from bootloader and jump to address 0x0001

// ToDo (not yet implemented):
// !AAAA=BBBB... - write words to program memory starting with AAAA
// !AAA=BB... - write bytes to data memory starting with AAA
// ?AAAA=BB - read BB words from program memory starting with AAAA
// ?AAA=BB - read BB bytes from data memory starting with AAA
// $A - call subprogram from address A
// ^A - jump to address A

// Errors:
// ERR01 - illegal hexadecimal digit was entered
// ERR02 - character '=' is expected
// ERR03 - enter is expected
// ERR04 - attempt to write in protected region (bootloader code)

#include <pic.h>

__CONFIG(CP_OFF & DEBUG_OFF & WRT_ALL & CPD_OFF & LVP_OFF & PWRTE_OFF & WDTE_OFF & FOSC_HS);

const char sign[] @0x7FA = "PDBLv1";

#define PROTECTION // flag to protect bootloader code

#define ENTER 13 // code of enter key

// all delays for 20 MHz

#define DELAY1US NOP();NOP();NOP();NOP();NOP()
#define DELAY10US delay(1,2)
#define DELAY15US delay(1,4)
#define DELAY20US delay(1,5)
#define DELAY25US delay(1,6)
#define DELAY30US delay(1,8)
#define DELAY40US delay(1,10)
#define DELAY50US delay(1,13)
#define DELAY60US delay(1,15)
#define DELAY75US delay(1,19)
#define DELAY100US delay(1,25)
#define DELAY150US delay(1,38)
#define DELAY200US delay(1,50)
#define DELAY300US delay(1,75)
#define DELAY400US delay(1,100)
#define DELAY500US delay(1,125)
#define DELAY600US delay(1,150)
#define DELAY750US delay(1,188)
#define DELAY900US delay(1,225)
#define DELAY1MS delay(1,250)
#define DELAY(x) delay(x,250)

void delay(unsigned short ms, unsigned char us4)
{
    unsigned short m;
    unsigned char s;
    for(m=0;m<ms;m++)
    {
      for(s=0;s<us4;s++)
      {
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
      }
    }
}

void serial_init()
{
#if 1
    SPBRG = 129; // 9600
#else
    SPBRG = 64; // 19200
#endif
    BRGH = 1;
    SYNC = 0;
    CREN = 0;
    CREN = 1;
    SPEN = 1;
    TXIE = 0;
    RCIE = 0;
    TX9 = 0;
    RX9 = 0;
    TXEN = 0;
    TXEN = 1;
}

void serial_err()
{
    if(OERR){CREN=0;CREN=1;}
    if(FERR){RCREG;}
}

void serial_send(unsigned char s)
{
    while(!TXIF) serial_err();
    TXREG = s;
    DELAY50US;
}

unsigned char serial_recv()
{
    while(!RCIF) serial_err();
    return RCREG;
}

unsigned char serial_read()
{
    unsigned char b = serial_recv();
    if(b>=0x20 && b<0x7F) serial_send(b);
    return b;
}

unsigned char serial_check(unsigned short ms)
{
    unsigned short i;
    unsigned char r = 0;
    for(i=0;i<ms;i++)
    {
        if(RCIF){r=RCREG;break;}
        serial_err();
        DELAY1MS;
    }
    return r;
}

void serial_print_nl()
{
    serial_send('\r');
    serial_send('\n');
}

void serial_print_hex(char h)
{
    if(h>=0 && h<10) serial_send('0'+h);
    else if(h>=10 && h<16) serial_send('A'+h-10);
}

void serial_print_byte(unsigned char b)
{
    serial_print_hex(b>>4);
    serial_print_hex(b&15);
}

void serial_print_word(unsigned short s)
{
    serial_print_byte(s>>8);
    serial_print_byte(s&255);
}

void serial_print_err(unsigned char n)
{
    serial_print_nl();
    serial_send('E');
    serial_send('R');
    serial_send('R');
    serial_print_byte(n);
}

void serial_print_ok()
{
    serial_print_nl();
    serial_send('O');
    serial_send('K');
}

signed char hex1(char h)
{
    char i = -1;
    if(h >= '0' && h <= '9') i = h-'0';
    else if(h >= 'a' && h <= 'f') i = 10+h-'a';
    else if(h >= 'A' && h <= 'F') i = 10+h-'A';
    return i;
}

void data_write(unsigned short a, unsigned char b)
{
    unsigned char* p = a;
    *p = b;
}

unsigned char data_read(unsigned short a)
{
    unsigned char* p = a;
    return *p;
}

void prog_write(unsigned short a, unsigned short b)
{
    FLASH_WRITE(a,b);
}

unsigned short prog_read(unsigned short a)
{
    return FLASH_READ(a);
}

unsigned short compact_nibbles(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
    return (a<<12)|(b<<8)|(c<<4)|d;
}

void do_write()
{
    signed char a,b,c,d,e;
    unsigned short adr = 0;
    unsigned short val = 0;
    a = hex1(serial_read());
    if(a < 0) serial_print_err(1);
    else
    {
        b = hex1(serial_read());
        if(b < 0) serial_print_err(1);
        else
        {
            c = serial_read();
            if(c=='=')
            {   // EEPROM
                adr = compact_nibbles(0,0,a,b);
                a = hex1(serial_read());
                if(a < 0) serial_print_err(1);
                else
                {
                    b = hex1(serial_read());
                    if(b < 0) serial_print_err(1);
                    else
                    {
                        val = compact_nibbles(0,0,a,b);
                        if(serial_recv()!=ENTER) serial_print_err(3);
                        else
                        {
                            eeprom_write(adr,val);
                            serial_print_ok();
                        }
                    }
                }
            }
            else
            {
                c = hex1(c);
                if(c < 0) serial_print_err(1);
                else
                {
                    d = serial_read();
                    if(d=='=')
                    {   // DATA
                        adr = compact_nibbles(0,a,b,c);
                        a = hex1(serial_read());
                        if(a < 0) serial_print_err(1);
                        else
                        {
                            b = hex1(serial_read());
                            if(b < 0) serial_print_err(1);
                            else
                            {
                                val = compact_nibbles(0,0,a,b);
                                if(serial_recv()!=ENTER) serial_print_err(3);
                                else
                                {
                                    data_write(adr,val);
                                    serial_print_ok();
                                }
                            }
                        }
                    }
                    else
                    {
                        d = hex1(d);
                        if(d < 0) serial_print_err(1);
                        else
                        {
                            e = serial_read();
                            if(e!='=') serial_print_err(2);
                            else
                            {   // FLASH
                                adr = compact_nibbles(a,b,c,d);
                                a = hex1(serial_read());
                                if(a < 0) serial_print_err(1);
                                else
                                {
                                    b = hex1(serial_read());
                                    if(b < 0) serial_print_err(1);
                                    else
                                    {
                                        c = hex1(serial_read());
                                        if(c < 0) serial_print_err(1);
                                        else
                                        {
                                            d = hex1(serial_read());
                                            if(d < 0) serial_print_err(1);
                                            else
                                            {
                                                val = compact_nibbles(a,b,c,d);
                                                if(serial_recv()!=ENTER) serial_print_err(3);
                                                else
                                                {
#ifdef PROTECTION
                                                    static unsigned short bootadr = 0;
                                                    if(bootadr==0)
                                                    {
                                                        bootadr = prog_read(0);
                                                        bootadr &= 0x07FF;
                                                    }
                                                    if(adr >= bootadr)
                                                    {
                                                        serial_print_err(4);
                                                        return;
                                                    }
                                                    if(adr==0) adr=1;
#endif
                                                    prog_write(adr,val);
                                                    serial_print_ok();
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void do_read()
{
    signed char a,b,c,d,e;
    unsigned short adr = 0;
    unsigned short val = 0;
    unsigned char byte = 0;
    a = hex1(serial_read());
    if(a < 0) serial_print_err(0);
    else
    {
        b = hex1(serial_read());
        if(b < 0) serial_print_err(0);
        else
        {
            c = serial_read();
            if(c==ENTER)
            {   // EEPROM
                adr = compact_nibbles(0,0,a,b);
                byte = eeprom_read(adr);
                serial_print_nl();
                serial_print_byte(byte);
            }
            else
            {
                c = hex1(c);
                if(c < 0) serial_print_err(0);
                else
                {
                    d = serial_read();
                    if(d==ENTER)
                    {   // DATA
                        adr = compact_nibbles(0,a,b,c);
                        byte = data_read(adr);
                        serial_print_nl();
                        serial_print_byte(byte);
                    }
                    else
                    {
                        d = hex1(d);
                        if(d < 0) serial_print_err(0);
                        else
                        {
                            if(serial_recv()!=ENTER) serial_print_err(3);
                            else
                            {   // FLASH
                                adr = compact_nibbles(a,b,c,d);
                                val = prog_read(adr);
                                serial_print_nl();
                                serial_print_word(val);
                            }
                        }
                    }
                }
            }
        }
    }
}

main()
{
    ADCON1 = 0x06;
    INTCON = 0x00;
    TRISC = 0xFF; // RC6 and RC7 must be inputs
    serial_init();
    unsigned char b = serial_check(1000);
    if(b==ENTER)
    {
        serial_print_nl();
        serial_send('P');
        serial_send('D');
        serial_send('B');
        serial_send('L');
        serial_send('v');
        serial_send('1');
        serial_print_nl();
        serial_send('>');
        b = 0;
        do
        {
            if(b!=ENTER)
            {
                b = serial_read();
            }
            if(b==ENTER)
            {
                serial_print_nl();
                serial_send('>');
                b = 0;
            }
            if(b=='?'){do_read();b=ENTER;}
            if(b=='!'){do_write();b=ENTER;}
        }
        while(b!='.');
    }
/*
    // TEST: print next character from the range [0x20..0x7E] every second
    char a = 0x20;
    while(1)
    {
      serial_send(a);
      if(++a==127) a=0x20;
      DELAY(1000);
    }
 */
    asm("goto 1"); // jump to the loaded code
}
