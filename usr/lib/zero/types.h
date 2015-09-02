#ifndef __ZERO_TYPES_H__
#define __ZERO_TYPES_H__

/*
 * interface for machine-specific types:
 * m_cframe     - call/stack frame structure
 * - other machine-specific declarations
 */
#if defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__)
#include <zero/ia32/types.h>
#elif defined(__x86_64__) || defined(__amd64__)
#include <zero/x86-64/types.h>
#elif defined(__arm__)
#include <zero/arm/types.h>
#elif defined(__ppc__)
#include <zero/ppc/types.h>
#endif

#endif /* __ZERO_TYPES_H__ */

