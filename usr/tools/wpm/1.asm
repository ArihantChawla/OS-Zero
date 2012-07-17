_val:
	.long	-0xb4b4b4b4, 0xb5b5b5b5
	.byte	0xa7, '\033', 'c'
_start:
	mov	$_val, %r0
	mov	$0xb6b6b6b6, 4(%r0)
	hlt


