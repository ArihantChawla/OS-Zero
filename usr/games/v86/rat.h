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

#define v86rat64getsign(rp) ((rp)->num & 0x80000000)
struct v86rat64 {
    uint32_t num;
    uint32_t den;
};

#if defined(M_64BIT)

#define v86rat128getsign(rp) ((rp)->num[0] & UINT64_C(0x8000000000000000))
struct v86rat128 {
    uint64_t num[1];
    uint64_t den[1];
};

#define v86rat256getsign(rp) ((rp)->num[1] & UINT64_C(0x8000000000000000))
struct v86rat256 {
    uint64_t num[2];
    uint64_t den[2];
};

#define v86rat512getsign(rp) ((rp)->num[3] & UINT64_C(0x8000000000000000))
struct v86rat512 {
    uint64_t num[4];
    uint64_t den[4];
};

#define v86rat512getsign(rp) ((rp)->num[7] & UINT64_C(0x8000000000000000))
struct v86rat1024 {
    uint64_t num[8];
    uint64_t den[8];
};

#elif defined(M_32BIT)

#define v86rat128getsign(rp) ((rp)->num[1] & 0x80000000)
struct v86rat128 {
    uint32_t num[2];
    uint32_t den[2];
};

#define v86rat256getsign(rp) ((rp)->num[3] & 0x80000000)
struct v86rat256 {
    uint32_t num[4];
    uint32_t den[4];
};

#define v86rat512getsign(rp) ((rp)->num[7] & 0x80000000)
struct v86rat512 {
    uint32_t num[8];
    uint32_t den[8];
};

#define v86rat128getsign(rp) ((rp)->num[15] & 0x80000000)
struct v86rat1024 {
    uint32_t num[16];
    uint32_t den[16];
};

#endif /* M_64BIT || M_32BIT */

#endif /* __V86_RAT_H__ */

