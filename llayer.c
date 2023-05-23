#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <termios.h>
#include <stdio.h>
#include <strings.h>

#include "llayer.h"

#include <unistd.h>
#include <signal.h>

#define BAUDRATE B38400



#define _POSIX_SOURCE 1 
#define FALSE 0
#define TRUE 1


struct termios oldtio;
struct termios newtio;

volatile int STOP=FALSE;
int ns; 
int countF=0, countI=0;







int sendF(int fd, int transreceive, unsigned char m);
unsigned char readF(int fd, int transreceive);
int readI(int fd, unsigned char *buff, int ns);
int sendI(int fd, char *buff);

int llwrite( char buff[], int fd, int tr){
    
    int c2 = 4; 
    int b=0; 
    unsigned char r;

    int nsNew = (ns)?0:1; int nsOld = ns;


    for (int i = 0; i < c2; i++)
    {
        
        b= sendI(fd, buff);
        if (b<0) 
        {
            printf("error2\n"); continue;
        } 

        r= readF(fd, tr); 
        

        if (r==CTRL_RR(nsNew))
        {
           ns=nsNew; return b;
        }
        else if (r==CTRL_REJ(nsOld))
        {
            i=0; continue;
        }
        else if (r==0) continue;
        
        
        
    }
    printf("too many tries\n"); return -1;

    //new^


}


int llread( char buff[] , int fd){

    int b = readI(fd, buff, ns);
    


    if (b<0)
    {
        switch (b)
        {
        case -6:
            sendF(fd, 1, CTRL_RR(ns));
            break;

        case -5:
            sendF(fd, 1, CTRL_REJ(ns));
            break;
        
        default:
            break;
        }
    }
    else{
            ns=(ns)?0:1; sendF(fd, 1, CTRL_RR(ns));
    }
    return b;


}


int llclose(int fd, char port[], int tr){
    int c1=0, c2=6; unsigned char d;
    if (tr == 1)
    {
        while (c2>c1)
        {
            sendF(fd, 1, CTRL_DC);
            d=readF(fd,1);
            if (d==CTRL_DC);
            {
                sendF(fd, 1, CTRL_UA); break;
            }
        }
        
    }
    else if (tr==2)
    {
        while (c2>c1)
        {
            d=readF(fd,1);
            if (d==CTRL_DC)
            {
                sendF(fd, 1, CTRL_DC); break;
            }
        }
    }
    else {
        printf("tr error\n"); return -1; 
    }
    
    tcsetattr(fd, TCSANOW, &oldtio);
    close(fd); return 0;

    //^



}

int llopen(int fd, char port[], int tr){ printf("llopen start - %d\n", tr);
    int c1=0, c2=6; //ntransm 
    int count=0;
    if( tr!=1) 
    {
        if(tr!=2) printf("error -> tr"); }

    
    char buff[255]; int res;

    fd = open( port , O_RDWR | O_NOCTTY);
    if(fd<0){ printf("error");  return -1;}

    struct termios oldtio;
    struct termios newtio; 

    if ( tcgetattr(fd,&oldtio) == -1) { 
        printf("tcgetattr");
        exit(-1);
    }
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;


   
        newtio.c_cc[VTIME]=0;   newtio.c_cc[VMIN]=5;
        //newtio.c_cc[VTIME]=1;   newtio.c_cc[VMIN]=0;
    


    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
        printf("tcsetattr");
        exit(-1);
    }

    //new
    //if(tr==1) {printf("Start typing: \n"); gets(buff); printf("stop\n");}
    for (int i = 0; i < 255; i++) {
        buff[i] = 'a';
    }

    ns=1;
    // printf("\n here \n");
    if(tr==1) //t
    {
        while (c2>c1)
        {
            sendF(fd,2,CTRL_SET);
            if (readF(fd,1)==CTRL_UA)
            {
                printf("readF return 1");
                return 1;
            }
            c1++;
        } 
        printf("error9"); return -9;
    }
    else if(tr==2) //r
    {
        while (readF(fd,1)!=CTRL_SET)
        {
            
            printf("tr=2 readF count->%d", count); count++;
        }
        sendF(fd, 1, CTRL_UA);
        
        
    }
    else 
    {  printf("error -> tr"); return -1; }

    return 0;

}


unsigned char readF(int fd, int transreceive)
{   printf("readF starts\n");
    int Start = 0, flagRcv = 1, aRvc=2, cRvc=3, bccOk=4, end=5;

    int st; unsigned char buff;
    unsigned char add, ctrl; int b;

    if(transreceive==1) add=0x03;
    else if(transreceive==2) add=0x01;
    else return -1;
    st=0;
    while (st != end)
    {
        printf("\nhere1\n");
        b = read(fd, &buff, 1);
        if(b==0)
        {
            printf("b==0\n");
        }
        if(b < 0) {
            printf("\nhere\n");
            return 0;
        }
        switch (st)
        {
        case 0: //Start
                printf("case start\n");
                if (buff == FLAG)
                {
                    st = flagRcv;
                }
                break;
        case 1: //flag
            printf("case flag\n");
            if (buff==add)
            {
                st = aRvc;
            }
            else if (buff==FLAG) st=flagRcv;
            else st=Start;

            break;


        case 2: //arvc
            if(buff == FLAG)  st=flagRcv;
            else{ st=cRvc;  ctrl=buff;  }
            break;
        
        case 3: //crvc
            if(buff==(add^ctrl)) st=bccOk;
            else if(buff==FLAG) st = flagRcv;
            else st=Start;
            
            break;

        case 4: //bcc
            if(buff==FLAG) st = end;
            else st=Start;
            
            break;
        
        default:
            break;
        }
    }
    

    return ctrl;
}

