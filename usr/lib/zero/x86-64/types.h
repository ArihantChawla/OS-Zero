#ifndef __ZERO_X86_64_TYPES_H__
#define __ZERO_X86_64_TYPES_H__

/* TODO: structs and types here like in <zero/ia32/types.h> :) */

#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/types.h>
#include <zero/x86/types.h>

/* C call frame */
struct m_stkframe {
    /* automatic variables go here */
    int64_t fp;         // caller frame pointer
    int64_t pc;         // return address
    /* call parameters on stack go here in 'reverse order' */
};

#endif /* __ZERO_X86_64_TYPES_H__ */

