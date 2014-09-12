#ifndef __STDBOOL_H__
#define __STDBOOL_H__

#include <features.h>

//#if !defined(__STDC_VERSION__) || (__STDC_VERSION__ < 199901L)
//#include <stdint.h>
//typedef uint_fast8_t _Bool;
//#endif
//#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define bool                          _Bool
#define true                          1
#define false                         0
#define __bool_true_false_are_defined 1
//#endif

#endif /* __STDBOOL_H__ */

