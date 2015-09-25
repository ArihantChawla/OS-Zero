#define __ASSEMBLER__ 1
#include <kern/conf.h>
#include <kern/unit/ia32/boot.h>

/* GRUB support */
_MBMAGIC     = MBMAGIC
_MBFLAGS     = MBFLAGS
_MBCHKSUM    = MBCHKSUM

/* globals */
.globl	_start, start
#if !(LOCORE)
.globl	kernidt
#endif
.globl	kerngdt, kernpagedir, kerniomap
.extern	kinit
	
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
.long	0	// header_addr
.long	0	// load_addr
.long	0	// load_end_addr
.long	0	// bss_end_addr
.long	0	// entry_addr
.long	0	// video mode type (linear)
.long	GFXWIDTH	// video width
.long	GFXHEIGHT	// video height
.long	GFXDEPTH	// video depth

	/* kernel startup */
_start2:
	/* set kernel stack up */
	movl	$KERNSTKTOP, %esp
	movl	%esp, %ebp
	call	kinit	// call kinit()

.align	NBPG

#if !(LOCORE)
/* IDT; interrupt descriptor table; shared between processors */
kernidt:
	.space	NBPG
/* page directory index page */
kernpagedir:
	.space	NBPG
/* kernel I/O protection bitmap */
kerniomap:
	.space	8192
#endif

/* per-CPU GDTs; kernel segment descriptor tables */
.align	4096
	
#if (SMP)

kerngdt:
#if (NGDT < 16)
	.space	(NCPU * 16)
#else
#error fix GDT size in boot.S
#endif

#else
kerngdt:
	.space	(8 * NGDT)

#endif /* SMP */
	
