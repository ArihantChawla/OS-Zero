#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <zvc/cal.h>

/* bitmap */
uint8_t                  calnametab[32] ALIGNED(CLSIZE);
static struct caltokenq *calfiletokenq;

/*
 * initialise lookup table
 * leave digits 0..9 unset to check for first character in names
 */
void
calinitnametab(void)
{
    int i;

    setbit(calnametab, '$');
    setbit(calnametab, '%');
    setbit(calnametab, '@');
    for (i = 'a' ; i <= 'z' ; i++) {
        setbit(calnametab, i);
    }
    for (i = 'A' ; i <= 'Z' ; i++) {
        setbit(calnametab, i);
    }
}

/* read input line */
char *
calgetline(FILE *fp)
{
    size_t  sz = 128;
    size_t  len = 0;
    char   *line = NULL;
    char   *cp;
    int     ch = !fp ? EOF : fgetc(fp);

    /* skip leading whitespace */
    while (isspace(ch) && ch != EOF) {
        ch = fgetc(fp);
    }
    if (ch != EOF) {
        line = malloc(sz);
        cp = line;
        do {
            /* copy line until EOF or '\n' */
            /* skip concatenation character ('_') */
            if (ch != '_') {
                *cp++ = (char)ch;
                len++;
                if (len == sz) {
                    sz <<= 1;
                    line = realloc(line, sz);
                        cp = &line[len];
                }
            }
            ch = fgetc(fp);
        } while (ch != EOF && ch != '\n');
        if (ch == '\n') {
            /* NUL-terminate */
                line[len] = '\0';
        }
    }

    return line;
}

/*
 * add tokens first..last into queue
 * if last is NULL, add just first
 */
static void
calqueue(struct caltoken **head, struct caltoken **tail,
         struct caltoken *first, struct caltoken *last)
{
    last->next = NULL;
    if (!*head) {
        first->prev = NULL;
        *head = first;
        *tail = last;
    } else if (*tail) {
        first->prev = *tail;
        (*tail)->next = first;
        if (last) {
            *tail = last;
        } else {
            *tail = first;
        }
    } else {
        (*head)->next = first;
        first->prev = *head;
        if (last) {
            *tail = last;
        } else {
            *tail = first;
        }
    }

    return;
}

/*
 * get micro string
 * terminate with NUL
 */
char *
calgetmicro(char *str, char **retstr)
{
    char *cp = str;

    if (str) {
        while ((*str) && *str != '"') {
            str++;
        }
        if (*str) {
            *str++ = '\0';
        }
    }
    if (cp != str) {
        *retstr = str;
    } else {
        cp = NULL;
    }

    return cp;
}

struct caltoken *
caltokenize(char *line, struct caltoken **tailret)
{
    struct caltoken *head = NULL;
    struct caltoken *tail = NULL;
    struct caltoken *tok;
    char            *cp = line;
    char            *ptr;
    char            *str;
    size_t           sz = 16;
    size_t           len = 0;
    
    if (cp) {
        while (isspace(*cp)) {
            cp++;
        }
        while (*cp) {
            if (*cp == '"') {
                cp++;
                ptr = cp;
                str = calgetmicro(cp, &cp);
                if (str) {
                    tok = malloc(sizeof(struct caltoken));
                    tok->type = CAL_MICRO_TOKEN;
                    tok->parm = CAL_NONE;
                    tok->str = str;
                    calqueue(&head, &tail, tok, NULL);
                } else {
                    fprintf(stderr, "unterminated micro %s\n", ptr);

                    exit(1);
                }
            } else if (calisname(*cp)) {
                tok = malloc(sizeof(struct caltoken));
                if (tok) {
                    tok->type = CAL_NAME_TOKEN;
                    tok->parm = CAL_NONE;
                    tok->str = malloc(sz);
                    if (tok->str) {
                        ptr = tok->str;
                        while (calisname(*cp) || isdigit(*cp)) {
                            *ptr++ = *cp++;
                            len++;
                            if (len == sz) {
                                sz <<= 1;
                                tok->str = realloc(tok->str, sz);
                                ptr = &tok->str[len];
                            }
                        }
                        *ptr = '\0';
                    }
                    if (len) {
                        calqueue(&head, &tail, tok, NULL);
                    }
                }
            }
        }
    }
    if (head) {
        *tailret = tail;
    }
    
    return head;
}

struct caltoken *
calreadfile(char *fname, struct caltoken **tailret)
{
    struct caltoken *head = NULL;
    struct caltoken *tail = NULL;
    struct caltoken *tok1;
    struct caltoken *tok2;
    struct caltoken *nltok;
    char            *line;
    char            *cp;
    FILE            *fp = fopen(fname, "r");
    long             nl = 0;
    long             comm = 0;
    
    if (fp) {
        line = calgetline(fp);
        while (line) {
            cp = &line[0];
            nl = 1;
            if (*cp == ',') {
                /* continuation line */
                cp++;
                nl = 0;
            } else if (*cp == '*') {
                /* comment */
                nl = 0;
                comm = 1;
            }
            if (!comm) {
                tok1 = caltokenize(line, &tok2);
                if (tok1) {
                    calqueue(&head, &tail, tok1, tok2);
                }
            }
            if (nl) {
                nltok = malloc(sizeof(struct caltoken));
                nltok->type = CAL_NEWLINE_TOKEN;
                nltok->parm = CAL_NONE;
                calqueue(&head, &tail, nltok, NULL);
                nl = 0;
            }
            line = calgetline(fp);
        }
        fclose(fp);
    }
    if (head) {
        *tailret = tail;
    }
    
    return head;
}

int
main(int argc, char *argv[])
{
    struct caltoken *tok;
    int              i = argc;
    
    if (argc < 2) {
        fprintf(stderr, "need file arguments\n");
        
        exit(1);
    }
    calfiletokenq = malloc(argc * sizeof(struct caltokenq));
    for (i = 1 ; i < argc ; i++) {
        tok = calreadfile(argv[i], &calfiletokenq[i].tail);
        if (tok) {
            calfiletokenq[i].head = tok;
        } else {
            fprintf(stderr, "error reading %s\n", argv[i]);
            
            exit(1);
        }
    }
    calfiletokenq[i].head = NULL;
    calfiletokenq[i].tail = NULL;
    
    exit(0);
}

