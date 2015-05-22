#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <zero/randmt32.h>
#include <zero/trix.h>
#include <dungeon/cell.h>

static long dngcountnbors1(struct celldng *dng, long caveid,
                           long x, long y, long lim);
static long dngcountnbors0(struct celldng *dng, long caveid,
                           long x, long y, long lim);
static void dnggencave(struct celldng *dng, long caveid);
static void dngfindcave(struct celldng *dng, long caveid, long x, long y);
static void dngbuildcave(struct celldng *dng, long caveid);
static void dngrmcave(struct celldng *dng, long caveid);
static long dngconncaves(struct celldng *dng);
static long dngturncor(long dir);
static long dngturncor2(long dir);
static void dngfindedge(struct celldng *dng, long caveid,
                        long *retx, long *rety, long *retdir);
static void dngfindcoredge(struct celldng *dng, long caveid,
                           long *retx, long *rety, long *retdir);
static long dngchkcorpnt(struct celldng *dng, long caveid,
                         long x, long y, long dir);
static struct cellcor * dngtrycor(struct celldng *dng, long caveid,
                                  long x, long y, long dir,
                                  long backtrack);

/* macros */
#define dnggetcellbit(dng, x, y)                                        \
    (bitset((dng)->map, (y) * (dng)->width + (x)))
#define dngsetcellbit(dng, x, y)                                        \
    (setbit((dng)->map, y * (dng)->height + x))
#define dngrevdir(dir) (dngrevdirtab[(dir)])
#define dngmovedir(x, y, dir)                                           \
    ((x) += dngdirofstab[(dir)].xval, (y) += dngdirofstab[(dir)].yval)
#define dngsetcellid(dng, x, y, cid)                                    \
    ((dng)->caveidtab[(dng)->height * (y) + (x)] = (cid))
#define dnggetcellid(dng, x, y)                                         \
    ((dng)->caveidtab[(dng)->height * (y) + (x)])

static long dngdirtab[DNG_NDIR]
= {
    DNG_NORTH,
    DNG_SOUTH,
    DNG_EAST,
    DNG_WEST,
    DNG_NORTHEAST,
    DNG_NORTHWEST,
    DNG_SOUTHWEST,
    DNG_SOUTHEAST,
};
static long dngrevdirtab[DNG_NDIR]
= {
    DNG_SOUTH,
    DNG_NORTH,
    DNG_WEST,
    DNG_EAST,
    DNG_SOUTHWEST,
    DNG_SOUTHEAST,
    DNG_NORTHEAST,
    DNG_NORTHWEST
};
static struct cellcoord dngdirofstab[DNG_NDIR]
= {
    { 0, -1 },  // DNG_NORTH
    { 0, 1 },   // DNG_SOUTH
    { 1, 0 },   // DNG_EAST
    { -1, 0 },  // DNG_WEST
    { 1, -1 },  // DNG_NORTHEAST
    { -1, -1 }, // DNG_NORTHWEST
    { -1, -1 }, // DNG_SOUTHWEST
    { 1, 1 }    // DNG_SOUTHEAST
};

/* initialise dungeon generator */
void
cellinitdng(struct celldng *dng, long ncave, long width, long height)
{
    long              num = width * height;
    long              ncavemax = 16;
    long              ncormax = 16;
    struct cellcave **cavetab = calloc(ncavemax, sizeof(struct cellcave *));
    long             *idtab = malloc(num * sizeof(long));
    struct cellcor  **cortab;
    struct cellcave  *cave;
    char             *map;
    long              ndx;
    long              lim;

    srandmt32(~0L);
    if (!cavetab) {
        fprintf(stderr, "CELL: failed to allocate cave table\n");

        exit(1);
    }
    if (!idtab) {
        fprintf(stderr, "CELL: failed to allocate cave map\n");

        exit(1);
    }
    for (ndx = 0 ; ndx < num ; ndx++) {
        idtab[num] = DNG_NOCAVE;
    }
    dng->caveidtab = idtab;
    for (ndx = 0 ; ndx < ncave ; ndx++) {
        cave = calloc(1, sizeof(struct cellcave));
        map = calloc(width * height / CHAR_BIT, sizeof(char));
        if (!map) {
            fprintf(stderr, "CELL: failed to allocate cave bitmap\n");
            
            exit(1);
        }
        cortab = calloc(ncormax, sizeof(struct cellcor *));
        if (!cortab) {
            fprintf(stderr, "CELL: failed to allocate corridor table\n");
            
            exit(1);
        }
        cavetab[ndx] = cave;
    }
    dng->width = width;
    dng->height = height;
    dng->map = map;
    dng->ncormax = ncormax;
    dng->cortab = cortab;
    dng->ncave = ncave;
    dng->ncavemax = ncavemax;
    dng->cavetab = cavetab;
    dng->caveparm.rndval = randmt32();
    dng->caveparm.niter = 50000;
    /* set cave parameters */
    dng->caveparm.minsize = 16;
    dng->caveparm.maxsize = 500;
    dng->caveparm.closeprob = 45;
    dng->caveparm.ninvnbor = 4;
    dng->caveparm.nrmnbor = 3;
    dng->caveparm.nfillnbor = 4;
    /* set corridor parameters */
    dng->corparm.breakout = 100000;
    dng->corparm.spacing = 2;
    dng->corparm.minlen = 2;
    dng->corparm.maxlen = 5;
    dng->corparm.maxturn = 10;

    return;
}

