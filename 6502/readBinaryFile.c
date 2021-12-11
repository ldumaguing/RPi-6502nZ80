#include <stdio.h>

void main() {
   FILE *fp;
   char buffer[32768];
   
   fp = fopen("rom.bin", "rb");

   fread(buffer, 32768, 1, fp);

   printf("%x\n", buffer[0]);
   printf("%x\n", buffer[1]);
   fclose(fp);
}
