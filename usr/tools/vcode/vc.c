#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <vcode/vc.h>

#define tohex(c) (isdigit(c) ? (c) - '0' : tolower(c) - 'a' + 10)
#define tobin(c) ((c) == '0' ? 0 : 1)
#define todec(c) (tolower(c) - '0')

struct vcvec *vcvecstk;

#if 0
static struct vcvec *
vcgetvec(char *str, char **retstr)
{
    char         *ptr = str;
    struct vcvec *vec = NULL;
    vcfloat       fval = 0.0;
    vcint         ival;
    long          ndx = 0;
    long          bool;
    vcfloat       mul;
    size_t        n = 4;
    size_t        len = 0;

    if (*str == '(') {
        vec = malloc(sizeof(struct vcvec));
        vec->data = malloc(n * sizeof(struct vcval));
        str++;
        do {
            while (isspace(*str)) {
                str++;
            }
            if (*str != ')') {
                ival = 0;
                ndx = 0;
                bool = 0;
                if (*str == '0') {
                    str++;
                    if (tolower(*str) == 'x') {
                        str++;
                        mul = 1.0 / 16;
                        while (isxdigit(*str) || *str == '.') {
                            if (*str == '.') {
                                fval = (vcfloat)ival;
                                ndx = 1;
                            } else if (ndx) {
                                fval += (vcfloat)tohex(*str) * mul;
                                mul *= 1 / 16;
                            } else {
                                ival <<= 4;
                                ival += tohex(*str);
                            }
                            str++;
                        }
                    } else if (tolower(*str) == 'b') {
                        str++;
                        mul = 1.0 / 2;
                        while (*str == '0' || *str == '1' || *str == '.') {
                            if (*str == '.') {
                                fval = (vcfloat)ival;
                                ndx = 1;
                            } else if (ndx) {
                                fval += (vcfloat)tobin(*str) * mul;
                                mul *= 1 / 2;
                            } else {
                                ival <<= 1;
                                ival += tobin(*str);
                            }
                            str++;
                        }
                    }
                } else if (*str == 'T') {
                    str++;
                    bool = 1;
                    ival = VC_TRUE;
                } else if (*str == 'F') {
                    str++;
                    bool = 1;
                    ival = VC_FALSE;
                } else {
                    mul = 1.0 / 10;
                    while (isdigit(*str) || *str == '.') {
                        if (*str == '.') {
                            fval = (vcfloat)ival;
                            ndx = 1;
                        } else if (ndx) {
                            fval += (vcfloat)todec(*str) * mul;
                            mul *= 1 / 10;
                        } else {
                            ival *= 10;
                            ival += todec(*str);
                        }
                        str++;
                    }                    
                }
                if (len == n) {
                    n <<= 1;
                    vec->data = realloc(vec->data, n * sizeof(struct vcval));
                }
                if (bool) {
                    vec->data[len].type = VC_BOOL;
                    vec->data[len].data.i = ival;
                } else if (ndx) {
                    vec->data[len].type = VC_FLOAT;
                    vec->data[len].data.f = fval;
                } else {
                    vec->data[len].type = VC_INT;
                    vec->data[len].data.i = ival;
                }
                len++;
            }
        } while (*str != ')');
        if (*str == ')') {
            str++;
        } else {
            fprintf(stderr, "unterminated vector: %s\n", ptr);

            exit(1);
        }
    }
    if (len) {
        vec->nval = len;
        *retstr = str;
    }

    return vec;
}
#endif

static struct vcvec *
vcgetivec(char *str, char **retstr)
{
    char         *ptr = str;
    struct vcvec *vec = NULL;
    vcint         ival;
    size_t        n = 4;
    size_t        len = 0;

    if (*str == '(') {
        vec = malloc(sizeof(struct vcvec));
        vec->type = VC_INT;
        vec->data = malloc(n * sizeof(struct vcval));
        str++;
        do {
            while (isspace(*str)) {
                str++;
            }
            if (*str != ')') {
                ival = 0;
                if (*str == '0') {
                    str++;
                    if (tolower(*str) == 'x') {
                        str++;
                        while (isxdigit(*str)) {
                            ival <<= 4;
                            ival += tohex(*str);
                            str++;
                        }
                    } else if (tolower(*str) == 'b') {
                        str++;
                        while (*str == '0' || *str == '1' || *str == '.') {
                            ival <<= 1;
                            ival += tobin(*str);
                            str++;
                        }
                    }
                } else {
                    while (isdigit(*str) || *str == '.') {
                        ival *= 10;
                        ival += todec(*str);
                        str++;
                    }                    
                }
                if (len == n) {
                    n <<= 1;
                    vec->data = realloc(vec->data, n * sizeof(struct vcval));
                }
                vec->data[len].data.i = ival;
                len++;
            }
        } while (*str != ')');
        if (*str == ')') {
            str++;
        } else {
            fprintf(stderr, "unterminated vector: %s\n", ptr);

            exit(1);
        }
    }
    if (len) {
        vec->nval = len;
        *retstr = str;
    }

    return vec;
}

