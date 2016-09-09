/* Change the define to point to the function you want to use */
#define mandel_softlab mandel_sse
//#define mandel_vodnaya mandel_sse

/* The gnu assembler supports C-style macros nicely */
//#define MAX_ITER (1 << 14)
#define MAX_ITER (1 << 16)

/* SSE constants, aligned to prevent access errors */
	.align 16

ones:
.float 1.0, 1.0, 1.0, 1.0
twos:
.float 2.0, 2.0, 2.0, 2.0
fours:
.float 4.0, 4.0, 4.0, 4.0
quarter:
.float 0.25, 0.25, 0.25, 0.25
sixteenth:
.float 0.0625, 0.0625, 0.0625, 0.0625

dones:
.double 1.0, 1.0
dtwos:
.double 2.0, 2.0
dfours:
.double 4.0, 4.0
dquarter:
.double 0.25, 0.25
dsixteenth:
.double 0.0625, 0.0625
	
maxiter:
.long MAX_ITER, MAX_ITER, MAX_ITER, MAX_ITER
dmaxiter:
.quad MAX_ITER, MAX_ITER

.globl mandel_double
.type mandel_double,@function
mandel_double:
	xor      %ecx, %ecx
	movapd   dfours(%rip), %xmm5
	movapd   %xmm0, %xmm6 # real parts
	movapd   %xmm1, %xmm7 # imaginary parts
	
	xorpd    %xmm3, %xmm3 # Clear counters

1:	movapd   %xmm0, %xmm2
	mulpd    %xmm1, %xmm2 # xmm2 = x * y
	mulpd    %xmm0, %xmm0 # xmm0 = x * x
	mulpd    %xmm1, %xmm1 # xmm1 = y * y
	
	movapd   %xmm0, %xmm4
	addpd    %xmm1, %xmm4 # xmm4 = x*x + y*y
	subpd    %xmm1, %xmm0 # xmm0 = x*x - y*y
	addpd    %xmm6, %xmm0 # xmm0 = new real part

	movapd   %xmm2, %xmm1
	addpd	 %xmm1, %xmm1
	addpd    %xmm7, %xmm1 # xmm1 = new imaginary part

	cmplepd  %xmm5, %xmm4
	movapd   %xmm4, %xmm2 # xmm2 is all 1s in non-overflowed pixels
	movmskpd %xmm4, %eax # lower four bits reflect comparison
	
	andpd    dones(%rip), %xmm4
	addpd    %xmm4, %xmm3 # Increment counters

	or       %eax, %eax   # Have they all overflowed?
	jz       2f

	inc      %ecx
	cmp      $MAX_ITER, %ecx # Done all the iterations?
	jnz      1b

2:	cvtps2dq %xmm3, %xmm3 # Done: write counters to memory
	movapd	 %xmm3, (%rdi)
	ret
.size mandel_double, .-mandel_double

.globl mandel_softlab
.type mandel_softlab,@function
mandel_softlab:
	xor     %ecx, %ecx
	movaps  fours(%rip), %xmm5
	movaps  %xmm0, %xmm6 # real parts
	movaps  %xmm1, %xmm7 # imaginary parts
	
	xorps   %xmm3, %xmm3 # Clear counters

1:	movaps  %xmm0, %xmm2
	mulps   %xmm1, %xmm2 # xmm2 = x * y
	mulps   %xmm0, %xmm0 # xmm0 = x * x
	mulps   %xmm1, %xmm1 # xmm1 = y * y
	
	movaps  %xmm0, %xmm4
	addps   %xmm1, %xmm4 # xmm4 = x*x + y*y
	subps   %xmm1, %xmm0 # xmm0 = x*x - y*y
	addps   %xmm6, %xmm0 # xmm0 = new real part

	movaps  %xmm2, %xmm1
	addps	%xmm1, %xmm1
	addps   %xmm7, %xmm1 # xmm1 = new imaginary part

	cmpleps %xmm5, %xmm4
	movaps  %xmm4, %xmm2 # xmm2 is all 1s in non-overflowed pixels
	movmskps %xmm4, %eax # lower four bits reflect comparison
	
	andps   ones(%rip), %xmm4
	addps   %xmm4, %xmm3 # Increment counters

	or      %eax, %eax   # Have they all overflowed?
	jz      2f

	inc     %ecx
	cmp     $MAX_ITER, %ecx # Done all the iterations?
	jnz     1b

2:	cvtps2dq %xmm3, %xmm3 # Done: write counters to memory
	movaps	%xmm3, (%rdi)
	ret
.size mandel_softlab, .-mandel_softlab

.globl mandel_codep1
.type mandel_codep1,@function
mandel_codep1:
	movaps  %xmm0, %xmm4
	xorps	%xmm6, %xmm6
	movaps	%xmm1, %xmm5
	mov     $MAX_ITER, %ecx
	movaps  fours(%rip), %xmm7

1:	movaps  %xmm0, %xmm2
	mulps	%xmm0, %xmm0 # xmm0 = x * x
	movaps  %xmm1, %xmm3
	addps   %xmm1, %xmm1 # xmm1 = 2 * y
	mulps   %xmm2, %xmm1 # xmm1 = 2 * x * y
	movaps  %xmm0, %xmm2 # xmm2 = x * x
	mulps   %xmm3, %xmm3 # xmm3 = y * y
	addps   %xmm5, %xmm1 # xmm1 = new imaginary part
	subps   %xmm3, %xmm0 # xmm0 = x * x - y * y
	addps   %xmm3, %xmm2 # xmm2 = x * x + y * y
	
	cmpleps %xmm7, %xmm2 # xmm2 is all 1s in non-overflowed pixels
	addps   %xmm4, %xmm0 # xmm0 = new real part

	movmskps %xmm2, %eax # lower four bits reflect comparison
	test    %eax, %eax
	jz      2f
	andps   %xmm7, %xmm2
	addps   %xmm2, %xmm6 # Increment counters by 4
	
	sub     $1, %ecx     # Done all the iterations? 
	jnz     1b
	
