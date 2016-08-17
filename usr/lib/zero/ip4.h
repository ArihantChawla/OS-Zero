#ifndef __ZERO_IP4_H__
#define __ZERO_IP4_H__

/* Internet Protocol Version 4 support package for libzero */

#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/pack.h>  // pack ALL structs */

#define IP4_DGRAM_VERSION      4
#define ip4dgramhasopts(dg)    ((dg)->ihlen > IP4_DGRAM_MIN_HDR_SIZE / 4)
/* IP4 DATAGRAM HEADER */
#define IP4_DGRAM_MIN_HDR_SIZE 20
/* flg-bits */
#define IP4_DGRAM_DONT_FRAG    0x02 // request not to fragment
#define IP4_DGRAM_MORE_FRAGS   0x01 // all fragments but last set this to 1
/* protocol-field values */
#define IP4_DGRAM_ICMP         0x01 // ICMP datagram
#define IP4_DGRAM_IGMP         0x02 // IGMP datagram
#define IP4_DGRAM_GGP          0x03 // GGP datagram
#define IP4_DGRAM_IP_IN_IP     0x04 // IP-in-IP encapsulation
#define IP4_DGRAM_TCP          0x06 // TCP datagram
#define IP4_DGRAM_EGP          0x08 // EGP datagram
#define IP4_DGRAM_IGRP         0x09 // IGRP datagram
#define IP4_DGRAM_UDP          0x11 // UDP datagram
#define IP4_DGRAM_GRE          0x2f
#define IP4_DGRAM_ESP_EXT      0x32 // encapsulating security payload header
#define IP4_DGRAM_AUTH_EXT     0x33 // authentication header
#define IP4_DGRAM_SKIP         0x39
#define IP4_DGRAM_EIGRP        0x58
#define IP4_DGRAG_OSPF         0x59
#define IP4_DGRAM_L2TP         0x73
struct ip4dgram {
    unsigned version   : 4;     // 4 for IPv4
    unsigned ihlen     : 4;     // internet header length in 32-bit words
    uint8_t  tos;               // type of service
    uint16_t dglen;                // total length
    uint16_t ident;             // used for packet reassembly
    unsigned flg       : 2;     // fragmentation control-flags
    unsigned _reserved : 1;     // reserved bit
    unsigned fragofs   : 13;    // fragment offset in message, 64-bit units
    uint8_t  ttl;               // time to live
    uint8_t  protocol;          // see above
    uint16_t hdrchksum;         // 16-bit header checksum
    uint32_t srcadr;            // source address
    uint32_t destadr;           // destination address
    /* followed by variale options, padding, and data fields */
};

/* NOTE: TCP-header is padded to a multiple of 32 bits with zero-bits */
#define ip4padtcphdr(hdr, nb)                                           \
    do {                                                                \
        uint8_t *_u8ptr = (uint8_t *)(hdr) + (nb);                      \
        long     _ofs = sizeof(uint32_t) - ((nb) & (sizeof(uint32_t) - 1)); \
                                                                        \
        swittch (_ofs) {                                                \
            case 3:                                                     \
                _u8ptr[2] = 0x00;                                       \
            case 2:                                                     \
                _u8ptr[1] = 0x00;                                       \
            case 1:                                                     \
                _u8ptr[0] = 0x00;                                       \
            case 0:
            default:                                                    \
                                                                        \
                break;                                                  \
        }                                                               \
    } while (0)

#define TCP4_OPT_LIST_END       0x00 // kind == 0
#define TCP4_OPT_NO_OP          0x01 // kind == 1
#define TCP4_MAX_SEG_SZZE       0x02 // length 4 octets
struct tcp4maxsegsize {
    uin8t_t  kind;              // kind == 0x02
    uint8_t  len;               // lengt == 0x04 32-bit words
    uint16_t val;
}
/* bits for the ctlbits-field */
#define TCP4_CWR_BIT 0x80       // congestion window reduced
#define TCP4_ECE_BIT 0x40       // ECN echo (ECE)
#define TCP4_URG_BIT 0x20       // urgent pointer field significant
#define TCP4_ACK_BIT 0x10       // acknowledgeme nt fiedl significant
#define TCP4_PSH_BIT 0x08       // push function
#define TCP4_RST_BIT 0x04       // reset the connection
#define TCP4_SYN_BIT 0x02       // syrnchronize sequence numbers
#define TCP4_FIN_BIT 0x01       // no more data from sender
struct tcp4hdr {
    uint16_t srcport;           // source port
    uint16_t destport;          // destination port
    uint32_t seqnum;            // sequence number
    uint32_t acknum;            // acknowledgement number
    unsigned dataofs   : 4;     // TCP header size in 32-bit words
    unsigned _reserved : 6;     // 6-bit reserved field
    unsigned ctlbits   : 6;     // control bits
    uint16_t window;            // # of octets the sender is willing to accept
    uint16_t chksum;            // checksum
    uint16_t urgptr;            // urgent pointer
    /* followed by variable options and data */
};

#endif /* __ZERO_IP4_H__ */

