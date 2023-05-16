#define TOUT 5

#define CTRL_SET 0x03
#define CTRL_DC  0x0B
#define CTRL_UA  0x07


#define CTRL_RR(n) ((n << 7) | 0x05)
#define CTRL_REJ(n) ((n << 7) | 0x01)


#define FLAG 0x5C


int llwrite( char buff[], int fd, int tr);
int llread( char buff[], int fd);
int llclose(int fd, char port[], int tr);
int llopen(char port[], int tr);