long
cellbuilddng(struct celldng *dng, long nlvl)
{
    long id;
    
    for (id = 0 ; id < nlvl ; id++) {
        dnggencave(dng, id);
        dngbuildcave(dng, id);
    }
    dng->ncave = nlvl;
    dngconncaves(dng);

    return nlvl;
}

/* count surrounding neighbors of the cell at (x, y) */
static long
dngcountnbors1(struct celldng *dng, long caveid, long x, long y, long lim)
{
    struct cellcave  *cave = dng->cavetab[caveid];
    long              cnt = 0;
    struct cellcoord *pnt;
    long              dir;
    long              x1;
    long              y1;

    pnt = dngdirofstab;
    for (dir = 0 ; dir < lim ; dir++) {
        x1 = x + pnt->xval;
        y1 = y + pnt->yval;
        if (dnggetcellbit(dng, x1, y1)) {
            cnt++;
        }
        pnt++;
    }

    return cnt;
}

static long
dngcountnbors0(struct celldng *dng, long caveid, long x, long y, long lim)
{
    struct cellcave  *cave = dng->cavetab[caveid];
    long              cnt = 0;
    struct cellcoord *pnt;
    long              dir;
    long              x1;
    long              y1;

    pnt = dngdirofstab;
    for (dir = 0 ; dir < lim ; dir++) {
        x1 = x + pnt->xval;
        y1 = y + pnt->yval;
        if (!dnggetcellbit(dng, x1, y1)) {
            cnt++;
        }
        pnt++;
    }

    return cnt;
}

static void
dnggencave(struct celldng *dng, long caveid)
{
    struct cellcave *cave = dng->cavetab[caveid];
    long             w = dng->width;
    long             h = dng->height;
    char            *map = calloc(h, w / CHAR_BIT);
    long             closeprob = dng->caveparm.closeprob;
    long             ndx;
    long             n = w * h;
    long             x;
    long             y;
    long             lim;

    /* initialise map bitmap */
    for (ndx = 0; ndx < n ; ndx++) {
        if (dngprobpct() < closeprob) {
            /* close a cell (set bit to 1) */
            setbit(map, ndx);
        }
    }
    /*
     * iterate over the map, picking cells at random; if the cell has > lim
     * neighbors close it, otherwise open it
     */ 
    n = dng->caveparm.niter;
    lim = dng->caveparm.ninvnbor;
    for (ndx = 0 ; ndx < n ; ndx++) {
        x = randmt32() % w;
        y = randmt32() % h;
        if (dngcountnbors1(dng, caveid, x, y, DNG_NDIR) > lim) {
            setbit(map, y * w + x);
        } else {
            clrbit(map, y * w + x);
        }
    }
    /*
     * smooth cave edges and single blocks by removing cells with
     * >= lim empty neighbors
     */
    lim = dng->caveparm.nrmnbor;
    for (n = 0 ; n < 5 ; n++) {
        for (y = 0 ; y < h ; y++) {
            for (x = 0 ; x < w ; x++) {
                if (dnggetcellbit(dng, x, y)
                    && dngcountnbors0(dng, caveid, x, y, DNG_NDIR2) >= lim) {
                    clrbit(map, y * w + x);
                }
            }
        }
    }
    /* fill in empty cells with at least lim neighbors to get rid of holes */
    lim = dng->caveparm.nfillnbor;
    for (y = 0 ; y < h ; y++) {
        for (x = 0 ; x < w ; x++) {
            if (!dnggetcellbit(dng, x, y)
                && dngcountnbors1(dng, caveid, x, y, DNG_NDIR2) >= lim) {
                setbit(map, y * w + x);
            }
        }
    }
    dng->cavetab[caveid] = cave;

    return;
}

