#ifndef __VPU_CONF_H__
#define __VPU_CONF_H__

#include <zero/param.h>

#define VPU_TEXT_ADR    PAGESIZE
#define VPU_MEM_SIZE    (1024 * 1024 * 1024)    // 1 gigabyte
#define VPU_FB_ADR      0xe0000000              // 3.5 gigabytes
#define VPU_FB_WIDTH    1280
#define VPU_FB_HEIGHT   720
#define VPU_THR_STKSIZE PAGESIZE
#define VPU_NTHR        16

#endif /* __VPU_CONF_H__ */

