/*
 * TODO
 * ----
 * - visualisation; DAT is blue, program #1 is green, program #2 is yellow
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <corewar/cw.h>
#include <corewar/rc.h>
#include <corewar/zeus.h>
#if (ZEUSWINX11)
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/Xutil.h>
#endif

extern long            rcnargtab[CWNOP];
extern const char     *cwopnametab[CWNOP];
extern struct cwinstr *cwoptab;

#define ZEUSDEFLINE 256

char *
zeusdisasm(struct cwinstr *op, int *lenret)
{
    char   *ptr = malloc(ZEUSDEFLINE * sizeof(char));
    char   *str = ptr;
    size_t  len = ZEUSDEFLINE;
    int     ret;
    char    ch;

    if (op) {
        ret = snprintf(str, len, "\t%s\t", cwopnametab[op->op]);
        if (ret < 0) {
            fprintf(stderr, "failed to construct debug line\n");
            
            exit(1);
        }
        len -= ret;
        str += ret;
        ret = snprintf(str, len, "\t"); 
        if (ret < 0) {
            fprintf(stderr, "failed to construct debug line\n");
            
            exit(1);
        }
        len -= ret;
        str += ret;
        if (rcnargtab[op->op] == 2) {
            ch = '\0';
            if (op->aflg & CWIMMBIT) {
                ch = '#';
            } else if (op->aflg & CWINDIRBIT) {
                ch = '@';
            } else if (op->aflg & CWPREDECBIT) {
                ch = '<';
            }
            if (ch) {
                ret = snprintf(str, len, "%c", ch);
                if (ret < 0) {
                    fprintf(stderr, "failed to construct debug line\n");
                    
                    exit(1);
                }
                len -= ret;
                str += ret;
            }
            if (op->aflg & CWSIGNBIT) {
                ret = snprintf(str, len, "%d", op->a - CWNCORE);
                if (ret < 0) {
                    fprintf(stderr, "failed to construct debug line\n");
                    
                    exit(1);
                }
                len -= ret;
                str += ret;
            } else {
                ret = snprintf(str, len, "%d", op->a);
                if (ret < 0) {
                    fprintf(stderr, "failed to construct debug line\n");
                    
                    exit(1);
                }
                len -= ret;
                str += ret;
            }
        }
        ch = '\0';
        if (op->bflg & CWIMMBIT) {
            ch = '#';
        } else if (op->bflg & CWINDIRBIT) {
            ch = '@';
        } else if (op->aflg & CWPREDECBIT) {
            ch = '<';
        }
        if (ch) {
            ret = snprintf(str, len, "\t%c", ch);
            if (ret < 0) {
                fprintf(stderr, "failed to construct debug line\n");
                
                exit(1);
            }
            len -= ret;
            str += ret;
        } else {
            ret = snprintf(str, len, "\t");
            if (ret < 0) {
                fprintf(stderr, "failed to construct debug line\n");
                
                exit(1);
            }
            len -= ret;
            str += ret;
        }
        if (op->bflg & CWSIGNBIT) {
            ret = snprintf(str, len, "%d\n", op->b - CWNCORE);
            if (ret < 0) {
                fprintf(stderr, "failed to construct debug line\n");
                
                exit(1);
            }
            len -= ret;
            str += ret;
        } else {
            ret = snprintf(str, len, "%d\n", op->b);
            if (ret < 0) {
                fprintf(stderr, "failed to construct debug line\n");
                
                exit(1);
            }
            len -= ret;
            str += ret;
        }
        if (len) {
            *str = '\0';
            *lenret = ZEUSDEFLINE - len;
        } else {
            *lenret = 0;
        }
    }

    return ptr;
}

void
zeusshowmem(void)
{
    char           *cp;
    struct cwinstr *op;
    long            l;
    int             dummy;

    for (l = 0 ; l < CWNCORE ; l++) {
        op = &cwoptab[l];
        if (*(uint64_t *)op) {
            fprintf(stderr, "%ld\t", l);
            cp = zeusdisasm(op, &dummy);
            if (!cp) {
                fprintf(stderr, "failed to allocate memory\n");

                exit(1);
            }
            fprintf(stderr, "%s\n", cp);
            free(cp);
        }
    }
}

