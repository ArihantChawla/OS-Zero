#define __ASSEMBLER__ 1
#include <kern/conf.h>
#include <kern/unit/x86/bios.h>
#include <kern/unit/x86/boot.h>

/* GRUB support */
_MBMAGIC	= MBMAGIC
_MBFLAGS	= MBFLAGS
_MBCHKSUM	= MBCHKSUM

/* globals */
.globl	_start, start
#if !(LOCORE)
.globl	kernidt
#endif
.globl	kerngdt, kernpagedir, kerniomap, kernpagetab4, kerntsstab
.globl	kernsysstktab, kernusrstktab
.extern	kmain
	
.text	32

/* kernel entry */
_start:
start:
	cli		// disable interrupts
	movw	$BIOSWARMREBOOT, BIOSREBOOTMODE
	jmp	_start2	// flush CPU pipeline

.align	4

/* MULTIBOOT header; must be in first 8 kilobytes of kernel image */
mboothdr:
.long	_MBMAGIC
.long	_MBFLAGS
.long	_MBCHKSUM
.long	0		// header_addr
.long	0		// load_addr
.long	0		// load_end_addr
.long	0		// bss_end_addr
.long	0		// entry_addr
.long	0		// video mode type (linear)
.long	GFXWIDTH	// video width
.long	GFXHEIGHT	// video height
.long	GFXDEPTH	// video depth

	/* kernel startup */
_start2:
	/* set kernel stack up */
	movl	$BOOTSTKTOP, %esp
#if 0
	movl	$kernsysstktab, %eax
	addl	$(NCPU * KERNSTKSIZE), %eax
	movl	%eax, %esp
#endif
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx			// store boot-header address
#if (LONGMODE)
	call	_chkcpuid
	call	_chklongmode
	cmp	$0, %eax
	je	_protmode
	movl	$1, 4(%esp)		// longmode argument for kmain()
_protmode:
#else
	xorl	%eax, %eax
#endif
	pushl	%eax			// store longmode argument for kmain()
	call	kmain			// call kmain()

/* check for presence of CPUID by trying to flip bit 21 in %eflags */
_chkcpuid:	
	pushfl
	popl	%eax
	movl	%eax, %ecx
	xorl	$(1 << 21), %eax
	pushl	%eax
	popfl
	pushfl
	popl	%eax
	pushl	%ecx
	popfl
	xorl	%ecx, %eax
	ret

_chklongmode:
	movl	$(1 << 31), %eax
	cpuid
	cmp	$((1 << 31) | 1), %eax
	jb	_nolongmode
	movl	$1, %eax
	ret
_nolongmode:
	movl	$0, %eax
	ret

.data
	
.align	NBPG

#if !(LOCORE)
/* IDT; interrupt descriptor table; shared between processors */
kernidt:
	.space	NBPG
/* page directory index page */
kernpagedir:
	.space	NBPG

#if defined(__x86_64__) || defined(__amd64__)
	
/* 64-bit memory management */

/* page-map level-4 table */
kernpagetab4:	
	.space	NBPG
/* page directory pointer table */
kernpagetab3:	
	.space	NBPG
/* page directory table */
kernpagetab2:	
	.space	NBPG
/* page table */
kernpagetab1:
	.space	NBPG

#endif /* 64-bit */
#endif /* !LOCORE */

/* per-CPU GDTs; kernel segment descriptor tables */
.align	4096
	
#if (SMP)

kerngdt:
#if (NGDT < 16)
	.space	(16 * 8 * NCPU)
#else
	.space	(8 * NGDT * NCPU)
#endif

#else
kerngdt:
	.space	(8 * NGDT)
#endif /* SMP */
	
/* kernel TSS structures */
kerntsstab:
	.space	(NCPU * TSSLEN)
	
	.align	NBPG

/* kernel I/O protection bitmap */
kerniomap:
	.space	8192

/* kernel stacks */
kernsysstktab:
	.space	(NCPU * KERNSTKSIZE)
kernusrstktab:
	.space	(NCPU * KERNSTKSIZE)

