#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <zero/mlt.h>

void *mltl0tab[MLTNL0TAB];
long  mltl0reftab[MLTNL0TAB];
long  mltl1reftab[MLTNTAB];
long  mltl2reftab[MLTNTAB];
long  mltl3reftab[MLTNTAB];
long  mltl4reftab[MLTNTAB];
long  mltl5reftab[MLTNTAB];
long  mltl6reftab[MLTNTAB];
long  mltl7reftab[MLTNTAB];
long *mltreftab[8] = { mltl0reftab, mltl1reftab, mltl2reftab, mltl3reftab,
                      mltl4reftab, mltl5reftab, mltl6reftab, mltl7reftab };

#define _mltgetptrtab() calloc(MLTNTAB, sizeof(void *))
#define _mltgetvaltab()                                                  \
    memset(malloc(MLTNTAB * sizeof(MLTVAL_T)),                            \
           MLTINVBYTE,                                                   \
           MLTNTAB * sizeof(MLTVAL_T))

long
_mlttrim(void **pstk,
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
        ref = mltreftab[lvl];
        ndx = nstk[lvl];
        if ((nuke) && lvl < MLTNPTRLVL) {
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
mltadd(void **tab,
       MLTVAL_T val)
{
    long  retval = -1;
    void *ptr;
    long *ref;
    long  ofs = MLTNPTRLVL * MLTNLVLBIT;
    long  lvl;
    long  ndx;
    void *ptrstk[MLTNPTRLVL];
    long  ndxstk[MLTNPTRLVL];

    lvl ^= lvl;
    ndx = val >> ofs;
    while (ofs) {
        ptr = tab[ndx];
        ofs -= MLTNLVLBIT;
        if (!ptr) {
            if (ofs) {
                ptr = tab[ndx] = _mltgetptrtab();
            } else {
                ptr = tab[ndx] = _mltgetvaltab();
            }
        }
        ref = mltreftab[lvl];
        tab = ptrstk[lvl] = ptr;
        ndxstk[lvl] = ndx;
        lvl++;
        if (!ptr) {

            break;
        }
        ref[ndx]++;
        ndx = (val >> ofs) & MLTKEYMASK;
    }
    if (!ptr) {
        if (!_mlttrim(ptrstk, ndxstk, lvl)) {
            tab[ndxstk[0]] = NULL;
        }
    } else {
        ((MLTVAL_T *)ptr)[ndx] = val;
        retval ^= retval;
    }

    return retval;
}

long
mltdel(void **tab,
       MLTVAL_T val)
{
    long  retval = -1;
    void *ptr;
    void *ptrstk[MLTNPTRLVL];
    long  ndxstk[MLTNPTRLVL];
    long  ofs = MLTNPTRLVL * MLTNLVLBIT;
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
        ofs -= MLTNLVLBIT;
        lvl++;
        ndx = (val >> ofs) & MLTKEYMASK;
    }
    if (ptr) {
        ((MLTVAL_T *)ptr)[ndx] = MLTINVAL;
        if (!_mlttrim(ptrstk, ndxstk, lvl)) {
            tab[ndxstk[0]] = NULL;
        }
        retval ^= retval;
    }

    return retval;
}

MLTVAL_T
mltfind(void **tab,
       MLTVAL_T val)
{
    MLTVAL_T  retval = MLTINVAL;
    void     *ptr = NULL;
    long      ofs = MLTNPTRLVL * MLTNLVLBIT;
    long      ndx;

    ndx = val >> ofs;
    while (ofs) {
        ptr = tab[ndx];
        if (!ptr) {

            break;
        }
        ofs -= MLTNLVLBIT;
        tab = ptr;
        ndx = (val >> ofs) & MLTKEYMASK;
    }
    if (ptr) {
        retval = ((MLTVAL_T *)ptr)[ndx];
    }

    return retval;
}

MLTVAL_T
mltfindprev(void **tab,
            MLTVAL_T val)
{
    MLTVAL_T   retval = MLTINVAL;
    void     **tptr = tab;
    void      *ptr = NULL;
    long       ofs = MLTNPTRLVL * MLTNLVLBIT;
    long       ndx;
    long       lvl;
    long       lim;
    long       ndxstk[MLTNTREELVL];

    val--;
    ndxstk[0] = (val >> ofs) & MLTKEYMASK0;
    for (lvl = 1 ; lvl < MLTNTREELVL ; lvl++) {
        ofs -= MLTNLVLBIT;
        ndxstk[lvl] = (val >> ofs) & MLTKEYMASK;
    }
    lvl ^= lvl;
    lim ^= lim;
    while (lvl >= lim) {
        ndx = ndxstk[lvl];
        if (ndx >= lim) {
            if (lvl == MLTNTREELVL - 1) {
                if (ptr) {
                    do {
                        retval = ((MLTVAL_T *)ptr)[ndx];
                        ndx--;
                    } while (ndx >= lim && retval == MLTINVAL);
                    if (retval == MLTINVAL) {
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

    return retval;
}

MLTVAL_T
mltfindnext(void **tab,
            MLTVAL_T val)
{
    MLTVAL_T   retval = MLTINVAL;
    void     **tptr = tab;
    void      *ptr = NULL;
    long       ofs = MLTNPTRLVL * MLTNLVLBIT;
    long       ndx;
    long       lvl;
    long       lim;
    long       ndxstk[MLTNTREELVL];

    val++;
    ndxstk[0] = (val >> ofs) & MLTKEYMASK0;
    for (lvl = 1 ; lvl < MLTNTREELVL ; lvl++) {
        ofs -= MLTNLVLBIT;
        ndxstk[lvl] = (val >> ofs) & MLTKEYMASK;
    }
    lvl = 0;
    while (lvl >= 0) {
        if (lvl) {
            lim = MLTNTAB;
        } else {
            tptr = tab;
            lim = MLTNL0TAB;
        }
#if (MLTNTREELVL == 8)
        fprintf(stderr, "LVL: %lx (%02lx%02lx%02lx%02lx%02lx%02lx%02lx%02lx)\n",
                lvl,
                ndxstk[0], ndxstk[1], ndxstk[2], ndxstk[3],
                ndxstk[4], ndxstk[5], ndxstk[6], ndxstk[7]);
#elif (MLTNTREELVL == 4)
        fprintf(stderr, "LVL: %lx (%lx:%lx:%lx:%lx)\n",
                lvl,
                ndxstk[0], ndxstk[1], ndxstk[2], ndxstk[3]);
#endif
        ndx = ndxstk[lvl];
        if (ndx < lim) {
            do {
                ptr = tptr[ndx];
                ndx++;
            } while (ndx < lim && !ptr);
            if (ndx == lim) {
                if (!lvl) {

                    return MLTINVAL;
                } else {
                    while ((lvl) && ndxstk[lvl] == MLTNTAB) {
                        ndxstk[lvl] = 0;
                        lvl--;
                    }
                    ndxstk[lvl]++;
                }
            }
            if ((ptr) && lvl == MLTNTREELVL - 1) {
                retval = *(MLTVAL_T *)tptr;

                return retval;
            } else if (ptr) {
                tptr = ptr;
                lvl++;
            } else if (lvl < MLTNTREELVL - 1) {
                while ((lvl) && ndxstk[lvl] == MLTNTAB) {
                    ndxstk[lvl] = 0;
                    lvl--;
                }
                ndxstk[lvl]++;
            } else {
                ndxstk[lvl] = 0;
                lvl--;
            }
        } else if (!lvl) {

            return MLTINVAL;
        } else {
#if 0
            for (ndx = lvl + 1 ; ndx < MLTNTREELVL ; ndx++) {
                ndxstk[ndx] = 0;
            }
#endif
            ndxstk[lvl]++;
            lvl++;
        }
    }
#if 0
    if (ptr) {
        retval = ((MLTVAL_T *)ptr)[ndx];
    }
#endif

    return retval;
}

#if defined(MLTTEST)
int
main(int argc,
     char *argv[])
{
    MLTVAL_T val = UINT64_C(0x0102030405060000);
    MLTVAL_T tmp;

    mltadd(mltl0tab, 0);
    mltadd(mltl0tab, val - 2 * MLTNTAB);
    mltadd(mltl0tab, val - MLTNTAB);
    mltadd(mltl0tab, val);
    mltadd(mltl0tab, UINT64_C(0x7ffffffffffefffe));
    tmp = mltfind(mltl0tab, val - 2 * MLTNTAB);
    fprintf(stderr, "FIND: %llx\n", tmp);
    tmp = mltfind(mltl0tab, val);
    fprintf(stderr, "FIND: %llx\n", tmp);

    tmp = mltfindprev(mltl0tab, val + 1);
    fprintf(stderr, "FINDPREV: %llx\n", tmp);
    tmp = mltfindprev(mltl0tab, val);
    fprintf(stderr, "FINDPREV: %llx\n", tmp);
    tmp = mltfindprev(mltl0tab, val - MLTNTAB);
    fprintf(stderr, "FINDPREV: %llx\n", tmp);
    tmp = mltfindprev(mltl0tab, val + 100 * MLTNTAB + 1);
    fprintf(stderr, "FINDPREV: %llx\n", tmp);
    mltdel(mltl0tab, val);
    tmp = mltfindprev(mltl0tab, val + 10000 * MLTNTAB + 1);
    fprintf(stderr, "FINDPREV: %llx\n", tmp);
    fprintf(stderr, "FINDPREV: %llx\n", mltfindprev(mltl0tab, UINT64_C(0x7fffffffffffffff)));

    fprintf(stderr, "\nWARNING: mltfindnext() doesn't work correctly\n\n");

    fprintf(stderr, "FINDNEXT: %llx\n", mltfindnext(mltl0tab, 0));
    fprintf(stderr, "FINDNEXT: %llx\n", mltfindnext(mltl0tab, val - 2 * MLTNTAB));
    fprintf(stderr, "FINDNEXT: %llx\n", mltfindnext(mltl0tab, val - 4 * MLTNTAB));
    fprintf(stderr, "FINDNEXT: %llx\n", mltfindnext(mltl0tab, val - MLTNTAB));
    fprintf(stderr, "FINDNEXT: %llx\n", mltfindnext(mltl0tab, val - 100 * MLTNTAB + 1));
    fprintf(stderr, "FINDNEXT: %llx\n", mltfindnext(mltl0tab, UINT64_C(0x7ffffffffffefffe)));
    fprintf(stderr, "FINDNEXT: %llx\n", mltfindnext(mltl0tab, UINT64_C(0x7ffffffffffefffd)));

    exit(0);
}
#endif

