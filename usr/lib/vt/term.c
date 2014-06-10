#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <ui/ui.h>
#include <vt/vt.h>
#include <vt/term.h>

void
termsetsigs(void)
{
//    signal(SIGHUP, termexitsig);
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGILL, SIG_DFL);
#if defined(SIGSYS)
    signal(SIGSYS, SIG_DFL);
#endif
    signal(SIGALRM, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGCHLD, SIG_DFL);

    return;
};

#if (TERMXORG)
void
terminitconn(struct term *term)
{
    struct uienv_xorg *env = term->vt->ui.env;

    term->connfd = ConnectionNumber(env->display);

    return;
}

long
terminitscr(struct term *term, long n)
{
    return 0;
}

long
terminitwin(struct term *term)
{
    return 0;
}

void
termmapwin(struct term *term)
{
    return;
}

void
termsyncwin(struct term *term)
{
    struct uienv_xorg *env = term->vt->ui.env;

    XSync(env->display, False);
}

void
termfreescr(struct term *term)
{
    return;
}
#endif

struct term *
termrun(struct vt *vt)
{
    struct term *term = malloc(sizeof(struct term));
    char        *path;
//    pid_t        pid;

    if (!term) {
        vtfree(vt);
        fprintf(stderr, "out of memory\n");

        exit(1);
    }
    term->vt = vt;
    /* set signals up */
    termsetsigs();
    /*
     * set input field separator for shell to TAB for old shells that don't
     * reset it. This is for security reasons to prevent users from setting
     * IFS to make system() execute a different program.
     */
    putenv("IFS= \t");
    /* export terminal type */
    putenv("TERM=vt100");
    /* allocate and read path */
    path = malloc(PATH_MAX);
    if (!path) {
        vtfree(vt);
        fprintf(stderr, "out of memory\n");

        exit(1);
    }
    if (!getcwd(path, PATH_MAX)) {
        vtfree(vt);
        free(path);
        fprintf(stderr, "out of memory\n");

        exit(1);
    }
    /* initialise desktop connection */
    terminitconn(term);
    /* initialise terminal screens */
    if (!terminitscr(term, TERMNSCREEN)) {
        vtfree(vt);
        free(path);

        exit(1);
    }
    /* initialise terminal windows */
    if (!terminitwin(term)) {
        vtfree(vt);
        free(path);
        termfreescr(term);

        exit(1);
    }
    /* map terminal windows */
    termmapwin(term);
    /* synchronize desktop */
    termsyncwin(term);

    return term;
}

