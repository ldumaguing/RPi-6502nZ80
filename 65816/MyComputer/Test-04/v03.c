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

int clk, fi2c, Address, RWB = 0;
int TicToc = 0;  // Tic = 1, Toc = 0

#define signal_elements 1
int Signals[] = {
    19  // RWB
};
/*
VPB, MLB, VPA
VDA, MX, E, RWB
*/

#define sleep usleep ( 50000 * 3 )

#define ROM_size 32768
#define RAM_size 32768
#define starting_ROM_address 32768   // 8000
#define sixtyfourK 65536
char ROM[ROM_size];
char RAM[RAM_size];

void TIC();
void TOC();

// **************************************************************************************
void loadROM () {
    printf ( "Loading ROM...\n" );
    FILE* fp;
    fp = fopen ( "rom.bin", "rb" );
    fread ( ROM, ROM_size, 1, fp );
    fclose ( fp );
}

// **************************************************************************************
void define_ByteData ( int* bydt ) {
    /*
        int m = 1;
        if ( ( Address >= starting_ROM_address ) & ( Address <= sixtyfourK ) ) {
            int X = ROM[Address - starting_ROM_address];
            printf ( " ..%04x", X );
            for ( int i=0; i<8; i++ ) {
                bydt[i] = ( X & m ) ? 1 : 0;
                m = m << 1;
            }
            return;
        }

        if ( Address < starting_ROM_address ) {
            int X = RAM[Address];
            printf ( " ...%04x", X );
            for ( int i=0; i<8; i++ ) {
                bydt[i] = ( X & m ) ? 1 : 0;
                m = m << 1;
            }
        }
    */


    /*
    int m = 1;
    int X = 234;
    printf ( " ...%04x", X );
    for ( int i=0; i<8; i++ ) {
        bydt[i] = ( X & m ) ? 1 : 0;
        m = m << 1;
    }
    */
}

// **************************************************************************************
void save_CPU_datum() {
    TIC();
    sleep;

    /*
    if ( ioctl ( fi2c, I2C_SLAVE, 0x20 ) < 0 ) {
        perror ( "Failed to connect to 20 in mode_Write.\n" );
        return;
    }

    char config[1];
    config[0] = 0xff;
    write ( fi2c, config, 1 );

    TIC();
    sleep;

    char buf[1];
    if ( read ( fi2c, buf, 5 ) != 5 ) {
        perror ( "Failed to read in the buffer\n" );
        return;
    }

    int relROMaddr = Address - starting_ROM_address;
    printf ( " %06x.%d", Address, relROMaddr );
    printf ( "/%x/", buf[0] );
    */
}

// **************************************************************************************
void set_CPU_Read() {
    int relROMaddr = Address - starting_ROM_address;


    if ( ( fi2c = open ( "/dev/i2c-1", O_RDWR ) ) < 0 ) {
        perror ( "failed to open the bus\n" );
        return;
    }

    if ( ioctl ( fi2c, I2C_SLAVE, 0x20 ) < 0 ) {
        perror ( "Failed to connect to 20\n" );
        return;
    }


    char config[1];
    config[0] = 0xff;
    write ( fi2c, config, 1 );



 
    if ( ( Address >= starting_ROM_address ) & ( Address <= sixtyfourK ) ) {
        printf ( " %02x", ROM[relROMaddr] );
        char fish[1];
        fish[0] = ROM[relROMaddr];
        if ( write ( fi2c, fish, 1 ) != 1 )
            perror ( "Failed to write to PCF\n" );
    } else {
        printf (" NOP");
        char fish[1];
        fish[0] = ( char ) 234;  // NOP
        if ( write ( fi2c, fish, 1 ) != 1 )
            perror ( "Failed to write to PCF\n" );
    }
 
   /*
    printf ( " NOP" );
    char fish[1];
    fish[0] = ( char ) 234;  // NOP
    if ( write ( fi2c, fish, 1 ) != 1 )
        perror ( "Failed to write to PCF\n" );

   */
}

// **************************************************************************************
void get_Address () {
    char buf[5] = {'a', 'a', 'a', 'a', 0};
    int addr;


    if ( ( fi2c = open ( "/dev/i2c-1", O_RDWR ) ) < 0 ) {
        perror ( "failed to open the bus\n" );
        return;
    }


    char config[1];
    config[0] = 0x00;
    write ( fi2c, config, 1 );


    if ( ioctl ( fi2c, I2C_SLAVE, 0x21 ) < 0 ) {
        perror ( "Failed to connect to 21\n" );
        return;
    }
    if ( read ( fi2c, buf, 5 ) != 5 ) {
        perror ( "Failed to read in the buffer\n" );
        return;
    }
    addr = buf[0];


    write ( fi2c, config, 1 );


    if ( ioctl ( fi2c, I2C_SLAVE, 0x3a ) < 0 ) {
        perror ( "Failed to connect to 3a\n" );
        return;
    }
    if ( read ( fi2c, buf, 5 ) != 5 ) {
        perror ( "Failed to read in the buffer\n" );
        return;
    }
    addr += ( buf[0] << 8 );


    write ( fi2c, config, 1 );


    if ( ioctl ( fi2c, I2C_SLAVE, 0x20 ) < 0 ) {
        perror ( "Failed to connect to 20\n" );
        return;
    }
    if ( read ( fi2c, buf, 5 ) != 5 ) {
        perror ( "Failed to read in the buffer\n" );
        return;
    }
    addr += ( buf[0] << 16 );


    Address = addr;
    printf ( " %06x", Address );
}

