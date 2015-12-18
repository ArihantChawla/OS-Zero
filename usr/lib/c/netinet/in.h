#ifndef __NETINET_IN_H__
#define __NETINET_IN_H__

#include <inttypes.h>
#include <sys/socket.h>
#include <zero/cdefs.h>
#include <zero/param.h>

#define INET_ADDRSTRLEN     16
#define INET6_ADDRSTRLEN    46

#define IPPROTO_RAW         1
#define IPPROTO_IP          2
#define IPPROTO_ICMP        3
#define IPPROTO_RAW         4
#define IPPROTO_TCP         5
#define IPPROTO_UDP         6

#define IPV6_JOIN_GROUP     1
#define IPV6_LEAVE_GROUP    2
#define IPV6_MULTICAST_HOPS 3
#define IPV6_MULTICAST_IF   4
#define IPV6_MULTICAST_LOOP 5

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

struct in_addr {
    in_addr_t s_addr;
};

struct sockaddr_in {
    sa_family_t sin_family;
    in_port_t   sin_port;
#if defined(EMPTY)
    char        sin_addr[EMPTY] ALIGNED(CLSIZE);
#else
    char        sin_addr[1] ALIGNED(CLSIZE);
#endif
};

#define IN6ADDR_ANY_INIT { 0 }  // all 0-bits
#define IN6ADDR_LOOPBACK_INIT                                           \
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }
struct in6_addr {
    uint8_t s6_addr[16];        // network byte-order
};

/* NOTE: sin6_port and sin6_addr are in network byte-order */
struct sockaddr_in6 {
    sa_family_t     sin6_family;    // AF_INET6
    in_port_t       sin6_port;      // port #
    uint32_t        sin6_flowinfo;  // traffic class and flow information
    struct in6_addr sin6_addr ALIGNED(CLSIZE);
    uint32_t        sin6_scope_id;
};

struct ipv6_mreq {
    struct in6_addr ipv6mr_multiaddr;   // IPv6 multicast address
    unsigned int    ipv6mr_interface;
};

const struct in6_addr in6addr_any;
const struct in6_addr in6addr_loopback;

#endif /* __NETINET_IN_H__ */

