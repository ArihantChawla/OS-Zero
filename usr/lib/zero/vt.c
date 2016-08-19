#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <stdio.h>

#define VTKEYSIGN 0

typedef uint64_t vtval_t;

#define VTINVAL    UINT64_C(0xffffffffffffffff)
#define VTINVBYTE  0xff

#if (VTKEYSIGN)
#define VTNKEYBIT  63
#define VTNL0TAB   (1U << (VTNLVLBIT - 1))
#else
#define VTNKEYBIT  64
#define VTNL0TAB   (1U << VTNLVLBIT)
#endif
#define VTNTAB     (1UL << VTNLVLBIT)
#define VTNLVLBIT  8
#define VTNTREELVL 8
#define VTNPTRLVL  7
#define VTKEYMASK  (VTNTAB - 1)
#define VTKEYMASK0 (VTNL0TAB - 1)

void *vtl0tab[VTNL0TAB];
long  vtl0reftab[VTNL0TAB];
long  vtl1reftab[VTNTAB];
long  vtl2reftab[VTNTAB];
long  vtl3reftab[VTNTAB];
long  vtl4reftab[VTNTAB];
long  vtl5reftab[VTNTAB];
long  vtl6reftab[VTNTAB];
long  vtl7reftab[VTNTAB];
long *vtreftab[8] = { vtl0reftab, vtl1reftab, vtl2reftab, vtl3reftab,
                      vtl4reftab, vtl5reftab, vtl6reftab, vtl7reftab };

#define _vtgetptrtab() calloc(VTNTAB, sizeof(void *))
#define _vtgetvaltab()                                                  \
    memset(malloc(VTNTAB * sizeof(vtval_t)),                            \
           VTINVBYTE,                                                   \
           VTNTAB * sizeof(vtval_t))

long
_vttrim(void **pstk,
        long *nstk,
        long lvl)
{
    long  nref = -1;
    void *ptr;
    void *ref;
    long  ndx;
    long  nuke;

    nuke ^= nuke;
    while (lvl--) {
        ptr = pstk[lvl];
        ref = vtreftab[lvl];
        ndx = nstk[lvl];
        if ((nuke) && lvl < VTNPTRLVL) {
            ((void **)ptr)[nuke] = NULL;
            nuke ^= nuke;
        }
        nref = ((long *)ref)[ndx];
        if (!--nref) {
            free(ptr);
            nuke = ndx;
        }
        ((long *)ref)[ndx] = nref;
    }

    return nref;
}

long
vtadd(void **tab,
      vtval_t val)
{
    long  retval = -1;
    void *ptr;
    long *ref;
    long  ofs = VTNPTRLVL * VTNLVLBIT;
    long  lvl;
    long  ndx;
    void *ptrstk[VTNPTRLVL];
    long  ndxstk[VTNPTRLVL];

    lvl ^= lvl;
    ndx = val >> ofs;
    while (ofs) {
        ptr = tab[ndx];
        ofs -= VTNLVLBIT;
        if (!ptr) {
            if (ofs) {
                ptr = tab[ndx] = _vtgetptrtab();
            } else {
                ptr = tab[ndx] = _vtgetvaltab();
            }
        }
        ref = vtreftab[lvl];
        tab = ptrstk[lvl] = ptr;
        ndxstk[lvl] = ndx;
        lvl++;
        if (!ptr) {

            break;
        }
        ref[ndx]++;
        ndx = (val >> ofs) & VTKEYMASK;
    }
    if (!ptr) {
        if (!_vttrim(ptrstk, ndxstk, lvl)) {
            tab[ndxstk[0]] = NULL;
        }
    } else {
        ((vtval_t *)ptr)[ndx] = val;
        retval ^= retval;
    }

    return retval;
}

long
vtdel(void **tab,
      vtval_t val)
{
    long  retval = -1;
    void *ptr;
    void *ptrstk[VTNPTRLVL];
    long  ndxstk[VTNPTRLVL];
    long  ofs = VTNPTRLVL * VTNLVLBIT;
    long  lvl;
    long  ndx;

    lvl ^= lvl;
    ndx = val >> ofs;
    while (ofs) {
        ptr = tab[ndx];
        if (!ptr) {

            break;
        }
        tab = ptrstk[lvl] = ptr;
        ndxstk[lvl] = ndx;
        ofs -= VTNLVLBIT;
        lvl++;
        ndx = (val >> ofs) & VTKEYMASK;
    }
    if (ptr) {
        ((vtval_t *)ptr)[ndx] = VTINVAL;
        if (!_vttrim(ptrstk, ndxstk, lvl)) {
            tab[ndxstk[0]] = NULL;
        }
        retval ^= retval;
    }

    return retval;
}

vtval_t
vtfind(void **tab,
       vtval_t val)
{
    vtval_t  retval = VTINVAL;
    void    *ptr = NULL;
    long     ofs = VTNPTRLVL * VTNLVLBIT;
    long     ndx;

    ndx = val >> ofs;
    while (ofs) {
        ptr = tab[ndx];
        if (!ptr) {

            break;
        }
        ofs -= VTNLVLBIT;
        tab = ptr;
        ndx = (val >> ofs) & VTKEYMASK;
    }
    if (ptr) {
        retval = ((vtval_t *)ptr)[ndx];
    }

    return retval;
}

vtval_t
vtfindprev(void **tab,
           vtval_t val)
{
    vtval_t   retval = VTINVAL;
    void    **tptr = tab;
    void     *ptr = NULL;
    long      ofs = VTNPTRLVL * VTNLVLBIT;
    long      ndx;
    long      lvl;
    long      lim;
    long      ndxstk[VTNTREELVL];
    
    val--;
    ndxstk[0] = (val >> ofs) & VTKEYMASK0;
    for (lvl = 1 ; lvl < VTNTREELVL ; lvl++) {
        ofs -= VTNLVLBIT;
        ndxstk[lvl] = (val >> ofs) & VTKEYMASK;
    }
    lvl ^= lvl;
    lim ^= lim;
    while (lvl >= lim) {
#if 0
        fprintf(stderr, "LVL: %lx (%lx:%lx:%lx:%lx)\n",
                lvl,
                ndxstk[0], ndxstk[1], ndxstk[2], ndxstk[3]);
#endif
        ndx = ndxstk[lvl];
        if (ndx >= lim) {
            if (lvl == VTNTREELVL - 1) {
                if (ptr) {
                    do {
                        retval = ((vtval_t *)ptr)[ndx];
                        ndx--;
                    } while (ndx >= lim && retval == VTINVAL);
                    if (retval == VTINVAL) {
                        lvl--;
                        ndxstk[lvl]--;
                    } else {
                        
                        return retval;
                    }
                } else {
                    lvl ^= lvl;
                    ndxstk[lvl]--;
                }
            } else {
                if (!lvl) {
                    tptr = tab;
                }
                do {
                    ptr = tptr[ndx];
                    ndx--;
                } while (ndx >= lim && !ptr);
                if (!ptr) {
                    if (lvl) {
                        lvl--;
                        ndxstk[lvl]--;
                    } else {

                        return retval;
                    }
                } else {
                    tptr = ptr;
                    lvl++;
                }
            }
        } else {
            lvl--;
        }
    }
#if 0
    if (ptr) {
        retval = ((vtval_t *)ptr)[ndx];
    }
#endif

    return retval;
}

