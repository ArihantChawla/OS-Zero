.include <hook.def>
.import <bzero.asm>

memalloc:	
	mov	$16384, %r0
	hook	$PALLOC
	mov	%r0, ptr
	ret

memzero:
	mov	$65536, %r0	// address
	mov	$4, %r1		// # of pages
	hook	$0
	hlt

zero:
	mov	ptr, %r0
	mov	$4096, %r1
	hook	$0
	ret

memfree:
	mov	ptr, %r0
	hook	$2
	ret

_start:
	thr	$memzero
	call	memalloc
	call	zero
	call	memfree
	hlt

ptr:	.long	0x00000000

_foo:	.space	4096, 0xff

