#ifndef __GFX_RASTER_H__
#define __GFX_RASTER_H__

#define gfxcopyop(s, d)    (s)
#define gfxcopyinvop(s, d) (~(s))
#define gfxclearop(s, d)   (0)
#define gfxsetop(s, d)     (~0)
#define gfxnoop(s, d)      (d)
#define gfxinvertop(s, d)  (~(d))
#define gfxandop(s, d)     ((s) & (d))
#define gfxandreverseop(s, d) ((s) & (~(d)))
#define gfxandinvertop(s, d)  ((~(s)) & (d))
#define gfxorop(s, d)         ((s) | (d))
#define gfxorinvertop(s, d)   ((~(s)) | (d))
#define gfxorreverseop(s, d)  ((s) | (~(d)))
#define gfxxorop(s, d)        ((s) ^ (d))
#define gfxnorop(s, d)        ((~s) & (~(d)))
#define gfxequivop(s, d)      ((~(s)) ^ (d))
#define gfxnandop(s, d)       (~((s) & (d)))

#endif /* __GFX_RASTER_H__ */

