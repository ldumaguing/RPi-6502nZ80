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

int clk, fi2c;

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
        else {
            printf ( "%d", atoi ( value_str ) ? 1 : 0 );
            if ( i == 2 ) printf ( " " );
        }
    }

    printf ( "\n" );
}

// **************************************************************************************
void HardwarePhase ( int* gpios ) {
    // ********** TIC
    if ( write ( clk, "1", 1 ) != 1 ) {
        perror ( "Error writing to clock" );
        exit ( 1 );
    }
    printf ( "Tic: " );
    display_Signals ( gpios );
    usleep ( 50000 * 3 );


    // ********** TOC
    if ( write ( clk, "0", 1 ) != 1 ) {
        perror ( "Error writing to clock" );
        exit ( 1 );
    }
    printf ( "Toc: " );
    display_Signals ( gpios );
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
}

void set_CLK_direction() {
    printf ( "set_direction\n" );
    int fd = open ( "/sys/class/gpio/gpio6/direction", O_WRONLY );
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

    for ( int i = 0; i < 7; i++ ) {
        sprintf ( buff, "/sys/class/gpio/gpio%d/direction", gpios[i] );
        fd = open ( buff, O_WRONLY );
        if ( write ( fd, "in", 3 ) != 3 ) {
            perror ( "Error in signal direction" );
            exit ( 1 );
        }
        close ( fd );
    }
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
    set_CLK_direction();
    open_i2c();
    export_Signals ( GPIOs );

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
    close ( fi2c );

    while ( read ( STDIN_FILENO, &ch, 1 ) == 1 );
    tcsetattr ( STDIN_FILENO, TCSANOW, &orig_term );

    return 0;
}
/*
int clk, fi2c;
int ROM_size = 32768;    // 8000 to ffff
int RAM_size = 32768;    // 0000 to 7fff

// **************************************************************************************
char mode_Read(char* rom, char* ram, int currAddr)
{
    int romAddr0 = 32768;

    if (ioctl(fi2c, I2C_SLAVE, 0x20) < 0) {
        perror("Failed to connect to 20\n");
        return 0;
    }

    char writeBuffer[1];
    if (currAddr < 32768) {   // RAM
        writeBuffer[0] = (char) ram[currAddr];
        if (write(fi2c, writeBuffer, 1) != 1)
            perror("Failed to write to PCF\n");
    } else {                  // ROM
        writeBuffer[0] = (char) rom[currAddr - romAddr0];
        if (write(fi2c, writeBuffer, 1) != 1)
            perror("Failed to write to PCF\n");
    }

    return writeBuffer[0];
}

char mode_Write(char* ram, int currAddr)
{
    char buf[1];

    if (ioctl(fi2c, I2C_SLAVE, 0x20) < 0) {
        perror("Failed to connect to 20 in mode_Write.\n");
        return 0;
    }
    char config[1] = {0};
    config[0] = 0xff;
    write(fi2c, config, 1);

    if (read(fi2c, buf, 5) != 5) {
        perror("Failed to read in the buffer\n");
        return 0;
    }
    if (currAddr < 32768) {
        ram[currAddr] = buf[0];
    }

    return buf[0];
}

// **************************************************************************************
void open_i2c()
{
    if ((fi2c = open("/dev/i2c-1", O_RDWR)) < 0) {
        perror("failed to open the bus\n");
        return;
    }
    printf("Opened i2c, %d\n", fi2c);
}

// **************************************************************************************
void export_GPIOs(int* GPIOs)
{
    int fd;
    printf("export_Addresses\n");
    printf("   opening...\n");

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1) {
        perror("Unable to open /sys/class/gpio/export");
        exit(1);
    }

    char snum[5];
    for (int i = 0; i < 3; i++) {
        sprintf(snum, "%d", GPIOs[i]);
        if (write(fd, snum, 2) != 2) {
            perror("   Error writing to /sys/class/gpio/export");
            exit(1);
        }
    }

    close(fd);
}

void set_GPIOs_directions(int* GPIOs)
{
    printf("set_direction\n");
    int fd;
    char buff[256];

    for (int i = 0; i < 3; i++) {
        sprintf(buff, "/sys/class/gpio/gpio%d/direction", GPIOs[i]);
        fd = open(buff, O_WRONLY);
        if (write(fd, "in", 3) != 3) {
            perror("Error");
            exit(1);
        }
        close(fd);
    }
}

void unexport_GPIOs(int* GPIOs)
{
    printf("unexport_Addresses\n");
    printf("   closing...\n");
    int fd;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd == -1) {
        perror("Unable to open /sys/class/gpio/unexport");
        exit(1);
    }

    char snum[5];
    for (int i = 0; i < 3; i++) {
        sprintf(snum, "%d", GPIOs[i]);
        if (write(fd, snum, 2) != 2) {
            perror("Error writing to /sys/class/gpio/unexport");
            exit(1);
        }
    }

    close(fd);
}

// **************************************************************************************
void export_CLK()
{
    printf("export_CLK\n");

    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1) {
        perror("Unable to open /sys/class/gpio/export");
        exit(1);
    }

    if (write(fd, "26", 2) != 2) {
        perror("Error writing to /sys/class/gpio/export");
        exit(1);
    }

    close(fd);
}

void set_CLK_direction()
{
    printf("set_direction\n");
    int fd = open("/sys/class/gpio/gpio26/direction", O_WRONLY);
    if (fd == -1) {
        perror("Unable to open /sys/class/gpio/gpio26/direction");
        exit(1);
    }

    if (write(fd, "out", 3) != 3) {
        perror("Error writing to /sys/class/gpio/gpio26/direction");
        exit(1);
    }

    close(fd);
}

void unexport_CLK()
{
    printf("unexport_CLK\n");
    int fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd == -1) {
        perror("Unable to open /sys/class/gpio/unexport");
        exit(1);
    }

    if (write(fd, "26", 2) != 2) {
        perror("Error writing to /sys/class/gpio/unexport");
        exit(1);
    }

    close(fd);
}

// **************************************************************************************
// **************************************************************************************
// **************************************************************************************
int get_RW(int* GPIOs)
{
    char value_str[3];
    int fd;
    char buff[256];

    sprintf(buff, "/sys/class/gpio/gpio%d/value", GPIOs[0]);
    fd = open(buff, O_RDONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio value for reading!\n");
        exit(1);
    }
    if (-1 == read(fd, value_str, 3)) {
        fprintf(stderr, "Failed to read value!\n");
        exit(1);
    }
    close(fd);
    return (atoi(value_str) ? 1 : 0);
}

int get_ADDR()
{
    char buf[5] = {'a', 'a', 'a', 'a', 0};
    int addr;

    if (ioctl(fi2c, I2C_SLAVE, 0x21) < 0) {
        perror("Failed to connect to 21\n");
        return 0;
    }
    if (read(fi2c, buf, 5) != 5) {
        perror("Failed to read in the buffer\n");
        return 0;
    }
    addr = buf[0];


    if (ioctl(fi2c, I2C_SLAVE, 0x22) < 0) {
        perror("Failed to connect to 22\n");
        return 0;
    }
    if (read(fi2c, buf, 5) != 5) {
        perror("Failed to read in the buffer\n");
        return 0;
    }
    addr += (buf[0] * 256);


    return addr;
}

void HardwarePhase(char* rom, char* ram, int* gpios)
{
    int addr;
    char datum;

    addr = get_ADDR();

    // ********** TIC
    if (write(clk, "1", 1) != 1) {
        perror("Error writing to /sys/class/gpio/gpio26/value");
        exit(1);
    }
    int isRead = get_RW(gpios);
    if (isRead) {
        datum = mode_Read(rom, ram, addr);
    } else {
        datum = mode_Write(ram, addr);
    }

    usleep(50000 * 3);


    // ********** TOC
    if (write(clk, "0", 1) != 1) {
        perror("Error writing to /sys/class/gpio/gpio26/value");
        exit(1);
    }

    usleep(50000 * 3);

    printf(" %04x %s %02x\n", addr, isRead ? "r" : "W", datum);
}

// **************************************************************************************
void loadROM(char* rom)
{
    printf("Loading ROM...\n");
    FILE* fp;
    fp = fopen("rom.bin", "rb");
    fread(rom, ROM_size, 1, fp);
    fclose(fp);
}

// **************************************************************************************
int main(void)
{
    char RAM[RAM_size];
    char ROM[ROM_size];
    loadROM(ROM);

    int GPIOs[3] = {
        21, 19, 13   // RW, SYNC, MLB
    };

    export_GPIOs(GPIOs);
    set_GPIOs_directions(GPIOs);

    struct termios orig_term, raw_term;
    tcgetattr(STDIN_FILENO, &orig_term);
    raw_term = orig_term;
    raw_term.c_lflag &= ~(ECHO | ICANON);
    raw_term.c_cc[VMIN] = 0;
    raw_term.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw_term);


    export_CLK();
    set_CLK_direction();
    open_i2c();
    printf("Press [ESC] to quit.\n");
    clk = open("/sys/class/gpio/gpio26/value", O_WRONLY);
    if (clk == -1) {
        perror("EEK: Unable to open /sys/class/gpio/gpio26/value");
        exit(1);
    }

    char ch = 0;
    char pressed = 0;
    int again = 0;
    do {
        int len = read(STDIN_FILENO, &ch, 1);
        if (len == 1) {
            printf("You pressed char 0x%02x : %c\n", ch,
                   (ch >= 32 && ch < 127) ? ch : ' ');
            if (ch == 'p') {
                pressed = 'p';
                again++;
            } else
                pressed = 0;
        }

        if (pressed == 'p') {
            if (again) {
                HardwarePhase(ROM, RAM, GPIOs);
                again = 0;
            }
            continue;
        }

        HardwarePhase(ROM, RAM, GPIOs);

    } while (ch != 27);

    close(clk);

    unexport_CLK();
    unexport_GPIOs(GPIOs);
    close(fi2c);

    while (read(STDIN_FILENO, &ch, 1) == 1);
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);

    return 0;
}
*/


