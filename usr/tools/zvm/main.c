#include <stdlib.h>
#include <zvm/zvm.h>

int
main(int argc, char *argv[])
{
#if (ZVMEFL)
    ecore_init();
    ecore_app_args_set(argc, argv);
#endif
    exit(zvmmain(argc, argv));
}

