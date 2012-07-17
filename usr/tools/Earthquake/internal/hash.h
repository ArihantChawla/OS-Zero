/*
 * hash.h - internal header for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_INTERNAL_HASH_H
#define EARTHQUAKE_INTERNAL_HASH_H

#include <stdint.h>

#include <sys/types.h>
#include <sys/param.h>

unsigned long __Ehashstr(uint8_t *str, unsigned long nhashitems);

#endif /* EARTHQUAKE_INTERNAL_HASH_H */

