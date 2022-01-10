#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include <math.h>

int clk, fi2c, Address, BA, FullAddr;
int TicToc = 0;  // Tic = 1, Toc = 0
int Signals[7] = { 0, 0, 0, 0, 0, 0, 0};

void close_BAs ( int* );

// **************************************************************************************
void DataPhase ( int* gpios ) {
    char buff[256];
    int fd;
    for ( int i = 7; i < 15; i++ ) {
        sprintf ( buff, "/sys/class/gpio/gpio%d/direction", gpios[i] );
        fd = open ( buff, O_WRONLY );
        if ( fd == -1 ) {
            perror ( "DataPhase1 open error" );
            exit ( 1 );
        }
        if ( write ( fd, "out", 3 ) != 3 ) {
            perror ( "Error in address direction" );
            exit ( 1 );
        }
        close ( fd );
    }


    int DataPhase_fds[8];
    int ByteData[] = { 0, 1, 0, 1, 0, 1, 1, 1 };
    for ( int i = 7; i < 15; i++ ) {
        sprintf ( buff, "/sys/class/gpio/gpio%d/value", gpios[i] );
        DataPhase_fds[i - 7] = open ( buff, O_WRONLY );
        if ( DataPhase_fds[i - 7] == -1 ) {
            perror ( "DataPhase1 value error" );
            exit ( 1 );
        }
        sprintf ( buff, "%d", ByteData[i - 7] );
        if ( write ( DataPhase_fds[i - 7], buff, 1 ) != 1 ) {
            perror ( "DataPhase assign value error" );
            exit ( 1 );
        }
    }

    for ( int i=0; i<8; i++ ) close ( DataPhase_fds[i] );
}

// **************************************************************************************
void AddressPhase ( int* gpios ) {
    char value_str[3];
    char buff[256];
    int fds[8];
    int a, b = 0;
    for ( int i = 7; i < 15; i++ ) {
        sprintf ( buff, "/sys/class/gpio/gpio%d/direction", gpios[i] );
        fds[i - 7] = open ( buff, O_WRONLY );
        if ( write ( fds[i - 7], "in", 3 ) != 3 ) {
            perror ( "Error in address direction" );
            exit ( 1 );
        }

        sprintf ( buff, "/sys/class/gpio/gpio%d/value", gpios[i] );
        fds[i - 7] = open ( buff, O_RDONLY );
        if ( -1 == fds[i - 7] ) {
            fprintf ( stderr, "Failed to open gpio value for reading!\n" );
            exit ( 1 );
        }
        if ( -1 == read ( fds[i - 7], value_str, 3 ) ) {
            fprintf ( stderr, "Failed to read value!\n" );
            exit ( 1 );
        }
        a = atoi ( value_str ) ? 1 : 0;
        b += a;
        b = b << 1;
    }

    BA = b >> 1;
    FullAddr = ( BA << 16 ) + Address;
    printf ( ".%02x.%06x", BA, FullAddr );

    for ( int i = 0; i < 8; i++ ) close ( fds[i] );
}

// **************************************************************************************
void display_Address ( int* gpios ) {
    char buf[5] = {'a', 'a', 'a', 'a', 0};
    int addr;

    if ( ioctl ( fi2c, I2C_SLAVE, 0x20 ) < 0 ) {
        perror ( "Failed to connect to 21\n" );
        return;
    }
    if ( read ( fi2c, buf, 5 ) != 5 ) {
        perror ( "Failed to read in the buffer\n" );
        return;
    }
    addr = buf[0];
    // printf ( "\nA: %02x", addr );

    if ( ioctl ( fi2c, I2C_SLAVE, 0x22 ) < 0 ) {
        perror ( "Failed to connect to 22\n" );
        return;
    }
    if ( read ( fi2c, buf, 5 ) != 5 ) {
        perror ( "Failed to read in the buffer\n" );
        return;
    }
    // printf ( "\nB: %02x\n", buf[0] );

    addr += ( buf[0] << 8 );
    Address = addr;
    printf ( " %04x", addr );
}

