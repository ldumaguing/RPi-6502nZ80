#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include<sys/ioctl.h>

void export_Addresses ( int * );
void set_direction_Addresses ( int * );
void unexport_Addresses ( int * );

void loadROM ( char * );

int get_current_address ( int * );
int get_RW ( int * );
void printDBus();

void mode_Read ( char *, char *, int, int );
void mode_Write ( char *, int );

int isAnalyzer = 0;

// **************************************************************************************
int main ( void ) {
    int GPIOs[17] = {
        22, 27, 17,  4, 14, 15, 18, 23,  // a15 to a8
        24, 25,  8,  7, 12, 16, 20, 21,  // a7 to a0
        19                               // RW
    };

    int RAM_size = 32768;    // 0000 to 7fff
    char RAM[RAM_size];

    int ROM_size = 32768;    // 8000 to ffff
    char ROM[ROM_size];
    int ROM_start_addr = 65536 - ROM_size;
    loadROM ( ROM );

    struct termios orig_term, raw_term;
    tcgetattr ( STDIN_FILENO, &orig_term );
    raw_term = orig_term;
    raw_term.c_lflag &= ~ ( ECHO | ICANON );
    raw_term.c_cc[VMIN] = 0;
    raw_term.c_cc[VTIME] = 0;
    tcsetattr ( STDIN_FILENO, TCSANOW, &raw_term );








    export_Addresses ( GPIOs );
    set_direction_Addresses ( GPIOs );

    printf ( "Press [ESC] to quit.\n" );

    char ch = 0;
    int counter = 0;
    int address = 0;
    do {
        read ( STDIN_FILENO, &ch, 1 );









// ********************
        usleep ( 5000 );

        // *****
        printf ( "%6d ", counter );
        counter++;


        if ( get_RW ( GPIOs ) ) {
            // ***** Read
            address = get_current_address ( GPIOs );
            mode_Read ( RAM, ROM, ROM_start_addr, address );
        } else {
            // ***** Write
            address = get_current_address ( GPIOs );
            mode_Write ( RAM, address );
        }
        //printDBus();
// ********************






        printf ( "\n" );
    } while ( ch != 27 );




    unexport_Addresses ( GPIOs );













    while ( read ( STDIN_FILENO, &ch, 1 ) ==1 );
    tcsetattr ( STDIN_FILENO, TCSANOW, &orig_term );

    return 0;
}

// **************************************************************************************
void mode_Read ( char *ram, char *rom, int romAddr0, int currAddr ) {
    int file;
    if ( ( file=open ( "/dev/i2c-1", O_RDWR ) ) < 0 ) {
        perror ( "failed to open the bus\n" );
        return;
    }
    if ( ioctl ( file, I2C_SLAVE, 0x38 ) < 0 ) {
        perror ( "Failed to connect to the sensor\n" );
        return;
    }




    char writeBuffer[1];
    if ( currAddr < 32768 ) { // RAM
        printf ( " RAM: %02x", ram[currAddr] );
        writeBuffer[0] = ( char ) rom[currAddr];
        if ( write ( file, writeBuffer, 1 ) != 1 )
            perror ( "Failed to write to PCF\n" );
    } else {                  // ROM
        printf ( " ROM: %02x", rom[currAddr - romAddr0] );
        writeBuffer[0] = ( char ) rom[currAddr - romAddr0];
        if ( write ( file, writeBuffer, 1 ) != 1 )
            perror ( "Failed to write to PCF\n" );
    }





    close ( file );
}

void mode_Write ( char *ram, int currAddr ) {
    int file;
    if ( ( file=open ( "/dev/i2c-1", O_RDWR ) ) < 0 ) {
        perror ( "failed to open the bus\n" );
        return;
    }
    if ( ioctl ( file, I2C_SLAVE, 0x38 ) < 0 ) {
        perror ( "Failed to connect to the sensor\n" );
        return;
    }

    printf ( "x\n" );



    char buf[5];
    if ( read ( file, buf, 5 ) != 5 ) {
        perror ( "Failed to read in the buffer\n" );
        return;
    }

    if ( currAddr < 32768 ) {

        ram[currAddr] = buf[0];
        printf ( ".%02x.%02x.%02x", currAddr, ram[currAddr], buf[0] );
    }







    close ( file );
}

void loadROM ( char *rom ) {
    FILE *fp;
    fp = fopen ( "rom.bin", "rb" );
    fread ( rom, 32768, 1, fp );
    fclose ( fp );
}

// **************************************************************************************
int get_current_address ( int *GPIOs ) {
    char value_str[3];
    int fd;
    char buff[256];

    int address = 0;
    for ( int i = 0; i < 16; i++ ) {
        sprintf ( buff, "/sys/class/gpio/gpio%d/value", GPIOs[i] );
        fd = open ( buff, O_RDONLY );
        if ( -1 == fd ) {
            fprintf ( stderr, "Failed to open gpio value for reading!\n" );
            exit ( 1 );
        }
        if ( -1 == read ( fd, value_str, 3 ) ) {
            fprintf ( stderr, "Failed to read value!\n" );
            exit ( 1 );
        }
        address = ( address << 1 ) + atoi ( value_str );

        close ( fd );
    }
    printf ( "%04x", address );

    return address;
}

int get_RW ( int *GPIOs ) {
    char value_str[3];
    int fd;
    char buff[256];

    sprintf ( buff, "/sys/class/gpio/gpio%d/value", GPIOs[16] );
    fd = open ( buff, O_RDONLY );
    if ( -1 == fd ) {
        fprintf ( stderr, "Failed to open gpio value for reading!\n" );
        exit ( 1 );
    }
    if ( -1 == read ( fd, value_str, 3 ) ) {
        fprintf ( stderr, "Failed to read value!\n" );
        exit ( 1 );
    }
    printf ( ": %s", atoi ( value_str ) ? "r" : "W" );
    close ( fd );
    return ( atoi ( value_str ) ? 1 : 0 );
}

void printDBus() {
    int file;

    if ( ( file=open ( "/dev/i2c-1", O_RDWR ) ) < 0 ) {
        perror ( "failed to open the bus\n" );
        exit ( 1 );
    }
    if ( ioctl ( file, I2C_SLAVE, 0x38 ) < 0 ) {
        perror ( "Failed to connect to the sensor\n" );
        exit ( 1 );
    }

    int BUFFER_SIZE = 5;
    char buf[BUFFER_SIZE];
    if ( read ( file, buf, BUFFER_SIZE ) != BUFFER_SIZE ) {
        perror ( "Failed to read in the buffer\n" );
        exit ( 1 );
    }

    printf ( " %02x", buf[0] );

    close ( file );
}




// **************************************************************************************
void export_Addresses ( int *GPIOs ) {
    char buff[256];
    sprintf ( buff, "/sys/class/gpio/gpio%d", GPIOs[0] );
    FILE *fptr = fopen ( buff, "r" );
    if ( fptr ) {
        fclose ( fptr );
        isAnalyzer = 1;
        return;
    }

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

void set_direction_Addresses ( int *GPIOs ) {
    if ( isAnalyzer ) return;

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
    if ( isAnalyzer ) return;

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













