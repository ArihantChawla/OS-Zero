#ifndef __ZERO_DECK_H__
#define __ZERO_DECK_H__

/* deck events */
#define DECK_EVENT_ERROR            0
#define DECK_EVENT_REPLY            1
#define DECK_EVENT_EXPOSE           2
#define DECK_EVENT_KEYPRESS         3
#define DECK_EVENT_KEYRELEASE       4
#define DECK_EVENT_BUTTONPRESS      5
#define DECK_EVENT_BUTTONRELEASE    6
#define DECK_EVENT_MOTION           7
#define DECK_EVENT_ENTERWINDOW      8
#define DECK_EVENT_LEAVEWINDOW      9
#define DECK_EVENT_FOCUSIN          10
#define DECK_EVENT_FOCUSOUT         11
#define DECK_EVENT_MAPNOTIFY        12
#define DECK_EVENT_UNMAPNOTIFY      13
#define DECK_EVENT_DESTROYNOTIFY    14
#define DECK_EVENT_VISIBILITYNOTIFY 15
#define DECK_EVENT_PROPERTYATTACH   16
#define DECK_EVENT_PROPERTYDETACH   17
#define DECK_EVENT_PROPERTYCHANGE   18
#define DECK_EVENT_PROPERTYQUERY    19
#define DECK_EVENT_MESSAGE          20
#define DECK_EVENT_DATAIN           21
#define DECK_EVENT_DATAOUT          22
#define DECK_EVENT_OPERATION        23

struct deckfb {
    uint32_t  depth;
    uint32_t  width;
    uint32_t  height;
    void     *base;         // framebuffer base address
    void     *drawbuf;      // double-buffer for drawing
    void     *scrbuf;       // screen buffer
};

#define DECK_GFX_LFB 0
/* structure for functionality similar to X and such desktop/screen servers */
struct deck {
    /* screen ID (similar to Display */
    uint32_t  scrid;    // screen structure kernel address
    long      gfxtype;  // graphics interface such as DECK_GFX_LFB
    void     *gfx;      // graphics interface
};

struct deckanyev {
    uint32_t type;
    uint32_t window;
};

struct deckexposeev {
    struct deckanyev hdr;       // common event members
    uint32_t         count;     // at least this many follow on the same window
};

/* KEYPRESS and KEYRELEASE event structure */
/* use sign bit to indicate keyreleases */

#define deckkeyisrelease(kep)                                           \
    ((kep)->keysym & 0x80000000)
#define deckkeygetsym(kep)                                              \
    ((kep)->keysym & 0x7fffffff)
#define deckkeybuttonstate(kep, num)                                    \
    ((kep)->butmodmask & (1L << num))
struct deckkeyev {
    struct deckanyev hdr;               // common event members
    int32_t          keysym;            // Unicode/ISO-10646 character
    int32_t          butmodmask;        // state of buttons and modifier keys
};

/* BUTTONPRESS, BUTTONRELEASE and MOTION event structure */
struct deckptrev {
    struct deckanyev hdr;               // common event members
    int32_t          button;            // button number
    int32_t          butmodmask;        // state of buttons and modifier keys
    int32_t          x;                 // window X-coordinate
    int32_t          y;                // window Y-coordinate
    int32_t          z;                // [optional] window Z-coordinate; scroll wheel
};

/*
 * ENTERWINDOW, LEAVEWINDOW, FOCUSIN and FOCUSOUT, MAPNOTIFY, UNMAPNOTIFY and
 * DESTROYNOTIFY event structure
 */
struct deckcrossev {
    struct deckanyev hdr;
};

#define DECK_VISIBILITY_UNMAPPED      0
#define DECK_VISIBILITY_VISIBLE       1
#define DECK_VISIBILITY_OBSCURED      2
#define DECK_VISIBILITY_PART_OBSCURED 3
struct deckvisev {
    struct deckanyev hdr;
    uint32_t         state;
};

/*
 * PROPERTYATTACH, PROPERTYDETACH, PROPERTYCHANGE and PROPERTYQUERY
 * event structure
 */
struct deckpropev {
    struct deckanyev hdr;
    uint32_t         atom;
    uint32_t         len;
    uint8_t          data[EMPTY];
};

/* MESSAGE event structure */
struct deckmsgev {
    struct deckanyev hdr;
    uint32_t         fmt;
    uint32_t         enc;
    uint32_t         atom;
    uint32_t         len;
    uint8_t          data[EMPTY];
};

#define DECK_DATA_ASCII       0
#define DECK_DATA_ISO8859     1
#define DECK_DATA_UTF8        2
#define DECK_DATA_UCS2        3
#define DECK_DATA_UCS4        4
#define DECK_DATA_BGRA32      5
#define DECK_DATA_BGR24       6
#define DECK_DATA_BGR555      7
#define DECK_DATA_BGR565      8
#define DECK_DATA_VGA8        9
#define DECK_DATA_TIF         10
#define DECK_DATA_PNG         11
#define DECK_DATA_JPEG        12
#define DECK_DATA_WAV44K1LE16 13
#define DECK_DATA_WAV96KLE24  14
#define DECK_DATA_WAV96KLE32  15
#define DECK_DATA_WAV192KLE24 16
#define DECK_DATA_WAV192KLE32 17
#define DECK_DATA_MIDI        18

/* DECK_DATA_IN and DECK_DATA_OUT event structure */
struct deckdataev {
    struct deckanyev hdr;
    uint32_t         fmt;
    uint32_t         enc;
    uint32_t         len;
    uint8_t          data[EMPTY];
};

/* OPERATION event */

/* srv (service) member values */
#define DECK_SRV    0
#define DECK_AUDSRV 1
#define DECK_VIDSRV 2

/* op (operation) member values */
#define DECK_OP_DRAW     0
#define DECK_OP_RPCQUERY 1
#define DECK_OP_RPCOPEN  2
#define DECK_OP_RPCCMD   3
/* OPDRAW and OPRPC event structure */
struct deckopev {
    struct deckanyev hdr;       // common event members
    uint32_t         srv;       // service number
    uint32_t         op;        // operation
    uint32_t         len;       // number of data bytes
    uint8_t          data[EMPTY];
};

/* OPDRAW data */
#define DECK_OBJ_TEXT      0
#define DECK_OBJ_IMAGE     1
#define DECK_OBJ_STREAM    2
#define DECK_OBJ_PIXEL     3
#define DECK_OBJ_TRIANGLE  4
#define DECK_OBJ_RECTANGLE 5
#define DECK_OBJ_CIRCLE    6
#define DECK_OBJ_ELLIPSE   7
#define DECK_OBJ_POINT     8
#define DECK_OBJ_BEZIER    9
struct deckopdraw {
    struct deckanyev hdr;
    uint32_t         obj;
    uint32_t         gc;
    uint32_t         x;
    uint32_t         y;
    uint32_t         z;
};

struct deckgc {
    uint32_t font;
    uint32_t func;
    uint32_t mask;
    uint32_t fgpix;
    uint32_t bgpix;
};

#endif /* __ZERO_DECK_H__ */

