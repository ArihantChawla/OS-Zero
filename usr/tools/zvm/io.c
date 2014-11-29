#include <zvm/conf.h>

#if (_ZERO_SOURCE)
#include <zero/kbd.h>
#endif
#include <zvm/zvm.h>
#include <zvm/io.h>

void
zvminitio(void)
{
#if (_ZERO_SOURCE)
//    kbdinit();
#endif

    return;
}

