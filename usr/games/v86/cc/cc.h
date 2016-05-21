#ifndef __V86_CC_CC_H__
#define __V86_CC_CC_H__

#include <ctype.h>

/* check if c is valid symbol character */
#define v86ccissymchar(c)                                               \
    (isalpha(c) || isdigit(c) || (c) == '_')
/* encode symbol character to internal format of VM_SYM_CHAR_BITS bits */
#define v86ccpacksymchar(c)                                             \
    ((isdigit(c))                                                       \
     ? ((c) - '0')                                                      \
     : ((isxdigit(toupper(c)))                                          \
        ? (10 + (toupper(c) - 'A'))                                     \
        : ((isalpha(c))                                                 \
           ? ((islower(c))                                              \
              ? (16 + ((c) - 'a'))                                      \
              : (16 + ('z' - 'a') + 1 + ((c) - 'A')))                   \
           : (((c) == '_')                                              \
              ? (16 + ('z' - 'a') + 1 + ('Z' - 'A') + 1)))))

#endif /* __V86_CC_CC_H__ */

