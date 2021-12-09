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





#define BUFFER_SIZE 19

// the time is in the registers in decimal form
int bcdToDec(char b) { return (b/16)*10 + (b%16); }

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

/*   
   char writeBuffer[1] = {0x00};
   if(write(file, writeBuffer, 1)!=1){
      perror("Failed to reset the read address\n");
      return 1;
   }
*/

   
   char buf[3];
   if(read(file, buf, 3) == -1){
      perror("Failed to read in the buffer\n");
      return 1;
   }
   printf("%d\n", atoi(buf[0]));

   /*
   printf("The RTC time is %02d:%02d:%02d\n", bcdToDec(buf[2]),
         bcdToDec(buf[1]), bcdToDec(buf[0]));
   float temperature = buf[0x11] + ((buf[0x12]>>6)*0.25);
   printf("The temperature is %.2f°C\n", temperature);
   */

  // printf("%d\n", buf);
   close(file);
   return 0;
}


