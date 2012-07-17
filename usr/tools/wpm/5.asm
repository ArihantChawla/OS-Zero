//msg:	.byte 'h', 'e', 'l', 'l', 'o', '\n', '\0'
msg:	.asciz	"hello, world\n"

.align	4
	
_start:	
	mov	$msg, %r0
	movb	*%r0, %r1
	mov	$0x01, %r2
	cmp	$0x00, %r1
	bz	done
loop:
	inc	%r0
	outb	%r1, %r2
	movb	*%r0, %r1
	cmp	$0x00, %r1
	bnz	loop
done:
	hlt
