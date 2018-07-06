#ifndef __KERN_UNIT_X86_IOAPIC_H__
#define __KERN_UNIT_X86_IOAPIC_H__

#include <stdint.h>
#include <mach/param.h>
#include <zero/trix.h>

void ioapicinit(long unit);

#define IOAPICID  0x00
#define IOAPICVER 0x01
#define IOAPICTAB 0x10

#define IOAPICDISABLED 0x00010000
#define IOAPICLEVEL    0x00008000
#define IOAPICACTIVELO 0x00002000
#define IOAPICLOGICAL  0x00000800

#define __STRUCT_IOAPIC_SIZE                                            \
    (5 * sizeof(int32_t))
#define __STRUCT_IOAPIC_PAD                                             \
    (rounduppow2(__STRUCT_IOAPIC_SIZE, CLSIZE) - __STRUCT_IOAPIC_SIZE)
struct ioapic {
    uint32_t reg;
    uint32_t _pad1[3];
    uint32_t data;
    uint8_t  _pad[__STRUCT_IOAPIC_PAD];
};

extern volatile struct ioapic *ioapic;

static __inline__ uint32_t
ioapicread(uint32_t reg)
{
    ioapic->reg = reg;

    return ioapic->data;
}

static __inline__ void
ioapicwrite(uint32_t val, uint32_t reg)
{
    ioapic->reg = reg;
    ioapic->data = val;
}

#endif /* __KERN_UNIT_X86_IOAPIC_H__ */

