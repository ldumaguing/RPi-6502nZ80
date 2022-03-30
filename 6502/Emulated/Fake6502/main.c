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

    c.mem[0xc000] = 0xa9;
    c.mem[0xc001] = 0xff;
    c.mem[0xc002] = 0x8d;
    c.mem[0xc003] = 0x02;
    c.mem[0xc004] = 0x60;
    c.mem[0xc005] = 0xa9;
    c.mem[0xc006] = 0x55;
    c.mem[0xc007] = 0x8d;

    c.mem[0xc008] = 0x00;
    c.mem[0xc009] = 0x60;
    c.mem[0xc00a] = 0xa9;
    c.mem[0xc00b] = 0xaa;
    c.mem[0xc00c] = 0x8d;
    c.mem[0xc00d] = 0x00;
    c.mem[0xc00e] = 0x60;
    c.mem[0xc00f] = 0x4c;

    c.mem[0xc010] = 0x05;
    c.mem[0xc011] = 0xc0;

    // reset vector
    c.mem[0xfffc] = 0x00;
    c.mem[0xfffd] = 0xc0;


    printf ( "%x.%x.%x.%x %x.%x.%x.%x\n", c.a, c.x, c.y, c.flags, c.clockticks, c.ea, c.opcode, c.pc );
    reset6502 ( &c );
    printf ( "\n" );
    for ( int i; i<100; i++ ) {
        printf ( "%x.%x.%x.%x %x.%x.%x.%x\n", c.a, c.x, c.y, c.flags, c.clockticks, c.ea, c.opcode, c.pc );
        step ( &c );
    }
    printf ( "%x.%x.%x.%x %x.%x.%x.%x\n", c.a, c.x, c.y, c.flags, c.clockticks, c.ea, c.opcode, c.pc );
    printf ( "%x.%x\n", c.mem[0xfffc], c.mem[0xfffd] );


    return 0;
}
