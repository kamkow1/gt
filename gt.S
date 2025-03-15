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

#endif // GT_PLATFORM == GT_PLATFORM_LINUX_x86_64

