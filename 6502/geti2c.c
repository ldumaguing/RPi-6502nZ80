/** Simple I2C example to read the time from a DS3231 module
* Written by Derek Molloy for the book "Exploring Raspberry Pi" */

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include <termios.h>
#include <unistd.h>

#define BUFFER_SIZE 256

// the time is in the registers in decimal form
int bcdToDec(char b) { return (b/16)*10 + (b%16); }

int main(){
   int file;
   printf("Starting the DS3231 test application\n");
   if((file=open("/dev/i2c-1", O_RDWR)) < 0){
      perror("failed to open the bus\n");
      return 1;
   }
   if(ioctl(file, I2C_SLAVE, 0x38) < 0){
      perror("Failed to connect to the sensor\n");
      return 1;
   }
   /*
   char writeBuffer[1] = {0x00};
   if(write(file, writeBuffer, 1)!=1){
      perror("Failed to reset the read address\n");
      return 1;
   }
   */

   char buf[BUFFER_SIZE];
   if(read(file, buf, BUFFER_SIZE)!=BUFFER_SIZE){
      perror("Failed to read in the buffer\n");
      return 1;
   }
   printf("The RTC time is %02d:%02d:%02d\n", bcdToDec(buf[0x11]),
         bcdToDec(buf[1]), bcdToDec(buf[0]));
   float temperature = buf[0x11] + ((buf[0x12]>>6)*0.25);
   printf("The temperature is %.2f°C\n", temperature);
   printf(">>> %d - %d\n", buf[0x11], buf[17]);

   printf("%d\n", buf[0]);
   printf("%d\n", buf[1]);
   printf("%d\n", buf[2]);
   printf("%d\n", buf[3]);
   printf("%d\n", buf[4]);
   printf("%d\n", buf[5]);
   printf("%d\n", buf[6]);
   printf("%d\n", buf[7]);


















   close(file);
   return 0;
}
