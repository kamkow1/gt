#define GT_ASSEMBLY
#include "gt.h"

#if GT_PLATFORM == GT_PLATFORM_LINUX_x86_64

    .global gt_get_context
    .align 2
    .type gt_get_context, @function
    gt_get_context:
     /* copy registers */
        movq    %r8,  GT_REG_OFFSET(GT_REG_R8) (%rdi)
        movq	%r9,  GT_REG_OFFSET(GT_REG_R9) (%rdi)
        movq	%r10, GT_REG_OFFSET(GT_REG_R10)(%rdi)
        movq	%r11, GT_REG_OFFSET(GT_REG_R11)(%rdi)
        movq	%r12, GT_REG_OFFSET(GT_REG_R12)(%rdi)
        movq	%r13, GT_REG_OFFSET(GT_REG_R13)(%rdi)
        movq	%r14, GT_REG_OFFSET(GT_REG_R14)(%rdi)
        movq	%r15, GT_REG_OFFSET(GT_REG_R15)(%rdi)
        movq	%rdi, GT_REG_OFFSET(GT_REG_RDI)(%rdi)
        movq	%rsi, GT_REG_OFFSET(GT_REG_RSI)(%rdi)
        movq	%rbp, GT_REG_OFFSET(GT_REG_RBP)(%rdi)
        movq	%rbx, GT_REG_OFFSET(GT_REG_RBX)(%rdi)
        movq	%rdx, GT_REG_OFFSET(GT_REG_RDX)(%rdi)
        movq	%rax, GT_REG_OFFSET(GT_REG_RAX)(%rdi)
        movq	%rcx, GT_REG_OFFSET(GT_REG_RCX)(%rdi)

        movq    (%rsp), %rcx
        movq    %rcx, GT_REG_OFFSET(GT_REG_RIP)(%rdi)

        leaq    8(%rsp), %rcx
        movq    %rcx, GT_REG_OFFSET(GT_REG_RSP)(%rdi)

     /* return 0 */
        xorl %eax, %eax
        ret
    .size gt_get_context, . - gt_get_context

    .global gt_set_context
    .align 2
    .type gt_set_context, @function
    gt_set_context:
     /* set registers */
        movq	GT_REG_OFFSET(GT_REG_R8)(%rdi), %r8
        movq	GT_REG_OFFSET(GT_REG_R9)(%rdi), %r9
        movq	GT_REG_OFFSET(GT_REG_R10)(%rdi), %r10
        movq	GT_REG_OFFSET(GT_REG_R11)(%rdi), %r11
        movq	GT_REG_OFFSET(GT_REG_R12)(%rdi), %r12
        movq	GT_REG_OFFSET(GT_REG_R13)(%rdi), %r13
        movq	GT_REG_OFFSET(GT_REG_R14)(%rdi), %r14
        movq	GT_REG_OFFSET(GT_REG_R15)(%rdi), %r15
        movq	GT_REG_OFFSET(GT_REG_RSI)(%rdi), %rsi
        movq	GT_REG_OFFSET(GT_REG_RBP)(%rdi), %rbp
        movq	GT_REG_OFFSET(GT_REG_RBX)(%rdi), %rbx
        movq	GT_REG_OFFSET(GT_REG_RDX)(%rdi), %rdx
        movq	GT_REG_OFFSET(GT_REG_RAX)(%rdi), %rax
        movq	GT_REG_OFFSET(GT_REG_RCX)(%rdi), %rcx
        movq	GT_REG_OFFSET(GT_REG_RSP)(%rdi), %rsp

        pushq GT_REG_OFFSET(GT_REG_RIP)(%rdi)
        movq GT_REG_OFFSET(GT_REG_RDI)(%rdi), %rdi

        /* return 0 */
        xorl %eax, %eax
        ret
    .size gt_set_context, . - gt_set_context

#elif GT_PLATFORM == GT_PLATFORM_LINUX_ARM
    .global gt_context_trampoline
    .align 2
    gt_context_trampoline: 
        /* call func with initial arguments */
        blx     r5

        /* call exit if link is NULL */
        cmp     r4, #0
        bne     no_exit

        mov     r0, #0
        ldr     r1, =exit
        blx     r1

        /* set link context */
    no_exit:
        mov     r0, r4
        ldr     r1, =gt_set_context
        blx     r1
    .size gt_context_trampoline, . - gt_context_trampoline

    .global gt_get_context
    .align 2
    gt_get_context:
        /* copy registers */
        str    r0,  [r0, #GT_REG_OFFSET(GT_REG_R0)]
        str    r1,  [r0, #GT_REG_OFFSET(GT_REG_R1)]
        str    r2,  [r0, #GT_REG_OFFSET(GT_REG_R2)]
        str    r3,  [r0, #GT_REG_OFFSET(GT_REG_R3)]
	str    r4,  [r0, #GT_REG_OFFSET(GT_REG_R4)]
	str    r5,  [r0, #GT_REG_OFFSET(GT_REG_R5)]
	str    r6,  [r0, #GT_REG_OFFSET(GT_REG_R6)]
	str    r7,  [r0, #GT_REG_OFFSET(GT_REG_R7)]
	str    r8,  [r0, #GT_REG_OFFSET(GT_REG_R8)]
	str    r9,  [r0, #GT_REG_OFFSET(GT_REG_R9)]
	str    r10, [r0, #GT_REG_OFFSET(GT_REG_R10)]
	str    r11, [r0, #GT_REG_OFFSET(GT_REG_R11)]
	str    sp,  [r0, #GT_REG_OFFSET(GT_REG_SP)]
	str    lr,  [r0, #GT_REG_OFFSET(GT_REG_LR)]

        /* return 0 */
        eor   r0, r0
        mov   pc, lr
    .size gt_get_context, . - gt_get_context

    .global gt_set_context
    .align 2
    gt_set_context:
        /* set registers */
        ldr    r0,  [r0, #GT_REG_OFFSET(GT_REG_R0)]
        ldr    r1,  [r0, #GT_REG_OFFSET(GT_REG_R1)]
        ldr    r2,  [r0, #GT_REG_OFFSET(GT_REG_R2)]
        ldr    r3,  [r0, #GT_REG_OFFSET(GT_REG_R3)]
	ldr    r4,  [r0, #GT_REG_OFFSET(GT_REG_R4)]
	ldr    r5,  [r0, #GT_REG_OFFSET(GT_REG_R5)]
	ldr    r6,  [r0, #GT_REG_OFFSET(GT_REG_R6)]
	ldr    r7,  [r0, #GT_REG_OFFSET(GT_REG_R7)]
	ldr    r8,  [r0, #GT_REG_OFFSET(GT_REG_R8)]
	ldr    r9,  [r0, #GT_REG_OFFSET(GT_REG_R9)]
	ldr    r10, [r0, #GT_REG_OFFSET(GT_REG_R10)]
	ldr    r11, [r0, #GT_REG_OFFSET(GT_REG_R11)]
	ldr    sp,  [r0, #GT_REG_OFFSET(GT_REG_SP)]
	ldr    lr,  [r0, #GT_REG_OFFSET(GT_REG_LR)]

        /* return 0 */
        eor   r0, r0
        mov   pc, lr
    .size gt_set_context, . - gt_set_context

#endif // GT_PLATFORM == GT_PLATFORM_LINUX_x86_64

