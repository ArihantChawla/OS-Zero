/* valhalla assembler configuration and command-line options */

#ifndef __VAS_OPT_H__
#define __VAS_OPT_H__

#include <zero/trix.h>

extern void vasinitalign(void);

#define vasalign(adr, x)                                                \
    (!((x) & (sizeof(x) - 1))                                           \
     ? (adr)                                                            \
     : rounduppow2(adr,sizeof(x)))
#define vasalignword(adr)                                               \
    vasalign(adr, sizeof(vasword_t))
#define vasaligntok(adr, type)                                          \
    vasalign(adr, vastokalntab[(type)])
#if (ZVM)
#define vasalignop(adr)                                                 \
    vasalign(adr, sizeof(struct zvmopcode))
#endif

#endif /* __VAS_OPT_H__ */

