#include <kern/conf.h>

#if (ACPICA)

#include <zero/trix.h>

#include <kern/io/drv/pc/acpi.h>
#include <kern/mem/vm.h>
#if defined(__x86_64__) || defined(__amd64__)
#error implement virtual memory for 64-bit x86
#else
#include <kern/unit/ia32/link.h>
#endif

ACPI_STATUS
AcpiOsInitialize(void)
{

    return;
}

ACPI_STATUS
AcpiOsTerminate(void)
{

    return;
}

ACPI_PHYSICAL_ADDRESS
AcpiOsGetRootPointer(void)
{
    return (ACPI_PHYSICAL_ADDRESS)acpifind();
}

ACPI_STATUS
AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES *obj, ACPI_STRING *newval)
{
    *newval = NULL;

    return;
}

ACPI_STATUS
AcpiOsTableOverride(ACPI_TABLE_HEADER *tab, ACPI_TABLE_HEADER *newtab)
{
    *newtab = NULL;

    return;
}

void *
AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS physadr, ACPI_SIZE len)
{
    uintptr_t adr;
    uintptr_t ofs;

    ofs = (uintptr_t)physadr & (PAGESIZE - 1);
    len = rounduppow2(len, PAGESIZE);
    adr = (uintptr_t)kmalloc(len);
    vmmapseg(&_pagetab,
             (uintptr_t)adr,
             (uintptr_t)physadr,
             (uintptr_t)(physadr + len),
             PAGEPRES | PAGEWRITE);

    return (void *)(adr | ofs);
}

void
AcpiOsUnmapMemory(void *virtadr, ACPI_SIZE len)
{
    vmfreephys(virtadr, len);
}

ACPI_STATUS
AcpiOsGetPhysicalAddress(void *virtadr, ACPI_PHYSICAL_ADDRESS *physret)
{
    *physret = (ACPI_PHYSICAL_ADDRESS)vmphysadr(virtadr);

    return;
}

void *
AcpiOsAllocate(ACPI_SIZE size)
{
    void *ptr;

    ptr = kmalloc(size);

    return ptr;
}

/* TODO: kfree() doesn't unmap memory yet */

void
AcpiOsFree(void *virtadr)
{
    kfree(virtadr);

    return;
}

/* TODO: make sure FALSE and TRUE are correct values */

BOOLEAN
AcpiOsReadable(void *virtadr, ACPI_SIZE len)
{
    unsigned long *pte;
    unsigned long  pval;
    long           n;

    n = rounduppow2(len, PAGESIZE) >> PAGESIZELOG2;
    pte = (unsigned long *)&_pagetab + vmpagenum(virtadr);
    while (n--) {
        pval = *pte;
        if (!pval) {

            return FALSE;
        }
        pte++;
    }

    return TRUE;
}

BOOLEAN
AcpiOsWritable(void *virtadr, ACPI_SIZE len)
{
    unsigned long *pte;
    unsigned long  pval;
    long           n;

    n = rounduppow2(len, PAGESIZE) >> PAGESIZELOG2;
    pte = (unsigned long *)&_pagetab + vmpagenum(virtadr);
    while (n--) {
        pval = *pte;
        if (!(pval & PAGEWRITE)) {

            return FALSE;
        }
        pte++;
    }

    return TRUE;
}

#endif /* ACPICA */

