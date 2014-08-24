#if (APIC)
#include <kern/unit/x86/apic.h>
#endif
#include <kern/unit/x86/pic.h>

#if (APIC)
_APICEOIADR	= APICEOIADR
#endif

#if (APIC)
.macro	_eoi
	/* send APIC EOI (end of interrupt) */
	movl	$0x00000000, _APICEOIADR
	/* write zero to APIC task priority register - TODO: is this necessary? */
//	movl	$0x00000000, 0xfee00080
.endm
#endif /* APIC */

.macro	_eoi1
	/* send EOI to the PIC1 */
	movb	$0x20, %al
	outb	%al, $PIC1
.endm
.macro	_eoi2
	/* send EOI to PIC2 */
	outb	%al, $PIC2
.endm

