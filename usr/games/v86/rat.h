#ifndef __V86_RAT_H__
#define __V86_RAT_H__

#include <stdint.h>

#if defined(_MSC_VER)
#if defined(_WIN64)
#define M_64BIT
#elif defined(_WIN32)
#define M_32BIT
#endif
#elif defined(__x86_64__) || defined(__amd64__)
#define M_64BIT
#elif defined(__i386__)
#define M_32BIT
#elif defined(__arm__)
#define M_32BIT
#elif defined(__ppc__)
#define M_32BIT
#endif

struct v86rat64 {
    int32_t num;
    int32_t den;
};

#if defined(M_64BIT)

struct v86rat128 {
    int64_t num[2];
    int64_t den[2];
};

struct v86rat256 {
    int64_t num[4];
    int64_t den[4];
};

struct v86rat512 {
    int64_t num[8];
    int64_t den[8];
};

struct v86rat1024 {
    int64_t num[16];
    int64_t den[16];
};

#elif defined(M_32BIT)

struct v86rat128 {
    int32_t num[4];
    int32_t den[4];
};

struct v86rat256 {
    int32_t num[8];
    int32_t den[8];
};

struct v86rat512 {
    int32_t num[16];
    int32_t den[16];
};

struct v86rat1024 {
    int32_t num[32];
    int32_t den[32];
};

#endif /* M_64BIT || M_32BIT */

#endif /* __V86_RAT_H__ */

