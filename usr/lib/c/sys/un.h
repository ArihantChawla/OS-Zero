#ifndef __SYS_UN_H__
#define __SYS_UN_H__

#include <stddef.h>
#include <sys/socket.h>
#if (_UNIX_SOURCE)
#include <string.h>
#endif

struct sockaddr_un {
    sa_family_t sun_family;     // AF_UNIX
    char        sun_path[108];
};

#if (_UNIX_SOURCE)
#define SUN_LEN(ptr)                                                    \
    (offsetof(struct sockaddr_un, sun_path)                             \
     + strnlen(ptr->sun_path, sizeof(ptr->sun_path)))
#endif

#endif /* __SYS_UN_H__ */

