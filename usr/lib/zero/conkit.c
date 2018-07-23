/*
 * work-in-progress alpha-quality hazard pointers based on ConcurrencyKit :)
 *
 * Please support the CK project up at https://github.com/concurrencykit
 */

#include <stddef.h>
#include <string.h>
#include <mach/atomic.h>
#include <mach/asm.h>
#include <zero/conkit.h>
#include <zero/trix.h>

#define hazinitstknode(hp) ((hp)->stk.head = (hp)->stk.gen = NULL)

void
hazinitptr(struct hazptr *hp,
           m_atomic_t degree,
           m_atomic_t threshold,
           void (*recl)(void *))
{
    hazinitstknode(hp);
    hp->nstk = 0;
    hp->nfree = 0;
    hp->thresh = threshold;
    hp->degree = degree;
    hp->recl = recl;
    m_membar();

    return;
}

struct hazrec *
hazreuse(struct hazptr *hptr)
{
    struct hazrec *rec = hptr->parent;

    if (m_atomread(&hptr->nfree)) {
        if (m_atomread(&rec->state) == HAZPTRFREE) {
            m_membar();
            if (m_cmpswap(&rec->state, HAZPTRUSED, HAZPTRFREE)) {
                m_atomdec(&hptr->nfree);

                return rec;
            }
        }
    }

    return NULL;
}

static void
hazzeroptrtab(void **tab, size_t nptr)
{
    size_t nleft = nptr;
    size_t n;

    do {
        n = min(8, nleft);
        switch (n) {
            default:
            case 8:
                tab[7] = NULL;
            case 7:
                tab[6] = NULL;
            case 6:
                tab[5] = NULL;
            case 5:
                tab[4] = NULL;
            case 4:
                tab[3] = NULL;
            case 3:
                tab[2] = NULL;
            case 2:
                tab[1] = NULL;
            case 1:
                tab[0] = NULL;
            case 0:

                break;
        }
        nleft -= n;
        tab += n;
    } while (nleft);
}

void
hazregptr(struct hazrec *rec, struct hazptr *hp, void **ptrtab)
{
    hazzeroptrtab(ptrtab, hp->degree);
    rec->state = HAZPTRUSED;
    rec->ptrtab = ptrtab;
    rec->hptr = hp;
    rec->npend = 0;
    rec->npeak = 0;
    rec->nrecl = 0;
    hp->parent = rec;
    m_membar();
    hazpushptr(hp);
}

void
hazunregptr(struct hazrec *rec)
{
    struct hazptr *hptr = rec->hptr;

    rec->npend = 0;
    rec->npeak = 0;
    rec->nrecl = 0;
    m_membar();
    m_atomwrite(rec->state, HAZPTRFREE);
    m_atominc(&hptr->nfree);

    return;
}

/*
 * search hp's [global] parent record for ptr. return 1 if found, 0 otherwise.
 */
static long
hazseekptr(struct hazptr *hp, m_atomic_t degree, void *ptr)
{
    struct hazrec  *rec = hp->parent;
    void          **tab = rec->ptrtab;
    m_atomic_t      ndx;
    void           *hazptr;

    while (hp) {
        if (m_atomread(&rec->state) != HAZPTRFREE
            && !(tab = m_atomread(&rec->ptrtab))) {
            for (ndx = 0 ; ndx < degree ; ndx++) {
                hazptr = m_atomread(&tab[ndx]);
                if (hazptr == ptr) {

                    return 1;
                }
            }
        }
        hp = hazpopptr(hp);
    }

    return 0;
}

static void *
hazbufptr(struct hazptr *hp, m_atomic_t *nhaz)
{
    struct hazrec  *rec = hp->parent;
    void          **buf = rec->buf;
    void          **tab = rec->ptrtab;
    m_atomic_t      n = 0;
    m_atomic_t      ndx;
    void           *ptr;

    while (hp) {
        if (m_atomread(&rec->state) !=  HAZPTRFREE
            && !(tab = m_atomread(&rec->ptrtab))) {
            for (ndx = 0 ; ndx < hp->degree ; ndx++) {
                if (n > HAZBUFPTRS) {

                    break;
                }
                ptr = m_atomread(&tab[ndx]);
                if (ptr) {
                    buf[n] = ptr;
                    n++;
                }
            }
        }
        hp = hazpopptr(hp);
    }
    *nhaz = n;

    return hp;
}

static int
hazcmpptr(const void *a, const void *b)
{
    long          diff;
    void * const *x;
    void * const *y;

    x = a;
    y = b;
    diff = (*x > *y) - (*x < * y);

    return diff;
}

void
hazreaprec(struct hazrec *thrrec)
{
    struct hazptr   *hp = thrrec->hptr;
    void           **buf = thrrec->buf;
    struct hazdata  *haz;
    void            *mark;
    void            *match;
    size_t           n;
    struct hazptr   *head;
    struct hazptr   *prev;
    struct hazptr   *next;

    mark = hazseekptr(hp, hp->degree, &n);
    qsort(buf, n, sizeof(void *), hazcmpptr);
    prev = NULL;
    while (hp) {
        haz = &hp->data;
        match = bsearch(&haz->ptr, buf, n, sizeof(void *), hazcmpptr);
        if (!match
            || (!mark && hazseekptr(mark, hp->degree, haz->ptr))) {
            prev = hp;

            continue;
        }
        thrrec->npend--;
        if (prev) {
            prev->next = hp->next;
        } else {
            do {
                head = thrrec->pend;
                hp->next = head;
                if (m_cmpswapptr(&thrrec->pend, head, hp)) {

                    break;
                }
                m_waitspin();
            } while (1);
        }
        hp->recl(haz->data);
        thrrec->nrecl--;
    }

    return;
}

