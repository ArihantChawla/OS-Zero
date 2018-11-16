#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/hash.h>
#include <mach/param.h>
#include <mach/asm.h>
#include <kern/malloc.h>
#include <kern/sched.h>
#include <kern/proc/task.h>

#define taskgetwaittab() kcalloc(sizeof(struct taskwaittab))

struct taskwaittab *k_waithash[TASKWAITHASHITEMS] ALIGNED(PAGESIZE);
struct taskwaittab *k_wakehash[TASKWAITHASHITEMS];

/* add task to wait table */
void
schedsetwait(struct task *task)
{
    uintptr_t            chan = task->waitchan;
    long                 key = tmhash32((uint32_t)chan);
    long                 n = TASKWAITTABITEMS;
    struct taskwaittab  *tab = NULL;
    struct taskwaittab  *cur;
    struct taskwaittab  *prev;
    struct taskwaittab **hptr;
    uintptr_t            uptr;
    struct taskwaittab  *head;

    key &= (TASKWAITHASHITEMS - 1);
    m_lkbit((m_atomic_t *)&k_waithash[key], TASK_LK_BIT_POS);
    uptr = (uintptr_t)k_waithash[key];
    uptr &= ~TASK_LK_BIT;
    prev = NULL;
    cur = (struct taskwaittab *)uptr;
    head = cur;
    while (cur) {
        /* scan whole chain, move tables matching chan to head of chain */
        if (cur->chan == chan) {
            tab = cur->next;
            if (n >= TASKWAITTABITEMS) {
                /* task has not been stored yet */
                n = m_fetchadd((m_atomic_t *)&cur->n, 1);
                if (n < TASKWAITTABITEMS) {
                    /* room for new item in cur */
                    cur->buf[n] = task;
                } else {
                    /* cur is full */
                    m_atomwrite((m_atomic_t *)&cur->n, TASKWAITTABITEMS);
                }
            }
            if (prev) {
                /* move table cur to head of chain */
                prev->next = tab;
            }
            cur->next = head;
            head = cur;
            cur = (void *)((uintptr_t)cur | TASK_LK_BIT);
            m_atomwrite((m_atomic_t *)&k_waithash[key], cur);
            if (hptr) {
                m_clrbit((m_atomic_t *)hptr, TASK_LK_BIT_POS);
            }
            cur = tab;
        } else {
            prev = cur;
            hptr = &cur;
            cur = cur->next;
        }
    }
    if (n >= TASKWAITTABITEMS) {
        tab = taskgetwaittab();
        tab->n = 1;
        tab->buf[0] = task;
        tab->next = head;
        m_atomwrite(&k_waithash[key], tab);
    }

    return;
}

/* wake up tasks waiting on chan; move them to the wake-hash */
void
schedwakeup(uintptr_t chan)
{
    long                key = tmhash32((uint32_t)chan);
    long                n = TASKWAITTABITEMS;
    uintptr_t           uptr;
    struct taskwaittab *tab;
    struct taskwaittab *cur;
    struct taskwaittab *prev;
    struct taskwaittab *next;

    key &= (TASKWAITHASHITEMS - 1);
    m_lkbit((m_atomic_t *)&k_waithash[key], TASK_LK_BIT_POS);
    uptr = (uintptr_t)k_waithash[key];
    uptr &= ~TASK_LK_BIT;
    prev = NULL;
    cur = (struct taskwaittab *)uptr;
    while (cur) {
        /* scan whole chain, move tables matching chan to head of chain */
        if (cur->chan == chan) {
            next = cur->next;
            if (prev) {
                /* move table cur to head of chain */
                prev->next = next;
            }
            m_lkbit((m_atomic_t *)&k_wakehash[key], TASK_LK_BIT_POS);
            uptr = (uintptr_t)k_wakehash[key];
            uptr &= ~TASK_LK_BIT;
            tab = (struct taskwaittab *)uptr;
            cur->next = tab;
            m_atomwrite((m_atomic_t *)&k_waithash[key], cur);
            cur = next;
        } else {
            prev = cur;
            cur = cur->next;
        }
    }

    return;
}

