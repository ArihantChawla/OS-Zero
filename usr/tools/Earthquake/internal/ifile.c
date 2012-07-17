/*
 * ifile.c - internal file management for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#define FALSE 0
#define TRUE  1

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <main.h>

struct __Efile *
__Eopen(uint8_t *filename, int readwrite, int allocbuf)
{
    struct __Efile *file;
    int fd;
    int flags;

    if (readwrite) {
	flags = O_RDWR;
    } else {
	flags = O_RDONLY;
    }
    fd = open(filename, flags);
    if (fd < 0) {

	return NULL;
    }
    file = __Eallocfile(filename, allocbuf);
    if (file == NULL) {
	close(fd);

	return NULL;
    }
    file->fd = fd;

    return file;
}

void
__Eclose(struct __Efile *file)
{
    __Efreefile(file);
    close(file->fd);

    return;
}

ssize_t
__Eread(struct __Efile *file, size_t count)
{
    uint8_t *buf;
    ssize_t nb;
    ssize_t nbread;
    int fd;
    
    buf = file->bufptr;
    nb = 0;
    nbread = 0;
    fd = file->fd;
    while (count) {
	errno = 0;
        nb = read(fd, buf, count);
        if (nb <= 0) {
            if (errno == EINTR) {
                errno = 0;
		
                continue;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
		
                return nbread;
            } else if (nb == 0) {
		file->eof = TRUE;
		
                return nbread;
            }
        }
        buf += nb;
        count -= nb;
        nbread += nb;
    }
    
    return nbread;
}

int
__Ewritef(int fd, uint8_t *buf, size_t count)
{
    uint8_t *u8ptr;
    ssize_t nb;
    ssize_t nbwritten;
    int outfd;

    u8ptr = buf;
    nb = 0;
    nbwritten = 0;
    outfd = fd;
    while (nb) {
	errno = 0;
        nbwritten = write(outfd, u8ptr, nb);
        if (nb < 0) {
            if (errno == EINTR) {
                errno = 0;
		
                continue;
            }

	    return -1;
        }
        u8ptr += nb;
        nb -= nbwritten;
    }
    
    return 0;
}

int
__Egetc(struct __Efile *file)
{
    ssize_t nbread;

    if (file->bufptr == file->endptr) {
	if (file->eof) {

	    return -1;
	}
	file->bufptr = file->buf;
	nbread = __Eread(file, BUFSIZ);
	if (nbread == 0) {

	    return -1;
	}
	file->endptr = file->buf + nbread;
    }

    return ((int)file->bufptr++);
}

void
__Eungetc(struct __Efile *file, uint8_t u8)
{
    off_t offset;
    size_t nb;

    if (file->bufptr > file->buf) {
	file->bufptr--;
	*file->bufptr = u8;
    } else {
	offset = file->endptr - file->bufptr;
	lseek(file->fd, -offset, SEEK_CUR);
	nb = offset - 1;
	file->bufptr = file->buf;
	*file->bufptr++ = u8;
	nb = __Eread(file, nb);
    }

    return;
}

struct __Efile *
__Eallocfile(uint8_t *name, int allocbuf)
{
    void *ptr;
    struct __Efile *file;
    size_t namelen;

    file = calloc(1, sizeof(struct __Efile));
    if (file == NULL) {

	return NULL;
    }
    if (name) {
	namelen = strlen(name);
	ptr = calloc(1, namelen + 1);
	if (ptr == NULL) {
	    __Efreefile(file);
	    
	    return NULL;
	}
	strncpy(ptr, name, namelen);
	file->name = ptr;
    }
    ptr = valloc(BUFSIZ);
    if (ptr == NULL) {
	__Efreefile(file);
	
	return NULL;
    }
    file->buf = ptr;
    file->bufptr = ptr;
    file->endptr = ptr;

    return file;
}

void
__Efreefile(struct __Efile *file)
{
    if (file->name) {
	free(file->name);
    }
    if (file->buf) {
	free(file->buf);
    }

    return;
}

