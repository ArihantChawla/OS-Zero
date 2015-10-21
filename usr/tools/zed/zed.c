#include <stddef.h>
#include <stdio.h>
#include <zed/zed.h>
#include <zed/file.h>

struct zed zed;

void
zedinitbuf(size_t nbuf)
{
    struct zedfile *filetab = calloc(nbuf, sizeof(struct zedfile));

    if (!filetab) {
        fprintf(stderr, "ZED: failed to allocate buftab\n");

        exit(1);
    }
    zed.curfile = -1;
    zed.nfile = nbuf;
    zed.filetab = filetab;
}

int
main(int argc, char *argv[])
{
    zedgetopt(&zed, argc, argv);
    zedinitbuf(1);

    exit(1);
}

