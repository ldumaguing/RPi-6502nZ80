   org 0x0000
   nop
   nop
   nop
   nop

fish:
   ld   a, 0xD6
   ld  (0x58), a
   nop
   nop
   jp fish


