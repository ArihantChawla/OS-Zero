#ifndef __KERN_SYS_STAT_H__
#define __KERN_SYS_STAT_H__

#define NODE_FIFO  0x00001000   // named pipe
#define NODE_CHR   0x00002000   // character special [device]
#define NODE_DIR   0x00004000   // directory
#define NODE_BLK   0x00006000   // block special
#define NODE_REG   0x00008000   // regular file
#define NODE_LNK   0x0000a000   // symbolic link
#define NODE_SOCK  0x0000c000   // socket
#define NODE_MQ    0x00010000   // message queue
#define NODE_SEM   0x00020000   // semaphore
#define NODE_SHM   0x00040000   // shared memory
#define NODE_WHT   0x00100000   // white-out; name lookups return -ENOENT
#define NODE_MASK  0x0004f000

#endif /* __KERN_SYS_STAT_H__ */

