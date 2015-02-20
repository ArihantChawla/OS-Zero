#ifndef __ZED_REND_H__
#define __ZED_REND_H__

#include <gfx/rgb.h>

#if defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(__amd64__)
#define zedgetblue(rp)  gfxgetblue_p(rp)
#define zedgetgreen(rp) gfxgetgreen_p(rp)
#define zedgetred(rp)   gfxgetred_p(rp)
#define zedgetalpha(rp) gfxgetalpha_p(rp)
#endif
typedef struct argb32 zedrend32;
#else /* !X86 */
#define zedgetblue(rp)  gfxgetblue(*(rp))
#define zedgetgreen(rp) gfxgetgreen(*(rp))
#define zedgetred(rp)   gfxgetred(*(rp))
#define zedgetalpha(rp) gfxgetalpha(*(rp))
typedef argb32_t zedrend32;
#endif

/* 64-bit encoding for ARGB + Unicode */
struct zedrendpix {
    zedrend32 pix;      // 32-bit ARGB-pixel
    int32_t   data;     // 32-bit Unicode character
} PACK();
typedef struct zedrendpix zedrendpix;

#endif /* __ZED_REND_H__ */

