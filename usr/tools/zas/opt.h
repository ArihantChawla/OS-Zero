#ifndef __ZAS_OPT_H__
#define __ZAS_OPT_H__

#include <zero/trix.h>

#define zasalign(adr, x)                                                \
    (!((x) & (sizeof(x) - 1))                                           \
     ? (adr)                                                            \
     : rounduppow2(adr,sizeof(x)))
#define zasalignword(adr)                                               \
    zasalign(adr, sizeof(zasword_t))
#define zasaligntok(adr, type)                                          \
    zasalign(adr, zastokalntab[(type)])
#if (ZVM)
#define zasalignop(adr)                                                 \
    zasalign(adr, sizeof(struct zvmopcode))
#endif

#endif /* __ZAS_OPT_H__ */

