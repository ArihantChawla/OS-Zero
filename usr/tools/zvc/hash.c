#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <zvc/cal.h>

#define NNAMEHASH 4096
static struct caltoken *namehash[NNAMEHASH];

void
caladdname(struct caltoken *token)
{
    long  key = 0;
    char *cp = token->str;

    if ((cp) && calisname(*cp)) {
        key += *cp++;
        while (calisname(*cp) || isdigit(*cp)) {
            key <<= 4;
            key += *cp++;
        }
        key &= (NNAMEHASH - 1);
        token->prev = NULL;
        token->next = namehash[key];
        if (token->next) {
            token->next->prev = token;
        }
        namehash[key] = token;
    }

    return;
}

struct caltoken *
calfindname(char *str)
{
    struct caltoken *tok = NULL;
    long             key = 0;
    char            *cp = str;

    if ((cp) && calisname(*cp)) {
        key += *cp++;
        while (calisname(*cp) || isdigit(*cp)) {
            key <<= 4;
            key += *cp++;
        }
        key &= (NNAMEHASH - 1);
        tok = namehash[key];
        while ((tok) && strcmp(tok->str, str)) {
            tok = tok->next;
        }
    }

    return tok;
}

