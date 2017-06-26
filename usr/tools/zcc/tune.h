#ifndef __ZCC_TUNE_H__
#define __ZCC_TUNE_H__

/* compiler optimisation flags */
#define ZCC_TUNE_NONE    0x00000000U
#define ZCC_TUNE_ALIGN   0x00000001U
#define ZCC_TUNE_INLINE  0x00000002U
#define ZCC_TUNE_UNROLL  0x00000004U
/*
 * TODO
 * ----
 * TUNE_RESCHED // instruction rescheduling
 */

#endif /* __ZCC_TUNE_H__ */

