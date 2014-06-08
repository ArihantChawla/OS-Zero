#include <vt/conf.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

int
vtopenpty_posix(char **masterpath, char **slavepath)
{
    int     fd = -1;
    size_t  len;
    char   *slave;
    char   *path;

#if (VT_POSIX_OPENPT)
    fd = posix_openpt(O_RDWR);
#elif (VT_GETPT)
    fd = getpt();
#elif (VT_DEV_PTMX)
    fd = open(VT_PTMX_DEVICE, O_RDWR);
#endif
    if (fd < 0) {
        fprintf(stderr, "failed to open PTY device\n");

        return -1;
    }
    if (grantpt(fd)) {
        fprintf(stderr, "grantpt() failed\n");
        close(fd);

        return -1;
    }
    if (unlockpt(fd)) {
        fprintf(stderr, "unlockpt() failed\n");
        close(fd);

        return -1;
    }
    slave = ptsname(fd);
    if (!slave) {
        fprintf(stderr, "ptsname() failed\n");
        close(fd);

        return -1;
    }
    len = strlen(slave) + 1;
    if (masterpath) {
        path = malloc(len);
        if (!path) {
            fprintf(stderr, "failed to allocate masterpath\n");
            close(fd);

            return -1;
        }
        strncpy(path, slave, len);
        path[len] = '\0';
        *masterpath = path;
    }
    if (slavepath) {
        path = malloc(len);
        if (!path) {
            fprintf(stderr, "failed to allocate slavepath\n");
            free(slave);
            free(path);
            close(fd);

            return -1;
        }
        strncpy(path, slave, len);
        path[len] = '\0';
        *slavepath = path;
    }

    return fd;
}

