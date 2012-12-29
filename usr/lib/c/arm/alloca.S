#if defined(__arm__)

.globl alloca

.text 32

/*
 * registers at call time
 * ----------------------
 * r0	- size argument
 * sp	- stack pointer
 * lr	- return address
 */
alloca:
	sub	sp, sp, r0	// subtract size from stack pointer
	mov	r0, sp		// set return value
	bx	lr		// return

#endif /* __arm__ */

