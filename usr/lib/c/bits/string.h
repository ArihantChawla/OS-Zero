#ifndef __BITS_STRING_H__
#define __BITS_STRING_H__

#include <stddef.h>

#define STRINGNLANG 1

#define _setcoll(coll)                                                  \
    (((coll) < 0 || ((coll) >= STRINGNLANG))                            \
     ? -1                                                               \
     : (localecolltab = colltab[(coll)], 0))

#if 0
struct _string {
    char    *curtok;
};
#endif

#endif /* __BITS_STRING_H__ */

