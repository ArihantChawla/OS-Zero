val:	.long	0xb4b4b4b4
_start:	
	mov	val, %r0
	mov	$0x04, %r1
loop:	
	inc	%r0
	dec	%r1
	cmp	$0x00, %r1
	bnz	loop
done:
	mov	%r0, val
	hlt