static struct vcvec *
vcgetfvec(char *str, char **retstr)
{
    char         *ptr = str;
    struct vcvec *vec = NULL;
    vcfloat       fval = 0.0;
    long          ndx = 0;
    vcfloat       mul;
    size_t        n = 4;
    size_t        len = 0;

    if (*str == '(') {
        vec = malloc(sizeof(struct vcvec));
        vec->type = VC_FLOAT;
        vec->data = malloc(n * sizeof(struct vcval));
        str++;
        do {
            while (isspace(*str)) {
                str++;
            }
            if (*str != ')') {
                fval = 0.0;
                ndx = 0;
                if (*str == '0') {
                    str++;
                    if (tolower(*str) == 'x') {
                        str++;
                        mul = 1.0 / 16;
                        while (isxdigit(*str) || *str == '.') {
                            if (*str == '.') {
                                ndx = 1;
                            } else if (ndx) {
                                fval += (vcfloat)tohex(*str) * mul;
                                mul *= 1 / 16;
                            } else {
                                fval *= 16;
                                fval += (vcfloat)tohex(*str);
                            }
                            str++;
                        }
                    } else if (tolower(*str) == 'b') {
                        str++;
                        mul = 1.0 / 2;
                        while (*str == '0' || *str == '1' || *str == '.') {
                            if (*str == '.') {
                                ndx = 1;
                            } else if (ndx) {
                                fval += (vcfloat)tobin(*str) * mul;
                                mul *= 1 / 2;
                            } else {
                                fval *= 2;
                                fval += (vcfloat)tobin(*str);
                            }
                            str++;
                        }
                    }
                } else {
                    mul = 1.0 / 10;
                    while (isdigit(*str) || *str == '.') {
                        if (*str == '.') {
                            ndx = 1;
                        } else if (ndx) {
                            fval += (vcfloat)todec(*str) * mul;
                            mul *= 1 / 10;
                        } else {
                            fval *= 10;
                            fval += (vcfloat)todec(*str);
                        }
                        str++;
                    }                    
                }
                if (len == n) {
                    n <<= 1;
                    vec->data = realloc(vec->data, n * sizeof(struct vcval));
                }
                vec->data[len].data.f = fval;
                len++;
            }
        } while (*str != ')');
        if (*str == ')') {
            str++;
        } else {
            fprintf(stderr, "unterminated vector: %s\n", ptr);

            exit(1);
        }
    }
    if (len) {
        vec->nval = len;
        *retstr = str;
    }

    return vec;
}

static struct vcvec *
vcgetbvec(char *str, char **retstr)
{
    char         *ptr = str;
    struct vcvec *vec = NULL;
    vcint         ival;
    size_t        n = 4;
    size_t        len = 0;

    if (*str == '(') {
        vec = malloc(sizeof(struct vcvec));
        vec->type = VC_BOOL;
        vec->data = malloc(n * sizeof(struct vcval));
        str++;
        do {
            while (isspace(*str)) {
                str++;
            }
            ival = 0;
            if (*str != ')') {
                if (*str == 'T') {
                    ival = VC_TRUE;
                    str++;
                } else if (*str == 'F') {
                    ival = VC_FALSE;
                    str++;
                } else {
                    fprintf(stderr, "unrecognized boolean value %s\n", str);
                }
                if (len == n) {
                    n <<= 1;
                    vec->data = realloc(vec->data, n * sizeof(struct vcval));
                }
                vec->data[len].data.i = ival;
                len++;
            }
        } while (*str != ')');
        if (*str == ')') {
            str++;
        } else {
            fprintf(stderr, "unterminated vector: %s\n", ptr);

            exit(1);
        }
    }
    if (len) {
        vec->nval = len;
        *retstr = str;
    }

    return vec;
}