2:	mulps   quarter(%rip), %xmm6 # Done: write (rescaled) counters to memory
	cvtps2dq %xmm6, %xmm6
	movaps	%xmm6, (%rdi)
	ret
.size mandel_codep1, .-mandel_codep1

.globl mandel_codep2
.type mandel_codep2,@function
mandel_codep2:
	movaps  %xmm0, %xmm4
	xorps   %xmm6, %xmm6
	movaps  %xmm1, %xmm5
	mov     $MAX_ITER, %ecx
	movaps  fours(%rip), %xmm7
	
1:	movaps  %xmm0, %xmm2
	mulps   %xmm1, %xmm2 # xmm2 = x * y
	mulps   %xmm0, %xmm0 # xmm0 = x * x
	mulps   %xmm1, %xmm1 # xmm1 = y * y
	addps   %xmm2, %xmm2 # xmm2 = 2 * x * y
	movaps  %xmm0, %xmm3
	addps   %xmm1, %xmm3 # xmm3 = x * x + y * y
	cmpleps %xmm7, %xmm3 # xmm3 is all 1s in non-overflowed pixels

	movmskps %xmm3, %eax # lower four bits reflect comparison
	test    %eax, %eax
	jz      2f
	
	subps   %xmm1, %xmm0 # xmm0 = x * x - y * y
	movaps  %xmm2, %xmm1
	
	andps   %xmm7, %xmm3 # xmm3 is 4.0 if not overflowed
	addps   %xmm3, %xmm6 # Add to counters
	addps   %xmm4, %xmm0 # xmm0 = new real part
	addps   %xmm5, %xmm1 # xmm1 = new imaginary part
	
	dec     %ecx	     # Done all the iterations? 	
	jnz     1b
	
2:	mulps   quarter(%rip), %xmm6 # Done: write (rescaled) counters to memory
	cvtps2dq %xmm6, %xmm6
	movaps	%xmm6, (%rdi)
	ret
.size mandel_codep2, .-mandel_codep2

.globl mandel_vodnaya
.type mandel_vodnaya,@function
mandel_vodnaya:
	movaps  %xmm0, %xmm4
	xorps   %xmm6, %xmm6
	movaps  %xmm1, %xmm5
	mov     $MAX_ITER, %ecx
	movaps  fours(%rip), %xmm7

1:	movaps  %xmm1, %xmm2
	mulps   %xmm2, %xmm2 # xmm2 = y * y
	mulps   %xmm0, %xmm1 # xmm1 = x * y
	movaps  %xmm2, %xmm3
	mulps   %xmm0, %xmm0 # xmm0 = x * x
	addps   %xmm0, %xmm2 # xmm2 = x * x + y * y
	
	cmpleps %xmm7, %xmm2 # xmm2 is all 1s in non-overflowed pixels
	addps   %xmm1, %xmm1 # xmm1 = 2 * x * y
	subps   %xmm3, %xmm0 # xmm0 = x * x - y * y
	addps   %xmm5, %xmm1 # xmm1 = new imaginary part
	addps   %xmm4, %xmm0 # xmm0 = new real part

	movmskps %xmm2, %eax # lower four bits reflect comparison
	test    %eax, %eax
	jz      2f
	
	andps   %xmm7, %xmm2 # xmm2 is 4.0 is not overflowed
	addps   %xmm2, %xmm6 # Add to counters

	sub     $1, %ecx
	jnz     1b

2:	mulps   quarter(%rip), %xmm6 # Done: write (rescaled) counters to memory
	cvtps2dq %xmm6, %xmm6
	movaps	%xmm6, (%rdi)
	ret
.size mandel_vodnaya, .-mandel_vodnaya

	/* TODO: in case i can do something faster */
.globl mandel_vendu
.type mandel_vendu,@function
mandel_vendu:
	xor     %ecx, %ecx
	movaps  fours(%rip), %xmm5
	movaps  %xmm0, %xmm6 # real parts
	movaps  %xmm1, %xmm7 # imaginary parts
	
	xorps   %xmm3, %xmm3 # Clear counters

1:	movaps  %xmm0, %xmm2
	mulps   %xmm1, %xmm2 # xmm2 = x * y
	mulps   %xmm0, %xmm0 # xmm0 = x * x
	mulps   %xmm1, %xmm1 # xmm1 = y * y
	
	movaps  %xmm0, %xmm4
	addps   %xmm1, %xmm4 # xmm4 = x*x + y*y
	subps   %xmm1, %xmm0 # xmm0 = x*x - y*y
	addps   %xmm6, %xmm0 # xmm0 = new real part

	movaps  %xmm2, %xmm1
	addps	%xmm1, %xmm1
	addps   %xmm7, %xmm1 # xmm1 = new imaginary part

	cmpleps %xmm5, %xmm4
	movaps  %xmm4, %xmm2 # xmm2 is all 1s in non-overflowed pixels
	movmskps %xmm4, %eax # lower four bits reflect comparison
	
	andps   ones(%rip), %xmm4
	addps   %xmm4, %xmm3 # Increment counters

	or      %eax, %eax   # Have they all overflowed?
	jz      2f

	inc     %ecx
	cmp     $MAX_ITER, %ecx # Done all the iterations?
	jnz     1b

2:	cvtps2dq %xmm3, %xmm3 # Done: write counters to memory
	movaps	%xmm3, (%rdi)
	ret
.size mandel_vendu, .-mandel_vendu

