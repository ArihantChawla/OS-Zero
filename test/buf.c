#define __KERNEL__ 1
#include <stddef.h>
#include <zero/mtx.h>
#include <mem.h>
#include <buf.h>

#if (BUFTEST)
#define kmalloc(nb) malloc(nb)
#define kfree(ptr)  free(ptr)
#endif

#define devgetblk(buf, blk) devfindblk(buf, blk, 0)

void *
devbufblk(struct devbuf *buf, blkid_t blk, void *data)
{
    struct buftab *tab;
    struct buftab *ptr = NULL;
    void          *ret = NULL;
    long           fail = 0;
    long           key0;
    long           key1;
    long           key2;
    long           key3;
    long           val;
    long           nlvl;
    void          *pstk[BUFNKEY] = { NULL, NULL, NULL, NULL };
    long           kstk[BUFNKEY] = { -1, -1, -1, -1 };

    key0 = bufkey0(blk);
    key1 = bufkey1(blk);
    key2 = bufkey2(blk);
    key3 = bufkey3(blk);
    mtxlk(&buf->lk, 1);
    ptr = buf->tab.ptr;
    if (!ptr) {
        ptr = kmalloc(NLVL0BLK * sizeof(struct buftab));
        if (ptr) {
            bzero(ptr, NLVL0BLK * sizeof(struct buftab));
        }
        pstk[0] = ptr;
    }
    if (ptr) {
        ptr = ptr[key0].ptr;
        if (!ptr) {
            ptr = kmalloc(NLVL1BLK * sizeof(struct buftab));
            if (ptr) {
                bzero(ptr, NLVL1BLK * sizeof(struct buftab));
            }
            pstk[1] = ptr;
        }
    } else {
        fail = 1;
    }
    if (ptr) {
        ptr = ptr[key1].ptr;
        if (!ptr) {
            ptr = kmalloc(NLVL2BLK * sizeof(struct buftab));
            if (ptr) {
                bzero(ptr, NLVL2BLK * sizeof(struct buftab));
            }
            pstk[2] = ptr;
        }
    } else {
        fail = 1;
    }
    if (ptr) {
        ptr = ptr[key2].ptr;
        if (!ptr) {
            ptr = kmalloc(NLVL3BLK * sizeof(struct buftab));
            if (ptr) {
                bzero(ptr, NLVL3BLK * sizeof(struct buftab));
            }
            pstk[3] = ptr;
        }
    } else {
        fail = 1;
    }
    if (!fail) {
        if (pstk[0]) {
            buf->tab.nref++;
            buf->tab.ptr = pstk[0];
        }
        tab = buf->tab.ptr;
        if (pstk[1]) {
            tab[key0].nref++;
            tab[key0].ptr = pstk[1];
        }
        tab = tab[key0].ptr;
        if (pstk[2]) {
            tab[key1].nref++;
            tab[key1].ptr = pstk[2];
        }
        tab = tab[key1].ptr;
        if (pstk[3]) {
            tab[key2].nref++;
            tab[key2].ptr = pstk[3];
        }
        tab = tab[key2].ptr;
        tab = &tab[key3];
        tab->nref++;
        tab->ptr = data;
        ret = data;
    }
    mtxunlk(&buf->lk, 1);
    
    return ret;
}

void *
devfindblk(struct devbuf *buf, blkid_t blk, long free)
{
    void          *ret = NULL;
    struct buftab *tab;
    long           key0;
    long           key1;
    long           key2;
    long           key3;

    key0 = bufkey0(blk);
    key1 = bufkey1(blk);
    key2 = bufkey2(blk);
    key3 = bufkey3(blk);
    mtxlk(&buf->lk, 1);
    tab = buf->tab.ptr;
    if (tab) {
        tab = tab[key0].ptr;
        if (tab) {
            tab = tab[key1].ptr;
            if (tab) {
                tab = tab[key2].ptr;
                if (tab) {
                    ret = tab[key3].ptr;
                }
            }
        }
    }
    if (!free) {
        mtxunlk(&buf->lk, 1);
    }

    return ret;
}

void *
devfreeblk(struct devbuf *buf, blkid_t blk)
{
    long           key0 = bufkey0(blk);
    long           key1 = bufkey1(blk);
    long           key2 = bufkey2(blk);
    long           key3 = bufkey3(blk);
    void          *ret = devfindblk(buf, blk, 1);
    void          *ptr;
    struct buftab *tab;
    long           nref;
    long           val;
    void          *pstk[BUFNKEY] = { NULL, NULL, NULL, NULL };

    if (ret) {
        tab = buf->tab.ptr;
        nref = --tab->nref;
        if (!nref) {
            pstk[0] = tab;
            buf->tab.ptr = NULL;
        }
        tab = tab[key0].ptr;
        nref = --tab->nref;
        if (!nref) {
            pstk[1] = tab;
            tab[key0].ptr = NULL;
        }
        tab = tab[key1].ptr;
        nref = --tab->nref;
        if (!nref) {
            pstk[2] = tab;
            tab[key1].ptr = NULL;
        }
        tab = tab[key2].ptr;
        nref = --tab->nref;
        if (!nref) {
            pstk[3] = tab;
            tab[key2].ptr = NULL;
        }
        tab = &tab[key3];
        nref = --tab->nref;
        if (!nref) {
            for (val = 0 ; val < BUFNKEY ; val++) {
                ptr = pstk[val];
                if (ptr) {
                    kfree(ptr);
                }
            }
        }
    }
    mtxunlk(&buf->lk, 1);

    return ret;
}

