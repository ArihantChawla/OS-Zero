/*
 * THANKS
 * ------
 * - Henry 'froggey' Harrington for amd64-fixes
 */

#include <signal.h>
#include <setjmp.h>
#include <zero/cdecl.h>

#if defined(ASMLINK)
ASMLINK
#endif
int
setjmp(jmp_buf env)
{
    __setjmp(env);
    _savesigmask(&env->sigmask);

    return 0;
}

#if defined(ASMLINK)
ASMLINK
#endif
void
longjmp(jmp_buf env,
        int val)
{
    _loadsigmask(&env->sigmask);
    __longjmp(env, val);

    /* NOTREACHED */
}

#if defined(ASMLINK)
ASMLINK
#endif
int
_setjmp(jmp_buf env)
{
    __setjmp(env);

    return 0;
}

#if defined(ASMLINK)
ASMLINK
#endif
void
_longjmp(jmp_buf env,
         int val)
{
    __longjmp(env, val);

    /* NOTREACHED */
}

