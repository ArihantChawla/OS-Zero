#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>

struct sockaddr_un sa_un;

int
main(int argc, char *argv)
{
    fprintf(stderr, "size == %ld, ofs == %ld, pathsize == %ld\n",
            (long)sizeof(struct sockaddr_un),
            (long)offsetof(struct sockaddr_un, sun_path),
            (long)sizeof(sa_un.sun_path));
}

