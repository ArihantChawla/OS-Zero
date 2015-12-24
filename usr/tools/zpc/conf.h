#ifndef __ZPC_CONF_H__
#define __ZPC_CONF_H__

#include <stdint.h>

#define ZPC32BIT    1
#define ZPCPAGESIZE 4096
#if (ZPCPAGESIZE == 4096)
#define ZPCNPOFSBIT 12
#if (ZPC32BIT)
#define ZPCNPFRMBIT (32 - ZPCNPOFSBIT)
#elif (ZPC64BIT)
#endif

#endif /* __ZPC_CONF_H__ */