/*
 * locate all caves within a map
 * NOTE: this function calls itself recursively
 */
static void
dngfindcave(struct celldng *dng, long caveid, long x, long y)
{
    struct cellcave *cave = dng->cavetab[caveid];
    long             dir = DNG_NODIR;
    long             x1;
    long             y1;
    long             ndx;
    long             lim;

    lim = DNG_NDIR;
    for (ndx =  0 ; ndx < lim ; ndx++) {
        x1 = x;
        y1 = y;
        dngmovedir(x1, y1, dir);
        if (!dnggetcellbit(dng, x, y)) {
            dngsetcellbit(dng, x, y);
            dngsetcellid(dng, x, y, caveid);
            cave->size++;
            dngfindcave(dng, caveid, x1, y1);
        }
    }

    return;
}

static void
dngbuildcave(struct celldng *dng, long caveid)
{
    struct cellcave  *cave = dng->cavetab[caveid];
    char             *map;
    long              w = dng->width;
    long              h = dng->height;
    long              x;
    long              y;
    long              id;
    long              ndx;
    long              lim;
    long              n;

    if (!cave) {
        fprintf(stderr, "CELL: failed to allocate cave\n");
        
        exit(1);
    }
    for (y = 0 ; y < h ; y++) {
        for (x = 0 ; x < w ; x++) {
            if (dnggetcellbit(dng, x, y)
                && (dnggetcellid(dng, x, y) == DNG_NOCAVE)) {
                dngfindcave(dng, caveid, x, y);
                n = cave->size;
                if (n <= dng->caveparm.minsize
                    || n > dng->caveparm.maxsize) {
                    dngrmcave(dng, caveid);
                } else {
                    id = dng->ncave;
                    lim = dng->ncavemax;
                    cave->id = id;
                    if (id == lim) {
                        lim <<= 1;
                        dng->cavetab = realloc(dng->cavetab,
                                               lim * sizeof(struct cellcave *));
                        if (!dng->cavetab) {
                            fprintf(stderr,
                                    "CELL: failed to reallocate cave table\n");

                            exit(1);
                        }
                        dng->ncavemax = lim;
                    }
                    dng->cavetab[id] = cave;
                    id++;
                    dng->ncave = id;
                }
            }
        }
    }

    return;
}

static void
dngrmcave(struct celldng *dng, long caveid)
{
    struct cellcave *cave = dng->cavetab[caveid];
    long             ndx1;
    long             ndx2;

    ndx1 = caveid;
    for (ndx2 = ndx1 + 1 ; ndx2 < dng->ncave - 1; ndx2++) {
        dng->cavetab[ndx1] = dng->cavetab[ndx2];
        ndx1++;
    }
    dng->cavetab[ndx2] = NULL;
    dng->ncave--;

    return;
}

