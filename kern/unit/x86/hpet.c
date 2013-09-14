#include <stddef.h>
#include <stdint.h>
#include <kern/conf.h>
#include <kern/util.h>
#include <kern/unit/x86/hpet.h>

#if (HPET)
#define EBDAADR     0x040e
#define ACPIBASEADR 0xe0000
#define ACPIMEMSZ   (0xfffff - ACPIBASEADR)

volatile struct hpet *hpetptr;

static long
hpetsum(uint8_t *ptr, unsigned long len)
{
    uint8_t sum = 0;

    while (len--) {
        sum += *ptr++;
    }

    return sum;
}

static struct hpet *
hpetfind(uintptr_t adr, unsigned long len)
{
    uint32_t    *lim = (uint32_t *)(adr + len);
    struct hpet *hpet = NULL;
    uint32_t    *ptr = (uint32_t *)adr;

    while (ptr < lim) {
        if (*ptr == HPETSIG && !hpetsum((uint8_t *)ptr, sizeof(struct hpet))) {
            hpet = (struct hpet *)ptr;

            return hpet;
        }
        ptr++;
    }

    return hpet;
}

void
hpetinit(void)
{
    uintptr_t      adr = (uintptr_t)(((uint16_t *)EBDAADR)[0]) << 4;
    unsigned long  len = 0xa0000 - adr;
    struct hpet   *hpet = NULL;

    hpet = hpetfind(adr, len);
    if (!hpet) {
        adr = ACPIBASEADR;
        hpet = hpetfind(ACPIBASEADR, ACPIMEMSZ);
    }
    if (hpet) {
        kprintf("HPET table found @ %p\n", hpet);
    } else {
        kprintf("HPET not found\n");
    }
    hpetptr = hpet;

    return;
}

#endif /* HPET */

