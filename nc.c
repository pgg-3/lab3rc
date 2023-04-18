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


int llclose(char port[]){
    struct termios oldtio,newtio; int res;
    char buff[255];
    int fd = open(port, O_RDWR | O_NOCTTY );
    if(fd<0){ printf("error");  exit(1);}


    if (tcgetattr(fd,&oldtio) == -1) {
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   
    newtio.c_cc[VMIN]     = 5;   

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    while (STOP==FALSE) {       
        res = read(fd,buff,sizeof(buff));   
        buff[res]=0;               
        printf(":%s:%d\n", buff, res);
        int k = write(fd,buff,sizeof(buff)); printf("%d bytes written\n", k);
        if (buff[0]=='z') STOP=TRUE;
    }



    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}

int main(int argc, char** argv){

    if (argc<2) {exit(1);} 

    llclose(argv[1]);
    return 0;

}