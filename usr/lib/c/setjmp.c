#include <features.h>
#include <signal.h>
#include <setjmp.h>
#include <zero/cdecl.h>

/* TODO: what version of POSIX? */
#if (_POSIX_C_SOURCE)
#define _savesigmask(sp) (*(sp) = sigblock(0))
#define _loadsigmask(sp) (sigsetmask(*(sp)))
#endif

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
ASMLINK NORET
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
ASMLINK NORET
#endif
void
_longjmp(jmp_buf env,
         int val)
{
    __longjmp(env, val);

    /* NOTREACHED */
}

