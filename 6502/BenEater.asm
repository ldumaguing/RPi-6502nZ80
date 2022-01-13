   * = $8000

reset:
   lda #$ff
   sta $6002

   lda #$50
   sta $6000

loop:
   ror
   sta $6000

   jmp loop
TheEnd

   * = $fffc
   .dsb (* - TheEnd), 0
   .word reset 
   .word $0000