struct vcvec *
vcgetvec(char *str, char **retstr)
{
    struct vcvec *vec = NULL;

    while (isspace(*str)) {
        str++;
    }
    if (*str == 'i') {
        str++;
        vec = vcgetivec(str, retstr);
    } else if (*str == 'f') {
        str++;
        vec = vcgetfvec(str, retstr);
    } else if (*str == 'b') {
        str++;
        vec = vcgetbvec(str, retstr);
    } else if (*str) {
        vec = vcgetivec(str, retstr);
    }

    return vec;
}

struct vcsegdes *
vcgetsegdes(char *str, char **retstr)
{
    char            *ptr = str;
    struct vcsegdes *des = NULL;
    vcint            ival;
    size_t           n = 4;
    size_t           len = 0;

    if (*str == '[') {
        des = malloc(sizeof(struct vcsegdes));
        des->data = malloc(n * sizeof(vcint));
        str++;
        do {
            ival = 0;
            while (isspace(*str)) {
                str++;
            }
            if (*str != ']') {
                if (*str == '0') {
                    str++;
                    if (tolower(*str) == 'x') {
                        str++;
                        while (isxdigit(*str)) {
                            ival <<= 4;
                            ival += tohex(*str);
                            str++;
                        }
                    } else if (tolower(*str) == 'b') {
                        str++;
                        while (*str == '0' || *str == '1') {
                            ival <<= 1;
                            ival += tobin(*str);
                            str++;
                        }
                    }
                } else {
                    while (isdigit(*str)) {
                        ival *= 10;
                        ival += todec(*str);
                        str++;
                    }                    
                }
                if (len == n) {
                    n <<= 1;
                    des->data = realloc(des->data, n * sizeof(vcint));
                }
                des->data[len] = ival;
                len++;
            }
        } while (*str != ']');
        if (*str == ']') {
            str++;
        } else {
            fprintf(stderr, "unterminated vector: %s\n", ptr);

            exit(1);
        }
    }
    if (len) {
        des->nseg = len;
        *retstr = str;
    }

    return des;
}

#if (!WPM)

int
main(int argc, char *argv[])
{
    char            *ptr;
    char            *str1 = "f( 0 5 3.700 0xf 0b11 )";
    char            *str2 = "[ 2 4 ]";
    char            *str3 = "b( T F T )";
    char            *str4 = "f( 7 14 6 5 2 )";
    char            *str5 = "( 7 )";
    struct vcvec    *vec1 = vcgetvec(str1, &ptr);
    struct vcvec    *vec2 = vcgetvec(str4, &ptr);
    struct vcvec    *vec3;
    struct vcvec    *vec4 = vcgetvec(str5, &ptr);
    struct vcsegdes *des = vcgetsegdes(str2, &ptr);
    struct vcvec    *bool = vcgetvec(str3, &ptr);
    long             l;

    fprintf(stderr, "vector of %ld values:\n", vec1->nval);
    for (l = 0 ; l < vec1->nval ; l++) {
        if (vec1->type == VC_INT) {
            fprintf(stderr, "INT: %ld\n", vec1->data[l].data.i);
        } else {
            fprintf(stderr, "FLOAT: %e\n", vec1->data[l].data.f);
        }
    }

    fprintf(stderr, "descriptor of %ld segments:\n", des->nseg);
    for (l = 0 ; l < des->nseg ; l++) {
        fprintf(stderr, "SEG: %ld\n", des->data[l]);
    }

    fprintf(stderr, "boolean of %ld values:\n", bool->nval);
    for (l = 0 ; l < bool->nval ; l++) {
        fprintf(stderr, "BOOL: %lx\n", bool->data[l].data.i);
    }

    vec3 = vcaddv(vec2, vec1);
    fprintf(stderr, "sum of %ld values:\n", vec3->nval);
    for (l = 0 ; l < vec3->nval ; l++) {
        if (vec3->type == VC_INT) {
            fprintf(stderr, "INT: %ld\n", vec3->data[l].data.i);
        } else {
            fprintf(stderr, "FLOAT: %e\n", vec3->data[l].data.f);
        }
    }

#if 0
    vec3 = vcadds(vec4, vec1);
    fprintf(stderr, "scalar sum of %ld values:\n", vec3->nval);
    for (l = 0 ; l < vec3->nval ; l++) {
        if (vec3->type == VC_INT) {
            fprintf(stderr, "INT: %ld\n", vec3->data[l].data.i);
        } else {
            fprintf(stderr, "FLOAT: %e\n", vec3->data[l].data.f);
        }
    }
#endif

    exit(0);
}

#endif

