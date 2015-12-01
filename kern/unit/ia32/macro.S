#define __ASSEMBLER__ 1
#include <kern/conf.h>
#if (APIC)
#include <kern/unit/x86/apic.h>
#endif
#include <kern/unit/x86/pic.h>

#if (APIC)
_APICEOIADR	= APICEOIADR
_APICEOI	= 0x00000000
#endif

/* INTERRUPT MANAGEMENT */

.macro	_trapenter
	pushal
#if 0
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%ss
#endif
.endm
.macro	_trapleave
#if 0
	popl	%ss
	popl	%fs
	popl	%es
	popl	%ds
#endif
	popal
.endm
.macro	_trapentererr
	pushal
	movl	32(%esp), %ecx	// move error code to %ecx
#if 0
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%ss
#endif
.endm
.macro	_trapleaveerr
#if 0
	popl	%ss
	popl	%fs
	popl	%es
	popl	%ds
#endif
	/* TODO: check to see if we need to schedule another thread */
	popal
	addl	$4, %esp	// skip error code
.endm

#if (APIC)
.macro	_apiceoi
	/* send APIC EOI (end of interrupt) */
	movl	$_APICEOI, _APICEOIADR
.endm
#endif /* APIC */

.macro	_piteoi1
	/* send EOI to PIC1 */
	movb	$PICEOI, %al
	outb	%al, $PIC1
.endm
.macro	_piteoi2
	/* send EOI to PIC2 */
	outb	%al, $PIC2
.endm

