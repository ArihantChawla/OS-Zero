#ifndef __KERN_LFB_H__
#define __KERN_LFB_H__

/* fmt values */
#define LFB_BGR888  0
#define LFB_BGRA32  1
#define LFB_BGR555  2
#define LFB_BGR565  3
struct lfbatr {
    long pixfmt;
    long width;
    long height;
};

#endif /* __KERN_LFB_H__ */

