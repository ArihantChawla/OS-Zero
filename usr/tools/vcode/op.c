#include <stdlib.h>
#include <vcode/vc.h>

struct vcvec *
vcaddv(struct vcvec *src, struct vcvec *dest)
{
    struct vcvec *vec = NULL;
    size_t        n = dest->nval;
    long          l;
    

    if (n != src->nval) {

        return NULL;
    }
    vec = malloc(sizeof(struct vcvec));
    vec->nval = n;
    vec->data = malloc(n * sizeof(struct vcval));
    for (l = 0 ; l < n ; l++) {
        if (dest->data[l].type == VC_INT
            && src->data[l].type == VC_INT) {
            vec->data[l].type = VC_INT;
            vec->data[l].data.i = dest->data[l].data.i + src->data[l].data.i;
        } else if (dest->data[l].type == VC_FLOAT
                   && src->data[l].type == VC_FLOAT) {
            vec->data[l].type = VC_FLOAT;
            vec->data[l].data.f = dest->data[l].data.f + src->data[l].data.f;
        } else if (dest->data[l].type == VC_INT
                   && src->data[l].type == VC_FLOAT) {
            vec->data[l].type = VC_FLOAT;
            vec->data[l].data.f = dest->data[l].data.i + src->data[l].data.f;
        } else if (dest->data[l].type == VC_FLOAT
                   && src->data[l].type == VC_INT) {
            vec->data[l].type = VC_FLOAT;
            vec->data[l].data.f = dest->data[l].data.f + src->data[l].data.i;
        } else {
            free(vec->data);
            free(vec);
            
            return NULL;
        }
    }

    return vec;
}

struct vcvec *
vcadds(struct vcvec *src, struct vcvec *dest)
{
    struct vcvec *vec = NULL;
    size_t        n = dest->nval;
    long          l;
    vcint         ival;
    vcfloat       fval;
    
    vec = malloc(sizeof(struct vcvec));
    vec->nval = n;
    vec->data = malloc(n * sizeof(struct vcval));
    if (src->data[0].type == VC_INT) {
        ival = src->data[0].data.i;
        for (l = 0 ; l < n ; l++) {
            if (dest->data[l].type == VC_INT) {
                vec->data[l].type = VC_INT;
                vec->data[l].data.i = dest->data[l].data.i + ival;
            } else if (dest->data[l].type == VC_FLOAT) {
                vec->data[l].type = VC_FLOAT;
                vec->data[l].data.f = dest->data[l].data.f + ival;
            } else {
                free(vec->data);
                free(vec);
                
                return NULL;
            }
        }
    } else {
        fval = src->data[0].data.f;
        for (l = 0 ; l < n ; l++) {
            if (dest->data[l].type == VC_INT) {
                vec->data[l].type = VC_FLOAT;
                vec->data[l].data.i = dest->data[l].data.i + fval;
            } else if (dest->data[l].type == VC_FLOAT) {
                vec->data[l].type = VC_FLOAT;
                vec->data[l].data.f = dest->data[l].data.f + fval;
            } else {
                free(vec->data);
                free(vec);
                
                return NULL;
            }
        }
    }
        
    return vec;
}

struct vcvec *
vcsubv(struct vcvec *src, struct vcvec *dest)
{
    struct vcvec *vec = NULL;
    size_t        n = dest->nval;
    long          l;
    

    if (n != src->nval) {

        return NULL;
    }
    vec = malloc(sizeof(struct vcvec));
    vec->nval = n;
    vec->data = malloc(n * sizeof(struct vcval));
    for (l = 0 ; l < n ; l++) {
        if (dest->data[l].type == VC_INT
            && src->data[l].type == VC_INT) {
            vec->data[l].type = VC_INT;
            vec->data[l].data.i = dest->data[l].data.i - src->data[l].data.i;
        } else if (dest->data[l].type == VC_FLOAT
                   && src->data[l].type == VC_FLOAT) {
            vec->data[l].type = VC_FLOAT;
            vec->data[l].data.f = dest->data[l].data.f - src->data[l].data.f;
        } else if (dest->data[l].type == VC_INT
                   && src->data[l].type == VC_FLOAT) {
            vec->data[l].type = VC_FLOAT;
            vec->data[l].data.f = dest->data[l].data.i - src->data[l].data.f;
        } else if (dest->data[l].type == VC_FLOAT
                   && src->data[l].type == VC_INT) {
            vec->data[l].type = VC_FLOAT;
            vec->data[l].data.f = dest->data[l].data.f - src->data[l].data.i;
        } else {
            free(vec->data);
            free(vec);
            
            return NULL;
        }
    }

    return vec;
}

