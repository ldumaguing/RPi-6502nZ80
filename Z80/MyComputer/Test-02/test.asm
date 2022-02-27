Start:
   ld b, $69
   ld a, b
   ld ($1234), a 
   inc a
   jp Start
   ld a, $41
   ld a, $42
   ld a, $43

