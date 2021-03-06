#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void export_Addresses();
void set_direction();
void unexport_Addresses();

// **************************************************************************************
int main(void)
{
  struct termios orig_term, raw_term;
  tcgetattr(STDIN_FILENO, &orig_term);
  raw_term = orig_term;
  raw_term.c_lflag &= ~(ECHO | ICANON);
  raw_term.c_cc[VMIN] = 0;
  raw_term.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &raw_term);







  export_Addresses();
  set_direction();
  printf("Press [ESC] to quit.\n");

  char ch = 0;
  do {
    int len = read(STDIN_FILENO, &ch, 1);
    if (len == 1) {
      printf("You pressed char 0x%02x : %c\n", ch,
      (ch >= 32 && ch < 127) ? ch : ' ');
    }

    char value_str[3];
    int fd;




    // ***
    fd = open("/sys/class/gpio/gpio16/value", O_RDONLY);
    if (-1 == fd) {
      fprintf(stderr, "Failed to open gpio value for reading!\n");
		return(-1);
    }
    if (-1 == read(fd, value_str, 3)) {
      fprintf(stderr, "Failed to read value!\n");
		return(-1);
    }
    printf(" %d", atoi(value_str));
    close(fd);
    fd = open("/sys/class/gpio/gpio20/value", O_RDONLY);
    if (-1 == fd) {
      fprintf(stderr, "Failed to open gpio value for reading!\n");
		return(-1);
    }
    if (-1 == read(fd, value_str, 3)) {
      fprintf(stderr, "Failed to read value!\n");
		return(-1);
    }
    printf(" %d", atoi(value_str));
    close(fd);
    fd = open("/sys/class/gpio/gpio21/value", O_RDONLY);
    if (-1 == fd) {
      fprintf(stderr, "Failed to open gpio value for reading!\n");
		return(-1);
    }
    if (-1 == read(fd, value_str, 3)) {
      fprintf(stderr, "Failed to read value!\n");
		return(-1);
    }
    printf(" %d\n", atoi(value_str));
    close(fd);











    usleep(5000);
  } while (ch != 27);


  unexport_Addresses();







  while(read(STDIN_FILENO, &ch, 1)==1);
  tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
  
  return 0;
}

// **************************************************************************************
void export_Addresses() {
  printf("export_Addresses\n");

  printf("   opening...\n");
  int fd = open("/sys/class/gpio/export", O_WRONLY);
  if (fd == -1) {
    perror("Unable to open /sys/class/gpio/export");
    exit(1);
  }

  printf("   16\n");
  if (write(fd, "16", 2) != 2) {
    perror("16: Error writing to /sys/class/gpio/export");
    exit(1);
  }
  printf("   21\n");
  if (write(fd, "21", 2) != 2) {
    perror("21: Error writing to /sys/class/gpio/export");
    exit(1);
  }
  printf("   20\n");
  if (write(fd, "20", 2) != 2) {
    perror("20: Error writing to /sys/class/gpio/export");
    exit(1);
  }
  
  close(fd);
}

void set_direction() {
  printf("set_direction\n");
  // ***
  int fd = open("/sys/class/gpio/gpio16/direction", O_WRONLY);
  if (write(fd, "in", 3) != 3) {
    perror("Error writing to /sys/class/gpio/gpio16/direction");
    exit(1);
  }
  close(fd);
  fd = open("/sys/class/gpio/gpio20/direction", O_WRONLY);
  if (write(fd, "in", 3) != 3) {
    perror("Error writing to /sys/class/gpio/gpio20/direction");
    exit(1);
  }
  close(fd);
  fd = open("/sys/class/gpio/gpio21/direction", O_WRONLY);
  if (write(fd, "in", 3) != 3) {
    perror("Error writing to /sys/class/gpio/gpio21/direction");
    exit(1);
  }
  close(fd);
}

void unexport_Addresses() {
  printf("unexport_Addresses\n");

  printf("   closing...\n");
  int fd = open("/sys/class/gpio/unexport", O_WRONLY);
  if (fd == -1) {
    perror("Unable to open /sys/class/gpio/unexport");
    exit(1);
  }
  // ***
  printf("   16\n");
  if (write(fd, "16", 2) != 2) {
    perror("Error writing to /sys/class/gpio/unexport");
    exit(1);
  }
  printf("   21\n");
  if (write(fd, "21", 2) != 2) {
    perror("Error writing to /sys/class/gpio/unexport");
    exit(1);
  }
  printf("   20\n");
  if (write(fd, "20", 2) != 2) {
    perror("Error writing to /sys/class/gpio/unexport");
    exit(1);
  }











  close(fd);
}












