#ifndef __KERN_IO_DRV_PC_VBE2_H__
#define __KERN_IO_DRV_PC_VBE2_H__

#include <kern/unit/ia32/boot.h>

#define VBEINFOADR       0x1000
#define VBEMODEADR       0x2000
#define VBESEGSIZE       (1L << VBESEGSIZELOG2)
#define VBESEGSIZELOG2   16
#define VBEMODEBIT       0x0100  // 1 if VESA-defined
#define VBELINFBBIT      0x4000  // 1 for linear/flat frame buffer
#define VBEPRESVMEMBIT   0x8000  // 1 - preserve display memory, 0 - clear
#define VBESAVEMODE      0x81ff  // preserve contents, give access to all memory

/* function 0x00 - return VBE controller information */

#define VBESUCCESS       0x4f00
#define VBEFAILURE       0x4f01

#if !defined(__ASSEMBLY__)

/* sig */
#define VBESIG           { 'V', 'E', 'S', 'A' }
#define VBE2SIG          { 'V', 'B', 'E', '2' }
/* ver */
#define vbemajorver(ip)                                                 \
    ((ip)->ver >> 8)
#define vbeminorver(ip)                                                 \
    ((ip)->ver & 0xff)
/* cap */
/* bits per primary color */
#define VBEDAC8BIT       0x00000001      // switchable to 8, otherwise fixed 6
#define VBENONVGABIT     0x00000002
#define VBEPROGBLANKBIT  0x00000004
/* mode list terminator */
#define VBEMODELSTEND    0xffff          // -1
/* VBE 2.0 */
/* oemrev */
#define vbemajorrev(ip)                                                 \
    ((ip)->oemrev >> 8)
#define vbeminorrev(ip)                                                 \
    ((ip)->oemrev & 0xff)
/* 256 bytes for VBE 1, 512 for VBE 2 */
struct vbeinfo {
    char      sig[4];           // VBE2SIG
    uint16_t  ver;              // VBE version
    uint16_t  oem[2];
    uint32_t  cap;              // controller capabilities
    uint16_t  modelst[2];       // video mode list
    uint16_t  nblk;             // memory size in 64 kb blocks
#if 0
    /* VBE 2.0 */
    int16_t   oemrev;           // VBE software revision
    char     *oemvendor;        // vendor name
    char     *oemprod;          // product name
    char     *oemprodrev;       // product revision
    uint8_t   res[222];         // reserved for VBE implementation
    /* scratch area */
    uint8_t   oemdata[256];     // OEM strings
#endif
};

/* function 0x01 - return VBE mode information */

/* modeatr */
#define VBEMODEAVAILBIT  0x0001
#define VBEMODETTYBIT    0x0004
#define VBEMODECOLORBIT  0x0008
#define VBEMODEGFXBIT    0x0010
#define VBEMODENONVGABIT 0x0020
#define VBEMODENOWINBIT  0x0040
#define VBEMODELINFBBIT  0x0080
/* win1atr & win2atr */
#define VBEWINRELOCBIT   0x01
#define VBEWINREADBIT    0x02
#define VBEWINWRITEBIT   0x04
/* memmodel */
#define VBETEXTMODE      0x00
#define VBECGAMODE       0x01
#define VBEHERCULESMODE  0x02
#define VBEPLANARMODE    0x03
#define VBEPACKEDMODE    0x04
#define VBE256COLORMODE  0x05
#define VBEDIRECTMODE    0x06
#define VBEYUVMODE       0x07
/* dircolatr */
#define VBEPROGCOLORRAMP 0x01
#define VBERESUSABLE     0x02
struct vbemode {
    /* mandatory information for all VBE revisions */
    uint16_t   modeatr;
    uint8_t    win1atr;
    uint8_t    win2atr;
    uint16_t   wingran;
    uint16_t   nkbwin;
    uint16_t   win1seg;
    uint16_t   win2seg;
//    void     (*winfunc)();
    uint16_t   winfunc[2];
    uint16_t   nbline;
    /* mandatory information for VBE 1.2 and above */
    uint16_t   xres;
    uint16_t   yres;
    uint8_t    charw;
    uint8_t    charh;
    uint8_t    nplane;
    uint8_t    npixbit;
    uint8_t    nbank;
    uint8_t    memmodel;
    uint8_t    nkbbank;
    uint8_t    nimgpage;
    uint8_t    res1;
    /* direct color fields */
    uint8_t    rmasksize;
    uint8_t    rpos;
    uint8_t    gmasksize;
    uint8_t    gpos;
    uint8_t    bmasksize;
    uint8_t    bpos;
    uint8_t    resmasksize;
    uint8_t    respos;
    uint8_t    dircolatr;
    /* mandatory information for VBE 2.0 and above */
    uint32_t   fbadr;
    uint32_t   offscrofs;
    uint16_t   nkboffscr;
//    uint8_t    res2[206];
};

/* VBE screen information */
struct vbescreen {
    void *fbuf;
    void *dbuf;
    long  w;
    long  h;
    long  nbpp;
    long  fmt;
};

#endif /* !defined(__ASSEMBLY__) */

#endif /* __KERN_IO_DRV_PC_VBE2_H__ */