static long
dngconncaves(struct celldng *dng)
{
    long               w;
    long               h;
    long               ncave = dng->ncave;
    long               ncor = 0;
    long               corx = 0;
    long               cory = 0;
    long               dir = DNG_NODIR;
    struct cellcor    *cor;
    long               nconn = 0;
    long               nconnmax = ncave;
    struct cellcoord  *coord1;
    struct cellcoord  *coord2;
    struct cellcave  **conntab = calloc(ncave, sizeof(struct cellcave *));
    struct cellcave   *cave;
    long               id;
    long               x;
    long               y;
    long               x1;
    long               y1;
    long               num;
    long               ndx;
    long               lim;
    long               brkcnt;
    
    cave = dng->cavetab[randmt32() % ncave];
    w = dng->width;
    h = dng->height;
    conntab[nconn] = cave;
    nconn++;
    do {
        id = cave->id;
        if (!ncor) {
            ndx = randmt32() % nconn;
            cave = conntab[ndx];
            id = cave->id;
            dngfindedge(dng, id, &corx, &cory, &dir);
        } else if (dngprobpct() > 50) {
            ndx = randmt32() % nconn;
            cave = conntab[ndx];
            id = cave->id;
            dngfindedge(dng, id, &corx, &cory, &dir);
        } else {
            cave = NULL;
            dngfindcoredge(dng, id, &corx, &cory, &dir);
        }
        cor = dngtrycor(dng, id, corx, cory, dir, 0);
        if (cor) {
            for (ndx = 0 ; ndx < ncave ; ndx++) {
                cave = dng->cavetab[ndx];
                if ((cave) && (ncor)) {
                    x1 = cor->pnttab[ncor - 1].xval;
                    y1 = cor->pnttab[ncor - 1].yval;
                } else {
                    x1 = 0;
                    y1 = 0;
                }
                if (dnggetcellid(dng, x1, y1) == ndx) {
                    if (!cave || id != ndx) {
                        num = ndx + 1;
                        coord1 = &cor->pnttab[ndx];
                        coord2 = &cor->pnttab[num];
                        while (num < ncor) {
                            x1 = coord1->xval;
                            y1 = coord1->yval;
                            coord1 = coord2 - 1;
                            coord1->xval = x1;
                            coord1->yval = y1;
                            num++;
                            coord2++;
                        }
                        coord1 = cor->pnttab;
                        for (num = 0 ; num < ncor ; num++) {
                            x1 = coord1->xval;
                            y1 = coord1->yval;
                            dngsetcellbit(dng, x1, y1);
                            coord1++;
                        }
                        if (nconn == nconnmax) {
                            nconnmax <<= 1;
                            conntab = realloc(conntab,
                                              nconnmax * sizeof(struct cave *));
                            if (!conntab) {
                                fprintf(stderr,
                                        "CELL: failed to allocate connection table\n");
                                
                                exit(1);
                            }
                        }
                        conntab[nconn] = cave;
                        ncave--;
                        nconn++;

                        break;
                    }
                }
            }
        }
        lim = dng->corparm.breakout;
        brkcnt++;
        if (brkcnt >= lim) {
            free(cor);

            return 0;
        }
    } while (ncave > 0);
    /* FIXME: move connected caves to caves */
    free(dng->cavetab);
    dng->ncave = nconn;
    dng->ncavemax = nconnmax;
    dng->cavetab = conntab;

    return 1;
}

static long
dngturncor(long dir)
{
    long ret;

    if (dir == DNG_NODIR) {
        ret = dngdirtab[randmt32() % DNG_NDIR];
    } else {
        do {
            ret = dngdirtab[randmt32() % DNG_NDIR];
        } while (ret == dngrevdir(dir));
    }

    return ret;
}

static long
dngturncor2(long dir)
{
    long ret;

    if (dir == DNG_NODIR) {
        ret = dngdirtab[randmt32() % DNG_NDIR];
    } else {
        do {
            ret = dngdirtab[randmt32() % DNG_NDIR];
        } while (ret == dngrevdir(dir) || ret == dir);
    }

    return ret;
}

static void
dngfindedge(struct celldng *dng, long caveid, long *retx, long *rety,
            long *retdir)
{
    struct cellcave *cave = dng->cavetab[caveid];
    long             dir = DNG_NODIR;
    long             w = dng->width;
    long             h = dng->height;
    long             x;
    long             y;

    do {
        x = randmt32() % w;
        y = randmt32() % h;
        dir = dngturncor(dir);
        do {
            dngmovedir(x, y, dir);
            if (x < 0 || x >= w || y < 0 || y >= h) {

                break;
            } else if (!dnggetcellbit(dng, x, y)) {
                *retx = x;
                *rety = y;
                *retdir = dir;

                return;
            }
        } while (1);
    } while (1);
    *retdir = -1;

    return;
}

