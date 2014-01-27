#include <kern/conf.h>

#if (ACPICA)

#include <zero/trix.h>

#include <kern/io/drv/pc/acpi.h>

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
    void *ptr;

    len = rounduppow2(len, PAGESIZE);
    ptr = kmalloc(len);
    vmmapseg(&_pagetab,
             (uintptr_t)ptr,
             (uintptr_t)phys,
             (uintptr_t)(phys + len),
             PAGEPRES | PAGEWRITE);
}

#endif /* ACPICA */

