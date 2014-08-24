#if (APIC)
#include <kern/unit/x86/apic.h>
#endif
#include <kern/unit/x86/pic.h>

#if (APIC)
_APICEOIADR	= APICEOIADR
#endif

/* INTERRUPT MANAGEMENT */

.macro	_trapsaveregs
	pushal
//	pushl	%ds
//	pushl	%es
//	pushl	%fs
.endm
.macro	_traploadregs
//	popl	%fs
//	popl	%es
//	popl	%ds
	popal
.endm
.macro	_traploadregserr
//	popl	%fs
//	popl	%es
//	popl	%ds
	popal
	addl	$4, %esp	// skip error code
.endm

	#if (APIC)
.macro	_apiceoi
	/* send APIC EOI (end of interrupt) */
	movl	$0x00000000, _APICEOIADR
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

