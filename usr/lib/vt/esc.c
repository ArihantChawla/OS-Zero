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

void
vtescparse(struct vt *vt, char *str, char **retstr)
{
    long         num1 = 0;
    long         num2 = 0;
    long         cmd;
    vtescfunc_t *func;

    while (*str == VTESC) {
        str++;
        if (*str == VTCSI) {
            /* ESC[ command */
            str++;
            if (isdigit(*str)) {
                /* read first numerical parameter */
                num1 = vtescgetnum(str, &str);
                if (*str == ';') {
                    str++;
                    /* read second numerical parameter */
                    num2 = vtescgetnum(str, &str);
                }
            }
            cmd = *str;
            if (vtiscsicmd(cmd)) {
                str++;
                func = vtesc.csifunctab[cmd];
                func(vt, num1, num2);
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

