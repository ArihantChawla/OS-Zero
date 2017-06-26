#ifndef __ZPM_CONF_H__
#define __ZPM_CONF_H__

#include <zero/param.h>

#define ZPM_TEXT_ADR    PAGESIZE
#define ZPM_MEM_SIZE    (1024 * 1024 * 1024)    // 1 gigabyte
#define ZPM_FB_ADR      0xe0000000              // 3.5 gigabytes
#define ZPM_FB_WIDTH    1280
#define ZPM_FB_HEIGHT   720
#define ZPM_THR_STKSIZE PAGESIZE
#define ZPM_NTHR        16

#endif /* __ZPM_CONF_H__ */

