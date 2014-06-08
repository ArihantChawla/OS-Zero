#include <ctype.h>
#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <vt/vt.h>

typedef long vtescfunc_t(struct vt *vt, long num1, long num2);

#define vtisesccmd(c)  bitset(vtesccmdmap, c)
#define vtiscsicmd(c)  bitset(vtcsicmdmap, c)
#define vtishashcmd(c) bitset(vthashcmdmap, c)
uint8_t      vtesccmdmap[32] ALIGNED(CLSIZE);
uint8_t      vtcsicmdmap[32] ALIGNED(CLSIZE);
uint8_t      vthashcmdmap[32] ALIGNED(CLSIZE);
vtescfunc_t *vtescfunctab[256];
vtescfunc_t *vtcsifunctab[256];
vtescfunc_t *vthashfunctab[256];

long
vtescgetnum(char *str, char **retstr)
{
    long num = 0;

    while (isdigit(*str)) {
        num *= 10;
        num += *str - '0';
        str++;
    }
    *retstr = str;

    return num;
}

void
vtescparse(struct vt *vt, char *str, char **retstr)
{
    char        *ptr = str;
    long         num1 = 0;
    long         num2 = 0;
    long         cmd;
    vtescfunc_t *func;

    while (*ptr == VTESC) {
        ptr++;
        if (*ptr == VTCSI) {
            /* ESC[ command */
            ptr++;
            if (isdigit(*ptr)) {
                /* read first numerical parameter */
                num1 = vtescgetnum(ptr, &ptr);
                if (*ptr == ';') {
                    /* read second numerical parameter */
                    num2 = vtescgetnum(ptr, &ptr);
                }
            }
            cmd = *ptr++;
            if (vtiscsicmd(cmd)) {
                func = vtcsifunctab[cmd];
                func(vt, num1, num2);
            } else {
                /* invalid command */
                ptr--;
            }
        } else if (*ptr == VTCHARSET) {
            /* ESC( command */
            /* TODO: implement these? */
            ;
        } else if (*ptr == VTHASH) {
            /* ESC# command */
            if (ptr[1] == '8') {
                /* TODO: fill screen with E */
            } else {
                /* TODO: 3, 4, 5, 6 */
            }
        } else {
            /* TODO: ESC command */
        }
    }
}

