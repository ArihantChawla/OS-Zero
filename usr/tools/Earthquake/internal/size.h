/*
 * size.h - internal header file for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_INTERNAL_SIZE_H
#define EARTHQUAKE_INTERNAL_SIZE_H

/*
 * Sizes of basic types.
 */
#define __ECC_CHAR_SIZE               1
#define __ECC_SHORT_SIZE              2
#define __ECC_INT_SIZE                4
#define __ECC_LONG_SIZE               4
#define __ECC_LONG_LONG_SIZE          8
#define __ECC_POINTER_SIZE            4
#define __ECC_FLOAT_SIZE              4
#define __ECC_DOUBLE_SIZE             8
#define __ECC_LONG_DOUBLE_SIZE \
    ((__Erun.options.optim.longdbl128) \
     ? 16 \
     : 12)
#define __ECC_UNSIGNED_CHAR_SIZE      __ECC_CHAR_SIZE
#define __ECC_UNSIGNED_SHORT_SIZE     __ECC_SHORT_SIZE
#define __ECC_UNSIGNED_INT_SIZE       __ECC_INT_SIZE
#define __ECC_UNSIGNED_LONG_SIZE      __ECC_LONG_SIZE
#define __ECC_UNSIGNED_LONG_LONG_SIZE __ECC_LONG_LONG_SIZE

#endif /* EARTHQUAKE_INTERNAL_SIZE_H */

