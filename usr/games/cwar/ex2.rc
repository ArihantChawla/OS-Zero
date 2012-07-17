/*
 * example code to overwrite memory except self.
 * need to work on the cwar programming model.
 */

_start:
	wpt	-4(%pc)
	wpt	-8(%pc)
	wpt	-12(%pc)
	wpt	-16(%pc)
	wpt	-20(%pc)
	wpt	-24(%pc)
	wpt	-28(%pc)
	wpt	-32(%pc)
	cld
	mov	$_end, %edi
	mov	$_start, %esi
	mov	%edi, %ecx
	subl	%esi, %ecx
	shr	%ecx, $2
_loop:
	cmp	$0, %ecx
	je	_end
	mov	*%esi, *%edi
	add	$4, %esi
	add	$4, %edi
	dec	%ecx
	jmp	_loop
_end:

