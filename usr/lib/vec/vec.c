#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <vec/vec.h>

#define tohex(c) (isdigit(c) ? (c) - '0' : tolower(c) - 'a' + 10)
#define tobin(c) ((c) == '0' ? 0 : 1)
#define todec(c) (tolower(c) - '0')

struct vec *vecstk;

#if 0
static struct vec *
vecgetvec(char *str, char **retstr)
{
    char       *ptr = str;
    struct vec *vec = NULL;
    vecfloat    fval = 0.0;
    vecint      ival;
    long        ndx = 0;
    long        bool;
    vecfloat    mul;
    size_t      n = 4;
    size_t      len = 0;

    if (*str == '(') {
        vec = malloc(sizeof(struct vec));
        vec->data = malloc(n * sizeof(struct vecval));
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
                                fval = (vecfloat)ival;
                                ndx = 1;
                            } else if (ndx) {
                                fval += (vecfloat)tohex(*str) * mul;
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
                                fval = (vecfloat)ival;
                                ndx = 1;
                            } else if (ndx) {
                                fval += (vecfloat)tobin(*str) * mul;
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
                    ival = VEC_TRUE;
                } else if (*str == 'F') {
                    str++;
                    bool = 1;
                    ival = VEC_FALSE;
                } else {
                    mul = 1.0 / 10;
                    while (isdigit(*str) || *str == '.') {
                        if (*str == '.') {
                            fval = (vecfloat)ival;
                            ndx = 1;
                        } else if (ndx) {
                            fval += (vecfloat)todec(*str) * mul;
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
                    vec->data = realloc(vec->data, n * sizeof(struct vecval));
                }
                if (bool) {
                    vec->data[len].type = VEC_BOOL;
                    vec->data[len].data.i = ival;
                } else if (ndx) {
                    vec->data[len].type = VEC_FLOAT;
                    vec->data[len].data.f = fval;
                } else {
                    vec->data[len].type = VEC_INT;
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

static struct vec *
vecgetivec(char *str, char **retstr)
{
    char       *ptr = str;
    struct vec *vec = NULL;
    vecint      ival;
    size_t      n = 4;
    size_t      len = 0;

    if (*str == '(') {
        vec = malloc(sizeof(struct vec));
        vec->type = VEC_INT;
        vec->data = malloc(n * sizeof(struct vecval));
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
                    vec->data = realloc(vec->data, n * sizeof(struct vecval));
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

static struct vec *
vecgetfvec(char *str, char **retstr)
{
    char       *ptr = str;
    struct vec *vec = NULL;
    vecfloat    fval = 0.0;
    long        ndx = 0;
    vecfloat    mul;
    size_t      n = 4;
    size_t      len = 0;

    if (*str == '(') {
        vec = malloc(sizeof(struct vec));
        vec->type = VEC_FLOAT;
        vec->data = malloc(n * sizeof(struct vecval));
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
                                fval += (vecfloat)tohex(*str) * mul;
                                mul *= 1 / 16;
                            } else {
                                fval *= 16;
                                fval += (vecfloat)tohex(*str);
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
                                fval += (vecfloat)tobin(*str) * mul;
                                mul *= 1 / 2;
                            } else {
                                fval *= 2;
                                fval += (vecfloat)tobin(*str);
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
                            fval += (vecfloat)todec(*str) * mul;
                            mul *= 1 / 10;
                        } else {
                            fval *= 10;
                            fval += (vecfloat)todec(*str);
                        }
                        str++;
                    }                    
                }
                if (len == n) {
                    n <<= 1;
                    vec->data = realloc(vec->data, n * sizeof(struct vecval));
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

static struct vec *
vecgetbvec(char *str, char **retstr)
{
    char       *ptr = str;
    struct vec *vec = NULL;
    vecint      ival;
    size_t      n = 4;
    size_t      len = 0;

    if (*str == '(') {
        vec = malloc(sizeof(struct vec));
        vec->type = VEC_BOOL;
        vec->data = malloc(n * sizeof(struct vecval));
        str++;
        do {
            while (isspace(*str)) {
                str++;
            }
            ival = 0;
            if (*str != ')') {
                if (*str == 'T') {
                    ival = VEC_TRUE;
                    str++;
                } else if (*str == 'F') {
                    ival = VEC_FALSE;
                    str++;
                } else {
                    fprintf(stderr, "unrecognized boolean value %s\n", str);
                }
                if (len == n) {
                    n <<= 1;
                    vec->data = realloc(vec->data, n * sizeof(struct vecval));
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

struct vec *
vecgetvec(char *str, char **retstr)
{
    struct vec *vec = NULL;

    while (isspace(*str)) {
        str++;
    }
    if (*str == 'i') {
        str++;
        vec = vecgetivec(str, retstr);
    } else if (*str == 'f') {
        str++;
        vec = vecgetfvec(str, retstr);
    } else if (*str == 'b') {
        str++;
        vec = vecgetbvec(str, retstr);
    } else if (*str) {
        vec = vecgetivec(str, retstr);
    }

    return vec;
}

struct vecsegdes *
vecgetsegdes(char *str, char **retstr)
{
    char             *ptr = str;
    struct vecsegdes *des = NULL;
    vecint           ival;
    size_t           n = 4;
    size_t           len = 0;

    if (*str == '[') {
        des = malloc(sizeof(struct vecsegdes));
        des->data = malloc(n * sizeof(vecint));
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
                    des->data = realloc(des->data, n * sizeof(vecint));
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
    struct vec      *vec1 = vecgetvec(str1, &ptr);
    struct vec      *vec2 = vecgetvec(str4, &ptr);
    struct vec      *vec3;
    struct vec      *vec4 = vecgetvec(str5, &ptr);
    struct vecsegdes *des = vecgetsegdes(str2, &ptr);
    struct vec      *bool = vecgetvec(str3, &ptr);
    long             l;

    fprintf(stderr, "vector of %ld values:\n", vec1->nval);
    for (l = 0 ; l < vec1->nval ; l++) {
        if (vec1->type == VEC_INT) {
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

    vec3 = vecaddv(vec2, vec1);
    fprintf(stderr, "sum of %ld values:\n", vec3->nval);
    for (l = 0 ; l < vec3->nval ; l++) {
        if (vec3->type == VEC_INT) {
            fprintf(stderr, "INT: %ld\n", vec3->data[l].data.i);
        } else {
            fprintf(stderr, "FLOAT: %e\n", vec3->data[l].data.f);
        }
    }

#if 0
    vec3 = vecadds(vec4, vec1);
    fprintf(stderr, "scalar sum of %ld values:\n", vec3->nval);
    for (l = 0 ; l < vec3->nval ; l++) {
        if (vec3->type == VEC_INT) {
            fprintf(stderr, "INT: %ld\n", vec3->data[l].data.i);
        } else {
            fprintf(stderr, "FLOAT: %e\n", vec3->data[l].data.f);
        }
    }
#endif

    exit(0);
}

#endif

