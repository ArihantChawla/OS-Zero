#ifndef __V0_CONF_H__
#define __V0_CONF_H__

//#define V0_DEBUG_TABS
#define V0_PRINT_XCPT

#include <zero/param.h>

#define V0_NINST_MAX   256
#define V0_NIOPORT_MAX 1024 // must fit in val-field of struct v0op
#define V0_PAGE_SIZE   (4 * 1024)
#define _MEGABYTE      (1024U * 1024U)
#define V0_TEXT_ADR    V0_PAGE_SIZE
#define V0_MEM_SIZE    (4 * _MEGABYTE)
#define V0_FB_ADR      0xe0000000               // 3.5 gigabytes
#define V0_FB_WIDTH    1280
#define V0_FB_HEIGHT   720
#if defined(V0THR)
#define V0_THR_STKSIZE V0_PAGE_SIZE
#define V0_NTHR        16
#endif

#endif /* __V0_CONF_H__ */

