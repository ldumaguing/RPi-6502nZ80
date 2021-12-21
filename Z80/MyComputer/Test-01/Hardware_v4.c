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

void export_CLK();
void set_direction_CLK();
void unexport_CLK();

void export_Addresses(int*);
void set_direction_Addresses(int*);
void unexport_Addresses(int*);

void print_current_address(int*);
void print_RW(int*);
void printDBus();

int fi2c;

// **************************************************************************************
void open_i2c()
{
    if ((fi2c = open("/dev/i2c-1", O_RDWR)) < 0) {
        perror("failed to open the bus\n");
        return;
    }
    if (ioctl(fi2c, I2C_SLAVE, 0x38) < 0) {
        perror("Failed to connect to the sensor\n");
        return;
    }
}

// **************************************************************************************
void loadROM(char* rom)
{
    printf("Loading ROM...\n");
    FILE* fp;
    fp = fopen("rom.bin", "rb");
    fread(rom, 32768, 1, fp);
    fclose(fp);
}

int get_RW(int* GPIOs)
{
    char MREQ[3];
    char M1[3];
    char RD[3];
    char WR[3];
    int fd;
    char buff[256];

    sprintf(buff, "/sys/class/gpio/gpio%d/value", GPIOs[16]);
    fd = open(buff, O_RDONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio value for reading!\n");
        exit(1);
    }
    if (-1 == read(fd, MREQ, 3)) {
        fprintf(stderr, "Failed to read value!\n");
        exit(1);
    }
    close(fd);




    sprintf(buff, "/sys/class/gpio/gpio%d/value", GPIOs[18]);
    fd = open(buff, O_RDONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio value for reading!\n");
        exit(1);
    }
    if (-1 == read(fd, M1, 3)) {
        fprintf(stderr, "Failed to read value!\n");
        exit(1);
    }
    close(fd);



    sprintf(buff, "/sys/class/gpio/gpio%d/value", GPIOs[19]);
    fd = open(buff, O_RDONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio value for reading!\n");
        exit(1);
    }
    if (-1 == read(fd, WR, 3)) {
        fprintf(stderr, "Failed to read value!\n");
        exit(1);
    }
    close(fd);




    sprintf(buff, "/sys/class/gpio/gpio%d/value", GPIOs[20]);
    fd = open(buff, O_RDONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio value for reading!\n");
        exit(1);
    }
    if (-1 == read(fd, RD, 3)) {
        fprintf(stderr, "Failed to read value!\n");
        exit(1);
    }
    close(fd);

    if (atoi(MREQ) == 0) {

    }

    int X = atoi(MREQ) + atoi(WR);
    if (X == 0) return 1;   // Write

    return 0;
}

int get_current_address(int* GPIOs)
{
    char value_str[3];
    int fd;
    char buff[256];

    int address = 0;
    for (int i = 0; i < 16; i++) {
        sprintf(buff, "/sys/class/gpio/gpio%d/value", GPIOs[i]);
        fd = open(buff, O_RDONLY);
        if (-1 == fd) {
            fprintf(stderr, "Failed to open gpio value for reading!\n");
            exit(1);
        }
        if (-1 == read(fd, value_str, 3)) {
            fprintf(stderr, "Failed to read value!\n");
            exit(1);
        }
        address = (address << 1) + atoi(value_str);

        close(fd);
    }

    return address;
}

