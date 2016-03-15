#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*

 This checker is only for older preliminary version of NEDONAND-4 board!

 Input:
 BBBBAAAACOOO

 Output:
 DDDDVC

*/

int main(int argc, char** argv)
{
 char n[100],s[256],u[8];
 int i,j,r[5],v,c;
 FILE *f;
 signed char a,b,d;

 if(argc<2) return -1;
 strcpy(n,argv[1]);
 f = fopen(n,"rt");
 if(f==NULL) return -1;
 fscanf(f,"%s\n",s);
 while(!feof(f))
 {
   fscanf(f,"%3X:%2X,%2X,%2X,%2X,%2X\n",&i,&r[0],&r[1],&r[2],&r[3],&r[4]);
//   printf("0x%4.4X:",i);
   v = -1;
   a = (i>>4)&15;
   b = (i>>8)&15;
   switch(i&7)
   {
     case 0:
        strcpy(u,"RRC");
        d = (i&8)|(a>>1);
        c = a&1;
        break;
     case 1:
        strcpy(u,"RLC");
        d = ((a&7)<<1)|((i&8)>>3);
        c = (a&8)>>3;
        break;
     case 2:
        strcpy(u,"NAN");
        d = (~(a&b))&15;
        c = 1;
        break;
     case 3:
        strcpy(u,"XOR");
        d = a^b;
        c = 1;
        break;
     case 4:
     case 5:
     case 6:
     case 7:
        strcpy(u,"ADD");
        d = a + b + ((i&8)>>3);
        c = (d&16)>>4;
        if(a&8) a|=0xF0;
        if(b&8) b|=0xF0;
        d = a + b + ((i&8)>>3);
        if((d<0 && !(d&8)) || (d>0 && (d&8)))
           v = 1;
        else
           v = 0;
        break;
   }
   for(j=0;j<5;j++)
   {
//      printf(" 0x%2.2X",r[j]);
     if((d&15)!=(r[j]>>2) ||
        c!=(r[j]&1) ||
        (v>=0 && ((r[j]&2)>>1)!=v))
     {
       printf("%s %3.3X[%i] %2.2X a=%i b=%i c=%i -> d=%i (%i) c=%i (%i)",u,i,j,r[j],a,b,(i&8)>>3,d,r[j]>>2,c,r[j]&1);
       if(v>=0) printf(" v=%i (%i)",v,(r[j]&2)>>1);
       printf("\n");
     }
   }
//   printf("\n");
 }

 fclose(f);
 return 0;
}
