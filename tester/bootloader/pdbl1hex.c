/* pdbl1hex - Send HEX-file to the PDBLv1 device (PIC16F87X) */
/* Created by Alexander "Shaos" Shabarshin <me@shaos.net> */
/* and transferred into PUBLIC DOMAIN for http://nedocon.com */
/* Press RESET on the device and launch this program with arguments */
/* It works for Linux, Mac OS X and Windows (define WINDA) */

/*
 v1.0 (20 Nov 2011) - initial version with numeric ports
 v1.1 (26 Nov 2011) - ability to set device path instead of number
 v1.2 (13 Mar 2016) - support for windows (define WINDA)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//#define DEBUG
//#define NOMAIN
//#define WINDA

#ifdef WINDA
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#define FAST_TRICK
#define MAXPROG 0x2000

short *prog = NULL;
#ifdef WINDA
char sdev_[32] = "\\\\.\\COM";
#else
char sdev_[32] = "/dev/ttyS";
int fd;
#endif

int hex_digit(char c)
{
  int i = -1;
  switch(c)
  {
    case '0': i = 0; break;
    case '1': i = 1; break;
    case '2': i = 2; break;
    case '3': i = 3; break;
    case '4': i = 4; break;
    case '5': i = 5; break;
    case '6': i = 6; break;
    case '7': i = 7; break;
    case '8': i = 8; break;
    case '9': i = 9; break;
    case 'A': i = 10; break;
    case 'B': i = 11; break;
    case 'C': i = 12; break;
    case 'D': i = 13; break;
    case 'E': i = 14; break;
    case 'F': i = 15; break;
  }
  return i;
}

int hex_parse(char *s)
{
  int i,j,m,a,w,h;
#ifdef DEBUG
  printf("HEX%s",s);
#endif
  if(*s!=':') return 0;
  i = 1;
  h = hex_digit(s[i++]);
  if(h<0) return 0;
  m = h<<4;
  h = hex_digit(s[i++]);
  if(h<0) return 0;
  m |= h;
  h = hex_digit(s[i++]);
  if(h<0) return 0;
  a = h<<12;
  h = hex_digit(s[i++]);
  if(h<0) return 0;
  a |= h<<8;
  h = hex_digit(s[i++]);
  if(h<0) return 0;
  a |= h<<4;
  h = hex_digit(s[i++]);
  if(h<0) return 0;
  a |= h;
  if(m%2) return 0;
  m >>= 1;
  if(a%2) return 0;
  a >>= 1;
  if(s[i++]!='0') return 1;
  if(s[i++]!='0') return 1;
#ifdef DEBUG
  printf("\t%i word%c from address #%4.4X\n",m,(m==1)?' ':'s',a);
#endif
  for(j=0;j<m;j++)
  {
     h = hex_digit(s[i++]);
     if(h<0) return 0;
     w = h<<4;
     h = hex_digit(s[i++]);
     if(h<0) return 0;
     w |= h;
     h = hex_digit(s[i++]);
     if(h<0) return 0;
     w |= h<<12;
     h = hex_digit(s[i++]);
     if(h<0) return 0;
     w |= h<<8;
     if(a < MAXPROG)
     {
#ifdef DEBUG
        printf("\t[%4.4X]=%4.4X\n",a,w);
#endif
        prog[a] = w;
     }
     a++;
  }
  // TODO: checksum
  return 1;
}

unsigned long clock_ms(void)
{
#ifdef WINDA
  return (unsigned long)(1000.0*clock()/CLOCKS_PER_SEC);
#else
  return 1000LL*clock()/CLOCKS_PER_SEC;
#endif
}

void delay_ms(unsigned long ms)
{
  unsigned long t = clock_ms()+ms;
  while(clock_ms() < t);
}

#ifdef WINDA

HANDLE serial_handle = INVALID_HANDLE_VALUE; 

void write_win(const char *buf, int len)
{
    unsigned long result;
    if(serial_handle!=INVALID_HANDLE_VALUE)
    {
        WriteFile(serial_handle, buf, len?len:strlen(buf), &result, NULL);
    }
}

int read_win(char *buf, int len)
{
    unsigned long readn = 0;
    if(serial_handle!=INVALID_HANDLE_VALUE)
    {
        ReadFile(serial_handle, buf, len, &readn, NULL);
    }
    return (int)readn;
}

#endif

int com_init(int port)
{
  char sdev[32];
#ifdef WINDA
  char str[100],*po;
  int i,j;
  int errs = 0;
  DCB  dcb;
  COMMTIMEOUTS cto = { 0, 0, 0, 0, 0 };
#endif
  if(port>=0) sprintf(sdev,"%s%i",sdev_,port);
  else strcpy(sdev,sdev_);
  printf("Opening %s\n",sdev);
#ifdef WINDA
  memset(&dcb,0,sizeof(dcb));
  dcb.DCBlength       = sizeof(dcb);                   
  dcb.BaudRate        = 9600;
  dcb.Parity          = NOPARITY;
  dcb.fParity         = 0;
  dcb.StopBits        = ONESTOPBIT;
  dcb.ByteSize        = 8;
  dcb.fOutxCtsFlow    = 0;
  dcb.fOutxDsrFlow    = 0;
  dcb.fDtrControl     = DTR_CONTROL_DISABLE;
  dcb.fDsrSensitivity = 0;
  dcb.fRtsControl     = RTS_CONTROL_DISABLE;
  dcb.fOutX           = 0;
  dcb.fInX            = 0;
  dcb.fErrorChar      = 0;
  dcb.fBinary         = 1;
  dcb.fNull           = 0;
  dcb.fAbortOnError   = 0;
  dcb.wReserved       = 0;
  dcb.XonLim          = 2;
  dcb.XoffLim         = 4;
  dcb.XonChar         = 0x13;
  dcb.XoffChar        = 0x19;
  dcb.EvtChar         = 0;
  serial_handle       = CreateFile(sdev, GENERIC_READ | GENERIC_WRITE,
                                   0, NULL, OPEN_EXISTING, NULL, NULL);
  if(serial_handle==INVALID_HANDLE_VALUE) errs++;
  else
  {
        if(!SetCommMask(serial_handle, 0)) errs++;
        if(!SetCommTimeouts(serial_handle,&cto)) errs++;
        if(!SetCommState(serial_handle,&dcb)) errs++;
  }
  if(errs)
  {
        if(serial_handle!=INVALID_HANDLE_VALUE)
        {
           CloseHandle(serial_handle);
           serial_handle = INVALID_HANDLE_VALUE;
        }
        printf("Invalid port... (%i)\n",errs);
        return 0;
  }
#else
  fd = open(sdev, O_RDWR);
  if(fd < 0) return 0;
  struct termios tc;
  tcgetattr(fd, &tc);
  tc.c_iflag = 0;
  tc.c_oflag = 0;
  tc.c_cflag = CS8 | CREAD | CLOCAL;
  tc.c_lflag = 0;
  cfsetispeed(&tc, B9600);
  cfsetospeed(&tc, B9600);
  tcsetattr(fd, TCSANOW, &tc);
#endif
  return 1;
}

char* com_send(const char *command, unsigned long timeout)
{
  int r,i=-1;
#ifdef FAST_TRICK
  int j;
#endif
  static char buf[100];
  char c=0,*po,*poo;
  unsigned long t1;
#ifdef DEBUG
  printf("SEND '%s'\n",command);
#endif
  while(command[++i])
  {
#ifdef WINDA
    write_win(&command[i],1);
#else
    write(fd,&command[i],1);
#endif
    t1 = clock_ms();
    while(clock_ms()-t1 < timeout)
    {
#ifdef WINDA
      r = read_win(&c,1);
#else
      r = read(fd,&c,1);
#endif
      if(r > 0) break;
    }
//    printf("%c and %c 0x%2.2X (%i) t1=%lu %lu\n",command[i],c,c,r,t1,clock_ms());
    if(c!=command[i]) return NULL;
  }
  c = '\r';
#ifdef WINDA
  write_win(&c,1);
#else
  write(fd,&c,1);
#endif
  i = 0;
  *buf = 0;
  t1 = clock_ms();
  while(clock_ms()-t1 < timeout)
  {
#ifdef WINDA
    r = read_win(&buf[i],1);
#else
    r = read(fd,&buf[i],100-i);
#endif
    if(r > 0)
    {
      buf[i+r] = 0;
      i += r;
    }
#ifdef FAST_TRICK
    for(j=0;j<i;j++)
      if(buf[j]=='>') break;
    if(j!=i) break;
#endif
  }
  if(*buf)
  {
//    printf("%2.2X %2.2X %2.2X %2.2X %2.2X\n",buf[0],buf[1],buf[2],buf[3],buf[4]);
    po = buf;
    while(*po=='\n'||*po=='\r') po++;
    poo = strchr(po,'\n');
    if(poo!=NULL) *poo = 0;
    poo = strchr(po,'\r');
    if(poo!=NULL) *poo = 0;
  }
  else po = NULL;
#ifdef DEBUG
  printf("RECEIVED: '%s'\n",po);
#endif
  return po;
}

#ifndef NOMAIN

int main(int argc,char **argv)
{
  int i,r,w,ok,ver=0,p=0,k=0;
  char *rs,st[100],*po;
  FILE *f;
#ifdef WINDA
  p++; /* COM1 for Windows is a default */
