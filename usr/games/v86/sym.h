#ifndef __V86_SYM_H__
#define __V86_SYM_H__

#include <ctype.h>

#define V86_SYM_MAX_CHARS  15   // not counting the trailing '\0' (NUL)
#define V86_SYM_CHAR_BITS  6    // A-Z, _, 0-9
#define V86_SYM_HASH_ITEMS (1U << V86_SYM_HASH_BITS)
#define V86_SYM_HASH_BITS  10

/* check if c is valid symbol character */
#define v86issymchar(c)                                                 \
    (isalpha(toupper(c)) || isdigit(toupper(c)) || (c) == '_')
/* encode symbol character to internal format of VM_SYM_CHAR_BITS bits */
#define v86symcharid(c)                                                 \
    ((isalpha(toupper(c)))                                              \
     ? (toupper(c) - 'A')                                               \
     : ((isdigit(c))                                                    \
        ? ('Z' - 'A' + 1 + (c) - '0')                                   \
        : ('Z' - 'A' + 1 + '9' - '0' + 1)))

/* special adr-member value */
#define V86_SYM_UNRESOLVED (~(v86adr)0)

struct v86sym {
    char   *name;
    size_t  namelen;
    long    adr;
    long    val;
};

struct v86hashsym {
    struct v86sym     *sym;
    struct v86hashsym *next;
};

#endif /* __V86_SYM_H__ */

