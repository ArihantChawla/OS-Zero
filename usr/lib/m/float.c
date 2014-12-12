#include <fenv.h>
#include <float.h>

int
__flt_rounds(void)
{
    return FE_TONEAREST;
}

int
floatinit(int argc, char *argv[])
{
    
}