.globl	bzero
// r0 is address, r1 is length in bytes

bzero:
	shr	$2, %r1
	mov	$0, *%r0
	dec	%r1
	inc	%r0
	cmp	$0, %r1
	bnz	bzero
	ret
	
