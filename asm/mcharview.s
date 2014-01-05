
include "6510.s"
include "6510illegal.s"
include "word.s"
include "utils.s"
include "vic2.s"
include "startup.s"

		@ = $1f00

start:	sei
		lda border
		sta $d020
		lda background
		sta $d021
		lda ext0
		sta $d022
		lda ext1
		sta $d023
		set_d018 font, screen
		set_d016 0, 1, 1
		
		ldx #0
.b: 	lda color+256*0,x
		sta $d800,x
		lda color+256*1,x
		sta $d900,x
		lda color+256*2,x
		sta $da00,x
		lda color+256*3,x
		sta $db00,x
		inx
		bne .b
		
.dead:	beq .dead

border:
		byte 0
background:
		byte 0
ext0:
		byte 0
ext1:
		byte 0
		
		@ = $2000
screen:

		@ = $2400
color:
		
		@ = $2800
font:

end:
