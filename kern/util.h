#ifndef __KERN_UTIL_H__
#define __KERN_UTIL_H__

#include <stdint.h>

void bzero(void *adr, unsigned long len);
void bcopy(void *dest, void *src, unsigned long len);
void bfill(void *adr, uint8_t byte, unsigned long len);
int  memcmp(const void *ptr1, const void *ptr2, unsigned long nb);
int  strcmp(const char *str1, const char *str2);
long strncpy(char *dest, char *src, long len);
void kprintf(char *fmt, ...);

#endif /* __KERN_UTIL_H__ */

