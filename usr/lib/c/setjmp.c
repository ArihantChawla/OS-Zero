#include <features.h>
#include <signal.h>
#include <setjmp.h>
#include <zero/cdecl.h>

/* TODO: what version of POSIX? */
#if (_POSIX_SOURCE)

#if (SIG32BIT)
#define _savesigmask(sp) ((sp)->norm = sigblock(0))
#define _loadsigmask(sp) (sigsetmask((sp)->norm))
#else
#define _savesigmask(sp) (*(sp) = sigblock(0))
#define _loadsigmask(sp) (sigsetmask(*(sp)))
#endif

#elif (_BSD_SOURCE)

#if (SIG32BIT)
#define _savesigmask(sp) ((sp)->norm = siggetmask())
#define _loadsigmask(sp) (sigsetmask((sp)->norm))
#else
#define _savesigmask(sp) (*(sp) = siggetmask())
#define _loadsigmask(sp) (sigsetmask(*(sp)))
#endif

#endif /* _POSIX_SOURCE */

ASMLINK
int
setjmp(jmp_buf env)
{
    __setjmp(env);
#if !(USEOLDBSD)
    _savesigmask(&(env->sigmask));
#endif

    return 0;
}

ASMLINK NORET
void
longjmp(jmp_buf env,
        int val)
{
#if !(USEOLDBSD)
    _loadsigmask(&(env->sigmask));
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
int
sigsetjmp(sigjmp_buf env, int savesigs)
{
    __setjmp(env);
    if (savesigs) {
        _savesigmask(&(env->sigmask));
        env->havesigs = 1;
    }
}

ASMLINK NORET
void
siglongjmp(sigjmp_buf env, int val)
{
    if (env->havesigs) {
        _loadsigmask(&(env->sigmask));
    }
    __longjmp(env, val);
}

#endif

