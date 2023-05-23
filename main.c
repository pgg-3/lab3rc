#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "llayer.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 
#define FALSE 0
#define TRUE 1



int main(int argc, char** argv)
{
    int fd, tr=0;
/*
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if(fd<0) return -1;
*/
    char *port = argv[1];
    if (strcmp(argv[2],"1")==0) {
        tr=1;
    }
    else if (strcmp(argv[2],"2")==0) {
        tr=2;
    }
    else 
    {
        printf("tr error\n");
        return -1;
    }
    int llo =llopen(fd, port, tr);
    if(llo<0); 
    {
        printf("llopen error - %d\n",llo);
        return -1;
    }

    if(llclose(fd, port, tr)<0); 
    {
        printf("llclose error\n");
        return -1;
    }
    return 0;
}