struct vcvec *
vcsubs(struct vcvec *src, struct vcvec *dest)
{
    struct vcvec *vec = NULL;
    size_t        n = dest->nval;
    long          l;
    vcint         ival;
    vcfloat       fval;
    
    vec = malloc(sizeof(struct vcvec));
    vec->nval = n;
    vec->data = malloc(n * sizeof(struct vcval));
    if (src->data[0].type == VC_INT) {
        ival = src->data[0].data.i;
        for (l = 0 ; l < n ; l++) {
            if (dest->data[l].type == VC_INT) {
                vec->data[l].type = VC_INT;
                vec->data[l].data.i = dest->data[l].data.i - ival;
            } else if (dest->data[l].type == VC_FLOAT) {
                vec->data[l].type = VC_FLOAT;
                vec->data[l].data.f = dest->data[l].data.f - ival;
            } else {
                free(vec->data);
                free(vec);
                
                return NULL;
            }
        }
    } else {
        fval = src->data[0].data.f;
        for (l = 0 ; l < n ; l++) {
            if (dest->data[l].type == VC_INT) {
                vec->data[l].type = VC_FLOAT;
                vec->data[l].data.i = dest->data[l].data.i - fval;
            } else if (dest->data[l].type == VC_FLOAT) {
                vec->data[l].type = VC_FLOAT;
                vec->data[l].data.f = dest->data[l].data.f - fval;
            } else {
                free(vec->data);
                free(vec);
                
                return NULL;
            }
        }
    }
        
    return vec;
}

struct vcvec *
vcmulv(struct vcvec *src, struct vcvec *dest)
{
    struct vcvec *vec = NULL;
    size_t        n = dest->nval;
    long          l;
    

    if (n != src->nval) {

        return NULL;
    }
    vec = malloc(sizeof(struct vcvec));
    vec->nval = n;
    vec->data = malloc(n * sizeof(struct vcval));
    for (l = 0 ; l < n ; l++) {
        if (dest->data[l].type == VC_INT
            && src->data[l].type == VC_INT) {
            vec->data[l].type = VC_INT;
            vec->data[l].data.i = dest->data[l].data.i * src->data[l].data.i;
        } else if (dest->data[l].type == VC_FLOAT
                   && src->data[l].type == VC_FLOAT) {
            vec->data[l].type = VC_FLOAT;
            vec->data[l].data.f = dest->data[l].data.f * src->data[l].data.f;
        } else if (dest->data[l].type == VC_INT
                   && src->data[l].type == VC_FLOAT) {
            vec->data[l].type = VC_FLOAT;
            vec->data[l].data.f = dest->data[l].data.i * src->data[l].data.f;
        } else if (dest->data[l].type == VC_FLOAT
                   && src->data[l].type == VC_INT) {
            vec->data[l].type = VC_FLOAT;
            vec->data[l].data.f = dest->data[l].data.f * src->data[l].data.i;
        } else {
            free(vec->data);
            free(vec);
            
            return NULL;
        }
    }

    return vec;
}

struct vcvec *
vcmuls(struct vcvec *src, struct vcvec *dest)
{
    struct vcvec *vec = NULL;
    size_t        n = dest->nval;
    long          l;
    vcint         ival;
    vcfloat       fval;
    
    vec = malloc(sizeof(struct vcvec));
    vec->nval = n;
    vec->data = malloc(n * sizeof(struct vcval));
    if (src->data[0].type == VC_INT) {
        ival = src->data[0].data.i;
        for (l = 0 ; l < n ; l++) {
            if (dest->data[l].type == VC_INT) {
                vec->data[l].type = VC_INT;
                vec->data[l].data.i = dest->data[l].data.i * ival;
            } else if (dest->data[l].type == VC_FLOAT) {
                vec->data[l].type = VC_FLOAT;
                vec->data[l].data.f = dest->data[l].data.f * ival;
            } else {
                free(vec->data);
                free(vec);
                
                return NULL;
            }
        }
    } else {
        fval = src->data[0].data.f;
        for (l = 0 ; l < n ; l++) {
            if (dest->data[l].type == VC_INT) {
                vec->data[l].type = VC_FLOAT;
                vec->data[l].data.i = dest->data[l].data.i * fval;
            } else if (dest->data[l].type == VC_FLOAT) {
                vec->data[l].type = VC_FLOAT;
                vec->data[l].data.f = dest->data[l].data.f * fval;
            } else {
                free(vec->data);
                free(vec);
                
                return NULL;
            }
        }
    }
        
    return vec;
}

struct vcvec *
vcdivv(struct vcvec *src, struct vcvec *dest)
{
    struct vcvec *vec = NULL;
    size_t        n = dest->nval;
    long          l;
    

    if (n != src->nval) {

        return NULL;
    }
    vec = malloc(sizeof(struct vcvec));
    vec->nval = n;
    vec->data = malloc(n * sizeof(struct vcval));
    for (l = 0 ; l < n ; l++) {
        if (dest->data[l].type == VC_INT
            && src->data[l].type == VC_INT) {
            vec->data[l].type = VC_INT;
            vec->data[l].data.i = dest->data[l].data.i / src->data[l].data.i;
        } else if (dest->data[l].type == VC_FLOAT
                   && src->data[l].type == VC_FLOAT) {
            vec->data[l].type = VC_FLOAT;
            vec->data[l].data.f = dest->data[l].data.f / src->data[l].data.f;
        } else if (dest->data[l].type == VC_INT
                   && src->data[l].type == VC_FLOAT) {
            vec->data[l].type = VC_FLOAT;
            vec->data[l].data.f = dest->data[l].data.i / src->data[l].data.f;
        } else if (dest->data[l].type == VC_FLOAT
                   && src->data[l].type == VC_INT) {
            vec->data[l].type = VC_FLOAT;
            vec->data[l].data.f = dest->data[l].data.f / src->data[l].data.i;
        } else {
            free(vec->data);
            free(vec);
            
            return NULL;
        }
    }

    return vec;
}

