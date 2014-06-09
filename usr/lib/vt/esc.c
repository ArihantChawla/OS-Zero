/*
 * REFERENCE: http://www.termsys.demon.co.uk/vtansi.htm
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <vt/vt.h>

#define vtisesccmd(c)       bitset(vtesc.esccmdmap, c)
#define vtiscsicmd(c)       bitset(vtesc.csicmdmap, c)
#define vtishashcmd(c)      bitset(vtesc.hashcmdmap, c)
#define vtsetesccmd(c)      setbit(vtesc.esccmdmap, c)
#define vtsetcsicmd(c)      setbit(vtesc.csicmdmap, c)
#define vtsethashcmd(c)     setbit(vtesc.hashcmdmap, c)
#define vtsetescfunc(c, f)  (vtesc.escfunctab[(c)] = (f))
#define vtsetcsifunc(c, f)  (vtesc.csifunctab[(c)] = (f))
#define vtsethashfunc(c, f) (vtesc.hashfunctab[(c)] = (f))

static struct vtesc vtesc ALIGNED(PAGESIZE);

#define VTRESETCMD      0
#define VTBRIGHTCMD     1
#define VTDIMCMD        2
#define VTUNDERSCORECMD 4
#define VTBLINKCMD      5
#define VTREVERSECMD    7
#define VTHIDDENCMD     8

long vtatrbittab[9] ALIGNED(CLSIZE)
= {
    0,
    VTBRIGHT,
    VTDIM,
    0,
    VTUNDERSCORE,
    VTBLINK,
    0,
    VTREVERSE,
    VTHIDDEN
};

void
vtscroll(struct vt *vt, long nrow)
{
    ;
}

long
vtsetscroll(struct vt *vt, long narg, long *argtab)
{
    struct vtstate *state = &vt->state;

    if (!narg) {
        state->scrolltop = 0;
        state->scrollbottom = state->nrow - 1;
    } if (narg == 2) {
        state->scrolltop = argtab[0];
        state->scrollbottom = argtab[1];
    }

    return narg;
}

long
vtscrolldown(struct vt *vt, long narg, long *argtab)
{
    vtscroll(vt, -1);

    return -1;
}

long
vtscrollup(struct vt *vt, long narg, long *argtab)
{
    vtscroll(vt, 1);

    return 1;
}

long
vtsettab(struct vt *vt, long narg, long *argtab)
{
//    long     row = vt->state.row;
    long      col = vt->state.col;
    uint32_t *tabmap = vt->state.tabmap;

    setbit(tabmap, col);

    return col;
}

long
vtclrtab(struct vt *vt, long narg, long *argtab)
{
    uint32_t *tabmap = vt->state.tabmap;
    long      ncol;
    long      col;
    long      n;

    if (narg == 1 && argtab[0] == 3) {
        ncol = vt->state.ncol;
        n = rounduppow2(ncol, 32);
        while (n--) {
            *tabmap++ = 0;
        }
    } else {
        col = vt->state.col;
        clrbit(tabmap, col);
    }

    return narg;
}

long
vtclralltabs(struct vt *vt, long narg, long *argtab)
{
    uint32_t *tabmap = vt->state.tabmap;
    size_t   n = rounduppow2(vt->state.ncol, 32);

    n >>= 3;
    while (n--) {
        *tabmap++ = 0;
    }

    return 0;
}

long
vteraseline(struct vt *vt, long narg, long *argtab)
{
    long arg = 0;

    if (narg == 0) {
        /* erase to end of line */
    } else {
        arg = argtab[0];
        if (arg == 1) {
            /* erase to start of line */
        } else if (arg == 2) {
            /* erase entire line */
        }
    }

    return arg;
}

long
vterasedir(struct vt *vt, long narg, long *argtab)
{
    long arg = 0;

    if (narg == 0) {
        /* erase down */
    } else {
        arg = argtab[0];
        if (arg == 1) {
            /* erase up */
        } else if (arg == 2) {
            /* erase screen and move cursor to home */
        }
    }

    return arg;
}

void
vtresetatr(struct vt *vt)
{
    struct vtstate *state = &vt->state;

    state->fgcolor = VTDEFFGCOLOR;
    state->bgcolor = VTDEFBGCOLOR;
    state->textatr = VTDEFTEXTATR;

    return;
}

