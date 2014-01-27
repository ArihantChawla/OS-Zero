#include <stdint.h>
#include <sys/io.h>

#include <kern/io/drv/pc/pci.h>

uint16_t
pcireadconfw(uint8_t busid, uint8_t slotid, uint8_t funcid, uint8_t ofs)
{
    uint32_t adr;
    uint32_t bus = busid;
    uint32_t slot = slotid;
    uint32_t func = funcid;
    uint16_t tmp = 0;

    adr = (bus << 16)
        | (slot << 11)
        | (func << 8)
        | (ofs & 0xfc)
        | (0x80000000U);
    outl(adr, PCICONFADR);
    tmp = (uint16_t)(inl(PCICONFDATA) >> ((ofs & 0x02) << 3) & 0xffff);

    return tmp;
}

uint16_t
pcichkvendor(uint8_t bus, uint8_t slot, uint16_t *devret)
{
    uint16_t vendor;
    uint16_t dev;

    vendor = pcireadconfw(bus, slot, 0, 0);
    if (vendor != 0xffff && (devret)) {
        dev = pcireadconfw(bus, slot, 0, 2);
        *devret = dev;
    }

    return vendor;
}

void
pcichkdev(uint8_t busid, uint8_t devid)
{
    uint16_t vendor;
    uint8_t  func;

    func = 0;
}

