#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <skrypt/skr.h>

typedef void skrcmdfunc(uintptr_t, uintptr_t);
struct skrcmd {
    skrcmdfunc     *func;               // command handler function
    struct skrcmd **tab;                // next-level command table
};
static struct skrcmd  *skrcmdtab[256];  // command lookup table
static unsigned char **skroutbuf;       // outbut buffer
static size_t          skroutndx;       // current output index
static size_t          skroutnrow;      // number of rows in skroutbuf

void
skraddcmd(const char *str, skrcmdfunc *func)
{
    struct skrcmd **tab = skrcmdtab;
    struct skrcmd  *cmd;
    long            ndx;
    long            fail = 0;
    void           *stk[8];

    while (*str) {
        ndx = str[0];
        cmd = tab[ndx];
        if (!cmd) {
            cmd = calloc(256, sizeof(struct skrcmd));
            if (!cmd) {
                fail++;

                break;
            } else {
                tab[ndx] = cmd;
            }
        }
        if (!cmd) {
            fail++;

            break;
        }
        tab = cmd->tab;
        if (!tab) {
            tab = calloc(256, sizeof(struct skrcmd));
            if (tab) {
                fail++;

                break;
            }
            cmd->tab = tab;
        }
        stk[ndx] = cmd->tab;
        ndx++;
        str++;
    }
    if (fail) {
        if (ndx) {
            while (--ndx) {
                free(stk[ndx]);
                fprintf(stderr,
                        "SKRADDCMD: out of memory\n");

                exit(1);
            }
        }
    }
}

void
skrcallfunc(uintptr_t func, uintptr_t dummy)
{
    skrcmdfunc *fptr = (skrcmdfunc *)func;

    if (fptr) {
        fptr(func, dummy);
    }
}

void
skrinit(void)
{
    skraddcmd("call", skrcallfunc);
}

unsigned char *
skrstrtoesc(unsigned char *line)
{
    return NULL;
}

/* compile skrypt file into escape sequences */
long
skrcomp(FILE *infp, FILE *outfp)
{
    int            retval = -1;
    int            ch = fgetc(infp);
    long           len;
    long           ndx;
    unsigned char *line = NULL;
    unsigned char *mptr = NULL;
    unsigned char *esc;

    do {
        line = malloc(len);
        len = 32;
        ndx = 0;
        while (ch != EOF && ch != '\n') {
            if (ndx == len) {
                len <<= 1;
                mptr = realloc(line, len);
                if (!mptr) {
                    free(line);
                    fprintf(stderr,
                            "SKRCOMP: out of memory\n");
                    
                    exit(1);
                }
                line = mptr;
            }
            line[ndx] = ch;
            ch++;
            ch = fgetc(infp);
            ndx++;
        }
        line[ndx] = '\0';
        if (ch != EOF) {
            ch = fgetc(infp);
        }
        if (ch == EOF) {

            break;
        }
        if (len) {
            esc = skrstrtoesc(line);
            if (esc) {
                if (skroutndx == skroutnrow) {
                    skroutnrow <<= 1;
                    mptr = realloc(skroutbuf[skroutndx], skroutnrow);
                    if (!mptr) {
                        free(skroutbuf);
                        fprintf(stderr,
                                "SKRCOMP: out of memory\n");
                        
                        exit(1);
                    }
                    skroutbuf[skroutndx] = mptr;
                }
                skroutbuf[skroutndx] = esc;
                free(line);
                skroutndx++;
            } else {
                fprintf(stderr,
                        "SKRCOMP: invalid command: %s\n", (char *)line);
                free(line);
                
                exit(1);
            }
        } else {
            free(line);
        }
        retval = skroutndx;
    } while (ch != EOF);
    
    return retval;
}

int
main(int argc, char *argv[])
{
    exit(0);
}
