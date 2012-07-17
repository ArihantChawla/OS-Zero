.import <bzero.asm>

memzero:
	mov	$12288, %r0	// address
	mov	$4096, %r1	// length
	call	bzero
	hlt

_start:
	thr	$memzero
	hlt

_foo:	.space	4096, 0xff