#endif
  printf("\npdbl1hex v1.2 sends HEX-file to the PDBLv1 device\n\n");
  if(argc < 2)
  {
      printf("Usage:\n\tpdbl1hex filename.hex [N|DEVICE]\n\n");
      return -1;
  }
  if(argc > 2)
  {
      if(isdigit(argv[2][0])) p = atoi(argv[2]);
      else { p=-1; strncpy(sdev_,argv[2],32); }
  }
  prog = (short*)malloc(MAXPROG*sizeof(short));
  if(prog==NULL)
  {
      printf("\nERROR: Out of memory\n");
      return -2;
  }
  for(i=0;i<MAXPROG;i++) prog[i] = -1;
  f = fopen(argv[1],"rt");
  if(f==NULL)
  {
      printf("\nERROR: Can't open HEX file\n");
      return -3;
  }
  printf("HEX '%s'...\n",argv[1]);
  while(1)
  {
      fgets(st,100,f);
      if(feof(f)) break;
      if(!hex_parse(st))
      {
         printf("\nERROR: Invalid HEX file\n");
         fclose(f);
         free(prog);
         return -4;
      }
  }
  printf("HEX Ok\n");
  fclose(f);
  if(!com_init(p))
  {
      printf("\nERROR: Can't initialize serial port\n");
      free(prog);
      return -5;
  }
  rs = com_send("",1000);
  if(!strncmp(rs,"PDBLv",5))
  {
      ver = rs[5]-'0';
  }
  if(ver!=1)
  {
      printf("\nERROR: Can't detect PDBLv1 device\n");
      free(prog);
      return -6;
  }
  printf("PDBLv1 device detected\n");
  printf("Programming...\n");
  for(i=0;i<MAXPROG;i++)
  {
      if(prog[i] > 0)
      {
         k++;
         sprintf(st,"!%4.4X=%4.4X",i,prog[i]);
         rs = com_send(st,100);
         if(rs==NULL)
         {
            printf("\nERROR: Unexpected failure\n");
            free(prog);
            return -7;
         }
         if(strcmp(rs,"OK"))
         {
            printf("\nERROR: Failed programming at #%4.4X\n",i);
            free(prog);
            return -8;
         }
#ifdef DEBUG
         printf("%s -> %s\n",st,rs);
#else
         printf("o");fflush(stdout);
#endif
      }
  }
  printf("\nVerification...\n");
  for(i=0;i<MAXPROG;i++)
  {
      if(prog[i] > 0)
      {
         if(i==0) sprintf(st,"?0001");
         else sprintf(st,"?%4.4X",i);
         rs = com_send(st,100);
         if(rs==NULL)
         {
            printf("\nERROR: Unexpected failure\n");
            free(prog);
            return -9;
         }
         ok = 0;
         r = hex_digit(rs[0]);
         if(r >= 0)
         {
            w = r<<12;
            r = hex_digit(rs[1]);
            if(r >= 0)
            {
               w |= r<<8;
               r = hex_digit(rs[2]);
               if(r >= 0)
               {
                  w |= r<<4;
                  r = hex_digit(rs[3]);
                  if(r >= 0)
                  {
                     w |= r;
                     if(w==prog[i]) ok = 1;
                  }
               }
            }
         }
         if(!ok)
         {
            printf("\nERROR: Failed verification at #%4.4X (#%3.3X != #%3.3X)\n",i,w,prog[i]);
            free(prog);
            return -10;
         }
#ifdef DEBUG
         printf("%s -> %s\n",st,rs);
#else
         printf("o");fflush(stdout);
#endif
      }
  }
  printf("\nProgram OK (%i words)\n",k);
  free(prog);
#ifdef WINDA
  if(serial_handle!=INVALID_HANDLE_VALUE) CloseHandle(serial_handle);
#else
  if(fd>=0) close(fd);
#endif
  return 0;
}

#endif
