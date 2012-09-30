#define __ASSEMBLY__ 1
#include <kern/ia32/boot.h>

/* GRUB support */
_MBMAGIC     = MBMAGIC
_MBFLAGS     = MBFLAGS
_MBCHKSUM    = MBCHKSUM

/* globals */
.globl	_start, start
.globl	kernidt, kerngdt, kernpagedir, kerniomap
	
.text	32

/* kernel entry */
_start:
start:
	cli		// disable interrupts
	jmp	_start2	// flush CPU pipeline

.align	4

/* MULTIBOOT header; must be in first 8 kilobytes of kernel image */
mboothdr:
.long	_MBMAGIC
.long	_MBFLAGS
.long	_MBCHKSUM

	/* kernel startup */
_start2:
	/* set kernel stack up */
	movl	$KERNSTKTOP, %esp
	movl	%esp, %ebp
	call	kmain	// call kmain

.align	NBPG

/* IDT; interrupt descriptor table; shared between processors */
kernidt:
	.space	NBPG
/* per-CPU GDTs; kernel segment descriptor tables */
kerngdt:
	.space	NBPG
/* page directory index page */
kernpagedir:
	.space	NBPG
/* kernel I/O protection bitmap */
kerniomap:
	.space	8192

