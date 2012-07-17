/*
 * macro.h - internal macro management for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_INTERNAL_MACRO_H
#define EARTHQUAKE_INTERNAL_MACRO_H

#include <stdint.h>

#include <sys/types.h>
#include <sys/param.h>

#define __ECPP_MACRO_HASH_ITEMS 1024 /* must be power of two. */
#if (!powerof2(__ECPP_MACRO_HASH_ITEMS))
#   error __ECPP_MACRO_HASH_ITEMS must be power of two.
#endif
extern struct __Ecppmacro *__Ecppmacrohash[__ECPP_MACRO_HASH_ITEMS];

struct __Ecppmacro {
    uint8_t *name;
    uint8_t *def;
    uint8_t *args;
    struct __Ecppmacro *chain;
};

void __Ecpphashmacro(struct __Ecppmacro *macro);
struct __Ecppmacro * __Ecppfindmacro(uint8_t *name);

#define __ECPP_MACRO_NAME_MAX 128

#endif /* EARTHQUAKE_INTERNAL_MACRO_H */

