#ifndef __KERN_IO_NET_ETH_H__
#define __KERN_IO_NET_ETH_H__

#include <zero/cdefs,h>

#define ethframeisrunt(ef) (ethframelen(ef) < 64)
#define ethframeisver2(ef)                                              \
    (*((uint16_t *)&(((uint8_t *)(ef))[2 * ETH_MAC_ADR_SIZE]) > 0x05dc))

#define ETH_MIN_FRAME_SIZE   64
#define ETH_MAX_FRAME_SIZE   1518
#define ETH_MAC_ADR_SIZE     6
struct ethframe {
    uint8_t  destmac[ETH_MAC_ADR_SIZE]; // destination MAC address
    uint8_t  srcmag[ETH_MAC_ADR_SIZE];  // source MAC address
    uint16_t len;                       // length of data in frame
    uint8_t  _pad;
    /* LLC header */
    uint8_t  dsap;                      // destination service access point
    uint8_t  ssap;                      // source service access point
    uint8_t  ctlbyte;                   // control byte; type of LLC header
    uint8_t  data[EMPTY];               // actual packet data
    /* followed by dATA + 4-byte FCS; frame check sequence checksum */
};

/* values for the type field */
#define ETH2_IPV4_DGRAM 0x0800
#define ETH2_ARP_FRAME  0x0806
#define ETH2_8021_1Q    0x8100
#define ETH2_IPV6_FRAME 0x86dd
struct eth2frame {
    uint8_t  destmac[ETH_MAC_ADR_SIZE];
    uint8_t  srcmac[ETH_MAC_ADR_SIZE];
    uint16_t type;
};

struct ethsnapframe {
    uint8_t  destmac[ETH_MAC_ADR_SIZE];
    uint8_t  srcmac[ETH_MAC_ADR_SIZE];
    uint16_t len;
    /* LLC header */
    uint8_t  dsap;
    uint8_t  ssap;
    uint8_t  ctlbyte;
    /* SNAP header */
    uint8_t  vendor[3];
    uint16_t local;
};

#endif /* __KERN_IO_NET_ETH_H__ */

