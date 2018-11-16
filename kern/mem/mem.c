#include <stddef.h>
#include <stdint.h>
#include <zero/trix.h>
#include <kern/mem/mem.h>
#include <kern/mem/zone.h>
#include <kern/mem/page.h>
#include <kern/unit/ia32/vm.h>

extern struct memzone k_memzonetab[MEM_ZONES];

void
meminit(m_ureg_t base, m_ureg_t nbphys, m_ureg_t nbvirt)
{
    m_ureg_t lim = min(KERNVIRTBASE, nbphys);
    m_ureg_t end;
    m_ureg_t adr;

    vminitphys((uintptr_t)base, lim - base);
    end = min(KERNVIRTBASE, nbvirt);
    if (end > lim) {
        vminitvirt((void *)lim, end - lim, PAGEWRITE);
    }
#if defined(__x86_64__) || defined(__amd64__)
#error implement x86-64 memory management
#endif

    return;
}