long
vtescsetatr(struct vt *vt, long narg, long *argtab)
{
    long         n;
    long         cmd;

    for (n = 0 ; n < narg ; n++) {
        cmd = argtab[n];
        if (cmd >= 0 && cmd <= 8) {
            /* set attribute bits */
            if (!cmd) {
                vtresetatr(vt);
            } else {
                vt->state.textatr |= vtatrbittab[cmd];
            }
        } else if (cmd >= 30 && cmd <= 37) {
            /* set foreground color */
            vt->state.fgcolor = vtdefcolor(vtfgtodefcolor(cmd));
        } else if (cmd >= 40 && cmd <= 47) {
            /* set background color */
            vt->state.bgcolor = vtdefcolor(vtbgtodefcolor(cmd));
        }
    }

    return narg;
}

void
vtinitesc(void)
{
    vtsetcsicmd('r');
    vtsetcsifunc('r', vtsetscroll);
    vtsetcsicmd('D');
    vtsetcsifunc('D', vtscrolldown);
    vtsetcsicmd('M');
    vtsetcsifunc('M', vtscrollup);
    vtsetcsicmd('H');
    vtsetcsifunc('H', vtsettab);
    vtsetcsicmd('g');
    vtsetcsifunc('g', vtclrtab);
    vtsetcsicmd('J');
    vtsetcsifunc('J', vterasedir);
    vtsetcsicmd('K');
    vtsetcsifunc('K', vteraseline);
    vtsetcsicmd('m');
    vtsetcsifunc('m', vtescsetatr);
}

long
vtescgetnum(char *str, char **retstr)
{
    long num = 0;

    while (isdigit(*str)) {
        num *= 10;
        num += *str - '0';
        str++;
    }
    if (retstr) {
        *retstr = str;
    }

    return num;
}

char *
vtescgetstr(char *str, char **retstr)
{
    long  nb = 32;
    long  len = 0;
    char *ptr = malloc(nb);

    if (!ptr) {
        fprintf(stderr, "out of memory\n");

        exit(1);
    }
    while (*str != '"') {
        if (len == nb) {
            nb <<= 1;
            ptr = realloc(ptr, nb);
            if (!ptr) {
                fprintf(stderr, "out of memory\n");

                exit(1);
            }
        }
        *ptr++ = *str++;
        len++;
    }
    if (len == nb) {
        nb <<= 1;
        ptr = realloc(ptr, nb);
        if (!ptr) {
            fprintf(stderr, "out of memory\n");
            
            exit(1);
        }
    }
    ptr[len] = '\0';
    if ((len) && retstr) {
        *retstr = str;
    }

    return ptr;
}

#define VTESCNARG 32
void
vtescparse(struct vt *vt, char *str, char **retstr)
{
    long         num = 0;
    long         cmd;
    vtescfunc_t *func;
    long         narg = 1;
    long         argtab[VTESCNARG];
    char        *ptr;

    while (*str == VTESC) {
        str++;
        if (*str == VTCSI) {
            /* ESC[ command */
            str++;
            while (isdigit(*str)) {
                /* read first numerical parameter */
                num = vtescgetnum(str, &str);
                if (num && narg < VTESCNARG) {
                    argtab[narg] = num;
                    narg++;
                }
                if (*str == ';') {
                    str++;
                }
                if (*str == '"') {
                    /* associate string to key */
                    ptr = vtescgetstr(str, &str);
                    if (num < 0x80) {
                        vtkeystrtab[num] = ptr;
                    }
                }
            }
            cmd = *str;
            if (vtiscsicmd(cmd)) {
                str++;
                func = vtesc.csifunctab[cmd];
                func(vt, narg, argtab);
            }
        } else if (*str == VTCHARSET) {
            /* ESC( command */
            /* TODO: implement these? */
            ;
        } else if (*str == VTHASH) {
            /* ESC# command */
            if (str[1] == '8') {
                /* TODO: fill screen with E */
            } else {
                /* TODO: 3, 4, 5, 6 */
            }
        } else {
            /* TODO: ESC command */
        }
    }
    if (retstr) {
        *retstr = str;
    }

    return;
}

