#include <stdint.h>
#include <sys/io.h>

//#define LIST_TYPE  struct pcidev
//#define LISTQ_TYPE struct pcidevlist
//#include <zero/list.h>

#include <kern/conf.h>
#include <kern/util.h>
#include <kern/io/drv/pc/pci.h>

struct pcidev     pcidevtab[PCINDEV];
//struct pcidevlist pcidevlist;
long              pcifound;
long              pcindev;

long
pciprobe(void)
{
    outl(PCICONFBIT, PCICONFADR);
    if (inl(PCICONFADR) != PCICONFBIT) {
        kprintf("PCI controller not found\n");

        return 0;
    }
    kprintf("PCI controller found\n");

    return 1;
}

uint8_t
pcireadconfb(uint8_t busid, uint8_t slotid, uint8_t funcid, uint8_t ofs)
{
    uint32_t conf;
    uint32_t bus = busid;
    uint32_t slot = slotid;
    uint32_t func = funcid;
    uint16_t byte = 0;

    conf = (bus << 16) | (slot << 11) | (func << 8) | ofs | PCICONFBIT;
    outl(conf, PCICONFADR);
    byte = inb(PCICONFADR + (ofs & 0x03));

    return byte;
}

uint16_t
pcireadconfw(uint8_t busid, uint8_t slotid, uint8_t funcid, uint8_t ofs)
{
    uint32_t conf;
    uint32_t bus = busid;
    uint32_t slot = slotid;
    uint32_t func = funcid;
    uint16_t word = 0;

    conf = (bus << 16) | (slot << 11) | (func << 8) | ofs | PCICONFBIT;
    outl(conf, PCICONFADR);
//    tmp = inw(PCICONFDATA) >> ((ofs & 0x02) << 3) & 0xffff);
    word = inw(PCICONFDATA + (ofs & 0x02));

    return word;
}

uint32_t
pcireadconfl(uint8_t busid, uint8_t slotid, uint8_t funcid, uint8_t ofs)
{
    uint32_t conf;
    uint32_t bus = busid;
    uint32_t slot = slotid;
    uint32_t func = funcid;
    uint32_t longword = 0;

    conf = (bus << 16) | (slot << 11) | (func << 8) | ofs | PCICONFBIT;
    outl(conf, PCICONFADR);
//    tmp = inw(PCICONFDATA) >> ((ofs & 0x02) << 3) & 0xffff);
    longword = inl(PCICONFDATA);

    return longword;
}

#if 0
void
pciwriteconfb(uint8_t busid, uint8_t slotid, uint8_t funcid, uint8_t ofs,
                  uint8_t val)
{
    uint32_t conf;
    uint32_t bus = busid;
    uint32_t slot = slotid;
    uint32_t func = funcid;
    uint16_t byte = 0;

    conf = (bus << 16) | (slot << 11) | (func << 8) | ofs | PCICONFBIT;
    outb(conf, PCICONFADR);

    return byte;
}

void
pciwriteconfw(uint8_t busid, uint8_t slotid, uint8_t funcid, uint8_t ofs)
{
    uint32_t conf;
    uint32_t bus = busid;
    uint32_t slot = slotid;
    uint32_t func = funcid;
    uint16_t word = 0;

    conf = (bus << 16) | (slot << 11) | (func << 8) | ofs | PCICONFBIT;
    outl(conf, PCICONFADR);
//    tmp = inw(PCICONFDATA) >> ((ofs & 0x02) << 3) & 0xffff);
    word = inw(PCICONFDATA + (ofs & 0x02));

    return word;
}

uint16_t
pcireadconfl(uint8_t busid, uint8_t slotid, uint8_t funcid, uint8_t ofs)
{
    uint32_t conf;
    uint32_t bus = busid;
    uint32_t slot = slotid;
    uint32_t func = funcid;
    uint16_t longword = 0;

    conf = (bus << 16) | (slot << 11) | (func << 8) | ofs | PCICONFBIT;
    outl(conf, PCICONFADR);
//    tmp = inw(PCICONFDATA) >> ((ofs & 0x02) << 3) & 0xffff);
    longword = inl(PCICONFDATA);

    return longword;
}
#endif

