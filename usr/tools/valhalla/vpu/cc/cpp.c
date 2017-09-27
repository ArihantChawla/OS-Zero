#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zero/trix.h>
#include <cc/cc.h>

struct cctoken *ccprocif(struct cctoken *, struct cctoken **,
                          struct cctoken **);
struct cctoken *ccprocifdef(struct cctoken *, struct cctoken **,
                             struct cctoken **);
struct cctoken *ccprocifndef(struct cctoken *, struct cctoken **,
                              struct cctoken **);
struct cctoken *ccprocdefine(struct cctoken *, struct cctoken **,
                              struct cctoken **);

cctokenfunc_t *cppfunctab[16] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ccprocif,           // CC_IF_DIR
    NULL,               // CC_ELIF_DIR
    NULL,               // CC_ELSE_DIR
    NULL,               // CC_ENDIF_DIR
    ccprocifdef,        // CC_IFDEF_DIR
    ccprocifndef,       // CC_IFNDEF_DIR
    ccprocdefine        // CC_DEFINE_DIR
};

#if 0
struct cctoken *
ccprocif(struct cctoken *token, struct cctoken **nextret,
          struct cctoken **tailret)
{
    struct cctoken *head = NULL;
    struct cctoken *tail = NULL;
    long            block = 0;
    long            skip = 0;
    long            loop = 1;

    token = token->next;
    skip = !cceval(token, &token);
    while (loop) {
        while (token) {
            if (!skip) {
                ccqueuetoken(token, &head, &tail);
            }
            token = token->next;
            if (token->type == CC_ELSE_TOKEN) {
                if (skip) {
                    skip = !skip;
                    token = token->next;
                } else {
                    while (token->type != CC_ENDIF_TOKEN) {
                        token = token->next;
                    }
                    loop = 0;
                }
            } else if (token->type == CC_ELIF_TOKEN) {
                if (skip) {
                    skip = !cceval(token, &token);
                } else {
                    while (token->type != CC_ENDIF_TOKEN) {
                        token = token->next;
                    }
                    loop = 0;
                }
            } else if (token->type == CC_ENDIF_TOKEN) {
                loop = 0;
            }
        }
    }
    if (head) {
        *nextret = token;
    }

    return head;
}
#endif
struct cctoken *
ccprocif(struct cctoken *token, struct cctoken **nextret,
          struct cctoken **tailret)
{
    return NULL;
}

struct cctoken *
ccprocifdef(struct cctoken *token, struct cctoken **nextret,
             struct cctoken **tailret)
{
    return NULL;
}

struct cctoken *
ccprocifndef(struct cctoken *token, struct cctoken **nextret,
              struct cctoken **tailret)
{
    return NULL;
}

struct cctoken *
ccprocdefine(struct cctoken *token, struct cctoken **nextret,
              struct cctoken **tailret)
{
    return NULL;
}

