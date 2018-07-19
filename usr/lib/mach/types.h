#ifndef  __MACH_TYPES_H__
#define  __MACH_TYPES_H__

#include <stdint.h>
#include <signal.h>
#include <zero/cdefs.h>

#if defined(_WIN64)
#include <zero/msc/win64.h>
#elif defined(_WIN32)
#include <zero/msc/win32.h>
#elif (defined(__x86_64__) || defined(__amd64__)                        \
       || (defined(__i386__) || defined(__i486__)                       \
           || defined(__i586__) || defined(__i686__)))
#include <mach/x86/types.h>
#elif defined(__arm__)
#include <mach/arm/types.h>
#elif defined(__ppc__)
#include <mach/ppc/types.h>
#endif

/* machine types */
typedef int32_t    m_reg_t;
typedef uint32_t   m_ureg_t;
typedef uintptr_t  m_adr_t;
typedef void      *m_ptr_t;

#endif /* __MACH_TYPES_H__ */

