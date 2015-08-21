#include <stdlib.h>
#include <zvm/conf.h>
#include <zvm/zvm.h>
#if (ZVMEFL)
#include <zvm/efl.h>
#endif

int
main(int argc, char *argv[])
{
#if (ZVMEFL)
    ecore_init();
    ecore_app_args_set(argc, (const char **)argv);
#endif
    exit(zvmmain(argc, argv));
}

