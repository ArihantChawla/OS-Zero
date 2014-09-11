#ifndef __CPIO_H__
#define __CPIO_H__

#include <stdint.h>

/* c_magic */
#define MAGIC    "070707"

/* c_mode field values */
#define C_IRUSR  0000400    // user read
#define C_IWUSR  0000200    // user write
#define C_IXUSR  0000100    // user execute
#define C_IRGRP  0000040    // group read
#define C_IWGRP  0000020    // group write
#define C_IXGRP  0000010    // group execute
#define C_IROTH  0000004    // world read
#define C_IWOTH  0000002    // world write
#define C_IXOTH  0000001    // world execute
#define C_ISUID  0004000    // set user ID
#define C_ISGID  0002000    // set group ID
#define C_ISVTX  0001000    // on directories, restricted deletion flags
#define C_ISDIR  0040000    // directory
#define C_ISFIFO 0010000    // FIFO
#define C_ISREG  0100000    // regular file
#define C_ISBLK  0060000    // block special
#define C_ISCHR  0020000    // characte special
#define C_ISCTG  0110000    // reserved
#define C_ISLNK  0120000    // symbolic link
#define C_ISSOCK 0140000    // socket

/* CPIO heade structure for internal use */
struct __cpiohdr {
	uint8_t c_magic[6];
	uint8_t c_dev[6];
	uint8_t c_ino[6];
	uint8_t c_mode[6];
	uint8_t c_uid[6];
	uint8_t c_gid[6];
	uint8_t c_nlink[6];;
	uint8_t c_rdev[6];
	uint8_t c_time[11];
	uint8_t c_namesize[6];
	uint8_t c_filesize[11];
};

#endif /* __CPIO_H__ */

