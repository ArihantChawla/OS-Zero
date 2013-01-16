#ifndef __UNIT_X86_64_VM_H__
#define __UNIT_X86_64_VM_H__

#include <stdint.h>
#include <zero/param.h>

#define vmpagenum(adr) ((uint64_t)(adr) >> PTSHIFT)
#define vmpageofs(adr) ((uint64_t)(adr) & (PAGESIZE - 1))

#define PAGENOEXEC UINT64_C(0x8000000000000000U)

#define NPDPT      512    // # of page-directory pointer table entries
#define NPML4      512    // # of page-map level 4 table entries
#define PTSHIFT    12
#define VMPGMASK   UINT64_C(0x000ffffffffff000U)
#if (ADDRBITS == 48)
#define VMPHYSMASK UINT64_C(0x0000ffffffffffffU)
#elif (ADDRBITS == 56)
#define VMPHYSMASK UINT64_C(0x00ffffffffffffffU)
#elif (ADDRBITS == 64)
#define VMPHYSMASK UINT64_C(0xffffffffffffffffU)
#endif

#endif /* __UNIT_X86_64_VM_H__ */

