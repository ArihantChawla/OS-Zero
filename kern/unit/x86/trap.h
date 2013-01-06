#ifndef __UNIT_X86_TRAP_H__
#define __UNIT_X86_TRAP_H__

#define trapmkdesc(sel, f64, ring)                                      \
    (((f64) & 0x0000ffff)                                               \
     | ((sel) << 16)                                                    \
     | (ring)                                                           \
     | (((f64) & 0xffff0000) << 32)                                     \
     | TRAPDEFBITS)

#define TRAPGATE    UINT64_C(0x00000e0000000000)
#define TRAPUSER    UINT64_C(0x0000600000000000)
#define TRAPSYS     UINT64_C(0x0000000000000000)
#define TRAPPRES    UINT64_C(0x0000800000000000)
#define TRAPDEFBITS (TRAPGATE | TRAPPRES)

#endif /* __UNIT_X86_64_TRAP_H__ */

