#ifndef __ZERO_X86_64_TYPES_H__
#define __ZERO_X86_64_TYPES_H__

/* TODO: structs and types here like in <zero/ia32/types.h> :) */

#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/types.h>
#include <zero/x86/types.h>

/* C call frame */
struct m_stkframe {
    /* automatic variables go here */
    int64_t fp;         // caller frame pointer
    int64_t pc;         // return address
    /* call parameters on stack go here in 'reverse order' */
};

/* general purpose registers - 32 bytes */
struct m_genregs {
    int64_t rdi;
    int64_t rsi;
    int64_t rsp;
    int64_t rbp;
    int64_t rbx;
    int64_t rdx;
    int64_t rcx;
    int64_t rax;
    int64_t r15;
    int64_t r14;
    int64_t r13;
    int64_t r12;
    int64_t r11;
    int64_t r10;
    int64_t r9;
    int64_t r8;
};

struct m_segregs {
    int32_t ds;         // data segment
    int32_t es;         // data segment
    int32_t fs;         // thread-local storage
    int32_t gs;         // kernel per-CPU segment
};

struct m_jmpframe {
};

/* thread control block */
#define M_TCBFCTXSIZE 512
struct m_tcb {
    uint8_t            fctx[M_TCBFCTXSIZE];
    int64_t            fxsave;
    int64_t            pdbr;
//    struct m_trapframe iret;
    struct m_segregs   segregs;
    struct m_genregs   genregs;
    int64_t            trapnum;
    int64_t            err;
    struct m_jmpframe  frame;
};

#endif /* __ZERO_X86_64_TYPES_H__ */

