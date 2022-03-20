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

#include <math.h>

int clk, fi2c, Address, RWB = 0;


#define sleep usleep ( 50000 * 3 )

#define ROM_size 16384
#define RAM_size 49152
#define starting_ROM_address 49152   // C000
#define sixtyfourK 65536
unsigned char ROM[ROM_size];
unsigned char RAM[RAM_size];


// **************************************************************************************
void TIC()
{
    printf("\nTic:");
}

// --------------------------------------------------------------------------------------
void TOC()
{
    printf("\nToc:");
}

// **************************************************************************************
void loadROM()
{
    printf("Loading ROM...\n");
    FILE* fp;
    fp = fopen("rom.bin", "rb");
    fread(ROM, ROM_size, 1, fp);
    fclose(fp);
}

// **************************************************************************************
void CPU_Write()
{
}

// **************************************************************************************
void CPU_Read()
{
}

// **************************************************************************************
void get_Address()
{
}

// **************************************************************************************
void get_Signals()
{
}

// **************************************************************************************
void HardwarePhase()
{
    TOC();
    get_Signals();
    get_Address();
    sleep;


    if (RWB) {
        CPU_Read();
    } else {
        CPU_Write();
    }
    sleep;
}

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

    loadROM();



    printf("Press [ESC] to quit.\n");
    if (clk == -1) {
        perror("EEK: Unable to open clock.");
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
                HardwarePhase();
                again = 0;
            }
            continue;
        }

        HardwarePhase();

    } while (ch != 27);



    while (read(STDIN_FILENO, &ch, 1) == 1);
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);

    return 0;
}






