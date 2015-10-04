#define __ASSEMBLER__ 1
#include <kern/conf.h>
#include <kern/unit/x86/boot.h>

.globl	gdtinit, pginit, idtset, idtptr
#if (REENTRANTGDTINIT)
.globl	realgdtptr	
.globl	gdtptrtab
#else
.globl	gdtptr
#endif
.extern	kernpagedir

.text	32

.align	4

	/*
	 * initialize segmentation.
	 * - load segment registers DS, ES, FS, and SS with the DATA selector
	 * - GS is per-CPU segment
	 * - set code segment register CS up with a far jump
	 */
gdtinit:
#if (NEWCLI)
	cli
#endif
#if (REENTRANTGDTINIT)
	lgdt	(%eax)
#else
	lgdt	(gdtptr)
#endif
	movw	$DATASEL, %ax
	movw	$CPUSEL, %bx
//	movw	$STKSEL, %bx
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
//	movw	%bx, %ss
	movw	%ax, %ss
	movw	%bx, %gs
	ljmp	$TEXTSEL, $_gdtret
_gdtret:
#if !(NEWCLI)
	sti
#endif
	ret

	/*
	 * initialize paging.
	 * - load page directory physical address into CR3
	 * - set the PG-bit in CR0
	 * - finish with a near jump
	 */
pginit:
	movl	$kernpagedir, %eax
	movl	%eax, %cr3
	movl	%cr0, %eax
	orl	$CR0PG, %eax
	movl	%eax, %cr0
	jmp	_pgret
_pgret:
	ret

idtset:
	lidt	(idtptr)
	ret

.align	8
	
#if (REENTRANTGDTINIT)
realgdtptr:
	.short	0x0000
	.long	0x00000000
gdtptrtab:
	.space	8 * NCPU
#else
gdtptr:
	.short	0x0000
	.long	0x00000000
#endif
idtptr:
	.short	0x0000
	.long	0x00000000

