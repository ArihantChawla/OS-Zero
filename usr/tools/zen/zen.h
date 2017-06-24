#ifndef __ZEN_ZEN_H__
#define __ZEN_ZEN_H__

#include <stdint.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <zpm/op.h>

#define ZPM_NHASH          1024

/* number of items in scratch memory storage (vm->mem) */
#define ZEN_MEM_NWORD      (PAGESIZE / ZENWORD_SIZE)

/* word characteristics */
#define ZENWORD_MAX        INT32_MAX
#define ZENWORD_MIN        INT32_MIN
#define ZENWORD_UNSIGNED   0
#define ZENWORD_SIZE       4
typedef int32_t zenword_t;
#define bpfnsigbit(w)      (32 - lzero32(w))

#define zengetarg1(vm, args)

#endif /* __ZEN_ZEN_H__ */

