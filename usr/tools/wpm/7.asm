val:	.long	0xb4b5b6b7
_start:	
	mov	val, %r0
	mov	$0x04, %r1
loop:	
	shrl	$4, %r0
	dec	%r1
	cmp	$0x00, %r1
	bnz	loop
done:
	mov	%r0, val
	hlt
