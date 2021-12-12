/** Simple I2C example to read the time from a DS3231 module
* Written by Derek Molloy for the book "Exploring Raspberry Pi" */

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include <termios.h>
#include <unistd.h>

#define BUFFER_SIZE 5

// the time is in the registers in decimal form
int bcdToDec(char b) {
    return (b/16)*10 + (b%16);
}

int main() {
    int file;
    printf("Starting the DS3231 test application\n");
    if((file=open("/dev/i2c-1", O_RDWR)) < 0) {
        perror("failed to open the bus\n");
        return 1;
    }
    if(ioctl(file, I2C_SLAVE, 0x38) < 0) {
        perror("Failed to connect to the sensor\n");
        return 1;
    }

    char buf[BUFFER_SIZE];
    if(read(file, buf, BUFFER_SIZE) != BUFFER_SIZE) {
        perror("Failed to read in the buffer\n");
        return 1;
    }

    printf("%d\n", buf[0]);

    close(file);
    return 0;
}
