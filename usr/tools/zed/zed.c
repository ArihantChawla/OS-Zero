#include <stddef.h>
#include <zed/zed.h>

static struct zed zed;

void
zedinitbuf(size_t nbuf)
{
    struct zedbuf *buftab = calloc(nbuf, sizeof(struct zedbuf));

    if (!buftab) {
        fprintf(stderr, "ZED: failed to allocate buftab\n");

        exit(1);
    }
    zed.curbuf = -1;
    zed.nbuf = nbuf;
    zed.buftab = buftab;
}

int
main(int argc, char *argv[])
{
    zedinit(argc, argv);
}

