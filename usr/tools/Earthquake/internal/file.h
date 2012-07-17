/*
 * file.h - internal file management header for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_INTERNAL_FILE_H
#define EARTHQUAKE_INTERNAL_FILE_H

#include <stdint.h>

#include <sys/types.h>
#include <sys/param.h>

/*
 * <FIXME>
 * - this structure needs padding on some architectures.
 * </FIXME>
 */
struct __Efile {
    uint8_t *name;
    uint8_t *buf;
    uint8_t *bufptr;
    uint8_t *endptr;
    unsigned long eof;
    struct __Efile *prev;
    struct __Efile *next;
    int fd;
};

struct __Efile * __Eopen(uint8_t *filename, int readwrite, int allocbuf);

void __Eclose(struct __Efile *file);
ssize_t __Eread(struct __Efile *file, size_t count);
#define __Ewrite(f, b, n) __Ewritef((f)->fd, (b), (n))
int __Ewritef(int fd, uint8_t *buf, size_t count);
int __Egetc(struct __Efile *file);
void __Eungetc(struct __Efile *file, uint8_t u8);
struct __Efile * __Eallocfile(uint8_t *name, int allocbuf);
void __Efreefile(struct __Efile *file);

#endif /* EARTHQUAKE_INTERNAL_FILE_H */