int
pcireadconf1(uint8_t busid, uint8_t slotid, uint8_t funcid,
             uint16_t regid, uint8_t len)
{
    int retval;

    outl(pciconfadr1(busid, slotid, funcid, regid), PCICONFADR);
    switch (len) {
        case 4:
            retval = inl(PCICONFADR);

            break;
        case 2:
            retval = inw(PCICONFADR + (regid & 0x02));

            break;
        case 1:
            retval = inb(PCICONFADR + (regid & 0x03));

            break;
    }

    return retval;
}

void
pciwriteconf1(uint8_t busid, uint8_t slotid, uint8_t funcid,
              uint16_t regid, uint8_t len, uint32_t val)
{
    outl(pciconfadr1(busid, slotid, funcid, regid), PCICONFADR);
    switch (len) {
        case 4:
            outl(val, PCICONFADR);

            break;
        case 2:
            outw((uint16_t)val, PCICONFADR + (regid & 0x02));

            break;
        case 1:
            outb((uint8_t)val, PCICONFADR + (regid & 0x03));

            break;
    }

    return;
}

int
pcireadconf2(uint8_t busid, uint8_t slotid, uint8_t funcid,
             uint16_t regid, uint8_t len)
{
    int retval = ~0;

    if (!(slotid & 0x10)) {
        outb((uint8_t)(0xf0 | (funcid << 1)), PCICONFADR);
        outb(busid, 0xcfa);
        switch (len) {
            case 4:
                retval = inl(pciconfadr2(slotid, regid));

                break;
            case 2:
                retval = inw(pciconfadr2(slotid, regid));

                break;
            case 1:
                retval = inb(pciconfadr2(slotid, regid));

                break;
        }
        outb(0, PCICONFADR);
    }

    return retval;
}

void
pciwriteconf2(uint8_t busid, uint8_t slotid, uint8_t funcid,
              uint16_t regid, uint8_t len, uint8_t val)
{
    if (!(slotid & 0x10)) {
        outb((uint8_t)(0xf0 | (funcid << 1)), PCICONFADR);
        outb(busid, 0xcfa);
        switch (len) {
            case 4:
                outl(val, pciconfadr2(slotid, regid));
                
                break;
            case 2:
                outw(val, pciconfadr2(slotid, regid));

                break;
            case 1:
                outb(val, pciconfadr2(slotid, regid));

                break;
        }
        outb(0, PCICONFADR);
    }

    return;
}

uint16_t
pcichkvendor(uint8_t busid, uint8_t slotid, uint16_t *devret)
{
    uint16_t vendor;
    uint16_t dev;

    vendor = pcireadconfw(busid, slotid, 0, 0);
    if (vendor != 0xffff && (devret)) {
        dev = pcireadconfw(busid, slotid, 0, 2);
        *devret = dev;
    }

    return vendor;
}

#if 0
void
pcichkdev(uint8_t busid, uint8_t devid, uint8_t func)
{
    uint16_t vendor;
}
#endif

void
pciinit(void)
{
    struct pcidev *dev;
    uint16_t       vendor;
    uint16_t       devid;
    long           bus;
    long           slot;
    long           ndev;

    pcifound = pciprobe();
    if (pcifound) {
        ndev = 0;
        for (bus = 0 ; bus < 256 ; bus++) {
            for (slot = 0 ; slot < 32 ; slot++) {
                vendor = pcichkvendor(bus, slot, &devid);
                if (vendor != 0xffff) {
                    if (ndev < PCINDEV) {
                        pcidevtab[ndev].vendor = vendor;
                        pcidevtab[ndev].id = devid;
                        pcidevtab[ndev].bus = bus;
                        pcidevtab[ndev].slot = slot;
//                        listqueue(&pcidevlist, &pcidevtab[ndev]);
                        ndev++;
                    }
                }
            }
        }
    }
    pcindev = ndev;
    if (ndev) {
        dev = &pcidevtab[0];
        while (ndev--) {
            kprintf("PCI: bus: %x, slot: %x, vendor: 0x%x, device: 0x%x\n",
                    dev->bus, dev->slot, dev->vendor, dev->id);
            dev++;
        }
    }

    return;
}

