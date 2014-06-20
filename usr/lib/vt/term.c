#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <vt/term.h>
#include <ui/ui.h>
#include <ui/text.h>
#include <vt/vt.h>

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

#if (VTTEST)
void
vtprintinfo(struct vt *vt)
{
    fprintf(stderr, "VT: fd == %d, master: %s, slave: %s\n",
            vt->atr.fd, vt->atr.masterpath, vt->atr.slavepath);
    fprintf(stderr, "VT: %ld rows x %ld columns\n",
            vt->state.nrow, vt->state.ncol);
    fprintf(stderr, "VT: %ld buffer rows\n",
            vt->textbuf.nrow);
    fprintf(stderr, "VT: %ld screen rows\n",
            vt->scrbuf.nrow);
    fprintf(stderr, "VT: font %s (%ldx%ld)\n",
            vt->font.name, vt->font.boxw, vt->font.boxh);

    return;
}

int
main(int argc, char *argv[])
{
    struct vt    vt ALIGNED(CLSIZE);
    struct term *term;
    long         nrow;
    long         ncol;

    memset(&vt, 0, sizeof(struct vt));
    vtgetopt(&vt, argc, argv);
    if (!vtinit(&vt, argc, argv)) {
        fprintf(stderr, "failed to initialise VT\n");

        exit(1);
    }
    nrow = vt.textbuf.nrow;
    if (!nrow) {
        nrow = VTDEFBUFNROW;
    }
    ncol = vt.state.ncol;
    if (!ncol) {
        ncol = VTDEFNCOL;
        vt.state.ncol = ncol;
    }
    if (!uiinittextbuf(&vt.textbuf, nrow, ncol)) {
        vtfree(&vt);

        exit(1);
    }
    nrow = vt.state.nrow;
    if (!nrow) {
        nrow = VTDEFNROW;
        vt.state.nrow = nrow;
    }
    if (!uiinittextbuf(&vt.scrbuf, nrow, ncol)) {
        vtfree(&vt);

        exit(1);
    }
    vtprintinfo(&vt);
#if 0
    vt.state.nrow = 24;
    vt.state.ncol = 80;
    vt.textbuf.nrow = VTDEFBUFNROW;
    vt.scrbuf.nrow = 24;
#endif
    vt.state.w = vt.state.ncol * vt.font.boxw;
    vt.state.h = vt.state.nrow * vt.font.boxh;
    term = termrun(&vt);
    vtfree(&vt);

    exit(1);
}
#endif /* VTTEST */