#if 0
vtval_t
vtfindnext(void **tab,
           vtval_t val)
{
    vtval_t   retval = VTINVAL;
    void    **tptr = tab;
    void     *ptr = NULL;
    long      ofs = VTNPTRLVL * VTNLVLBIT;
    void    **pptr;
    long      ndx;
    long      lvl;
    long      lim;
    long      num;
    long      tmp;
    long      ndxstk[VTNTREELVL];
    
    val++;
    ndxstk[0] = (val >> ofs) & VTKEYMASK0;
    for (lvl = 1 ; lvl < VTNTREELVL ; lvl++) {
        ofs -= VTNLVLBIT;
        ndxstk[lvl] = (val >> ofs) & VTKEYMASK;
    }
    lvl ^= lvl;
    lim ^= lim;
    while (lvl >= 0) {
        if (!lvl) {
            lim = VTNL0TAB;
        } else {
            lim = VTNTAB;
        }
        tmp = ndxstk[0];
        pptr = tab[tmp];
        if (pptr) {
            num ^= num;
            while (num < lvl) {
                tmp = ndxstk[num];
                ptr = pptr[tmp];
                num++;
                if (ptr) {
                    pptr = ptr;
                } else {
                        ndxstk[num]++;
                        
                        continue;
                }
            }
#if 0
            if (num == VTNTREELVL - 1) {
                do {
                    retval = ((vtval_t *)ptr)[ndx];
                    ndx++;
                } while (ndx < lim && retval == VTINVAL);
                if (retval == VTINVAL) {
                    lvl--;
                    ndxstk[lvl]++;
                } else {
                    
                    return retval;
                }
            }
#endif
        } else {
            lvl ^= lvl;
            ndxstk[0]++;
        }
#if 0
        fprintf(stderr, "LVL: %lx (%lx:%lx:%lx:%lx)\n",
                lvl,
                ndxstk[0], ndxstk[1], ndxstk[2], ndxstk[3]);
#endif
        ndx = ndxstk[lvl];
        if (ndx < lim) {
            if (lvl == VTNTREELVL - 1) {
                if (ptr) {
                    do {
                        retval = ((vtval_t *)ptr)[ndx];
                        ndx++;
                    } while (ndx < lim && retval == VTINVAL);
                    if (retval == VTINVAL) {
                        lvl--;
                        ndxstk[lvl]++;
                    } else {
                        
                        return retval;
                    }
                } else {
                    lvl ^= lvl;
                    ndxstk[lvl]++;
                }
            } else {
                if (!lvl) {
                    tptr = tab;
                }
                do {
                    ptr = tptr[ndx];
                    ndx++;
//                    fprintf(stderr, "LVL %ld: %p\n", lvl, ptr);
                } while (ndx < lim && !ptr);
                if (!ptr) {
                    if (lvl) {
                        lvl--;
                        ndxstk[lvl]++;
                    } else {
                        
                        return retval;
                    }
                } else {
                    tptr = ptr;
                    lvl++;
                }
            }
        } else {
            lvl--;
        }
    }
#if 0
    if (ptr) {
        retval = ((vtval_t *)ptr)[ndx];
    }
#endif

    return retval;
}
#endif

vtval_t
vtfindnext(void **tab,
           vtval_t val)
{
    vtval_t   retval = VTINVAL;
    void    **tptr = tab;
    void     *ptr = NULL;
    vtval_t  *vptr = NULL;
    long      ofs = VTNPTRLVL * VTNLVLBIT;
    void    **pptr;
    long      ndx;
    long      lvl;
    long      lim;
    long      num;
    long      tmp;
    long      ndxstk[VTNTREELVL];
    
    val++;
    ndxstk[0] = (val >> ofs) & VTKEYMASK0;
    for (lvl = 1 ; lvl < VTNTREELVL ; lvl++) {
        ofs -= VTNLVLBIT;
        ndxstk[lvl] = (val >> ofs) & VTKEYMASK;
    }
    lvl ^= lvl;
    lim ^= lim;
    while (lvl >= lim) {
        if (!lvl) {
            lim = VTNL0TAB;
        } else {
            lim = VTNTAB;
        }
        tmp = ndxstk[0];
        pptr = tab[tmp];
        if (pptr) {
            num ^= num;
            while (num < lvl) {
                tmp = ndxstk[num];
                ptr = pptr[tmp];
                num++;
                if (ptr) {
                    pptr = ptr;
                } else {
                        ndxstk[num]++;
                        
                        continue;
                }
            }
        } else {
            lvl--;
            ndxstk[lvl]++;
        }
        fprintf(stderr, "LVL: %lx (%lx:%lx:%lx:%lx)\n",
                lvl,
                ndxstk[0], ndxstk[1], ndxstk[2], ndxstk[3]);
        lvl ^= lvl;
        ndx = ndxstk[lvl];
        if (ndx < lim) {
            if (!lvl) {
                tptr = tab;
            }
            if (lvl == VTNTREELVL - 1) {
                if (tptr) {
                    ptr = tptr;
                    vptr = ptr;
                    do {
                        fprintf(stderr, "#1\n");

                        retval = vptr[ndx];
                        ndx++;
                    } while (ndx < lim && retval == VTINVAL);
                    if (retval == VTINVAL) {
                        fprintf(stderr, "#2\n");

                        lvl--;
                        ndxstk[lvl]++;
                    } else {
                        fprintf(stderr, "LEAF: %lx (%lx:%lx:%lx:%lx)\n",
                                lvl,
                                ndxstk[0], ndxstk[1], ndxstk[2], ndxstk[3]);
                        
                        return retval;
                    }
                } else {
                    fprintf(stderr, "#3\n");
                    
                    lvl--;
                    ndxstk[lvl]++;
                }
            } else {
                do {
                    fprintf(stderr, "#4\n");

                    ptr = tptr[ndx];
                    ndx++;
//                    fprintf(stderr, "LVL %ld: %p\n", lvl, ptr);
                } while (ndx < lim && !ptr);
                tptr = ptr;
                if (!ptr) {
                    fprintf(stderr, "#5\n");

                    lvl--;
                    ndxstk[lvl]++;
                } else {
                    fprintf(stderr, "#6\n");

                    lvl++;
                }
            }
        } else {
            lvl--;
        }
    }
#if 0
    if (ptr) {
        retval = ((vtval_t *)ptr)[ndx];
    }
#endif
    fprintf(stderr, "RETURN: %lx (%lx:%lx:%lx:%lx)\n",
            lvl,
            ndxstk[0], ndxstk[1], ndxstk[2], ndxstk[3]);

    return retval;
}

#if 0
int
main(int argc,
     char *argv[])
{
    vtval_t val = UINT64_C(0x0102030405060000);
    vtval_t tmp;

    vtadd(vtl0tab, 0);
    vtadd(vtl0tab, val - 2 * VTNTAB);
    vtadd(vtl0tab, val - VTNTAB);
    vtadd(vtl0tab, val);
    vtadd(vtl0tab, UINT64_C(0x7ffffffffffefffe));
    tmp = vtfind(vtl0tab, val - 2 * VTNTAB);
    fprintf(stderr, "FIND: %llx\n", tmp);
    tmp = vtfind(vtl0tab, val);
    fprintf(stderr, "FIND: %llx\n", tmp);

    tmp = vtfindprev(vtl0tab, val + 1);
    fprintf(stderr, "FINDPREV: %llx\n", tmp);
    tmp = vtfindprev(vtl0tab, val);
    fprintf(stderr, "FINDPREV: %llx\n", tmp);
    tmp = vtfindprev(vtl0tab, val - VTNTAB);
    fprintf(stderr, "FINDPREV: %llx\n", tmp);
    tmp = vtfindprev(vtl0tab, val + 100 * VTNTAB + 1);
    fprintf(stderr, "FINDPREV: %llx\n", tmp);
    vtdel(vtl0tab, val);
    tmp = vtfindprev(vtl0tab, val + 10000 * VTNTAB + 1);
    fprintf(stderr, "FINDPREV: %llx\n", tmp);
    fprintf(stderr, "FINDPREV: %llx\n", vtfindprev(vtl0tab, UINT64_C(0x7fffffffffffffff)));

    /* TODO: findnext() doesn't work yet */
    fprintf(stderr, "\nWARNING: vtfindnext() doesn't work correctly\n\n");

    fprintf(stderr, "FINDNEXT: %llx\n", vtfindnext(vtl0tab, val - 2 * VTNTAB));
    fprintf(stderr, "FINDNEXT: %llx\n", vtfindnext(vtl0tab, val - 4 * VTNTAB));
    fprintf(stderr, "FINDNEXT: %llx\n", vtfindnext(vtl0tab, val - VTNTAB));
    fprintf(stderr, "FINDNEXT: %llx\n", vtfindnext(vtl0tab, val - 100 * VTNTAB + 1));
    fprintf(stderr, "FINDNEXT: %llx\n", vtfindnext(vtl0tab, val - 100 * VTNTAB + 1));

    exit(0);
}
#endif
