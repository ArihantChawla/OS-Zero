#include <kern/conf.h>

#if (ATA)

#include <stdint.h>
#include <sys/io.h>

#include <kern/util.h>
#include <kern/io/drv/pc/ata.h>
#include <kern/unit/x86/apic.h>

static uint16_t ataconftab[4][256];

#if 0
static const char *ataerrtab[6]
= {
    "unknown error",
    "OK",
    "formatter device error",
    "sector buffer error",
    "ECC circuitry error",
    "controller microprocessor error"
};
#endif

#define ATAIDATAPIBYTE1  0x14
#define ATAIDATAPIBYTE2  0xeb
#define ATAIDSATABYTE1   0x3c
#define ATAIDSATABYTE2   0xc3
#define ATAIDSATAPIBYTE1 0x69
#define ATAIDSATAPIBYTE2 0x96

/* use id ATASELMASTER or ATASELSLAVE */
long
ataprobedrv(uint16_t iobase, uint8_t id)
{
    uint16_t *confptr;
    long      ndx = 0;
    uint8_t   retval = 0;
    uint8_t   byte1;
    uint8_t   byte2;

    /* use ATA IDENTIFY command */
    outb(id, iobase + 6);               // select master or slave
    outb(0, iobase + 2);                // set sector count to zero
    outb(0, iobase + 3);                // set LBAlo to zero
    outb(0, iobase + 4);                // set LBAmid to zero
    outb(0, iobase + 5);                // set LBAhigh to zero
    outb(ATAIDENTIFY, iobase + 7);      // send IDENTIFY to command port
    /* check for ATAPI */
    byte1 = inb(iobase + 4);
    byte2 = inb(iobase + 5);
    if (byte1 == ATAIDATAPIBYTE1 && byte2 == ATAIDATAPIBYTE2) {
        /* ATAPI */
        
        retval = 2;
    } else if (byte1 == ATAIDSATABYTE1 && byte2 == ATAIDSATABYTE2) {
        /* SATA */

        retval = 3;
    } else if (byte1 == ATAIDSATAPIBYTE1 && byte2 == ATAIDSATAPIBYTE2) {
        /* SATAPI */

        retval = 4;
    }
    if (!retval) {
        do {
            byte1 = inb(iobase + 7);        // read status port
            if (!byte1) {
                
                return 0;
            } else if (byte1 & ATABSY) {
                
                continue;
            } else {
                byte1 = inb(iobase + 4);    // LBAmid
                byte2 = inb(iobase + 5);    // LBAhi
                if ((byte1) || (byte2)) {
                    
                    return 0;
                } else {
                    while (!retval) {
                        byte1 = inb(iobase + 7);
                        byte2 = byte1 & (ATADRQ | ATAERR);
                        if (!byte2) {
                            
                            continue;
                        } else if (byte2 & ATAERR) {
                            
                            return 0;
                        } else {
                            
                            retval = 1;
                        }
                    }
                }
            }
        } while (!retval);
    }
    if (retval) {
        if (iobase == ATAPRIMARY) {
            ndx = 0;
            if (id == ATASELSLAVE) {
                ndx++;
            }
        } else if (iobase == ATASECONDARY) {
            ndx = 2;
            if (id == ATASELSLAVE) {
                ndx++;
            }
        }
        confptr = &ataconftab[ndx][0];
        ndx = 256;
        while (ndx--) {
            confptr[ndx] = inw(iobase);
        }
    }

    return retval;
}

void
ataprobedrv2(uint16_t iobase)
{
    unsigned long timeout;
    uint16_t      port;
    uint8_t       byte1;
    uint8_t       byte2;

    /* debounce floating bus value for 20 milliseconds */
    kprintf("ATA: probing interface 0x%x\n", iobase);
    timeout = 20;
    port = iobase + 7;
    while (timeout) {
        byte1 = inb(port);
        if (byte1 != 0xff) {

            break;
        }
        usleep(1000);
        timeout--;
    }
    if (!timeout) {
        /* probe cylinder registers to detect drives */
        kprintf("ATA: probing cylinder registers\n");
        outb(0x55, iobase + 4);
        outb(0xaa, iobase + 5);
        byte1 = inb(iobase + 4);
        outb(0x55, 0x80);
        byte2 = inb(iobase + 5);
        if (byte1 != 0x55 || byte1 != 0xaa) {
            kprintf("ATA: no drive found on interface 0x%x\n", iobase);

            return;
        }
    }
    timeout = 5000;
    while (timeout) {
        /* probe for busy status */
        byte1 = inb(port);
        if (!(byte1 & ATABSY)) {

            break;
        }
        usleep(1000);
        timeout--;
    }
    if (!timeout) {
        kprintf("ATA: BSY != 0, no drive found on interface 0x%x\n", iobase);

        return;
    }
    /* execute drive diagnostics */
    outb(0x90, port);
    usleep(1);  // 400 nanoseconds */
    timeout = 5000;
    while (timeout) {
        byte1 = inb(port);
        if (!(byte1 & ATABSY)) {

            break;
        }
        usleep(1000);
        timeout--;
    }
    if (!timeout) {
        kprintf("ATA: diagnostics failed, no drive found on interface 0x%x\n",
                iobase);

        return;
    }
    byte1 = inb(iobase + 1);     // bits 0..6 are master diagnostic code
    byte2 = byte1 & 0x7f;       // bit 7 is 1 if slave error
#if 0
    if (byte2 >= ATANERR) {
        byte1 = 0;
    }
#endif
    kprintf("ATA: master on interface 0x%x: %x\n", iobase, byte2);
    if (!(byte1 & 0x80)) {
        kprintf("ATA: no slave drive on interface 0x%x\n", iobase);
    } else {
        outb(0xb0, iobase + 6);  // select slave
        usleep(1);              // 400 nanoseconds
        byte1 = inb(iobase + 1); // read slave diagnostic code
        byte2 = byte1 & 0x7f;
#if 0
        if (byte2 > ATANERR) {
            byte2 = 0;
        }
#endif
        kprintf("ATA: slave on interface 0x%x: %x\n", iobase, byte2);
    }

    return;
}

void
atainit(void)
{
    long status;
#if 0
    ataprobedrv2(0x1f0);
    ataprobedrv2(0x170);
#endif
    status = ataprobedrv(ATAPRIMARY, ATASELMASTER);
    kprintf("ATA: primary master 0x%lx\n", status);
    status = ataprobedrv(ATAPRIMARY, ATASELSLAVE);
    kprintf("ATA: primary slave 0x%lx\n", status);
    status = ataprobedrv(ATASECONDARY, ATASELMASTER);
    kprintf("ATA: secondary master 0x%lx\n", status);
    status = ataprobedrv(ATASECONDARY, ATASELSLAVE);
    kprintf("ATA: secondary slave 0x%lx\n", status);

    return;
}

#endif /* ATA */

