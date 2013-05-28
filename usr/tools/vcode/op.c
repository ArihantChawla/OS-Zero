#include <stdio.h>
#include <stdlib.h>
#include <vcode/vc.h>

/* add two vectors */
struct vcvec *
vcaddv(struct vcvec *src, struct vcvec *dest)
{
    struct vcvec *vec = NULL;
    size_t        n = dest->nval;
    long          t = dest->type;
    vcint         ival1;
    vcint         ival2;
    vcfloat       dval1;
    vcfloat       dval2;
    long          l;
    

    if (n != src->nval) {

        return NULL;
    }
    if (src->type != t) {

        return NULL;
    }
    vec = malloc(sizeof(struct vcvec));
    vec->type = t;
    vec->nval = n;
    vec->data = malloc(n * sizeof(struct vcval));
    if (t == VC_INT) {
        for (l = 0 ; l < n ; l++) {
            ival1 = dest->data[l].data.i;
            ival2 = src->data[l].data.i;
            vec->data[l].data.i = ival1 + ival2;
        }
    } else if (t == VC_FLOAT) {
        for (l = 0 ; l < n ; l++) {
            dval1 = dest->data[l].data.f;
            dval2 = src->data[l].data.f;
            vec->data[l].data.f = dval1 + dval2;
        }
    } else {
        fprintf(stderr, "invalid argument type for addition: %lx\n", t);
        free(vec->data);
        free(vec);
        vec = NULL;
    }

    return vec;
}

/* add two vectors */
struct vcvec *
vcsubv(struct vcvec *src, struct vcvec *dest)
{
    struct vcvec *vec = NULL;
    size_t        n = dest->nval;
    long          t = dest->type;
    vcint         ival1;
    vcint         ival2;
    vcfloat       dval1;
    vcfloat       dval2;
    long          l;
    

    if (n != src->nval) {

        return NULL;
    }
    if (src->type != t) {

        return NULL;
    }
    vec = malloc(sizeof(struct vcvec));
    vec->type = t;
    vec->nval = n;
    vec->data = malloc(n * sizeof(struct vcval));
    if (t == VC_INT) {
        for (l = 0 ; l < n ; l++) {
            ival1 = dest->data[l].data.i;
            ival2 = src->data[l].data.i;
            vec->data[l].data.i = ival1 - ival2;
        }
    } else if (t == VC_FLOAT) {
        for (l = 0 ; l < n ; l++) {
            dval1 = dest->data[l].data.f;
            dval2 = src->data[l].data.f;
            vec->data[l].data.f = dval1 - dval2;
        }
    } else {
        fprintf(stderr, "invalid argument type for subtraction: %lx\n", t);
        free(vec->data);
        free(vec);
        vec = NULL;
    }

    return vec;
}

/* add two vectors */
struct vcvec *
vcmulv(struct vcvec *src, struct vcvec *dest)
{
    struct vcvec *vec = NULL;
    size_t        n = dest->nval;
    long          t = dest->type;
    vcint         ival1;
    vcint         ival2;
    vcfloat       dval1;
    vcfloat       dval2;
    long          l;
    

    if (n != src->nval) {

        return NULL;
    }
    if (src->type != t) {

        return NULL;
    }
    vec = malloc(sizeof(struct vcvec));
    vec->type = t;
    vec->nval = n;
    vec->data = malloc(n * sizeof(struct vcval));
    if (t == VC_INT) {
        for (l = 0 ; l < n ; l++) {
            ival1 = dest->data[l].data.i;
            ival2 = src->data[l].data.i;
            vec->data[l].data.i = ival1 * ival2;
        }
    } else if (t == VC_FLOAT) {
        for (l = 0 ; l < n ; l++) {
            dval1 = dest->data[l].data.f;
            dval2 = src->data[l].data.f;
            vec->data[l].data.f = dval1 * dval2;
        }
    } else {
        fprintf(stderr, "invalid argument type for multiplication: %lx\n", t);
        free(vec->data);
        free(vec);
        vec = NULL;
    }

    return vec;
}

/* add two vectors */
struct vcvec *
vcdivv(struct vcvec *src, struct vcvec *dest)
{
    struct vcvec *vec = NULL;
    size_t        n = dest->nval;
    long          t = dest->type;
    vcint         ival1;
    vcint         ival2;
    vcfloat       dval1;
    vcfloat       dval2;
    long          l;
    

    if (n != src->nval) {

        return NULL;
    }
    if (src->type != t) {

        return NULL;
    }
    vec = malloc(sizeof(struct vcvec));
    vec->type = t;
    vec->nval = n;
    vec->data = malloc(n * sizeof(struct vcval));
    if (t == VC_INT) {
        for (l = 0 ; l < n ; l++) {
            ival1 = dest->data[l].data.i;
            ival2 = src->data[l].data.i;
            vec->data[l].data.i = ival1 / ival2;
        }
    } else if (t == VC_FLOAT) {
        for (l = 0 ; l < n ; l++) {
            dval1 = dest->data[l].data.f;
            dval2 = src->data[l].data.f;
            vec->data[l].data.f = dval1 / dval2;
        }
    } else {
        fprintf(stderr, "invalid argument type for division: %lx\n", t);
        free(vec->data);
        free(vec);
        vec = NULL;
    }

    return vec;
}

/* add two vectors */
struct vcvec *
vcmodv(struct vcvec *src, struct vcvec *dest)
{
    struct vcvec *vec = NULL;
    size_t        n = dest->nval;
    long          t = dest->type;
    vcint         ival1;
    vcint         ival2;
    long          l;
    

    if (n != src->nval) {

        return NULL;
    }
    if (src->type != t) {

        return NULL;
    }
    vec = malloc(sizeof(struct vcvec));
    vec->type = t;
    vec->nval = n;
    vec->data = malloc(n * sizeof(struct vcval));
    if (t == VC_INT) {
        for (l = 0 ; l < n ; l++) {
            ival1 = dest->data[l].data.i;
            ival2 = src->data[l].data.i;
            vec->data[l].data.i = ival1 % ival2;
        }
    } else {
        fprintf(stderr, "invalid argument type for modulus: %lx\n", t);
        free(vec->data);
        free(vec);
        vec = NULL;
    }

    return vec;
}

