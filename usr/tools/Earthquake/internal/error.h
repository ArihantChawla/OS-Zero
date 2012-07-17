/*
 * error.h - error handling for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_INTERNAL_ERROR_H
#define EARTHQUAKE_INTERNAL_ERROR_H

#include <stdint.h>

#include <sys/types.h>
#include <sys/param.h>

struct __Eerror {
    uint8_t *prog;
    unsigned long code;
    uint8_t *str;
};

#define __EARTHQUAKE_ALLOCATION_FAILURE 1
#define __EARTHQUAKE_UNEXPECTED_BRACE   2
#define __EARTHQUAKE_UNEXPECTED_EOF     3
#define __EARTHQUAKE_INVALID_TOKEN      4
#define __EARTHQUAKE_UNEXPECTED_TOKEN   5
#define __EARTHQUAKE_INVALID_CONSTANT   6
#define __EARTHQUAKE_INVALID_INCLUDE    7
#define __EARTHQUAKE_ERROR_MAX          7

void __Eprterror(struct __Eerror *error);

#endif /* EARTHQUAKE_INTERNAL_ERROR_H */

