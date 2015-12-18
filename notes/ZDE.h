#ifndef __ZDE_OBJ_H__
#define __ZDE_OBJ_H__

#define IP6ADRSIZE    16
#define NETADRMAXSIZE IP6ADRSIZE

#include <stdint.h>

typedef int16_t ioport_t;
typedef int32_t zdeword_t;
typedef int64_t zdetime_t;

#define ZDE_NOBJTYPE_BITS 31
#define ZDE_NOBJID_BITS   31
/* predefined object types */
#define ZDE_ERROR_OBJ     0x00000000
#define ZDE_PACKET_OBJ    0x00000001
#define ZDE_EVENT_OBJ     0x00000002
/* object flags */
#define ZDE_REMOTE_OBJECT (1 << 0)
/* adrtype values */
#define ZDE_VIRT_ADR      0x00000000
#define ZDE_PHYS_ADR      0x00000001
#define ZDE_IOPORT        0x00000002
#define ZDE_IOPORTMAP     0x00000003
struct zdeobj {
    zdetime_t     tmstamp;
    zdeword_t     type;
    zdeword_t     id;
    zdeword_t     flg;
    zdeword_t     adrtype;
    union {
        void     *ptr;
        ioport_t  port;
        uint8_t   netadr[NETADRMAXSIZE];
    };
};

#endif /* __ZDE_OBJ_H__ */

