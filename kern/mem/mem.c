#include <stddef.h>
#include <stdint.h>
#include <zero/trix.h>
#include <kern/mem/mem.h>
#include <kern/mem/zone.h>
#include <kern/mem/page.h>
#include <kern/unit/ia32/vm.h>

extern struct memzone k_memzonetab[MEM_ZONES];

void
meminit(m_ureg_t nbphys, m_ureg_t nbvirt)
{
    m_ureg_t lim = max(nbphys, KERNVIRTBASE);
    m_ureg_t adr;

    vmmapseg((m_ureg_t)nbvirt,
             (m_ureg_t)nbvirt,
             (m_ureg_t)lim,
             PAGEPRES | PAGEWRITE);
#if defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__)
    pageinitphys((m_ureg_t)&_eusr,
                 lim - (m_ureg_t)&_eusr);
    lim = max(nbvirt, KERNVIRTBASE);
    vminitvirt(&_eusr, nbvirt, PAGEWRITE);
#elif defined(__x86_64__) || defined(__amd64__)
#error implement x86-64 memory management
#endif
//    meminitbuf();
//    swapinit(0, 0x00000000, 1024);

    return;
}