// **************************************************************************************
void get_Signals () {
    char value_str[3];
    int fd;
    char buff[256];

    for ( int i = 0; i < signal_elements; i++ ) {
        sprintf ( buff, "/sys/class/gpio/gpio%d/value", Signals[i] );
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

        if ( i == 0 ) {
            printf ( " %s", atoi ( value_str ) ? "r" : "W" );
            RWB = atoi ( value_str );
        } else if ( i == 5 )
            printf ( " %s", atoi ( value_str ) ? "E" : "-" );
        else if ( i == 2 )
            printf ( " %s", atoi ( value_str ) ? "VPA" : "---" );
        else if ( i == 3 )
            printf ( " %s", atoi ( value_str ) ? "VDA " : "--- " );
        else {
            printf ( "%d", atoi ( value_str ) ? 1 : 0 );
            if ( i == 2 ) printf ( " " );
        }
    }
}

// **************************************************************************************
void HardwarePhase () {
    TOC();
    sleep;
    get_Signals ();
    get_Address ();

    sleep;
    if ( 1 ) {
        set_CPU_Read();
        sleep;
        TIC();
        sleep;
    } else {
        save_CPU_datum();
    }
}

// **************************************************************************************
void export_CLK() {
    printf ( "export_CLK\n" );

    int fd = open ( "/sys/class/gpio/export", O_WRONLY );
    if ( fd == -1 ) {
        perror ( "Unable to open clock" );
        exit ( 1 );
    }

    if ( write ( fd, "26", 2 ) != 2 ) {
        perror ( "Error writing to clock" );
        exit ( 1 );
    }

    close ( fd );


    printf ( "set_direction\n" );
    fd = open ( "/sys/class/gpio/gpio26/direction", O_WRONLY );
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

// --------------------------------------------------------------------------------------
void TIC() {
    /*
     * from LOW to HIGH
     */
    TicToc = 1;
    printf ( "\nTic:" );

    if ( write ( clk, "1", 1 ) != 1 ) {
        perror ( "Error writing to clock" );
        exit ( 1 );
    }
}

// --------------------------------------------------------------------------------------
void TOC() {
    /*
     * from HIGH to Low
     */
    TicToc = 0;
    printf ( "\nToc:" );

    if ( write ( clk, "0", 1 ) != 1 ) {
        perror ( "Error writing to clock" );
        exit ( 1 );
    }
}

// --------------------------------------------------------------------------------------
void unexport_CLK() {
    printf ( "unexport_CLK\n" );
    int fd = open ( "/sys/class/gpio/unexport", O_WRONLY );
    if ( fd == -1 ) {
        perror ( "Unable to open /sys/class/gpio/unexport" );
        exit ( 1 );
    }

    if ( write ( fd, "26", 2 ) != 2 ) {
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
void export_Signals () {
    int fd;
    printf ( "export_Signals\n" );
    printf ( "   opening...\n" );

    fd = open ( "/sys/class/gpio/export", O_WRONLY );
    if ( fd == -1 ) {
        perror ( "Unable to open /sys/class/gpio/export" );
        exit ( 1 );
    }

    char snum[5];
    for ( int i = 0; i < signal_elements; i++ ) {
        sprintf ( snum, "%d", Signals[i] );
        if ( write ( fd, snum, 2 ) != 2 ) {
            perror ( "   Error writing to signals" );
            exit ( 1 );
        }
    }
    close ( fd );



    char buff[256];
    int fds[signal_elements];
    for ( int i = 0; i < signal_elements; i++ ) {
        sprintf ( buff, "/sys/class/gpio/gpio%d/direction", Signals[i] );
        fds[i] = open ( buff, O_WRONLY );
        if ( write ( fds[i], "in", 3 ) != 3 ) {
            perror ( "Error in signal direction" );
            exit ( 1 );
        }
    }

    //for ( int i = 0; i < 7; i++ ) close ( fds[i] );
}

// --------------------------------------------------------------------------------------
void close_Signals () {
    printf ( "unexport Signals\n" );
    int fd;

    fd = open ( "/sys/class/gpio/unexport", O_WRONLY );
    if ( fd == -1 ) {
        perror ( "Unable to open /sys/class/gpio/unexport" );
        exit ( 1 );
    }

    char snum[5];
    for ( int i = 0; i < signal_elements; i++ ) {
        sprintf ( snum, "%d", Signals[i] );
        if ( write ( fd, snum, 2 ) != 2 ) {
            perror ( "Error closing signals" );
            exit ( 1 );
        }
    }

    close ( fd );
}

// **************************************************************************************
int main ( void ) {
    struct termios orig_term, raw_term;
    tcgetattr ( STDIN_FILENO, &orig_term );
    raw_term = orig_term;
    raw_term.c_lflag &= ~ ( ECHO | ICANON );
    raw_term.c_cc[VMIN] = 0;
    raw_term.c_cc[VTIME] = 0;
    tcsetattr ( STDIN_FILENO, TCSANOW, &raw_term );

    loadROM();
    export_CLK();
    open_i2c();
    export_Signals ();


    printf ( "Press [ESC] to quit.\n" );
    clk = open ( "/sys/class/gpio/gpio26/value", O_WRONLY );
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
                HardwarePhase ();
                again = 0;
            }
            continue;
        }

        HardwarePhase ();

    } while ( ch != 27 );

    close ( clk );

    unexport_CLK();
    close_Signals ();
    close ( fi2c );


    while ( read ( STDIN_FILENO, &ch, 1 ) == 1 );
    tcsetattr ( STDIN_FILENO, TCSANOW, &orig_term );

    return 0;
}




