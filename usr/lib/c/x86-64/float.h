#ifndef __X86_64_FLOAT_H__
#define __X86_64_FLOAT_H__

#include <ia32/float.h>

/*
 * values for FLT_EVAL_METHOD
 * -1 - indeterminable
 *  0 - evaluate just to the range and precision of the type
 *  1 - evaluate float and double to the range and precision of double
 *      evaluate long double to its range and precision
 *  2 - evaluate all operations and constants to the range and precision of
 *      long double
 */
#undef  FLT_EVAL_METHOD
#define FLT_EVAL_METHOD 0

#endif /* __X86_64_FLOAT_H__ */

