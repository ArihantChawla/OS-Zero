#ifndef __UNIT_X86_64_VM_H__
#define __UNIT_X86_64_VM_H__

#if defined(__x86_64__) || defined(__amd64__)

#include <stdint.h>
#include <zero/param.h>

extern uint64_t kernpagemapl4tab[512];

#define NADRBIT ADRBITS

static INLINE uint64_t
vmsignadr(void *adr)
{
    int64_t  val = (int64_t)adr;
    int64_t  sign = INT64_C(1) << (NADRBIT - 1);
    int64_t  lobits = (INT64_C(1) << NADRBIT) - 1;
    int64_t  res;

    val &= lobits;
    res = (val ^ sign) - sign;

    return res;
}

#define vmpt4num(adr)  (((uint64_t)(adr) >> 39) & 0x1ff)
#define vmpt3num(adr)  (((uint64_t)(adr) >> 30) & 0x1ff)
#define vmpt2num(adr)  (((uint64_t)(adr) >> 21) & 0x1ff)
#define vmpt1num(adr)  (((uint64_t)(adr) >> 12) & 0x1ff) 
#define vmpagenum(adr) (((uint64_t)(adr) >> PTSHIFT) & UINT64_C(0xfffffffff))
#define vmpageofs(adr) ((uint64_t)(adr) & (PAGESIZE - 1))

#define PAGEPRES    0x00000001
#define PAGEWRITE   0x00000002
#define PAGEUSER    0x00000004
#define PAGEWRTHRU  0x00000008
#define PAGENOCACHE 0x00000010
#define PAGEDIRTY   0x00000020
#define PAGESUPER   0x00000040
#if !defined(__ASSEMBLER__)
#define PAGENOEXEC  UINT64_C(0x8000000000000000U)
#endif

#define NPAGETAB    512  // # of page-structure table entries
#define PTSHIFT     12
#if !defined(__ASSEMBLER__)
#define VMPGMASK    UINT64_C(0xfffffffffffff000U)
#if (ADRBITS == 48)
#define VMPHYSMASK  UINT64_C(0x0000ffffffffffffU)
#elif (ADRBITS == 56)
#define VMPHYSMASK  UINT64_C(0x00ffffffffffffffU)
#elif (ADRBITS == 64)
#define VMPHYSMASK  UINT64_C(0xffffffffffffffffU)
#endif
#endif

#endif /* __UNIT_X86_64_VM_H__ */

#endif /* __x86_64__ || __amd64__ */