// **************************************************************************************
void display_Signals ( int* gpios ) {
    char value_str[3];
    int fd;
    char buff[256];

    for ( int i = 0; i < 7; i++ ) {
        sprintf ( buff, "/sys/class/gpio/gpio%d/value", gpios[i] );
        fd = open ( buff, O_RDONLY );
        if ( -1 == fd ) {
            fprintf ( stderr, "Failed to open gpio value for reading!\n" );
            exit ( 1 );
        }
        if ( -1 == read ( fd, value_str, 3 ) ) {
            fprintf ( stderr, "Failed to read value!\n" );
            exit ( 1 );
        }
        close ( fd );

        if ( i == 6 )
            printf ( " %s", atoi ( value_str ) ? "r" : "W" );
        else if ( i == 5 )
            printf ( " %s", atoi ( value_str ) ? "E" : "-" );
        else if ( i == 2 )
            printf ( " %s", atoi ( value_str ) ? "VPA" : "---" );
        else if ( i == 3 )
            printf ( " %s", atoi ( value_str ) ? "VDA " : "--- " );
        else {
            printf ( "%d", atoi ( value_str ) ? 1 : 0 );
            if ( i == 2 ) printf ( " " );
        }
        Signals[i] = atoi ( value_str ) ? 1 : 0;
    }
}

// **************************************************************************************
void HardwarePhase ( int* gpios ) {
    // ******************** TIC
    // Data mode
    TicToc = 1;
    printf ( "\nTic: " );
    if ( write ( clk, "1", 1 ) != 1 ) {
        perror ( "Error writing to clock" );
        exit ( 1 );
    }
    display_Signals ( gpios );
    display_Address ( gpios );
    DataPhase ( gpios );
    usleep ( 50000 * 3 );




    // ******************** TOC
    TicToc = 0;
    printf ( "\nToc: " );
    if ( write ( clk, "0", 1 ) != 1 ) {
        perror ( "Error writing to clock" );
        exit ( 1 );
    }
    display_Signals ( gpios );
    display_Address ( gpios );
    AddressPhase ( gpios );
    usleep ( 50000 * 3 );




}

// **************************************************************************************
void export_CLK() {
    printf ( "export_CLK\n" );

    int fd = open ( "/sys/class/gpio/export", O_WRONLY );
    if ( fd == -1 ) {
        perror ( "Unable to open clock" );
        exit ( 1 );
    }

    if ( write ( fd, "6", 2 ) != 2 ) {
        perror ( "Error writing to clock" );
        exit ( 1 );
    }

    close ( fd );



    printf ( "set_direction\n" );
    fd = open ( "/sys/class/gpio/gpio6/direction", O_WRONLY );
    if ( fd == -1 ) {
        perror ( "Unable to open clock" );
        exit ( 1 );
    }

    if ( write ( fd, "out", 3 ) != 3 ) {
        perror ( "Error writing to clock" );
        exit ( 1 );
    }

    close ( fd );
}

void unexport_CLK() {
    printf ( "unexport_CLK\n" );
    int fd = open ( "/sys/class/gpio/unexport", O_WRONLY );
    if ( fd == -1 ) {
        perror ( "Unable to open /sys/class/gpio/unexport" );
        exit ( 1 );
    }

    if ( write ( fd, "6", 2 ) != 2 ) {
        perror ( "Error writing to /sys/class/gpio/unexport" );
        exit ( 1 );
    }

    close ( fd );
}

// **************************************************************************************
void open_i2c() {
    if ( ( fi2c = open ( "/dev/i2c-1", O_RDWR ) ) < 0 ) {
        perror ( "failed to open the bus\n" );
        return;
    }
    printf ( "Opened i2c, %d\n", fi2c );
}

// **************************************************************************************
void export_Signals ( int* gpios ) {
    int fd;
    printf ( "export_Signals\n" );
    printf ( "   opening...\n" );

    fd = open ( "/sys/class/gpio/export", O_WRONLY );
    if ( fd == -1 ) {
        perror ( "Unable to open /sys/class/gpio/export" );
        exit ( 1 );
    }

    char snum[5];
    for ( int i = 0; i < 7; i++ ) {
        sprintf ( snum, "%d", gpios[i] );
        if ( write ( fd, snum, 2 ) != 2 ) {
            perror ( "   Error writing to signals" );
            exit ( 1 );
        }
    }
    close ( fd );



    char buff[256];
    int fds[7];
    for ( int i = 0; i < 7; i++ ) {
        sprintf ( buff, "/sys/class/gpio/gpio%d/direction", gpios[i] );
        fds[i] = open ( buff, O_WRONLY );
        if ( write ( fds[i], "in", 3 ) != 3 ) {
            perror ( "Error in signal direction" );
            exit ( 1 );
        }
    }

    for ( int i = 0; i < 7; i++ ) close ( fds[i] );
}

