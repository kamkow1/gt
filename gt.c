#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>

#include "gt.h"

#define GT_LOG_ERR(fmt, ...) printf("[gt] ERROR: " fmt, ##__VA_ARGS__)
#define GT_LOG_INF(fmt, ...) printf("[gt] INFO:  " fmt, ##__VA_ARGS__)

#if GT_PLATFORM == GT_PLATFORM_LINUX_x86_64

    // omit-frame-pointer makes it so that we can use RBX as a general purpose register
    // This is needed for GT_FETCH_LINKPTR() macro, which expands to inline asm() block
    __attribute__((optimize("omit-frame-pointer")))
    void    gt_context_trampoline(void)
    {
        register gt_Context *link = NULL;
        GT_FETCH_LINKPTR(link);

        if (link == NULL) {
            exit(0);
        }

        gt_set_context(link);
    }

    void    gt_make_context(gt_Context *ucp, void (*func)(void), int argc, ...)
    {
        gt_GenReg *sp;
        va_list va;
        int i;
        unsigned int link;

        link = (argc > 6 ? argc - 6 : 0) + 1;

        sp = (gt_GenReg *) ((uintptr_t) ucp->stack.stackptr + ucp->stack.size);
        sp -= link;
        sp = (gt_GenReg *) (((uintptr_t) sp & -16L) - 8);

        ucp->mach_context.gen_regs[GT_REG_RIP] = (uintptr_t) func;
        ucp->mach_context.gen_regs[GT_REG_RBX] = (uintptr_t) &sp[link];
        ucp->mach_context.gen_regs[GT_REG_RSP] = (uintptr_t) sp;

        sp[0] = (uintptr_t) &gt_context_trampoline;
        sp[link] = (uintptr_t) ucp->link;

        va_start(va, argc);

        for (i = 0; i < argc; i++) {
            switch (i) {
                case 0: ucp->mach_context.gen_regs[GT_REG_RDI] = va_arg(va, gt_GenReg); break;
                case 1: ucp->mach_context.gen_regs[GT_REG_RSI] = va_arg(va, gt_GenReg); break;
                case 2: ucp->mach_context.gen_regs[GT_REG_RDX] = va_arg(va, gt_GenReg); break;
                case 3: ucp->mach_context.gen_regs[GT_REG_RCX] = va_arg(va, gt_GenReg); break;
                case 4: ucp->mach_context.gen_regs[GT_REG_R8]  = va_arg(va, gt_GenReg); break;
                case 5: ucp->mach_context.gen_regs[GT_REG_R9]  = va_arg(va, gt_GenReg); break;
                default: sp[i - 5] = va_arg(va, gt_GenReg);                             break;
            }
        }

        va_end(va);
    }
#endif // GT_PLATFORM == GT_PLATFORM_LINUX_x86_64

static gt_GtEnv envs[GT_MAX_ENVIRONMENTS];
static int current_env;

static gt_Context exiter = {0};

static void gt_alloc_stack(gt_Context *ucp)
{
    if (ucp->stack.stackptr) {
        return;
    }

    int prot = PROT_READ | PROT_WRITE;
    int flags = MAP_ANONYMOUS | MAP_PRIVATE;
    ucp->stack.stackptr = mmap(NULL, GT_ENVIRONMENT_STACK, prot, flags, -1, 0);
    ucp->stack.size = GT_ENVIRONMENT_STACK;
}

static void gt_sched(void)
{
    int attempts = 0, candidate;

    while (attempts < GT_MAX_ENVIRONMENTS) {
        candidate = (current_env + attempts + 1) % GT_MAX_ENVIRONMENTS;
        if (envs[candidate].status == GT_RUNNABLE) {
            current_env = candidate;
            gt_set_context(&envs[current_env].state);
        }
        attempts++;
    }

    exit(1);
}

int     gt_alive     (void)
{
    int c = 0;
    for (int i = 0; i < GT_MAX_ENVIRONMENTS; i++) {
        if (envs[i].status == GT_RUNNABLE) {
            c += 1;
        }
    }
    return c;
}

int     gt_create    (gt_GtEntry entry, void *arg)
{
    int env;
    for (env = 0; env < GT_MAX_ENVIRONMENTS; env++) {
        if (envs[env].status == GT_UNUSED) {
            break;
        }
    }

    if (env == GT_MAX_ENVIRONMENTS) {
        return -1;
    }

    envs[env].status = GT_RUNNABLE;
    envs[env].arg = arg;

    gt_get_context(&envs[env].state);
    gt_alloc_stack(&envs[env].state);
    envs[env].state.link = &exiter;
    gt_make_context(&envs[env].state, entry, 0);
    
    return env;
}

void    gt_yield     (void)
{
    envs[current_env].state_reentered = 0;
    gt_get_context(&envs[current_env].state);
    if (envs[current_env].state_reentered++ == 0) {
        gt_sched();
    }
}

void    gt_exit      (void)
{
    envs[current_env].status = GT_UNUSED;
    gt_sched();
}

void    gt_destroy   (int id)
{
    envs[id].status = GT_UNUSED;
}

int     gt_getid     (void)
{
    return current_env;
}

void    gt_init      (gt_GtEntry main, void *arg)
{
    current_env = 0;
    gt_get_context(&exiter);
    gt_alloc_stack(&exiter);
    gt_make_context(&exiter, &gt_exit, 0);

    gt_create(main, arg);
    gt_set_context(&envs[current_env].state);
}

void *  gt_getarg    (void)
{
    int self = gt_getid();
    return envs[self].arg;
}

bool gt_is_blocking(void)
{
    return errno == EAGAIN || errno == EWOULDBLOCK;
}

int     gt_accept   (int fd, struct sockaddr *sa, socklen_t *sl)
{
    int fd1;
    while ((fd1 = accept(fd, sa, sl))) {
        if (fd1 < 0 && gt_is_blocking()) {
            gt_yield();
        } else {
            return fd1;
        }    
    }
    return -1;
}

int     gt_recv     (int fd, void *buf, size_t len, int flags)
{
    int n;
    while ((n = recv(fd, buf, len, flags))) {
        if (n < 0 && gt_is_blocking()) {
            gt_yield();
        } else {
            return n;
        }
    }
    return -1;
}

int     gt_send     (int fd, const void *buf, size_t len, int flags)
{
    int n;
    while ((n = send(fd, buf, len, flags))) {
        if (n < 0 && gt_is_blocking()) {
            gt_yield();
        } else {
            return n;
        }
    }
    return -1;
}

