#ifndef __VT_VT_H__
#define __VT_VT_H__

#include <vt/conf.h>
#include <vt/color.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <vt/term.h>
#include <ui/ui.h>
#include <ui/text.h>

#define VTBUFSIZE     PAGESIZE

#define VTDEFMODE     0x00000000U
#define VTDEFFGCOLOR  0xffffffffU
#define VTDEFBGCOLOR  0xff000000U
#define VTDEFTEXTATR  0x00000000U

/* Unicode plans not to add characters beyond 0x10ffff */
#define VTUCSMASK     0x001fffffU
#define VTATRMASK     0xffe00000U

/* character attribute bits */
#define VTBRIGHT      0x80000000
#define VTDIM         0x40000000
#define VTUNDERSCORE  0x20000000
#define VTBLINK       0x10000000
#define VTREVERSE     0x08000000
#define VTHIDDEN      0x04000000
#define VTANTIALIAS   0x02000000 // antialiased text (interpolation)
#define VTDRAWBG      0x01000000 // draw text background
#define VTDRAWFG      0x00800000 // draw text foreground
#define VTXTERM256    0x00400000 // 256-color xterm mode
#define VTFGMASK      0x000000ff // standard or 256-color xterm palette entry
#define VTBGMASK      0x0000ff00 // standard or 256-color xterm palette entry
#define VTFG256BIT    0x00000100 // foreground is xterm color
#define VTBG256BIT    0x00020000 // foreground is xterm color

#define VTXTERM256BIT 0x100 // xterm 256-color palette entry

/* Unicode character plus attributes such as underline */
//typedef int32_t vtchar_t;
/* character and its 32-bit ARGB-color */
//typedef int64_t vttext_t;

#define RING_ITEM    int32_t
#define RING_INVAL   0x00000000
#include <zero/ring.h>

#define VTESC          '\033'
/* sequence postfixes after ESC */
#define VTCSI          '['
#define VTFONTG0       '('
#define VTFONTG1       ')'
#define VTHASH         '#'
/* sequences prefixed with ESC */
#define VTCURSORUP     'A'
#define VTCURSORDOWN   'B'
#define VTCURSORRIGHT  'C'
#define VTCURSORLEFT   'D'
#define VTSPECCHARSET  'F'
#define VTASCIICHARSET 'G'
#define VTCURSORHOME   'H'
#define VTREVLINEFEED  'I'
#define VTCLRTOSCREND  'J'
#define VTCLRTOLINEEND 'K'
#define VTCURSORADR    'Y'

struct vtatr {
    int   fd;           // master PTY file descriptor
    char *masterpath;   // master teletype device
    char *slavepath;    // slave teletype device
};

struct vtstate {
    long      col;              // current column
    long      row;              // current column
    long      nrow;             // # of screen rows
    long      ncol;             // # of screen columns
    long      w;                // width in characters
    long      h;                // height in characters
    uint64_t  mode;             // private mode etc. mask
    uint64_t  flags;            // modifier and button mask
    int32_t   fgcolor;          // foreground text color
    int32_t   bgcolor;          // background text color
    uint32_t  textatr;          // current text attributes
    uint32_t *tabmap;           // TAB-position bitmap
    long      scrolltop;        // scrolling region top row
    long      scrollbottom;     // scrolling region bottom row
};

struct vtdevbuf {
    struct ringbuf in;          // input ring-buffer
    struct ringbuf out;         // output ring-buffer
};

struct vtiobuf {
    long  nin;                  // # of items in input buffer
    void *inptr;                // current input pointer
    void *inbuf;                // input buffer
    long  nout;                 // # of items in output buffer
    void *outptr;               // current output pointer
    void *outbuf;               // output buffer
};

struct vtcolormap {
    void *deftab;               // default colors
    void *xtermtab;             // xterm colors
};

struct vtsavecurs {
    long row;
    long col;
};

