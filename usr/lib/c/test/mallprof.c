#include <stdio.h>
#include <zero/trix.h>

#define LINELEN 4096

static char line[LINELEN] ALIGNED(PAGESIZE);

int
main(int argc, char *argv)
{
    long      aid;
    long      bid;
    long long cnt;

    scanf(&line, "\n");
    sscanf(&line, "NREQ[%lld]: %lld\n");
}

