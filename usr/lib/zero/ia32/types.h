#ifndef __ZERO_IA32_TYPES_H__
#define __ZERO_IA32_TYPES_H__

#include <stdint.h>
#include <stdint.h>
#include <signal.h>
#include <zero/cdecl.h>
#include <zero/param.h>

/* C call frame - 8 bytes */
struct m_stkframe {
    /* automatic variables go here */
    int32_t fp;         // caller frame pointer
    int32_t pc;         // return address
    /* call parameters on stack go here in 'reverse order' */
};

/* far pointer structure */
struct m_farptr {
    uint16_t lim;
    uint32_t adr;
} PACK();

/* return stack for IRET - 20 bytes */
struct m_trapframe {
    int32_t eip;	// old instruction pointer
    int16_t cs;		// code segment selector
    int16_t pad1;	// pad to 32-bit boundary
    int32_t eflags;	// machine status word
    /* present in case of privilege transition */
    int32_t uesp;	// user stack pointer
    int16_t uss;	// user stack segment selector
    int16_t pad2;	// pad to 32-bit boundary
};

/* general purpose registers - 32 bytes */
struct m_pusha {
    int32_t edi;
    int32_t esi;
    int32_t esp;
    int32_t ebp;
    int32_t ebx;
    int32_t edx;
    int32_t ecx;
    int32_t eax;
};

/* task state segment */
struct m_tss {
    uint16_t link, _linkhi;
    uint32_t esp0;
    uint16_t ss0, _ss0hi;
    uint32_t esp1;
    uint16_t ss1, _ss1hi;
    uint32_t esp2;
    uint16_t ss2, _ss2hi;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint16_t es, _eshi;
    uint16_t cs, _cshi;
    uint16_t ss, _sshi;
    uint16_t ds, _dshi;
    uint16_t fs, _fshi;
    uint16_t gs, _gshi;
    uint16_t ldt, _ldthi;
    uint16_t trace;
    uint16_t iomapofs;
//    uint8_t  iomap[8192] ALIGNED(CLSIZE);
} PACK();

/* data segment registers - 16 bytes */
struct m_segregs {
    int32_t ds;         // data segment
    int32_t es;         // data segment
    int32_t fs;         // buffer cache segment
    int32_t gs;         // per-CPU data segment
};

/* thread control block */
#define TCBFCTXSIZE 512
struct m_tcb {
    uint8_t            fctx[TCBFCTXSIZE];       // @ 0
    struct m_trapframe iret;                    // @ 512 bytes
    struct m_segregs   segregs;                 // @ 532 bytes
    int32_t            pdbr;                    // @ 548 bytes
    struct m_pusha     genregs;                 // @ 552 bytes
} PACK() ALIGNED(PAGESIZE);

#endif /* __ZERO_IA32_TYPES_H__ */

