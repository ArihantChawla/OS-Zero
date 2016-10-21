#include <stdlib.h>
#include <unistd.h>
#include <execinfo.h>
#include <zero/cdefs.h>

void *ptrtab[64];

NOINLINE void
bar(void)
{
    int size = backtrace(ptrtab, 64);
    backtrace_symbols_fmt_fd(ptrtab, size, NULL, STDIN_FILENO);

    return;
}

NOINLINE void
foo(void)
{
    bar();

    return;
}

int
main(int argc, char *argv[])
{
    foo();

    exit(0);
}

