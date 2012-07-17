/*
 * macros.h - predefined macros for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_MACROS_H
#define EARTHQUAKE_MACROS_H

#include <sys/param.h>

#ifndef __CC_CACHE_ALIGN__
#   define __CC_PAGE_ALIGN__ __CC_ALIGN__(CACHE_LINE_SIZE)
#endif
#ifndef __CC_PAGE_ALIGN__
#   define __CC_PAGE_ALIGN__ __CC_ALIGN__(PAGE_SIZE)
#endif

#endif /* EARTHQUAKE_MACROS_H */

