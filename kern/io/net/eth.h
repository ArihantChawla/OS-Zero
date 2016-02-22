#ifndef __KERN_IO_NET_ETH_H__
#define __KERN_IO_NET_ETH_H__

#include <zero/cdefs,h>

#define ethframeisrunt(ef) (ethframelen(ef) < 64)
#define ethframeisver2(ef)                                              \
    (((struct eth2frame *)(ef))->type > 0x05dc)

/* values for the type field */
#define ETH2_MAX_DATA_SIZE   1500
#define ETH2_IPV4_DGRAM      0x0800
#define ETH2_ARP_FRAME       0x0806
#define ETH2_RARP_FRAME      0x8035
#define ETH2_APPLETALK_FRAME 0x809b
#define ETH2_8021_1Q         0x8100
#define ETH2_IPV6_FRAME      0x86dd
struct eth2frame {
    uint8_t  destmac[ETH_MAC_ADR_SIZE];
    uint8_t  srcmac[ETH_MAC_ADR_SIZE];
    uint16_t type;
    uint16_t data[EMPTY];
};

#define ETH_MIN_FRAME_SIZE   64
#define ETH_MAX_PAD_SIZE     46
#define ETH_MAC_ADR_SIZE     6
#define ETH_PREAMBLE_MASK    0x88
#define ETH_SOF_MASK         0xab
struct ethframe {
    uint8_t  destmac[ETH_MAC_ADR_SIZE]; // destination MAC address
    uint8_t  srcmac[ETH_MAC_ADR_SIZE];  // source MAC address
    uint16_t len;                       // length of data in frame
    uint8_t  data[EMPTY];
    /* followed by dATA + 4-byte FCS; frame check sequence checksum */
};

struct ethsnapframe {
    uint8_t  destmac[ETH_MAC_ADR_SIZE];
    uint8_t  srcmac[ETH_MAC_ADR_SIZE];
    uint16_t len;
    uint8_t  data[EMPTY];
};

#endif /* __KERN_IO_NET_ETH_H__ */

