#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zero/trix.h>
#include <rex/rex.h>

#if (TRANSLATE)

static uint8_t specbitmap[32];

static void
init(void)
{
    setspecbit('^');
    setspecbit('$');
    setspecbit('.');
    setspecbit('*');
    setspecbit('+');
    setspecbit('[');
    setspecbit(']');
}

#endif /* TRANSLATE */

static int
matchstar(int c, char *regexp, char *data)
{
    char *rp = regexp;
    char *cp = data;
    int   retval;

    retval ^= retval;
    do {
        retval = matchhere(&rp, cp);
        if (retval) {

            return retval;
        }
    } while ((*cp) && (*cp++ == c || c == '.'));
    
    return retval;
}

static int
matchhere(char **regexp, char *data)
{
    char *rp = *regexp;
    char *cp = data;
    char  ch;
    int   retval;

    retval ^= retval;
    if (*rp == '$' && !rp[1]) {
        retval = 1;
        rp++;
    } else if ((cp) && (rp)) {
        if (!*cp) {
            retval = (!rp || !*rp);
        } else if (!*rp) {
            retval = 1;
            rp = NULL;
        } else if (rp[1] == '*') {
            retval = matchstar(*rp, rp + 2, cp);
            rp += 2;
#if 0
        } else if (*rp == '?') {
            retval = 1;
            rp++;
#endif
        } else if (*rp == '$') {
            retval = (!*cp);
            rp++;
        } else if (*rp == '^') {
            rp++;
            retval = matchhere(&rp, cp);
        } else {
            ch = *cp;
            while ((ch) && ch != *rp) {
                ch = *++cp;
            }
            if (!ch) {
                retval = (*rp == '$' || !*rp);
            } else if (rp[1] == '$') {
                retval = ((ch) && (*rp == '.' || *rp == ch) && !*++cp);
                rp += 2;
            } else {
                while (*cp && (ch) && (ch == '.' || ch == *cp++)) {
                    ch = *++rp;
                }
                retval = matchhere(&rp, cp);
            }
        }
    }
    *regexp = rp;
    
    return retval;
}

#if (TRANSLATE)

struct rex *
translate(char *regexp)
{
    struct rex *queue;
    struct rex *rex1;
    struct rex *rex2;
    char       *rp = regexp;
    char        ch;

    queue = calloc(1, sizeof(struct rex));
    rex1 = queue;
    while (*rp) {
        if (*rp == '.') {
            rex1->type = DOT;
            rp++;
        } else if (*rp == '*') {
            rex1->type = STAR;
            rp++;
        } else if (*rp == '^') {
            rex1->type = BEGIN;
            rp++;
        } else if (*rp == '$') {
            rex1->type = END;
            rp++;
        } else if (*rp == '[') {
            rex1->type = SET;
            if (rp[2] == '-') {
                for (ch = rp[1] ; ch < rp[3] ; ch++) {
                    setcbit(rex1, ch);
                }
            } else {
                while (*rp != ']') {
                    setcbit(rex1, *rp);
                    rp++;
                }
                if (*rp != ']') {
                    fprintf(stderr, "invalid regular expression %s\n", regexp);
                    
                    exit(1);
                }
                rp++;
            }
        } else {
            rex1->type = CHAR;
            ch = *rp;
            setcbit(rex1, ch);
            rp++;
        }
        if (*rp) {
            rex2 = calloc(1, sizeof(struct rex));
            rex1->next = rex2;
            rex1 = rex2;
        }
    }

    return queue;
}

void
printrex(struct rex *rex)
{
    int i;

    fprintf(stderr, "TYPE: %x:", rex->type);
    for (i = 0 ; i < 32 ; i++) {
        fprintf(stderr, " %x", rex->cbits[i]);
    }
    fprintf(stderr, "\n");
}

void
print(struct rex *queue)
{
    struct rex *rex = queue;

    while (rex) {
        printrex(rex);
        rex = rex->next;
    }
}

#endif /* TRANSLATE */

int
main(int argc, char *argv[])
{
    char       *regexp = argv[1];
    char       *rexptr = strdup(regexp);
#if (TRANSLATE)
    struct rex *queue;
#endif
    char       *rp = rexptr;
    char       *ifile = (argc == 3) ? argv[2] : NULL;
    FILE       *fp;
    int         ch;
    int         ns;
    int         i;
    int         n = LINEDEFLEN;
    char       *linebuf = malloc(n);
    char       *cp = linebuf;
    char       *mptr;
    int         retval;

#if (TRANSLATE)
    init();
#endif
    if (ifile) {
        fp = fopen(ifile, "r");
    } else {
        fp = stdin;
    }
    i ^= i;
    ns ^= ns;
    if (fp) {
#if (TRANSLATE)
        queue = translate(regexp);
        print(queue);
        while (!feof(fp)) {
            retval ^= retval;
            ch = getc(fp);
            if (ch == EOF || ch == '\n') {
                cp = linebuf;
                linebuf[i] = '\0';
                retval = match(queue, cp);
                if (retval) {
                    printf("%s\n", linebuf);
                }
                i ^= i;
            } else {
                linebuf[i] = ch;
                if (i == n) {
                    ns++;
                    n = LINEDEFLEN << ns;
                    mptr = realloc(linebuf, n);
                    if (!mptr) {
                        free(linebuf);
                        fprintf(stderr, "out of memory\n");

                        exit(1);
                    }
                    linebuf = mptr;
                }
                i++;
            }
        }
#else /* TRANSLATE */
        while (!feof(fp)) {
            retval ^= retval;
            ch = getc(fp);
            if (ch == EOF || ch == '\n') {
                rp = rexptr;
                cp = linebuf;
                linebuf[i] = '\0';
                do {
                    retval = matchhere(&rp, cp);
                    if (!retval) {
                        i ^= i;
                        linebuf[0] = '\0';

                        continue;
                    }
                } while ((rp) && *rp && *cp);
                if (retval) {
                    printf("%s\n", linebuf);
                }
                i ^= i;
            } else {
                linebuf[i] = ch;
                if (i == n) {
                    ns++;
                    n = LINEDEFLEN << ns;
                    mptr = realloc(linebuf, n);
                    if (!mptr) {
                        free(linebuf);
                        fprintf(stderr, "out of memory\n");

                        exit(1);
                    }
                    linebuf = mptr;
                    linebuf[i] = '\0';
                }
                i++;
            }
        }
#endif
    }

    exit(0);
}

