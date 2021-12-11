#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void export_CLK();
void set_direction_CLK();
void unexport_CLK();

// **************************************************************************************
int main ( void ) {
    int GPIOs[17] = {
        22, 27, 17,  4, 14, 15, 18, 23,  // a15 to a8
        24, 25,  8,  7, 12, 16, 20, 21,  // a7 to a0
        19                               // RW
    };

    struct termios orig_term, raw_term;
    tcgetattr ( STDIN_FILENO, &orig_term );
    raw_term = orig_term;
    raw_term.c_lflag &= ~ ( ECHO | ICANON );
    raw_term.c_cc[VMIN] = 0;
    raw_term.c_cc[VTIME] = 0;
    tcsetattr ( STDIN_FILENO, TCSANOW, &raw_term );







    export_CLK();
    set_direction_CLK();

    printf ( "Press [ESC] to quit.\n" );
    int fd = open ( "/sys/class/gpio/gpio26/value", O_WRONLY );
    if ( fd == -1 ) {
        perror ( "EEK: Unable to open /sys/class/gpio/gpio26/value" );
        exit ( 1 );
    }
    char ch = 0;
    int pause = 0;
    int counter = 0;
    do {
        int len = read ( STDIN_FILENO, &ch, 1 );
        if ( len == 1 ) {
            if ( ch == 'p' ) pause++;
            else pause = 0;
        }

        if ( pause % 2 ) continue;






// ********************
        if ( write ( fd, "1", 1 ) != 1 ) {
            perror ( "Error writing to /sys/class/gpio/gpio26/value" );
            exit ( 1 );
        }
        usleep ( 50000 * 2 );

        if ( write ( fd, "0", 1 ) != 1 ) {
            perror ( "Error writing to /sys/class/gpio/gpio26/value" );
            exit ( 1 );
        }
        usleep ( 50000 * 2 );
// ********************





        if ( ch == 'p' ) pause++;





        counter++;
    } while ( ch != 27 );

    close ( fd );



    unexport_CLK();













    while ( read ( STDIN_FILENO, &ch, 1 ) ==1 );
    tcsetattr ( STDIN_FILENO, TCSANOW, &orig_term );

    return 0;
}

// **************************************************************************************
void export_CLK() {
    printf ( "export_CLK\n" );

    int fd = open ( "/sys/class/gpio/export", O_WRONLY );
    if ( fd == -1 ) {
        perror ( "Unable to open /sys/class/gpio/export" );
        exit ( 1 );
    }

    if ( write ( fd, "26", 2 ) != 2 ) {
        perror ( "Error writing to /sys/class/gpio/export" );
        exit ( 1 );
    }

    close ( fd );
}

void set_direction_CLK() {
    printf ( "set_direction\n" );
    int fd = open ( "/sys/class/gpio/gpio26/direction", O_WRONLY );
    if ( fd == -1 ) {
        perror ( "Unable to open /sys/class/gpio/gpio26/direction" );
        exit ( 1 );
    }

    if ( write ( fd, "out", 3 ) != 3 ) {
        perror ( "Error writing to /sys/class/gpio/gpio26/direction" );
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

    if ( write ( fd, "26", 2 ) != 2 ) {
        perror ( "Error writing to /sys/class/gpio/unexport" );
        exit ( 1 );
    }

    close ( fd );
}

