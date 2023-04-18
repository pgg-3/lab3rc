#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>


#define BAUDRATE B38400



#define _POSIX_SOURCE 1 
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;


int llopen(char port[]){
    char buff[255]; int res;
    int fd=open(port,O_RDWR | O_NOCTTY);
    if(fd<0){ printf("error");  exit(1);}

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

    newtio.c_cc[VTIME]=0;  
    newtio.c_cc[VMIN]=5;

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("Start typing: \n"); gets(buff);

    int k = write(fd,buff,sizeof(buff)); printf("%d bytes written\n", k);
    while (STOP==FALSE) {       
        res = read(fd,buff,sizeof(buff));   
        buff[res]=0;               
        printf(":%s:%d\n", buff, res);
        
        if (buff[0]=='z') STOP=TRUE;
    }
    
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }


    close(fd);
    return 0;
}

int main(int argc, char** argv){
    int fd;
    if(argc<2) exit(1);

    llopen(argv[1]);

    return 0;
}