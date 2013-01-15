/*
 * enable long mode
 */

.globl	longmode
.extern kernpmaplvl4

.text	32

.align	4

	/*
	 * - paging is disabled
	 * - set CR4PAE-bit in CR4
	 * - load CR3 with physical address of PML4 (kernpmaplvl4)
	 * - set EFERLME-bit at EFERADR
	 * - enable paging
	 */
longmode:
	movl	%cr4, %eax
	orl	$CR4PAE, %eax
	movl	%eax, %cr4
	movl	$kernpmaplvl4, %cr3
	movl	EFERADR, %eax
	orl	$EFERLME, %eax
	movl	%eax, EFERADR
	ljmp	$TEXTSEL, $kinit

idtset:
	lidt (idtptr)
	ret

idtptr:
	.short	0x0000
	.long	0x00000000
	
