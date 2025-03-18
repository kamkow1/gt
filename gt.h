#ifndef GT_H_
#define GT_H_

// --------------------------------------------------------------------------------------
// OS/Arch defines
// --------------------------------------------------------------------------------------

#define GT_PLATFORM_LINUX_x86_64    1

#if defined(__linux__) && defined(__x86_64__)
#   define GT_PLATFORM GT_PLATFORM_LINUX_x86_64
#else
#   error "Unknown platform!"
#endif

// --------------------------------------------------------------------------------------
// Defaults
// --------------------------------------------------------------------------------------

#if !defined(GT_MAX_ENVIRONMENTS)
#   define GT_MAX_ENVIRONMENTS 1024
#endif // !defined(GT_MAX_ENVIRONMENTS)

#if !defined(GT_ENVIRONMENT_STACK)
#   define GT_ENVIRONMENT_STACK (16*1024)
#endif // !defined(GT_ENVIRONMENT_STACK)

#if GT_PLATFORM == GT_PLATFORM_LINUX_x86_64
#   define GT_REG_R8		(0)
#   define GT_REG_R9 		(1)
#   define GT_REG_R10 	    (2)
#   define GT_REG_R11		(3)
#   define GT_REG_R12		(4)
#   define GT_REG_R13		(5)
#   define GT_REG_R14		(6)
#   define GT_REG_R15		(7)
#   define GT_REG_RDI		(8)
#   define GT_REG_RSI		(9)
#   define GT_REG_RBP		(10)
#   define GT_REG_RBX		(11)
#   define GT_REG_RDX		(12)
#   define GT_REG_RAX		(13)
#   define GT_REG_RCX		(14)
#   define GT_REG_RSP		(15)
#   define GT_REG_RIP		(16)
#   define GT_REG_EFL		(17)
#   define GT_REG_CSGSFS	(18)
#   define GT_REG_ERR		(19)
#   define GT_REG_TRAPNO	(20)
#   define GT_REG_OLDMASK	(21)
#   define GT_REG_CR2		(22)

#   define GT_MCONTEXT_GREGS   (40)
#   define GT_REG_SZ           (8)

#   define GT_REG_OFFSET(__reg) (GT_MCONTEXT_GREGS + ((__reg) * GT_REG_SZ))
#endif // GT_PLATFORM == GT_PLATFORM_LINUX_x86_64

#ifndef GT_ASSEMBLY

#if GT_PLATFORM == GT_PLATFORM_LINUX_x86_64
#   define GT_FETCH_LINKPTR(dest) \
        asm("movq (%%rbx), %0" : "=r" ((dest)));
#endif // GT_PLATFORM == GT_PLATFORM_LINUX_x86_64

#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#if GT_PLATFORM == GT_PLATFORM_LINUX_x86_64
    typedef long long gt_GenReg, gt_GenRegSet[23];

    typedef struct {
        unsigned short cwd, swd, ftw, fop;
        unsigned long long rip, rdp;
        unsigned mxcsr, mxcr_mask;
        struct {
            unsigned short significand[4],
                           exponent,
                           padding[3];
        } _st[8];
        struct { unsigned element[4]; } _xmm[16];
        unsigned padding[24];
    } gt_FPURegState;

    typedef gt_FPURegState *gt_FPURegSet;

    typedef struct {
        gt_GenRegSet gen_regs;
        gt_FPURegSet fpu_regs;
        unsigned long long reserved[8];
    } gt_MachContext;
#endif // GT_PLATFORM == GT_PLATFORM_LINUX_x86_64

typedef struct {
    void *stackptr;
    int flags;
    size_t size;
} gt_Stack;

typedef struct gt_Context {
    unsigned long flags;
    struct gt_Context *link;
    gt_Stack stack;
    gt_MachContext mach_context;
} gt_Context;

void    gt_context_trampoline(void);
void    gt_make_context(gt_Context *ucp, void (*func)(void), int argc, ...);
int     gt_get_context(gt_Context *ucp);
int     gt_set_context(const gt_Context *ucp);


typedef void (*gt_GtEntry)(void);

typedef enum {
    GT_UNUSED   = 0,
    GT_RUNNABLE = 1,
} gt_GtEnvStatus;

typedef struct {
    gt_GtEnvStatus status;
    gt_Context state;
    int state_reentered;
    void *arg;
} gt_GtEnv;

void    gt_init      (gt_GtEntry main, void *arg);
int     gt_create    (gt_GtEntry entry, void *arg);
void    gt_yield     (void);
void    gt_exit      (void);
void    gt_destroy   (int id);
int     gt_getid     (void);
void *  gt_getarg    (void);
int     gt_alive     (void);

// Wrappers for nonblocking IO (ayo!??)

int     gt_accept   (int fd, struct sockaddr *sa, socklen_t *sl);
int     gt_recv     (int fd, void *buf, size_t len, int flags);
int     gt_send     (int fd, const void *buf, size_t len, int flags);
int     gt_read     (int fd, void *buf, size_t count);
int     gt_write    (int fd, const void *buf, size_t count);

#endif // GT_ASSEMBLY

#endif // GT_H_
