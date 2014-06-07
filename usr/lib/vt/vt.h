#ifndef __VT_VT_H__
#define __VT_VT_H__

#define VTDEFMODE    0x00000000U
#define VTDEFFGCOLOR 0xffffffffU
#define VTDEFBGCOLOR 0xff000000U
#define VTDEFTEXTATR 0x00000000U

/* Unicode plans not to add characters beyond 0x10ffff */
#define VTUCSMASK    0x001fffffU
#define VTATRMASK    0xffe00000U

/* character attribute bits */
#define VTREVERSE    0x00200000
#define VTUNDERLINE  0x00400000
#define VTBOLD       0x00800000
#define VTBLINK      0x01000000

/* Unicode character plus attributes such as underline */
//typedef int32_t vtchar_t;
/* character and its 32-bit ARGB-color */
//typedef int64_t vttext_t;

#define RING_ITEM    int32_t
#define RING_INVAL   0x00000000
#include <zero/ring.h>

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
#define VTCLRTLLINEEND 'K'
#define VTCURSORADR    'Y'

struct vt {
    struct ringbuf  inbuf;      // input ring-buffer
    struct ringbuf  outbuf;     // output ring-buffer
    int             fd;         // master PTY file descriptor
    char           *masterpath; // master teletype device
    char           *slavepath;  // slave teletype device
    uint64_t        mode;       // private mode etc. mask
    uint64_t        state;      // modifier and button mask
    uint32_t        fgcolor;    // foreground text color
    uint32_t        bgcolor;    // background text color
    uint32_t        textatr;    // current text attributes
};

#endif /* __VT_VT_H__ */

