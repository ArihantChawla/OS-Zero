#define __ASSEMBLY__ 1
#include <kern/unit/ia32/boot.h>
#include <kern/syscallnum.h>
#include <errno.h>

/*
 * - pass system call number in EAX
 * - up to 3 arguments in EBX, ECX, and EDX
 * - set carry-flag on error return; EAX stores errno
 * - if carry not set, EAX is return value
 */

/*
 * TODO
 * ----
 * - fix _syscall to deliver pending unmasked signals
 * - make _syscall check for another thread to run
 * - do system call profiling
 */

.globl	syscall, sysvec, errno

.text	32

syscall:
	pushal
	movl	%esp, %ebp
	movl	$_sysfunc, %edi
	movl	(%edi, %eax, 4), %esi
	call	*%esi
	
_syscall0:
	movl	$sysvec, %edi
	cmpl	$0, %eax
	jle	_invlsys
	cmpl	$NSYSCALL, %eax
	jge	_invlsys
	movl	(%edi, %eax, 4), %esi
	call	*%esi
	/* finish system call */
	jnc	_success
	jmp	_failure

_syscall1:
	movl	$sysvec, %edi
	cmpl	$0, %eax
	jle	_invlsys
	cmpl	$NSYSCALL, %eax
	jge	_invlsys
	pushl	%ebx
	movl	(%edi, %eax, 4), %esi
	call	*%esi
	/* finish system call */
	jnc	_success
	jmp	_failure

_syscall2:
	movl	$sysvec, %edi
	cmpl	$0, %eax
	jle	_invlsys
	cmpl	$NSYSCALL, %eax
	jge	_invlsys
	pushl	%ecx
	pushl	%ebx
	movl	(%edi, %eax, 4), %esi
	call	*%esi
	/* finish system call */
	jnc	_success
	jmp	_failure

_syscall3:
	movl	$sysvec, %edi
	cmpl	$0, %eax
	jle	_invlsys
	cmpl	$NSYSCALL, %eax
	jge	_invlsys
	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	movl	(%edi, %eax, 4), %esi
	call	*%esi
	/* finish system call */
	jnc	_success
	jmp	_failure

_invlsys:
	movl	$ENOSYS, errno
	movl	%ebp, %esp
	popal
	movl	$-1, %eax
	sti
	iret

_failure:
//	cmpl	$EINTR, %eax
//	je	_intr
	movl	%ebp, %esp
	movl	%eax, errno
	popal
	movl	$-1, %eax
	sti
	iret

#if 0
_intr:
	movl	%eax, errno
	/* pop actual return value */
	popl	%eax
	movl	%ebp, %esp
	movl	%eax, retval
	popal
	movl	retval, %eax
	sti
	iret
#endif

_success:	
	movl	%eax, retval
	movl	%ebp, %esp
	popal
	movl	retval, %eax
	sti
	iret

.data

errno:
	.long	0x00000000

retval:
	.long	0x00000000

.align NBPG

_sysfunc:
	.long	0x00000000
	.long	_syscall3	// sysctl
	.long	_syscall2	// exit
	.long	_syscall0	// abort
	.long	_syscall1	// fork
	.long	_syscall3	// exec
	.long	_syscall3	// throp
	.long	_syscall3	// pctl
	.long	_syscall3	// sigop
	.long	_syscall1	// brk
	.long	_syscall3	// map
	.long	_syscall2	// umap
	.long	_syscall3	// shmget
	.long	_syscall3	// shmat
	.long	_syscall1	// shmdt
	.long	_syscall3	// shmctl
	.long	_syscall3	// mnt
	.long	_syscall2	// umnt
	.long	_syscall3	// readdir
	.long	_syscall3	// open
	.long	_syscall2	// trunc
	.long	_syscall1	// close
	.long	_syscall3	// read
	.long	_syscall3	// readv
	.long	_syscall3	// write
	.long	_syscall3	// writev
	.long	_syscall3	// seek
	.long	_syscall3	// falloc
	.long	_syscall3	// stat
	.long	_syscall3	// fhint
	.long	_syscall3	// ioctl
	.long	_syscall3	// fctl
	.long	_syscall3	// poll
	.long	_syscall2	// select
	.long	_syscall0	// seminit
	.long	_syscall1	// semup
	.long	_syscall1	// semdown
	.long	_syscall3	// semop
	.long	_syscall3	// mqinit
	.long	_syscall3	// mqsend
	.long	_syscall3	// mqrecv
	.long	_syscall3	// mqop
	.long	_syscall3	// evreg
	.long	_syscall3	// evsend
	.long	_syscall3	// evrecv
	.long	_syscall3	// evop
sysvec:
	/* TODO: fill with sys_ function addresses */
	.space	4 * NSYSCALL

