#ifndef __KERN_UNIT_X86_PIT_H__
#define __KERN_UNIT_X86_PIT_H__

extern void irqtmr0(void);

void pitinit(void);
void pitsleep(long msec);

/* support package for the 8253 timer chip */

#define PITHZ       (1193182L / HZ)
#define PITCMD      (PITDUALBYTE | PITWAVEGEN)

/* only use channel 0 */
#define PITCHAN0    0x40 /* data port */
#define PITCTRL     0x43 /* mode/command register */
/* modes/commands */
#define PITTERMCNT  0x00 /* interrupt on terminal count */
#define PITONESHOT  0x02 /* hardware re-triggerable one shot */
#define PITRATEGEN  0x04 /* rate generator */
#define PITWAVEGEN  0x06 /* square wave generator */
#define PITSOFTSTB  0x08 /* software triggered strobe */
#define PITHARDSTB  0x0a /* hardware triggered strobe */
#define PITLOBYTE   0x10
#define PITHIBYTE   0x20
#define PITDUALBYTE 0x30
#define PITREADBACK 0xc0

#define pitsethz(hz)                                                    \
    do {                                                                \
        long _hz = 1193182L / (hz);                                     \
                                                                        \
        outb(_hz & 0xff, PITCHAN0);                                     \
        outb(_hz >> 8, PITCHAN0);                                       \
    } while (0)

#endif /* __KERN_UNIT_X86_PIT_H__ */