typedef ssize_t __termiofunc(int fd, void *buf, size_t nbyte);
typedef int     __termioop(int fd, long cmd, void *buf);
/* values for the flg member */
#define VTBUFDIRTY (1 << 0)     // terminal buffer has not been synchronized
#define VTBUFREAD  (1 << 1)     // terminal buffer has pending read operations
#define VTBUFWRITE (1 << 2)     // terminal buffer has pending write operations
struct vtbuf {
    volatile long     lk;       // mutex
    long              flg;      // VTBUFDIRTY, VTBUFREAD, VTBUFWRITE
    struct termiobuf *rdqueue;  // read queue buffer structure
    struct termiobuf *wrqueue;  // write queue buffer structure
    __termiofunc     *chop;     // getch() (read()) or putch() (write())
    fd_set           *wrfdset;  // write fd_set for select()
    fd_set           *rdfdset;  // read fd_set for select()
    fd_set           *errfdset; // error fd_set for select()
    struct timeval    timeout;  // possible timeout for select()
};

struct vtscreen {
    struct vt     *vttab;       // basic virtual terminal attributes (NVTSCREEN)
    int           *fdtab;       // TERMNSCREENFDS file descriptors
    struct vtbuf   buf;         // virtual terminal buffer structure
    size_t         nrow;        // # of rows in rowtab
    size_t         nscrrow;     // # of rows in scrtab
    struct vtrow **rowtab;      // current I/O buffer
    struct vtrow **scrtab;      // current screen rendition
};

struct vt {
    struct vtbuf      *buf;
    char              *path;
#if (TERMXORG)
    int                connfd;          // X connection file descriptor
#endif
    struct vtatr       atr;             // terminal attributes
    struct vtstate     state;           // terminal status
#if defined(__KERNEL__) && (__KERNEL__)
    struct vtdevbuf    devbuf;          // input and output buffers
#else
    struct vtiobuf     iobuf;
#endif
    struct uitextbuf   textbuf;         // text and rendition buffers
    struct ui          ui;              // user interface
    struct uifont      font;            // default font
    struct vtcolormap  colormap;        // terminal colormaps
    struct vtesctabs  *esctabs;         // escape sequence interface
    struct vtsavecurs  savecurs;        // saved cursor and attributes
};

typedef void vtescfunc_t(struct vt *, long, long *);
struct vtesctabs {
    uint8_t      escmap[32];
    uint8_t      csimap[32];
    uint8_t      hashmap[32];
    vtescfunc_t *escfunctab[256];
    vtescfunc_t *csifunctab[256];
    vtescfunc_t *hashfunctab[256];
};

#define vtisesccmd(vt, c)       bitset((vt)->esctabs->escmap, c)
#define vtiscsicmd(vt, c)       bitset((vt)->esctabs->csimap, c)
#define vtishashcmd(vt, c)      bitset((vt)->esctabs->hashmap, c)
#define vtsetesccmd(vt, c)      setbit((vt)->esctabs->escmap, c)
#define vtsetcsicmd(vt, c)      setbit((vt)->esctabs->csimap, c)
#define vtsethashcmd(vt, c)     setbit((vt)->esctabs->hashmap, c)
#define vtsetescfunc(vt, c, f)                                          \
    (vtsetesccmd(vt, c), (vt)->esctabs->escfunctab[(c)] = (f))
#define vtsetcsifunc(vt, c, f)                                          \
    (vtsetcsicmd(vt, c), (vt)->esctabs->csifunctab[(c)] = (f))
#define vtsethashfunc(vt, c, f)                                         \
    (vtsethashcmd(vt, c), (vt)->esctabs->hashfunctab[(c)] = (f))

#define vtdefcolor(i)     (((int32_t *)((vt)->colormap.deftab))[(i)])
#define vtxtermcolor(i)   (((int32_t *)((vt)->colormap.xtermtab))[(i)])
#define vtfgtodefcolor(i) ((i) - 30)
#define vtbgtodefcolor(i) ((i) - 40)

extern struct vtesctabs  vtesctabs;
extern char             *vtkeystrtab[128];

struct vt * vtrun(struct vt *vt);
void        vtgetopt(struct vt *vt, int argc, char *argv[]);
struct vt * vtinit(struct vt *vt, int argc, char *argv[]);
void        vtinitesc(struct vt *vt);
void        vtfree(struct vt *vt);
long        vtinitbuf(struct vt *vt);
void        vtfreebuf(struct vt *vt);

#endif /* __VT_VT_H__ */

