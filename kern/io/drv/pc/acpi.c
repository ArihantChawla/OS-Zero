#include <kern/conf.h>

#if (ACPI)

#include <stdint.h>
#include <stddef.h>

#include <kern/io/drv/pc/acpi.h>

#define EBDAADR 0x040e

volatile struct acpidesc *acpidesc;

struct acpidesc *
acpifind(void)
{
    uint64_t *ptr;
    uint64_t *lim;

    /* search extended BIOS data area */
    ptr = (uint64_t *)(EBDAADR << 4);
    lim = ptr + 512;
    while (ptr < lim) {
        if (*ptr == ACPIRSDPTRSIG) {

            return (struct acpidesc *)ptr;
        }
        ptr += 2;
    }
    /* search BIOS read-only memory 0xe0000..0xfffff */
    ptr = (uint64_t *)0xe0000;
    lim = (uint64_t *)0xfffff;
    while (ptr < lim) {
        if (*ptr == ACPIRSDPTRSIG) {

            return (struct acpidesc *)ptr;
        }
        ptr += 2;
    }

    return NULL;
}

void
acpiinit(void)
{
    acpidesc = acpifind();

    return;
}

#endif /* ACPI */

