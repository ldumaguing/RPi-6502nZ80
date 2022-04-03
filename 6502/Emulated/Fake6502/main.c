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


int main ( int argc, char *argv[] ) {
    context_t c;

    c.mem[0xc000] = 0xea;
    c.mem[0xc001] = 0xea;
    c.mem[0xc002] = 0x07;
    c.mem[0xc003] = 0x12;
    c.mem[0xc004] = 0x1a;
    c.mem[0xc005] = 0x4c;
    c.mem[0xc006] = 0x00;
    c.mem[0xc007] = 0xc0;
/*
    c.mem[0xc008] = 0x4c;
    c.mem[0xc009] = 0x00;
    c.mem[0xc00a] = 0xc0;
    c.mem[0xc00b] = 0xea;
    c.mem[0xc00c] = 0xea;
    c.mem[0xc00d] = 0x1a;
    c.mem[0xc00e] = 0x4c;
    c.mem[0xc00f] = 0x00;

    c.mem[0xc010] = 0xc0;
*/

    // test byte
    c.mem[0x0012] = 13;

    // reset vector
    c.mem[0xfffc] = 0x00;
    c.mem[0xfffd] = 0xc0;

printf(">>>>>>>>>>> %x\n", c.mem[0x0012]);
    //printf ( "%x.%x.%x.%x %x.%x.%x.%x\n", c.a, c.x, c.y, c.flags, c.clockticks, c.ea, c.opcode, c.pc );
    reset6502 ( &c );
    printf ( "start\n" );
    for ( int i; i<20; i++ ) {
        printf ( "%x.%x.%x.%x %x.%x.%x.%x\n", c.a, c.x, c.y, c.flags, c.clockticks, c.ea, c.opcode, c.pc );
        step ( &c );
    }
    printf ( "%x.%x.%x.%x %x.%x.%x.%x\n", c.a, c.x, c.y, c.flags, c.clockticks, c.ea, c.opcode, c.pc );
    printf ( "%x.%x\n", c.mem[0xfffc], c.mem[0xfffd] );

printf(">>>>>>>>>>> %x\n", c.mem[0x0012]);
    return 0;
}
