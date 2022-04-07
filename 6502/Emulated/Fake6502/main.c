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
    uint8_t *ROM = &c.mem[0x0200];

    FILE* fp;
    if ( argc < 2 )
        fp = fopen ( "rom.bin", "rb" );
    else
        fp = fopen ( argv[1], "rb" );
    fread ( ROM, ROM_size, 1, fp );
    fclose ( fp );

    // reset vector
    c.mem[0xfffc] = 0x00;
    c.mem[0xfffd] = 0x02;

    // ************************************************************************ PPU
    // ********** HexTileWindow
    c.mem[0x9f00] = 0;   // x
    c.mem[0x9f01] = 0;   // y
    c.mem[0x9f02] = 0;   // width
    c.mem[0x9f03] = 0;   // height
    c.mem[0x9f04] = 0;   // Signals:
                         //    0 - PPU waiting
                         //    1 - Hey PPU, refresh.
                         //    2 - PPU refreshing.

    reset6502 ( &c );
    printf ( "start\n" );
    for ( ;; ) {
        printf ( "%04x ", c.pc );
        step ( &c );
        printf ( "%02x% 20x\n", c.opcode, c.mem[0x9f00] );
        // printf ( "%x.%x.%x.%x %x.%x.%x", c.a, c.x, c.y, c.flags, c.pc, c.opcode, c.ea );

       // printf ( " %d\n", c.clockticks );
      //  c.clockticks = 0;

        /*
            while ( c.clockticks > 0 ) {
            printf ( ".\n" );
            c.clockticks--;
        }
        */




    }


    return 0;
}
