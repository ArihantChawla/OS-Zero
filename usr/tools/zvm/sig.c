#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <zas/zas.h>

void *
zvmsigbus(zasmemadr_t adr, size_t size)
{
    fprintf(stderr, "SIGBUS: %llx (%ld)\n", (long long)adr, (long)size);

    abort();
}

