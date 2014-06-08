#include <ctype.h>
#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <vt/vt.h>

#define vtisesccmd(c)   bitset(vtesc.esccmdmap, c)
#define vtiscsicmd(c)   bitset(vtesc.csicmdmap, c)
#define vtishashcmd(c)  bitset(vtesc.hashcmdmap, c)
#define vtsetesccmd(c)  setbit(vtesc.esccmdmap, c)
#define vtsetcsicmd(c)  setbit(vtesc.csicmdmap, c)
#define vtsethashcmd(c) setbit(vtesc.hashcmdmap, c)

static struct vtesc vtesc ALIGNED(PAGESIZE);

void
vtinitesc(void)
{
    ;
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

#define VTESCNARG 32
void
vtescparse(struct vt *vt, char *str, char **retstr)
{
    long         num = 0;
    long         cmd;
    vtescfunc_t *func;
    long         narg = 1;
    long         argtab[VTESCNARG];

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

