#include <stddef.h>
#include <kern/conf.h>
#include <kern/util.h>
#include <kern/obj.h>
#include <kern/mem.h>
#include <zero/param.h>
#include <kern/proc/proc.h>
#include <kern/unit/x86/cpu.h>
#include <kern/unit/ia32/boot.h>
#include <kern/unit/ia32/vm.h>

struct proc proctab[NPROC] ALIGNED(PAGESIZE);
struct thr  thrtab[NTHR] ALIGNED(PAGESIZE);

long
procinit(long id)
{
    struct proc *proc = &proctab[id];
    struct thr  *thr;
    void        *ptr;

    if (!id) {
        /* bootstrap */
        curproc = proc;
        /* process ID will be zero */
        thr = &thrtab[0];
        proc->thr = thr;
        curthr = thr;
    }
    if (proc) {
        proc->state = PROCINIT;
        /* initialise page directory */
//        kbzero(proc, sizeof(struct proc));
        ptr = kmalloc(NPDE * sizeof(pde_t));
        if (ptr) {
            kbzero(ptr, NPDE * sizeof(pde_t));
            proc->pdir = ptr;
        } else {
            kfree(proc);

            return -1;
        }
        /* initialise kernel-mode stack (wired) */
        ptr = kwalloc(KERNSTKSIZE);
        if (ptr) {
            kbzero(ptr, KERNSTKSIZE);
            proc->kstk = ptr;
        } else {
            kfree(proc->pdir);
            kfree(proc);

            return -1;
        }
        /* initialise descriptor table */
        ptr = kmalloc(NDESCTAB * sizeof(desc_t));
        if (ptr) {
            kbzero(ptr, NDESCTAB * sizeof(desc_t));
            proc->dtab = ptr;
        } else {
            kfree(proc->pdir);
            kfree(proc->kstk);
            kfree(proc);

            return -1;
        }
#if 0
        /* initialise VM structures */
        ptr = kmalloc(NVMHDRTAB * sizeof(struct vmpage));
        if (ptr) {
            kbzero(ptr, NVMHDRTAB * sizeof(struct vmpage));
            proc->vmhdrtab = ptr;
        } else {
            kfree(proc->pdir);
            kfree(proc->kstk);
            kfree(proc->dtab);
            kfree(proc);

            return -1;
        }
#endif
        proc->state = PROCREADY;
    }

    return 0;
}

void *
procgetdesc(struct proc *proc, long id)
{
    void      *ret = NULL;
    long       lim = NDESCTAB - 1;
    long       val1;
    long       val2;
    uintptr_t *tab2;
    uintptr_t *tab3;

    if (id <= lim) {
        ret = (void *)(proc->dtab[id]);
    } else {
        tab2 = proc->dtab2;
        if (tab2) {
            val1 = id >> NDESCTABLOG2;
            val2 = id & (NDESCTAB - 1);
            tab3 = (uintptr_t *)(tab2[val1]);
            if (tab3) {
                ret = (void *)(tab3[val2]);
            }
        }
    }

    return ret;
}

/* see <kern/proc.h> for definitions of scheduler classes */
struct proc *
newproc(int argc, char *argv[], char *envp[], int class)
{
    struct proc *proc = kmalloc(sizeof(struct proc));

    proc->state = PROCINIT;
    proc->class = class;
    proc->pid = taskgetpid();
    proc->parent = curproc->pid;
    proc->argc = argc;
    proc->argv = argv;
    proc->envp = envp;

    return proc;
}

