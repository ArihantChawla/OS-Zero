.import	<std.asm>

.align	4096

pglist:	.long	0x00000000
pgcur:	.long	0x00000000
pgnum:	.long	0x00000000
pglk:	.long	0x00000000

.globl	meminit

	// %r0 is start address
meminit:
	mov	%r0, %r1		// start address
	movl	$1048576, %r2		// total # of pages
	mov	%r0, %r4		// start address
	shrl	$12, %r1		// start page #
	sub	%r1, %r2		// # of free pages
	mov	%r2, pgnum
	mov	%r2, %r3
	shl	$2, %r3			// structure size in bytes
	sbrk	%r3
	mov	%r0, pglist
_meminit:	
	mov	%r4, *%r0
	add	$4096, %r4
	add	$4, %r0
	cmp	$0xc0000000, %r4
	bnz	_meminit
	ret

palloc:
	mtx	$1, $pglk
	mov	pgcur, %r1
	shl	$2, %r1
	mtx	$0, $pglk
	
