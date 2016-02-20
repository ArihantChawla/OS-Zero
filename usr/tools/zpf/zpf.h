#ifndef __ZPF_ZPF_H__
#define __ZPF_ZPF_H__

#include <stdint.h>
#include <zero/trix.h>
#include <zpm/op.h>

/* number of items in scratch memory storage (vm->mem) */
#define ZPF_MEM_NWORD      16   // scratch memory store region

/* word characteristics */
#define ZPFWORD_MAX        INT32_MAX
#define ZPFWORD_MIN        INT32_MIN
#define ZPFWORD_UNSIGNED   0
#define ZPFWORD_SIZE       4
typedef int32_t zpfword_t;
#define bpfnsigbit(w)      (32 - lzero32(w))

#endif /* __ZPF_ZPF_H__ */

