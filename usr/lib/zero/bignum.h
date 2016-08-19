#ifndef __ZERO_BIGNUM_H__
#define __ZERO_BIGNUM_H__

#if defined(__x86_64__) || defined(__amd64__) || defined(_M_AMD64)
#include <zero/x86-64/bignum.h>
#include <zero/op128.h>
#elif (defined(__i386__) || defined(__i486__)                           \
       || defined(__i586__) || defined(__i686__))
#include <zero/ia32/bignum.h>
#include <zero/op64.h>
#elif defined(__arm__)
#include <zero/arm/bignum.h>
#elif defined(__ppc__)
#include <zero/ppc/bignum.h>
#endif

#endif /* __ZERO_BIGNUM_H__ */