static void
dngfindcoredge(struct celldng *dng, long caveid,
               long *retx, long *rety, long *retdir)
{
    struct cellcave   *cave = dng->cavetab[caveid];
    long               ndir = 0;
    long               dirtab[DNG_NDIR];
    long               ncor = dng->ncor;
    struct cellcor   **cortab = dng->cortab;;
    struct cellcor    *cor;
    struct cellcoord  *ofs;
    long               x = *retx;
    long               y = *rety;
    long               dir = *retdir;
    long               w = dng->width;
    long               h = dng->height;
    long               x1;
    long               y1;
    long               ndx;

    do {
        if (ncor) {
            ndx = randmt32() % ncor;
        }
        cor = cortab[ndx];
        for (ndx = 0 ; ndx < DNG_NDIR ; ndx++) {
            ofs = &dngdirofstab[ndx];
            x1 = x + ofs->xval;
            y1 += y + ofs->yval;
            if (x1 >= 0 && x1 < w && y1 >= 0 && y1 <= h
                && dnggetcellbit(dng, x1, y1)) {
                dir = dngdirtab[ndx];
                ndir++;
            }
        }
    } while (!ndir);
    ndx = randmt32() % ndir;
    *retdir = dir;
    *retx = x;
    *rety = y;

    return;
}

static long
dngchkcorpnt(struct celldng *dng, long caveid, long x, long y, long dir)
{
    struct cellcave *cave = dng->cavetab[caveid];
    long             xofs = dngdirofstab[dir].xval;
    long             yofs = dngdirofstab[dir].yval;
    long             space = dng->corparm.spacing;
    long             w = dng->width;
    long             h = dng->height;
    long             x1;
    long             y1;
    long             ofs;

    for (ofs = -space ; ofs <= space ; ofs++) {
        if (xofs == 0) {
            x1 = x + space;
            if (x1 >= 0 && x1 < w && y >= 0 && y < h
                && dnggetcellbit(dng, x1, y)) {

                return 0;
            }
        } else if (yofs == 0) {
            y1 = y + space;
            if (x >= 0 && x < w && y1 >= 0 && y1 < h
                && dnggetcellbit(dng, x, y1)) {

                return 0;
            }
        }
    }

    return 1;
}

static struct cellcor *
dngtrycor(struct celldng *dng, long caveid,
          long x, long y,
          long dir, long backtrack)
{
    struct cellcave  *cave = dng->cavetab[caveid];
    long              ncor = dng->ncor;
    long              min = dng->corparm.minlen;
    long              max = dng->corparm.maxlen;
    long              nturn = dng->corparm.maxturn;
    struct cellcor   *cor = calloc(1, sizeof(struct cellcor));
    long              ncormax = dng->ncormax;
    long              npnt;
    long              npntmax;
    struct cellcoord *pnttab;
    struct cellcoord *coord;
    long              w = dng->width;
    long              h = dng->height;
    long              corx;
    long              cory;
    long              len;

    npntmax = 16;
    pnttab = calloc(npntmax, sizeof(struct cellcoord));
    if (!pnttab) {
        fprintf(stderr, "CELL: failed to allocate corridor point table\n");

        exit(1);
    }
    coord = pnttab;
    coord->xval = x;
    coord->yval = y;
    npnt = 1;
    coord++;
    while (nturn) {
        if (npnt == ncormax) {
            ncormax <<= 1;
            pnttab = realloc(pnttab,
                             ncormax * sizeof(struct cellcoord));
            coord = &pnttab[npnt];
        }
        len = min + (randmt32() % (max - min + 1));
        while (len) {
            len--;
            corx = x + dngdirofstab[dir].xval;
            cory = y + dngdirofstab[dir].yval;
            if (corx >= 0 && corx < w && cory >= 0 && cory < h
                && dnggetcellbit(dng, corx, cory)) {
                cor->n = npnt;
                cor->pnttab = pnttab;
                coord->xval = corx;
                coord->yval = cory;

                return cor;
            } else if (!(corx >= 0 && corx < w && cory >= 0 && cory < h)
                       || !dngchkcorpnt(dng, caveid, corx, corx, dir)) {
                free(cor);
                free(pnttab);

                return NULL;
            }
        }
        nturn--;
        if (nturn > 1) {
            if (backtrack) {
                dir = dngturncor(dir);
            } else {
                dir = dngturncor2(dir);
            }
        }
        npnt++;
        coord++;
    }
    cor->n = npnt;
    cor->pnttab = pnttab;
    if (ncor == ncormax) {
        ncormax <<= 1;
        dng->cortab = realloc(dng->cortab,
                              ncormax * sizeof(struct cellcor));
        if (!dng->cortab) {
            fprintf(stderr, "CELL: failed to reallocate corridor table\n");
            
            exit(1);
        }
        dng->ncormax = ncormax;
    }
    dng->cortab[ncor] = cor;
    ncor++;
    dng->ncor = ncor;

    return cor;
}

