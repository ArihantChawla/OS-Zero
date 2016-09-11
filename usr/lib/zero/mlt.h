#ifndef __ZERO_MLT_H__
#define __ZERO_MLT_H__

#define MLTKEYSIGN 0

#if !defined(MLTVAL_T)
typedef uint64_t MLTVAL_T;
#endif

#define MLTINVAL    UINT64_C(0xffffffffffffffff)
#define MLTINVBYTE  0xff

#if (MLTKEYSIGN)
#define MLTNKEYBIT  63
#define MLTNL0TAB   (1U << (MLTNLVLBIT - 1))
#else
#define MLTNKEYBIT  64
#define MLTNL0TAB   (1U << MLTNLVLBIT)
#endif
#define MLTNTAB     (1UL << MLTNLVLBIT)
#define MLTNLVLBIT  8
#define MLTNTREELVL 8
#define MLTNPTRLVL  7
#define MLTKEYMASK  (MLTNTAB - 1)
#define MLTKEYMASK0 (MLTNL0TAB - 1)

#endif /* __ZERO_MLT_H__ */

