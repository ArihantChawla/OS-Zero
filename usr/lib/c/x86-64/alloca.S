#if defined(__x86_64__) || defined(__amd64__)

.globl alloca

.text 64

/*
 * registers at call time
 * ----------------------
 * rdi	- size argument
 *
 * stack at call time
 * ------------------
 * return address <- stack pointer
 */
alloca:	
	subq	$8, %rdi	// adjust for popped return address
	movq 	%rsp, %rax	// copy stack pointer
	subq	%rdi, %rax	// reserve space; return value is in RAX
	ret			// return

#endif /* __x86_64__ || __amd64__ */

