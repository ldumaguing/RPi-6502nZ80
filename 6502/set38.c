/** Simple I2C example to read the time from a DS3231 module
* Written by Derek Molloy for the book "Exploring Raspberry Pi" */

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>




#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


int main(){
   int file;
   printf("Starting...\n");
   if((file=open("/dev/i2c-1", O_RDWR)) < 0){
      perror("failed to open the bus\n");
      return 1;
   }

   if(ioctl(file, I2C_SLAVE, 0x38) < 0) {
      perror("Failed to connect to the sensor\n");
      return 1;
   }
   
   char writeBuffer[1] = {0xf8};
   if(write(file, writeBuffer, 1) != 1){
      perror("Failed to reset the read address\n");
      return 1;
   }

   close(file);
   
   return 0;
}


