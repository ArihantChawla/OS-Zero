/*
 * align.h - internal alignments for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_INTERNAL_ALIGN_H
#define EARTHQUAKE_INTERNAL_ALIGN_H

#include <sys/types.h>
#include <sys/param.h>

/*
 * Alignments of basic elements.
 */
#define __ECC_CHAR_ALIGNMENT        __ECC_CHAR_SIZE
#define __ECC_SHORT_ALIGNMENT       __ECC_SHORT_SIZE
#define __ECC_INT_ALIGNMENT         __ECC_INT_SIZE
#define __ECC_LONG_ALIGNMENT        __ECC_LONG_SIZE
#define __ECC_LONG_LONG_ALIGNMENT   __ECC_LONG_LONG_SIZE
#define __ECC_POINTER_ALIGNMENT     __ECC_POINTER_SIZE
#define __ECC_FLOAT_ALIGNMENT       __ECC_FLOAT_SIZE
#define __ECC_DOUBLE_ALIGNMENT      __ECC_DOUBLE_SIZE
#define __ECC_LONG_DOUBLE_ALIGNMENT 16U

#define __ECC_ARRAY_ALIGNMENT       CACHE_LINE_SIZE
#define __ECC_FUNCTION_ALIGNMENT    CACHE_LINE_SIZE
#define __ECC_JUMP_ALIGNMENT        CACHE_LINE_SIZE
#define __ECC_LABEL_ALIGNMENT       CACHE_LINE_SIZE
#define __ECC_LOOP_ALIGNMENT        CACHE_LINE_SIZE
#define __ECC_STRING_ALIGNMENT      CACHE_LINE_SIZE
#define __ECC_STRUCT_ALIGNMENT      CACHE_LINE_SIZE
#define __ECC_UNION_ALIGNMENT       CACHE_LINE_SIZE

#define __ECC_SECTION_ALIGNMENT     PAGE_SIZE

/*
 * Data alignment.
 */
#define __CC_ALIGN(u,a)           __ROUND_UP_POWER_OF_2((u), (a))
#define __CC_ALIGN_CHAR(u)        __CC_ALIGN((u), __ECC_CHAR_ALIGNMENT)
#define __CC_ALIGN_SHORT(u)       __CC_ALIGN((u), __ECC_SHORT_ALIGNMENT)
#define __CC_ALIGN_INT(u)         __CC_ALIGN((u), __ECC_INT_ALIGNMENT)
#define __CC_ALIGN_LONG(u)        __CC_ALIGN((u), __ECC_LONG_ALIGNMENT)
#define __CC_ALIGN_LONG_LONG(u)   __CC_ALIGN((u), __ECC_LONG_LONG_ALIGNMENT)
#define __CC_ALIGN_POINTER(u)     __CC_ALIGN((u), __ECC_POINTER_ALIGNMENT)
#define __CC_ALIGN_FLOAT(u)       __CC_ALIGN((u), __ECC_FLOAT_ALIGNMENT)
#define __CC_ALIGN_DOUBLE(u)      __CC_ALIGN((u), __ECC_DOUBLE_ALIGNMENT)
#define __CC_ALIGN_LONG_DOUBLE(u) __CC_ALIGN((u), __ECC_LONG_DOUBLE_ALIGNMENT)

#define __CC_ALIGN_ARRAY(u)       __CC_ALIGN((u), __ECC_ARRAY_ALIGNMENT)
#define __CC_ALIGN_FUNCTION(u)    __CC_ALIGN((u), __ECC_FUNCTION_ALIGNMENT)
#define __CC_ALIGN_JUMP(u)        __CC_ALIGN((u), __ECC_JUMP_ALIGNMENT)
#define __CC_ALIGN_LABEL(u)       __CC_ALIGN((u), __ECC_LABEL_ALIGNMENT)
#define __CC_ALIGN_LOOP(u)        __CC_ALIGN((u), __ECC_LOOP_ALIGNMENT)
#define __CC_ALIGN_STRING(u)      __CC_ALIGN((u), __ECC_STRING_ALIGNMENT)
#define __CC_ALIGN_STRUCT(u)      __CC_ALIGN((u), __ECC_STRUCT_ALIGNMENT)
#define __CC_ALIGN_UNION(u)       __CC_ALIGN((u), __ECC_UNION_ALIGNMENT)

#define __CC_ALIGN_SECTION(u)       __CC_ALIGN((u), __ECC_SECTION_ALIGNMENT)

#endif /* EARTHQUAKE_INTERNAL_ALIGN_H */

