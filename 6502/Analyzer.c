#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include <termios.h>
#include<sys/ioctl.h>

void export_Addresses ( int * );
void set_direction ( int * );
void unexport_Addresses ( int * );
int printDBus();

// **************************************************************************************
int main ( void ) {
    struct termios orig_term, raw_term;
    tcgetattr ( STDIN_FILENO, &orig_term );
    raw_term = orig_term;
    raw_term.c_lflag &= ~ ( ECHO | ICANON );
    raw_term.c_cc[VMIN] = 0;
    raw_term.c_cc[VTIME] = 0;
    tcsetattr ( STDIN_FILENO, TCSANOW, &raw_term );

    int GPIOs[17] = {
        // 15  14  13  12  11  10   9   8
           22, 27, 17,  4, 14, 15, 18, 23,
        //  7   6   5   4   3   2   1   0
           24, 25,  8,  7, 12, 16, 20, 21,
        // RW
        19
    };



    export_Addresses ( GPIOs );
    set_direction ( GPIOs );
    printf ( "Press [ESC] to quit.\n" );

    char ch = 0;
    do {
        int len = read ( STDIN_FILENO, &ch, 1 );
        if ( len == 1 ) {
            printf ( "You pressed char 0x%02x : %c\n", ch,
                     ( ch >= 32 && ch < 127 ) ? ch : ' ' );
        }

        char value_str[3];
        int fd;
        char buff[256];



        int address = 0;
        for ( int i = 0; i < 16; i++ ) {
            sprintf ( buff, "/sys/class/gpio/gpio%d/value", GPIOs[i] );
            fd = open ( buff, O_RDONLY );
            if ( -1 == fd ) {
                fprintf ( stderr, "Failed to open gpio value for reading!\n" );
                return ( -1 );
            }
            if ( -1 == read ( fd, value_str, 3 ) ) {
                fprintf ( stderr, "Failed to read value!\n" );
                return ( -1 );
            }
            address = ( address << 1 ) + atoi ( value_str );

            close ( fd );
        }
        printf ( "%04x", address );

        sprintf ( buff, "/sys/class/gpio/gpio%d/value", GPIOs[16] );
        fd = open ( buff, O_RDONLY );
        if ( -1 == fd ) {
            fprintf ( stderr, "Failed to open gpio value for reading!\n" );
            return ( -1 );
        }
        if ( -1 == read ( fd, value_str, 3 ) ) {
            fprintf ( stderr, "Failed to read value!\n" );
            return ( -1 );
        }
        printf ( ": %s", atoi ( value_str ) ? "r" : "W" );
        close ( fd );

        if (printDBus() == -1) return -1;

        printf ( "\n" );
        usleep ( 5000 );
    } while ( ch != 27 );


    unexport_Addresses ( GPIOs );







    while ( read ( STDIN_FILENO, &ch, 1 ) ==1 );
    tcsetattr ( STDIN_FILENO, TCSANOW, &orig_term );

    return 0;
}

// **************************************************************************************
int printDBus() {
    int file;

    if((file=open("/dev/i2c-1", O_RDWR)) < 0) {
        perror("failed to open the bus\n");
        return -1;
    }
    if(ioctl(file, I2C_SLAVE, 0x38) < 0) {
        perror("Failed to connect to the sensor\n");
        return -1;
    }

    int BUFFER_SIZE = 5;
    char buf[BUFFER_SIZE];
    if(read(file, buf, BUFFER_SIZE)!=BUFFER_SIZE) {
        perror("Failed to read in the buffer\n");
        return -1;
    }

    printf(" %02x", buf[0]);

    close(file);
    return 0;
}

// **************************************************************************************
void export_Addresses ( int *GPIOs ) {
    int fd;
    printf ( "export_Addresses\n" );
    printf ( "   opening...\n" );

    fd = open ( "/sys/class/gpio/export", O_WRONLY );
    if ( fd == -1 ) {
        perror ( "Unable to open /sys/class/gpio/export" );
        exit ( 1 );
    }

    char snum[5];
    for ( int i=0; i < 17; i++ ) {
        sprintf ( snum, "%d", GPIOs[i] );
        if ( write ( fd, snum, 2 ) != 2 ) {
            perror ( "   Error writing to /sys/class/gpio/export" );
            exit ( 1 );
        }
    }

    close ( fd );
}

void set_direction ( int *GPIOs ) {
    printf ( "set_direction\n" );
    int fd;
    char buff[256];

    for ( int i=0; i < 17; i++ ) {
        sprintf ( buff, "/sys/class/gpio/gpio%d/direction", GPIOs[i] );
        fd = open ( buff, O_WRONLY );
        if ( write ( fd, "in", 3 ) != 3 ) {
            perror ( "Error" );
            exit ( 1 );
        }
        close ( fd );
    }
}

void unexport_Addresses ( int *GPIOs ) {
    printf ( "unexport_Addresses\n" );
    printf ( "   closing...\n" );
    int fd;

    fd = open ( "/sys/class/gpio/unexport", O_WRONLY );
    if ( fd == -1 ) {
        perror ( "Unable to open /sys/class/gpio/unexport" );
        exit ( 1 );
    }

    char snum[5];
    for ( int i=0; i < 17; i++ ) {
        sprintf ( snum, "%d", GPIOs[i] );
        if ( write ( fd, snum, 2 ) != 2 ) {
            perror ( "Error writing to /sys/class/gpio/unexport" );
            exit ( 1 );
        }
    }

    close ( fd );
}













