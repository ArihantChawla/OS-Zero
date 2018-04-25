//msg:	.byte 'h', 'e', 'l', 'l', 'o', '\n', '\0'

.include <stdio.def>

msg:	.asciz	"hello, world\n"

.align	4

_start:
	mov	$msg,   %r0         ; r0 = message-string address
	movb	    *%r0, %r1   ; r1 = first character
	mov	$STDOUT, %r2        ; r2 = file descriptor for stdout
	cmp	$0x00,  %r1         ; check if first character is 0
	bz	        done        ; yes
loop:
	inc	        %r0         ; increment address
	outb	    %r1, %r2    ; write character to stdout
	movb	    *%r0, %r1   ; read new character
	cmp	$0x00,  %r1         ; check if character non-zero
	bnz	loop                ; no
done:
	hlt

