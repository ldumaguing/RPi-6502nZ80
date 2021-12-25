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

    printf("%04x %6d\n", addr, addr);

    return addr;
}

void HardwarePhase(char* rom)
{
    if (write(clk, "1", 1) != 1) {
        perror("Error writing to /sys/class/gpio/gpio26/value");
        exit(1);
    }
    get_ADDR();
    usleep(50000 * 2);

    if (write(clk, "0", 1) != 1) {
        perror("Error writing to /sys/class/gpio/gpio26/value");
        exit(1);
    }
    usleep(50000 * 2);


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

// **************************************************************************************
int main(void)
{
    int ROM_size = 32768;    // 8000 to ffff
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
    int isRead = 0;
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

        isRead = get_RW(GPIOs);
        printf("%s ", isRead ? "r" : "W");
        if (pressed == 'p') {
            //  if ( write ( clk, "0", 1 ) != 1 ) {
            //      perror ( "Error writing to /sys/class/gpio/gpio26/value" );
            //      exit ( 1 );
            //  }
            //  usleep ( 50000 * 2 );

            if (again) {
                HardwarePhase(ROM);
                again = 0;
            }
            continue;
        }

        HardwarePhase(ROM);

    } while (ch != 27);

    close(clk);

    unexport_CLK();
    unexport_GPIOs(GPIOs);
    close(fi2c);

    while (read(STDIN_FILENO, &ch, 1) == 1);
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);

    return 0;
}



