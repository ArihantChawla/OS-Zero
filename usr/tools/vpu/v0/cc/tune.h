#ifndef __CC_TUNE_H__
#define __CC_TUNE_H__

/* compiler optimisation flags */
#define CC_TUNE_NONE    0x00000000U
#define CC_TUNE_ALIGN   0x00000001U
#define CC_TUNE_INLINE  0x00000002U
#define CC_TUNE_UNROLL  0x00000004U
/*
 * TODO
 * ----
 * TUNE_RESCHED // instruction rescheduling
 */

#endif /* __CC_TUNE_H__ */

