#ifndef __VT_VT_H__
#define __VT_VT_H__

#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>

#define VTBUFSIZE    PAGESIZE

#define VTDEFMODE    0x00000000U
#define VTDEFFGCOLOR 0xffffffffU
#define VTDEFBGCOLOR 0xff000000U
#define VTDEFTEXTATR 0x00000000U

/* Unicode plans not to add characters beyond 0x10ffff */
#define VTUCSMASK    0x001fffffU
#define VTATRMASK    0xffe00000U

/* character attribute bits */
#define VTREVERSE    0x80000000 // reverse color
#define VTUNDERLINE  0x40000000 // underlined text
#define VTBOLD       0x20000000 // bold text
#define VTBLINK      0x10000000 // blinking text
#define VTANTIALIAS  0x08000000 // antialiased text (interpolation)
#define VTDRAWBG     0x04000000 // draw text background
#define VTDRAWFG     0x02000000 // draw text foreground
#define VTFGMASK     0x00000100 // standard or 256-color xterm palette entry
#define VTBGMASK     0x0003fe00 // standard or 256-color xterm palette entry

#define VTREND256COL 0x100
struct vtrend {
    unsigned fgcolor : 9;       // standard or 256-color xterm palette entry
    unsigned bgcolor : 9;       // standard or 256-color xterm palette entry
    unsigned pad     : 7;       // extra room for later flags
    unsigned atr     : 7;       // specified text attributes
} PACK();

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
#define VTCHARSET      '('
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

struct vt {
    struct ringbuf   inbuf;             // input ring-buffer
    struct ringbuf   outbuf;            // output ring-buffer
    int              fd;                // master PTY file descriptor
    char            *masterpath;        // master teletype device
    char            *slavepath;         // slave teletype device
    uint64_t         mode;              // private mode etc. mask
    uint64_t         state;             // modifier and button mask
    uint32_t         fgcolor;           // foreground text color
    uint32_t         bgcolor;           // background text color
    uint32_t         textatr;           // current text attributes
    int32_t        **textbuf;           // Unicode text data
    struct vtrend  **rendbuf;           // rendition attribute data
};

typedef long vtescfunc_t(struct vt *vt, long num1, long num2);
struct vtesc {
    uint8_t      esccmdmap[32];
    uint8_t      csicmdmap[32];
    uint8_t      hashcmdmap[32];
    vtescfunc_t *escfunctab[256];
    vtescfunc_t *csifunctab[256];
    vtescfunc_t *hashfunctab[256];
};

#endif /* __VT_VT_H__ */