void close_Signals ( int* gpios ) {
    printf ( "unexport Signals\n" );
    int fd;

    fd = open ( "/sys/class/gpio/unexport", O_WRONLY );
    if ( fd == -1 ) {
        perror ( "Unable to open /sys/class/gpio/unexport" );
        exit ( 1 );
    }

    char snum[5];
    for ( int i = 0; i < 7; i++ ) {
        sprintf ( snum, "%d", gpios[i] );
        if ( write ( fd, snum, 2 ) != 2 ) {
            perror ( "Error closing signals" );
            exit ( 1 );
        }
    }

    close ( fd );
}

// **************************************************************************************
void export_BAs ( int* gpios ) {
    int fd;
    printf ( "export_BAs\n" );
    printf ( "   opening...\n" );

    fd = open ( "/sys/class/gpio/export", O_WRONLY );
    if ( fd == -1 ) {
        perror ( "Unable to open /sys/class/gpio/export" );
        exit ( 1 );
    }

    char snum[5];
    for ( int i = 7; i < 15; i++ ) {
        sprintf ( snum, "%d", gpios[i] );
        if ( write ( fd, snum, 2 ) != 2 ) {
            perror ( "   Error writing to BAs" );
            exit ( 1 );
        }
    }
    close ( fd );
}

void close_BAs ( int* gpios ) {
    printf ( "unexport Signals\n" );
    int fd;

    fd = open ( "/sys/class/gpio/unexport", O_WRONLY );
    if ( fd == -1 ) {
        perror ( "Unable to open /sys/class/gpio/unexport" );
        exit ( 1 );
    }

    char snum[5];
    for ( int i = 7; i < 15; i++ ) {
        sprintf ( snum, "%d", gpios[i] );
        if ( write ( fd, snum, 2 ) != 2 ) {
            perror ( "Error closing signals" );
            exit ( 1 );
        }
    }

    close ( fd );
}

// **************************************************************************************
int main ( void ) {
    int GPIOs[15] = {
        13, 19, 26,       // VPB, MLB, VPA
        14, 15, 18, 23,   // VDA, MX, E, RWB
        24, 25,  8,  7,   // D0, D1, D2, D3
        12, 16, 20, 21    // D4, D5, D6, D7
    };



    struct termios orig_term, raw_term;
    tcgetattr ( STDIN_FILENO, &orig_term );
    raw_term = orig_term;
    raw_term.c_lflag &= ~ ( ECHO | ICANON );
    raw_term.c_cc[VMIN] = 0;
    raw_term.c_cc[VTIME] = 0;
    tcsetattr ( STDIN_FILENO, TCSANOW, &raw_term );


    export_CLK();
    open_i2c();
    export_Signals ( GPIOs );
    export_BAs ( GPIOs );

    printf ( "Press [ESC] to quit.\n" );
    clk = open ( "/sys/class/gpio/gpio6/value", O_WRONLY );
    if ( clk == -1 ) {
        perror ( "EEK: Unable to open clock." );
        exit ( 1 );
    }

    char ch = 0;
    char pressed = 0;
    int again = 0;
    do {
        int len = read ( STDIN_FILENO, &ch, 1 );
        if ( len == 1 ) {
            printf ( "You pressed char 0x%02x : %c\n", ch,
                     ( ch >= 32 && ch < 127 ) ? ch : ' ' );
            if ( ch == 'p' ) {
                pressed = 'p';
                again++;
            } else
                pressed = 0;
        }

        if ( pressed == 'p' ) {
            if ( again ) {
                HardwarePhase ( GPIOs );
                again = 0;
            }
            continue;
        }

        HardwarePhase ( GPIOs );

    } while ( ch != 27 );

    close ( clk );

    unexport_CLK();
    close_Signals ( GPIOs );
    close_BAs ( GPIOs );
    close ( fi2c );


    while ( read ( STDIN_FILENO, &ch, 1 ) == 1 );
    tcsetattr ( STDIN_FILENO, TCSANOW, &orig_term );

    return 0;
}