int sendF(int fd, int transreceive, unsigned char m){ printf("sendF starts\n");
    unsigned char f[5], add;

    if(transreceive==1) add=0x03;
    else if(transreceive==2) add=0x01;
    else return -1;
    
    unsigned char bcc; bcc = (add^m);

    f[0]=FLAG;
    f[1]=add;
    f[2]=m;
    f[3]=bcc;
    f[4]=FLAG;

    countF++; return write(fd,f,5);
}

int readI(int fd, unsigned char *buff, int ns){
    int Start =0, flagRcv = 1, aRvc=2, cRvc=3, bccOk=4, end=5 , bcc2Ok=6;
    int st = Start; int b;

    int oldS=6;
    unsigned char *ar; ar = malloc(oldS * sizeof(unsigned char));
    int arC=0;
    while (st!= end)
    {
        b = read(fd,&buff,1);
        if(b==0)
        {
            printf("readI -> read function returned 0\n");
            continue;
        }
        else if (b<0)
        {
            free(ar);
            return -1;
        }
        
        ar[arC++]=buff;

        switch (st)
        {
        case 0: //Start
            if (buff == FLAG)
            {
                st = flagRcv;
            }
            break;
        case 1: //flag
            if (buff==FLAG)
            {
                st = end;
            }
            break;
        
        default:
            break;
        }
    }


    unsigned char b1; int ff=0,  newS;
    for (int f = 1; f < arC-1; f++)
    {
        b1=ar[f];
        if (b1 == 0x7D)
        {
            for (int m = f; m < arC; m++)
            {
                ar[m]=ar[m+1];
            }
            ar[f] = ar[f]^(0x20);

            oldS--;
            ar = realloc(ar,oldS*sizeof(unsigned char));
            arC--;
        
            ff++;
            //unfin
        }

        
    }
    
    unsigned char bcc2;
    for (int j = 4; j < arC-2 ; j++) bcc2= bcc2 ^ ar[j];
    
    
    st = Start;
    int i=0; int add = 0x01;
    int nsN = ns ? 0 : 1;
    unsigned char ctr1 = SEQNUM_TO_CONTROL(ns)  ,  ctr2 = SEQNUM_TO_CONTROL(nsN);
    
    while (st!=end )
    {
        switch (st)
        {
        case 0: //Start
            if (ar[i]==FLAG)
            {
                st = flagRcv;
            } else return -3;
            break;
        case 1: //flag
            if (ar[i]==add)
            {
                st = aRvc;
            }
            else return -3;

            break;


        case 2: //arvc
            if(ar[i]==ctr1)  st=cRvc;
            else if (ar[i]==ctr2) return -6;
            else return -2;
          
            
            break;
        
        case 3: //crvc
            if(ar[i]==(add ^ ctr1)) st=bccOk;
            else if (ar[i]==ctr2) return -6;
            else return -2;
            
            break;

        case 4: //bcc
            if(i==arC-2)
            {
                if (ar[i]=bcc2)
                {
                    st=bcc2Ok;
                }
                else return -5;
                
            }
            
            break;
        case 6: //bcc 2
            if(ar[i]=FLAG)
            {
                st=end;
    
            }
            else return -4;
            break;
        default:
            break;
        }

        i++;
    }

    int k=0;
    unsigned char *ar1; ar1 = malloc((arC +1) * sizeof(unsigned char));
    for (i = 4; i < arC-2; i++)
    {
        ar1[k]=ar[i];  
        k++;
    }
    free(ar);
    return k;

}

int sendI(int fd, char *buff)
{
    int lenght = 4, oldS=6;

    unsigned char ctrl = SEQNUM_TO_CONTROL(ns);
    unsigned char bcc=0x03^ctrl;
    unsigned char bcc2=0;

    unsigned char *ar1; ar1 = malloc(oldS * sizeof(unsigned char));
    int ar1C=0;

    ar1[ar1C]=FLAG; ar1C++;
    ar1[ar1C]=0x03; ar1C++;
    ar1[ar1C]=ctrl; ar1C++;
    ar1[ar1C]=bcc; ar1C++;

    for (int i = 0; i < lenght; i++)
    {
        ar1[ar1C]=buff[i]; ar1C++;
        bcc2 = bcc2^buff[i];
    }
    
    ar1[ar1C]=bcc2; ar1C++;

    unsigned char b1; int bb=0;
    for (int f = 1; f < ar1C; f++)
    {
        b1=ar1[f];
        if (b1 == 0x7D || b1 == FLAG)
        {
            oldS++;
            ar1 = realloc(ar1,oldS*sizeof(unsigned char));
            ar1C++;


            for (int i1  = ar1C; i1 > f+1; i1--)
            {
                ar1[i1] = ar1[i1-1];
            }
            
            ar1[f]=0x7D; ar1[f+1]=b1 ^ 0x20;

        
            bb++;
            //unfi
        }
    }
    

    ar1[ar1C]=FLAG; ar1C++;

    int r=write(fd,ar1,ar1C); 
    countI++;
    return r;

}