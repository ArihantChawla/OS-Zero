#ifndef __ZERO_TYPES_H__
#define __ZERO_TYPES_H__

/*
 * interface for machine-specific types:
 * m_cframe     - call/stack frame structure
 * - other machine-specific declarations
 */
#if defined(__x86_64__)
#include <zero/x86_64/types.h>
#elif defined(__i386__)
#include <zero/ia32/types.h>
#elif defined(__arm__)
#include <zero/arm/types.h>
#elif defined(__ppc__)
#include <zero/ppc/types.h>
#endif

#endif /* __ZERO_TYPES_H__ */

