#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <skrypt/skr.h>

typedef void skrcmdfunc(unsigned char *, unsigned char **);
struct skrcmd {
    skrcmdfunc     *func;               // command handler function
    struct skrcmd **tab;                // next-level command table
};
static struct skrcmd  *skrcmdtab[256];  // command lookup table
static unsigned char **skroutbuf;       // outbut buffer
static size_t          skroutndx;       // current output index
static size_t          skroutnrow;      // number of rows in skroutbuf

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
                            "SKRCOMP: failed to allocate line buffer\n");
                    
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
                                "SKRCOMP: failed to allocate output buffer\n");
                        
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
