#ifndef __KERN_IO_NET_PKT_H__
#define __KERN_IO_NET_PKT_H__

/* packet types */

#define NET_PKT_HOST        0
#define NET_PKT_BROADCAST   1
#define NET_PKT_MULTICAST   2
#define NET_PKT_FOREIGN     3
#define NET_PKT_OUTGOIND    4
#define NET_PKT_LOOPBACK    5
#define NET_PKT_FASTROUTE   6
#define NET_PKT_PROMISC     7
#define NET_PKT_ALLMULTI    8
#define NET_PKT_UNICAST     9

/* RX-ring header ststus */
#define NET_PKT_KERNEL      0
#define NET_PKT_USER        (1 << 0)
#define NET_PKT_COPY        (1 << 1)
#define NET_PKT_LOSING      (1 << 2)
#define NET_PKT_NOCHKSUM    (1 << 3)
#define NET_PKT_VLAN_OK     (1 << 4)
#define NET_PKT_TIMEOUT     (1 << 5)
/* TX-ring header status */
#define NET_PKT_AVAIL       0
#define NET_PKT_SEND_REQ    (1 << 0)
#define NET_PKT_SENDING     (1 << 1)
#define NET_PKT_INVAL_FMT   (1 << 2)    // wrong format
/* RX- and TX-ring header status */
#define NET_PKT_SOFT        (g1 << 29)
#define NET_PKT_SYS_HW      (1 << 30)
#define NET_PKT_RAW_HW      (1 << 31)
/* RX-ring - feature request bits */
#define NET_PKT_FILL_RXHASH (1 << 0)

#define NET_PKT_ALIGNMENT   16
#define netpktalign(x)                                                  \
    (((x) + NET_PKT_ALIGNMENT - 1) & ~(NET_PKT_ALIGNMENT - 1))
#define NET_PKT_HDRLEN      (

#endif /*  __KERN_IO_NET_PKT_H__ */

