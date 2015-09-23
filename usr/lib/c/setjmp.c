#include <features.h>
#include <setjmp.h>
#include <signal.h>
#include <zero/cdecl.h>

#if defined(_POSIX_SOURCE)

#if (!SIG32BIT)
#define _savesigmask(sp) sigprocmask(SIG_BLOCK, NULL, sp)
#define _loadsigmask(sp) sigprocmask(SIG_SETMASK, sp, NULL)
#else
#define _savesigmask(sp)
#define _loadsigmask(sp)
#endif

#elif defined(_BSD_SOURCE)

#if (SIG32BIT)
#define _savesigmask(sp) ((sp)->norm = sigblock(0))
#define _loadsigmask(sp) (sigsetmask((sp)->norm))
#else
#define _savesigmask(sp) (*(sp) = sigblock(0))
#define _loadsigmask(sp) (sigsetmask(*(sp)))
#endif

#else

#if (SIG32BIT)
#define _savesigmask(sp) ((sp)->norm = siggetmask())
#define _loadsigmask(sp) (sigsetmask((sp)->norm))
#else
#define _savesigmask(sp) (*(sp) = siggetmask())
#define _loadsigmask(sp) (sigsetmask(*(sp)))
#endif

#endif /* defined(_POSIX_SOURCE) */

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

ASMLINK
NORET
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

ASMLINK
NORET
void
_longjmp(jmp_buf env,
         int val)
{
    __longjmp(env, val);

    /* NOTREACHED */
}

#if defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE)

ASMLINK
int
sigsetjmp(sigjmp_buf env, int savesigs)
{
    __setjmp(env);

    if (savesigs) {
        _savesigmask(&(env->sigmask));
        env->havesigs = 1;
    }

    return 0;
}

ASMLINK
NORET
void
siglongjmp(sigjmp_buf env, int val)
{
    if (env->havesigs) {
        _loadsigmask(&(env->sigmask));
    }
    __longjmp(env, val);

    /* NOTREACHED */
}

#endif

