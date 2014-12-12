#include <fenv.h>

const fenv_t __fe_dfl_env
= {
    .rndmode = FE_TONEAREST;
};

int
fesetround(int mode)
{
    __fenv.rndmode = mode;
    if (!___fpusetround(mode)) {

        return mode;
    }

    return __fenv.randmode;
}

int
fenvinit(int argc, char *argv)
{
    fesetround(
}

