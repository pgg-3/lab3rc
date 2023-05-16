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
int tr; int ns; 
int countF=0;







int sendF(int fd, int transreceive, unsigned char m);
unsigned char readF(int fd, int transreceive);
int readI(int fd, char *buff, int ns);
int sendI(int fd, char *buff);

int llwrite( char buff[], int fd, int tr){
    
    int c2 = 4; 
    int b=0; 
    unsigned char r;

    int nsNew = (ns)?0:1; int nsOld = ns;


    for (int i = 0; i < c2; i++)
    {
        /*
        b= sendI(fd, buff ,nsOld);
        if (b<0) 
        {
            printf("error2\n"); continue;
        } */

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

    //int b = readI(fd, buff, ns);
    int b=1;


    if (b<0)
    {
        switch (b)
        {
        case -3:
            sendF(fd, 1, CTRL_RR(ns));
            break;

        case -4:
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
    int c1=0, c2=4; unsigned char d;
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

int llopen(char port[], int tr2){ 
    int c1=0, c2=4; //ntransm
    if( tr2!=1) 
    {
        if(tr2!=2) printf("error -> tr"); }

    int tr=tr2;
    char buff[255]; int res;

    int fd=open( port , O_RDWR | O_NOCTTY);
    if(fd<0){ printf("error");  return -1;}

    struct termios oldtio;
    struct termios newtio; 

    if ( tcgetattr(fd,&oldtio) == -1) { 
        perror("tcgetattr");
        exit(-1);
    }
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;


    if(tr==1) //trans
    {
        newtio.c_cc[VTIME]=0;   newtio.c_cc[VMIN]=5;
    }
    else if(tr==2) //receive
    {
        newtio.c_cc[VTIME]=0;   newtio.c_cc[VMIN]=5;
    }
    else{
        newtio.c_cc[VTIME]=0;   newtio.c_cc[VMIN]=5;
    }
    


    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    //new

    ns=1;

    if(tr==1) //t
    {
        while (c2>c1)
        {
            sendF(fd,2,CTRL_SET);
            if (readF(fd,1)==CTRL_UA)
            {
                return 1;
            }
            
        } 
        return -9;
    }
    else if(tr==2) //r
    {
        while (readF(fd,1)!=CTRL_SET)
        {
            
        }
        sendF(fd, 1, CTRL_UA);
        
        
    }
    else 
    {  printf("error -> tr"); return -1; }

    return 0;

}


unsigned char readF(int fd, int transreceive)
{
    int Start =0, flagRcv = 1, aRvc=2, cRvc=3, bccOk=4, end=5;

    int st = Start; unsigned char buff;
    unsigned char add, ctrl=0; int b;

    if(transreceive==1) add=0x03;
    else if(transreceive==2) add=0x01;
    else return -1;
    
    while (st != end)
    {
        b = read(fd, &buff, 1);
        if(b < 1) {
            return 0;
        }
        switch (st)
        {
        case 0: //Start
            if (buff== FLAG)
            {
                st = flagRcv;
            }
            break;
        case 1: //flag
            if (buff==add)
            {
                st = aRvc;
            }
            else if (buff==FLAG) st=flagRcv;
            else st=Start;

            break;


        case 2: //arvc
            if(buff==FLAG)  st=flagRcv;
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

int sendF(int fd, int transreceive, unsigned char m){
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

int readI(int fd, char *buff, int ns){

}

int sendI(int fd, char *buff)
{

}