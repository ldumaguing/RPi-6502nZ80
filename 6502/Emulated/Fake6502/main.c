#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "Myne/myne.h"

void mem_write ( context_t *c, uint16_t addr, uint8_t val ) {
    c->mem[addr] = val;
}

uint8_t mem_read ( context_t *c, uint16_t addr ) {
    return c->mem[addr];
}

#define ROM_size 16384

// **************************************************************************************
int main ( int argc, char *argv[] ) {
    context_t c;
    uint8_t *ROM = &c.mem[0xc000];
   
    FILE* fp;
    if (argc < 2)
        fp = fopen ( "rom.bin", "rb" );
    else
        fp = fopen ( argv[1], "rb" );
    fread ( ROM, ROM_size, 1, fp );
    fclose ( fp );

    // reset vector
    c.mem[0xfffc] = 0x00;
    c.mem[0xfffd] = 0xc0;




    reset6502 ( &c );
    printf ( "start\n" );
    for ( ;; ) {
        printf ( "%x.%x.%x.%x %x.%x.%x\n", c.a, c.x, c.y, c.flags, c.pc, c.opcode, c.ea );
        step ( &c );
    }


    return 0;
}
