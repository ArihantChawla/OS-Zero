/*
 * hazard pointer implementation based on ConcurrencyKit
 *
 * REFERENCES
 * ----------
 * - https://github.com/concurrencykit/ck/blob/master/src/ck_hp.c
 * - https://github.com/concurrencykit/ck/blob/master/src/ck_hp.h
 */

#include <mach/param.h>
#include <mach/atomic.h>
#include <mach/asm.h>

#define HAZPTRUSED 0L
#define HAZPTRFREE 1L
#if !defined(HAZBUFPTRS)
#define HAZBUFPTRS (PAGESIZE / sizeof(void *) - 16) // page with book-keeping
#endif

struct hazdata {
    void          *ptr;
    void          *data;
    struct hazptr *pend;
};

struct hazstk {
    m_atomicptr_t  head;
    char          *gen;
};

#define hazptrsetthresh(hp, val) m_atomwrite(&hp->thresh, val)
struct hazptr {
    struct hazptr   *prev; // next item on queue/stack
    struct hazptr   *next; // next item on stack
    struct hazstk    stk; // hazard-pointer stack
    m_atomic_t       nstk; // # of items on stack
    m_atomic_t       nfree; // # balance of register/unregister operations
    m_atomic_t       thresh;
    m_atomic_t       degree;
    struct hazrec   *parent;
    void           (*recl)(void *);
    struct hazdata   data; // pointer and generation
};

/* this is best to have aligned and to cacheline boundary */
struct hazrec {
    void          *buf[HAZBUFPTRS];
    m_atomic_t      state;
    void          **ptrtab;
    struct hazptr  *hptr; // global hazard pointer entry
    m_atomicptr_t   pend;
    m_atomic_t      npend;
    struct hazptr  *stkitem;
    m_atomic_t      npeak;
    uintmax_t       nrecl;
};

#define HAZPTR_STK_T struct hazptr;

static __inline__ void
hazpushptr(struct hazptr *hp)
{
    struct hazptr *cur;
    struct hazptr *next;

    do {
        cur = hp->stk.head;
        next = cur->next;
        if (m_cmpswapptr(&hp->stk.head, cur, next)) {
            m_atominc(&hp->nstk);

            return;
        }
        m_waitspin();
    } while (1);
}

static __inline__ struct hazptr *
hazpopptr(struct hazptr *hp)
{
    struct hazrec *rec = hp->parent;
    struct hazptr *head = NULL;
    struct hazptr *next;

    do {
        head = m_atomread(&hp->stk.head);
        if (head) {
            if (m_cmpswapptr(&hp->stk.head, head, head->next)) {

                return hp;
            }
        }
    } while (head);

    return NULL;
}

static __inline__ void *
__hazsetptr(struct hazrec *rec, m_atomic_t ndx, void *ptr)
{
    void **pptr = &rec->ptrtab[ndx];

    *pptr = ptr;
    m_membar();

    return pptr;
}

static __inline__ void *
hazsetptr(struct hazrec *rec, m_atomic_t ndx, void *ptr)
{
    void **pptr = &rec->ptrtab[ndx];

    *pptr = ptr;
    m_membar();

    return pptr;
}

static __inline__ void
hazclrrec(struct hazrec *rec)
{
    void **ptr = rec->ptrtab;
    long   ndx;

    for (ndx = 0 ; ndx < rec->hptr->degree ; ndx++) {
        ptr[ndx] = NULL;
    }

    return;
}

