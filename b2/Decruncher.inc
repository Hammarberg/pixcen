; ByteBoozer Decruncher    /HCL May.2003
; B2 Decruncher            December 2014

; call: Y = AddrLo
;       X = AddrHi

;Variables..        #Bytes
zp_base	= $02       ; -
bits	= zp_base   ;1
put	= zp_base+2 ;2

#macro	GetNextBit() {.(
	asl bits
	bne DgEnd
	jsr GetNewBits
DgEnd
.)}

#macro	GetLen() {.(
	lda #1
GlLoop
	.GetNextBit()
	bcc GlEnd
	.GetNextBit()
	rol
	bpl GlLoop
GlEnd
.)}

Decrunch
	sty Get1+1
	sty Get2+1
	sty Get3+1
	stx Get1+2
	stx Get2+2
	stx Get3+2

	ldx #0
	jsr GetNewBits
	sty put-1,x
	cpx #2
	bcc *-7
	lda #$80
	sta bits
DLoop
	.GetNextBit()
	bcs Match
Literal
	; Literal run.. get length.
	.GetLen()
	sta LLen+1

	ldy #0
LLoop
Get3	lda $feed,x
	inx
	bne *+5
	jsr GnbInc
L1	sta (put),y
	iny
LLen	cpy #0
	bne LLoop

	clc
	tya
	adc put
	sta put
	bcc *+4
	inc put+1

	iny
	beq DLoop

	; Has to continue with a match..

Match
	; Match.. get length.
	.GetLen()
	sta MLen+1

	; Length 255 -> EOF
	cmp #$ff
	beq End

	; Get num bits
	cmp #2
	lda #0
	rol
	.GetNextBit()
	rol
	.GetNextBit()
	rol
	tay
	lda Tab,y
	beq M8

	; Get bits < 8
M_1	.GetNextBit()
	rol
	bcs M_1
	bmi MShort
M8
	; Get byte
	eor #$ff
	tay
Get2	lda $feed,x
	inx
	bne *+5
	jsr GnbInc
	jmp Mdone
MShort
	ldy #$ff
Mdone
	;clc
	adc put
	sta MLda+1
	tya
	adc put+1
	sta MLda+2

	ldy #$ff
MLoop	iny
MLda	lda $beef,y
	sta (put),y
MLen	cpy #0
	bne MLoop

	;sec
	tya
	adc put
	sta put
	bcc *+4
	inc put+1

	jmp DLoop

End	rts

GetNewBits
Get1	ldy $feed,x
	sty bits
	rol bits
	inx
	bne GnbEnd
GnbInc	inc Get1+2
	inc Get2+2
	inc Get3+2
GnbEnd
	rts

Tab
	; Short offsets
	.byte %11011111 ; 3
	.byte %11111011 ; 6
	.byte %00000000 ; 8
	.byte %10000000 ; 10
	; Long offsets
	.byte %11101111 ; 4
	.byte %11111101 ; 7
	.byte %10000000 ; 10
	.byte %11110000 ; 13
