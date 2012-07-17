#ifndef __ZERO_DECK_H__
#define __ZERO_DECK_H__

/* deck events */
#define ERROR            0
#define REPLY            1
#define EXPOSE           2
#define KEYPRESS         3
#define KEYRELEASE       4
#define BUTTONPRESS      5
#define BUTTONRELEASE    6
#define MOTION           7
#define ENTERWINDOW      8
#define LEAVEWINDOW      9
#define FOCUSIN          10
#define FOCUSOUT         11
#define MAPNOTIFY        12
#define UNMAPNOTIFY      13
#define DESTROYNOTIFY    14
#define VISIBILITYNOTIFY 15
#define PROPERTYATTACH   16
#define PROPERTYDETACH   17
#define PROPERTYCHANGE   18
#define PROPERTYQUERY    19
#define MESSAGE          20
#define DATAIN           21
#define DATAOUT          22
#define OPERATION        23

struct anyev {
    uint32_t type;
    uint32_t window;
};

struct exposeev {
    struct anyev hdr;           // common event members
    uint32_t     count;         // at least this many follow on the same window
};

/* KEYPRESS and KEYRELEASE event structure */
/* use sign bit to indicate keyreleases */

#define keyisrelease(kep)                                               \
    ((kep)->keysym & 0x80000000)
#define keygetsym(kep)                                                  \
    ((kep)->keysym & 0x7fffffff)
#define keybuttonstate(kep, num)                                        \
    ((kep)->butmodmask & (1L << num))
struct keyev {
    struct anyev hdr;           // common event members
    int32_t      keysym;        // Unicode/ISO-10646 character
    int32_t      butmodmask;    // state of buttons and modifier keys
};

/* BUTTONPRESS, BUTTONRELEASE and MOTION event structure */
struct ptrev {
    struct anyev hdr;           // common event members
    int32_t      button;        // button number
    int32_t      butmodmask;    // state of buttons and modifier keys
    int32_t      x;             // window X-coordinate
    int32_t      y;             // window Y-coordinate
    int32_t      z;             // [optional] window Z-coordinate; scroll wheel
};

/*
 * ENTERWINDOW, LEAVEWINDOW, FOCUSIN and FOCUSOUT, MAPNOTIFY, UNMAPNOTIFY and
 * DESTROYNOTIFY event structure
 */
struct crossev {
    struct anyev hdr;
};

#define UNMAPPED     0
#define VISIBLE      1
#define OBSCURED     2
#define PARTOBSCURED 3
struct visev {
    struct anyev hdr;
    uint32_t     state;
};

/*
 * PROPERTYATTACH, PROPERTYDETACH, PROPERTYCHANGE and PROPERTYQUERY
 * event structure
 */
struct propev {
    struct anyev hdr;
    uint32_t     atom;
    uint32_t     len;
    uint8_t      data[EMPTY];
};

/* MESSAGE event structure */
struct msgev {
    struct anyev hdr;
    uint32_t     fmt;
    uint32_t     enc;
    uint32_t     atom;
    uint32_t     len;
    uint8_t      data[EMPTY];
};

#define DATAASCII       0
#define DATAISO8859     1
#define DATAUTF8        2
#define DATAUCS2        3
#define DATAUCS4        4
#define DATABGRA32      5
#define DATABGR24       6
#define DATABGR555      7
#define DATABGR565      8
#define DATAVGA8        9
#define DATATIF         10
#define DATAPNG         11
#define DATAJPEG        12
#define DATAWAV44K1LE16 13
#define DATAWAV96KLE24  14
#define DATAWAV96KLE32  15
#define DATAWAV192KLE24 16
#define DATAWAV192KLE32 17
#define DATAMIDI        18
#define 
/* DATAIN and DATAOUT event structure */
struct dataev {
    struct anyev hdr;
    uint32_t     fmt;
    uint32_t     enc;
    uint32_t     len;
    uint8_t      data[EMPTY];
};

/* OPERATION event */

/* srv (service) member values */
#define DECKSRV    0
#define AUDSRV     1
#define VIDSRV     2
/* op (operation) member values */
#define OPDRAW     0
#define OPRPCQUERY 1
#define OPRPCOPEN  2
#define OPRPCCMD   3
/* OPDRAW and OPRPC event structure */
struct opev {
    struct anyev hdr;           // common event members
    uint32_t     srv;           // service number; 0 for deck
    uint32_t     op;            // operation
    uint32_t     len;           // number of data bytes
    uint8_t      data[EMPTY];
};

/* OPDRAW data */
#define TEXT      0
#define IMAGE     1
#define STREAM    2
#define PIXEL     3
#define TRIANGLE  4
#define RECTANGLE 5
#define CIRCLE    6
#define ELLIPSE   7
#define POINT     8
#define BEZIER    9
struct opdraw {
    struct anyev hdr;
    uint32_t     obj;
    uint32_t     gc;
    uint32_t     x;
    uint32_t     y;
    uint32_t     z;
};

struct gc {
    uint32_t font;
    uint32_t func;
    uint32_t mask;
    uint32_t fgpix;
    uint32_t bgpix;
};

#endif /* __ZERO_DECK_H__ */

