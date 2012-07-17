_start:
	mov	val, %r0
	not	%r0
	mov	val, %r0
	mov	bits, %r1
	and	%r0, %r1
	mov	bits, %r1
	or	%r0, %r1
	mov	bits, %r1
	xor	%r0, %r1
	mov	bits, %r1
	add	%r0, %r1
	mov	bits, %r1
	sub	%r0, %r1
	hlt

val:	.long 0x80000001
bits:	.long 0xffffffff