struct vcvec *
vcdivs(struct vcvec *src, struct vcvec *dest)
{
    struct vcvec *vec = NULL;
    size_t        n = dest->nval;
    long          l;
    vcint         ival;
    vcfloat       fval;
    
    vec = malloc(sizeof(struct vcvec));
    vec->nval = n;
    vec->data = malloc(n * sizeof(struct vcval));
    if (src->data[0].type == VC_INT) {
        ival = src->data[0].data.i;
        for (l = 0 ; l < n ; l++) {
            if (dest->data[l].type == VC_INT) {
                vec->data[l].type = VC_INT;
                vec->data[l].data.i = dest->data[l].data.i / ival;
            } else if (dest->data[l].type == VC_FLOAT) {
                vec->data[l].type = VC_FLOAT;
                vec->data[l].data.f = dest->data[l].data.f / ival;
            } else {
                free(vec->data);
                free(vec);
                
                return NULL;
            }
        }
    } else {
        fval = src->data[0].data.f;
        for (l = 0 ; l < n ; l++) {
            if (dest->data[l].type == VC_INT) {
                vec->data[l].type = VC_FLOAT;
                vec->data[l].data.i = dest->data[l].data.i / fval;
            } else if (dest->data[l].type == VC_FLOAT) {
                vec->data[l].type = VC_FLOAT;
                vec->data[l].data.f = dest->data[l].data.f / fval;
            } else {
                free(vec->data);
                free(vec);
                
                return NULL;
            }
        }
    }
        
    return vec;
}

struct vcvec *
vcmodv(struct vcvec *src, struct vcvec *dest)
{
    struct vcvec *vec = NULL;
    size_t        n = dest->nval;
    long          l;
    

    if (n != src->nval) {

        return NULL;
    }
    vec = malloc(sizeof(struct vcvec));
    vec->nval = n;
    vec->data = malloc(n * sizeof(struct vcval));
    for (l = 0 ; l < n ; l++) {
        if (dest->data[l].type == VC_INT
            && src->data[l].type == VC_INT) {
            vec->data[l].type = VC_INT;
            vec->data[l].data.i = dest->data[l].data.i % src->data[l].data.i;
        } else if (dest->data[l].type == VC_FLOAT
                   && src->data[l].type == VC_FLOAT) {
            vec->data[l].type = VC_FLOAT;
            vec->data[l].data.f = dest->data[l].data.f % src->data[l].data.f;
        } else if (dest->data[l].type == VC_INT
                   && src->data[l].type == VC_FLOAT) {
            vec->data[l].type = VC_FLOAT;
            vec->data[l].data.f = dest->data[l].data.i % src->data[l].data.f;
        } else if (dest->data[l].type == VC_FLOAT
                   && src->data[l].type == VC_INT) {
            vec->data[l].type = VC_FLOAT;
            vec->data[l].data.f = dest->data[l].data.f % src->data[l].data.i;
        } else {
            free(vec->data);
            free(vec);
            
            return NULL;
        }
    }

    return vec;
}

struct vcvec *
vcmods(struct vcvec *src, struct vcvec *dest)
{
    struct vcvec *vec = NULL;
    size_t        n = dest->nval;
    long          l;
    vcint         ival;
    vcfloat       fval;
    
    vec = malloc(sizeof(struct vcvec));
    vec->nval = n;
    vec->data = malloc(n * sizeof(struct vcval));
    if (src->data[0].type == VC_INT) {
        ival = src->data[0].data.i;
        for (l = 0 ; l < n ; l++) {
            if (dest->data[l].type == VC_INT) {
                vec->data[l].type = VC_INT;
                vec->data[l].data.i = dest->data[l].data.i % ival;
            } else if (dest->data[l].type == VC_FLOAT) {
                vec->data[l].type = VC_FLOAT;
                vec->data[l].data.f = dest->data[l].data.f % ival;
            } else {
                free(vec->data);
                free(vec);
                
                return NULL;
            }
        }
    } else {
        fval = src->data[0].data.f;
        for (l = 0 ; l < n ; l++) {
            if (dest->data[l].type == VC_INT) {
                vec->data[l].type = VC_FLOAT;
                vec->data[l].data.i = dest->data[l].data.i % fval;
            } else if (dest->data[l].type == VC_FLOAT) {
                vec->data[l].type = VC_FLOAT;
                vec->data[l].data.f = dest->data[l].data.f % fval;
            } else {
                free(vec->data);
                free(vec);
                
                return NULL;
            }
        }
    }
        
    return vec;
}

