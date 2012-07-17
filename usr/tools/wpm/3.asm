_start:	
	mov	val, %r0
	mov	$-0x04, %r1
	mov	%r1, val
loop:	
	inc	%r0
	inc	%r1
	cmp	$0, %r1
	bnz	loop
done:
	mov	%r0, val
	hlt

val:
	.long	0xb4b4b4b4