char mode_Read(char* ram, char* rom, int romAddr0, int currAddr)
{
    char writeBuffer[1];
    if (currAddr >= 32768) {   // RAM
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
    char buf[5] = {'a', 'a', 'a', 'a', 0};

    if (read(fi2c, buf, 5) != 5) {
        perror("Failed to read in the buffer\n");
        return ' ';
    }
    if (currAddr >= 32768) {
        ram[currAddr] = buf[0];
    }

    return ram[currAddr];
}


void print_Signals(int* GPIOs)
{
    char value_str[3];
    int fd;
    char buff[256];

    int signal = 0;
    for (int i = 16; i < 21; i++) {
        sprintf(buff, "/sys/class/gpio/gpio%d/value", GPIOs[i]);
        fd = open(buff, O_RDONLY);
        if (-1 == fd) {
            fprintf(stderr, "Failed to open gpio value for reading!\n");
            exit;
        }
        if (-1 == read(fd, value_str, 3)) {
            fprintf(stderr, "Failed to read value!\n");
            exit;
        }
        signal = atoi(value_str);
        printf("%d", signal ? 1 : 0);
        close(fd);
    }
    printf("\n");
}



// **************************************************************************************
// **************************************************************************************
// **************************************************************************************
int main(void)
{
    int GPIOs[21] = {
        10, 22, 27, 17, 4, 14, 15, 18,  // a15 to a8
        23, 24, 25, 8, 7, 12, 16, 20,   // a7 to a0
        6, 13, 19, 26, 21               // MREQ, IORQ, M1, WR, RD
    };

    int RAM_size = 32768;    // 8000 to ffff
    int ROM_size = 32768;    // 0000 to 7fff
    char RAM[RAM_size];
    char ROM[ROM_size];
    int ROM_start_addr = 0;
    loadROM(ROM);

    struct termios orig_term, raw_term;
    tcgetattr(STDIN_FILENO, &orig_term);
    raw_term = orig_term;
    raw_term.c_lflag &= ~(ECHO | ICANON);
    raw_term.c_cc[VMIN] = 0;
    raw_term.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw_term);

    open_i2c();
    export_CLK();
    set_direction_CLK();
    export_Addresses(GPIOs);
    set_direction_Addresses(GPIOs);

    printf("Press [ESC] to quit.\n");
    int fd = open("/sys/class/gpio/gpio9/value", O_WRONLY);
    if (fd == -1) {
        perror("EEK: Unable to clock");
        exit(1);
    }

    char ch = 0;
    int isWrite = 0;
    int address = 0;
    do {
        read(STDIN_FILENO, &ch, 1);
        char datum = ' ';

        // ********** TIC
        // printf("Tic\n");
        // print_Signals(GPIOs);

        //     if (isRead) {
        if (write(fd, "1", 1) != 1) {
            perror("Error writing to /sys/class/gpio/gpio9/value");
            exit(1);
        }
        usleep(50000 * 2);
        // ***** Read
        /*
        datum = mode_Read(RAM, ROM, ROM_start_addr, address);
        printf("%04x %s %02x ", address, isRead ? "W" : "r", datum);
        print_Signals(GPIOs);
        //   }

        else {
          // ***** Write
          char config[1] = {0};
          config[0] = 0xFF;
          write ( fi2c, config, 1 );
          datum = mode_Write ( RAM, address );
        }
        */





        // ********** TOC
        // printf("Toc\n");
        if (write(fd, "0", 1) != 1) {
            perror("Error writing clock");
            exit(1);
        }
        usleep(50000 * 2);



        isWrite = get_RW(GPIOs);
        address = get_current_address(GPIOs);


        if (isWrite) {
            /*
            char config[1] = {0};
            config[0] = 0xFF;
            write(fi2c, config, 1);
            datum = mode_Write(RAM, address);
*/
            printf("%04x %2d %02x ", address, isWrite, datum);
            print_Signals(GPIOs);
        } else {
            datum = mode_Read(RAM, ROM, ROM_start_addr, address);
            printf("%04x %2d %02x ", address, isWrite, datum);
            print_Signals(GPIOs);
        }


        // printf("***\n");
    } while (ch != 27);

    close(fd);

    unexport_CLK();
    unexport_Addresses(GPIOs);
    close(fi2c);

    while (read(STDIN_FILENO, &ch, 1) == 1);
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);

    return 0;
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

    if (write(fd, "9", 2) != 2) {
        perror("Error writing to /sys/class/gpio/export");
        exit(1);
    }

    close(fd);
}

void set_direction_CLK()
{
    printf("set_direction\n");
    int fd = open("/sys/class/gpio/gpio9/direction", O_WRONLY);
    if (fd == -1) {
        perror("Unable to open clock");
        exit(1);
    }

    if (write(fd, "out", 3) != 3) {
        perror("Error writing to clock");
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

    if (write(fd, "9", 2) != 2) {
        perror("Error writing to /sys/class/gpio/unexport");
        exit(1);
    }

    close(fd);
}

// **************************************************************************************
void export_Addresses(int* GPIOs)
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
    for (int i = 0; i < 21; i++) {
        sprintf(snum, "%d", GPIOs[i]);
        if (write(fd, snum, 2) != 2) {
            perror("   Error writing to /sys/class/gpio/export");
            exit(1);
        }
    }

    close(fd);
}

void set_direction_Addresses(int* GPIOs)
{
    printf("set_direction\n");
    int fd;
    char buff[256];

    for (int i = 0; i < 21; i++) {
        printf("*\n");
        sprintf(buff, "/sys/class/gpio/gpio%d/direction", GPIOs[i]);
        fd = open(buff, O_WRONLY);
        if (write(fd, "in", 3) != 3) {
            perror("Error");
            exit(1);
        }
        close(fd);
    }
}

void unexport_Addresses(int* GPIOs)
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
    for (int i = 0; i < 21; i++) {
        sprintf(snum, "%d", GPIOs[i]);
        if (write(fd, snum, 2) != 2) {
            perror("Error writing to /sys/class/gpio/unexport");
            exit(1);
        }
    }

    close(fd);
}

















