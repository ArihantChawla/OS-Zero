#ifndef __NETINET_IP_VAR_H__
#define __NETINET_IP_VAR_H__

#include <stdint.h>

/* IP-header overlay for other protocols (TCP, UDP) */
struct ipovly {
    uint8_t        ih_xl[9];    // unused
    uint8_t        ih_pr;       // protocol
    int16_t        ih_len;      // protocol length
    struct in_addr ih_src;
    struct in_addr ih_dst;
};

#endif /* __NETINET_IP_VAR_H__ */

