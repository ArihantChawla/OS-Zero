#define VT_POSIX_OPENPT 1
#define VT_GETPT        0
#define VT_DEV_PTMX     0
#define VT_PTMX_DEVICE  "/dev/ptmx"

int
vtopenpty_posix(char **masterpath, char **slavepath)
{
    int     mfd;
    size_t  len;
    char   *slave;
    char   *path;

#if (VT_POSIX_OPENPT)
    mfd = posix_openpt(O_RDWR);
#elif (VT_GETPT)
    mfd = getpt();
#elif (VT_DEV_PTMX)
    mfd = open(VT_PTMX_DEVICE, O_RDWR);
#endif
    if (mfd < 0) {
        fprintf(stderr, "failed to open PTY device\n");

        return -1;
    }
    if (grantpt(mfd)) {
        fprintf(stderr, "grantpt() failed\n");
        close(mfd);

        return -1;
    }
    if (unlockpt(mfd)) {
        fprintf(stderr, "unlockpt() failed\n");
        close(mfd);

        return -1;
    }
    slave = ptsname(mfd);
    if (!slave) {
        fprintf(stderr, "ptsname() failed\n");
        close(mfd);

        return -1;
    }
    len = strlen(slave) + 1;
    if (masterpath) {
        path = malloc(1, len);
        if (!path) {
            fprintf(stderr, "failed to allocate masterpath\n");
            close(mfd);

            return -1;
        }
        strncpy(path, slave, len);
        path[len] = '\0';
        *masterpath = path;
    }
#if 0
    if (slavepath) {
        path = malloc(1, len);
        if (!path) {
            fprintf(stderr, "failed to allocate slavepath\n");
            free(slave);
            free(path);
            close(mfd);

            return -1;
        }
        strncpy(path, slave, len);
        path[len] = '\0';
        *slavepath = path;
    }
#endif

    return mfd;
}

