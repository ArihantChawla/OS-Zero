#ifndef  __MACH_TYPES_H__
#define  __MACH_TYPES_H__

#include <stdint.h>

#if defined(_WIN64)
#include <zero/msc/win64.h>
#elif defined(_WIN32)
#include <zero/msc/win32.h>
#elif defined(__x86_64__) || defined(__amd64__)
#include <mach/x86/types.h>
#include <mach/x86-64/types.h>
#elif (defined(__i386__) || defined(__i486__)                           \
       || defined(__i586__) || defined(__i686__))
#include <mach/x86/types.h>
#include <mach/ia32/types.h>
#elif defined(__arm__)
#include <mach/arm/types.h>
#elif defined(__ppc__)
#include <mach/ppc/types.h>
#endif

/* atomic types */
typedef m_reg_t    m_atomic_t;
typedef int8_t     m_atomic8_t;
typedef int16_t    m_atomic16_t;
typedef int32_t    m_atomic32_t;
typedef int64_t    m_atomic64_t;
typedef void      *m_atomicptr_t;
typedef int8_t    *m_atomicptr8_t;
typedef uintptr_t  m_atomicadr_t;

#endif /* __MACH_TYPES_H__ */

