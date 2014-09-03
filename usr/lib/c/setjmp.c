#include <features.h>
#include <signal.h>
#include <setjmp.h>
#include <zero/cdecl.h>

/* TODO: what version of POSIX? */
#if (_POSIX_C_SOURCE)
#define _savesigmask(sp) (*(sp) = sigblock(0))
#define _loadsigmask(sp) (sigsetmask(*(sp)))
#elif (_BSD_SOURCE)
#define _savesigmask(sp) (*(sp) = siggetmask())
#define _loadsigmask(sp) (sigsetmask(*(sp)))
#endif

ASMLINK
int
setjmp(jmp_buf env)
{
    __setjmp(env);
#if !(USEOLDBSD)
    _savesigmask(&env->sigmask);
#endif

    return 0;
}

ASMLINK NORET
void
longjmp(jmp_buf env,
        int val)
{
#if !(USEOLDBSD)
    _loadsigmask(&env->sigmask);
#endif
    __longjmp(env, val);

    /* NOTREACHED */
}

ASMLINK
int
_setjmp(jmp_buf env)
{
    __setjmp(env);

    return 0;
}

ASMLINK NORET
void
_longjmp(jmp_buf env,
         int val)
{
    __longjmp(env, val);

    /* NOTREACHED */
}

#if (_POSIX_C_SOURCE) || (_XOPEN_SOURCE)
ASMLINK
sigsetjmp(sigjmp_buf env, int savesigs)
{
    __setjmp(env);
    if (savesigs) {
        _savesigmask(&env->sigmask);
        env->havesigs = 1;
    }
}

ASMLINK NORET
siglongjmp(sigjmp_buf env, int val)
{
    if (env->havesigs) {
        _loadsigmask(&env->sigmask);
    }
    __longjmp(env, val);
}
#endif

