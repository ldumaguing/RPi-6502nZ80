   * = $C000

reset:
   lda #$ff
   sta $6002

loop:
   lda #$55
   sta $6000

   lda #$aa
   sta $6000

   jmp loop
TheEnd

   * = $fffc
   .dsb (* - TheEnd), $ea
   .word reset 
   .word $0000
