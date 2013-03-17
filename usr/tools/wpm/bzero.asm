.globl	bzero
// r0 is address, r1 is length in bytes

bzero:
	mov	%r1, %r2	// count
	shr	$2, %r1		// count >> 2
	and	$3, %r2		// count & 0x03
	mov	%r1, %r3	// count >> 2
	shl	$2, %r3		// count
	mov	%r3, %r1	// copy
	cmp	$0, %r2		// byte-count
	bz	bzerol
//	dec	%r2, %r1
	/* set byte by byte to zero */
bzerob:	
	movb	$0, *%r0
	dec	%r2
	inc	%r0
	cmp	$0, %r2
	bnz	bzerob
	cmp	$0, %r1
	bz	bzerofin
	/* set 32-bit word at a time to zero */
bzerol:
	mov	$0, *%r0
	sub	$4, %r1
	add	$4, %r0
	cmp	$0, %r1
	bnz	bzerol
bzerofin:	
	ret
	
