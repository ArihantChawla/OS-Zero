#ifndef __FDLIBM_ZERO_H__
#define __FDLIBM_ZERO_H__

#define I387FPU                                                         \
    (defined(__i386__)                                                  \
    || defined(__i486__)                                                \
    || defined(__i586__)                                                \
    || defined(__i686__)                                                \
    || defined(__x86_64__)                                              \
    || defined(__amd64__))

#endif /* __FDLIBM_ZERO_H__ */
