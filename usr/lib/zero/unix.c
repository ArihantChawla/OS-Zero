#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#if !defined(EAGAIN)
#define EAGAIN EWOULDBLOCK
#endif

void *
zreadfile(char *filename, size_t *sizeret)
{
    void        *buf = NULL;
    struct stat  statbuf;
    ssize_t      nread;
    size_t       nleft;
    int          fd;

    if (sizeret) {
        *sizeret = 0;
    }
    if (stat(filename, &statbuf) < 0) {

        return NULL;
    }
    if (!S_ISREG(statbuf.st_mode)) {

        return NULL;
    }
    fd = open(filename, O_RDONLY);
    if (fd < 0) {

        return NULL;
    }
    nread = 0;
    nleft = statbuf.st_size;
    if (nleft) {
        buf = malloc(nleft);
        if (buf) {
            while (nleft) {
                nread = read(fd, buf, nleft);
                if (nread < 0) {
                    if (errno == EINTR) {
                        
                        continue;
                    } else {
                        free(buf);
                        
                        return NULL;
                    }
                } else {
                    nleft -= nread;
                }
            }
            if (sizeret) {
                *sizeret = statbuf.st_size;
            }
        }
    }
    close(fd);

    return buf;
}

int
zwritefile(char *filename, void *buf, size_t nb)
{
    ssize_t nwritten;
    size_t  nleft;
    int     fd;

    fd = open(filename, O_WRONLY);
    if (fd < 0) {

        return -1;
    }
    nwritten = 0;
    nleft = nb;
    if ((buf) && (nleft)) {
        while (nleft) {
            nwritten = write(fd, buf, nleft);
            if (nwritten < 0) {
                if (errno == EINTR) {
                    
                    continue;
                } else {
                    close(fd);
                    unlink(filename);
                    
                    return -1;
                }
            } else {
                nleft -= nwritten;
            }
        }
    }
    close(fd);

    return 0;
}

/* read in blocking mode */
ssize_t
zread(int fd, void *buf, size_t nb)
{
    ssize_t len;
    ssize_t nread;

    len = 0;
    while (nb) {
	errno = 0;
	nread = read(fd, buf, nb);
	if (nread <= 0) {
	    if (errno == EINTR) {

		continue;
	    } else if (len == 0) {

		return -1;
	    }

	    return len;
	}
	nb -= nread;
	len += nread;
	buf += nread;
    }

    return len;
}

/* read in non-blocking mode */
ssize_t
zreadnb(int fd, void *buf, size_t nb)
{
    ssize_t len;
    ssize_t nread;

    len = 0;
    while (nb) {
	errno = 0;
	nread = read(fd, buf, nb);
	if (nread <= 0) {
	    if (errno == EINTR) {

		continue;
	    } else if (errno == EAGAIN) {

		return len;
	    }

	    return -1;
        }
	nb -= nread;
	len += nread;
	buf += nread;
    }

    return len;
}

/* write in blocking mode */
ssize_t
zwrite(int fd, void *buf, size_t nb)
{
    ssize_t len;
    ssize_t nwritten;

    len = 0;
    while (nb) {
	errno = 0;
	nwritten = write(fd, buf, nb);
	if (nwritten <= 0) {
	    if (errno == EINTR) {

		continue;
	    }

	    return len;
	}
	buf += nwritten;
	nb -= nwritten;
	len += nwritten;
    }

    return len;
}

/* write in non-blocking mode */
ssize_t
zwritenb(int fd, void *buf, size_t nb)
{
    ssize_t len;
    ssize_t nwritten;

    len = 0;
    while (nb) {
	errno = 0;
	nwritten = write(fd, buf, nb);
	if (nwritten <= 0) {
	    if (errno == EINTR) {

		continue;
	    } else if (errno == EAGAIN) {

		return len;
	    }

	    return -1;
        }
	buf += nwritten;
	nb -= nwritten;
	len += nwritten;
    }

    return len;
}

