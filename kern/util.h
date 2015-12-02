#ifndef __KERN_UTIL_H__
#define __KERN_UTIL_H__

#include <stdint.h>
#include <zero/cdecl.h>

void     kbzero(void *adr, uintptr_t len);
void     kmemset(void *adr, int byte, uintptr_t len);
void     kmemcpy(void *dest, const void *src, uintptr_t len);
intptr_t kmemcmp(const void *ptr1, const void *ptr2, uintptr_t len);
intptr_t kstrcmp(const char *str1, const char *str2);
long     kstrncpy(char *dest, const char *src, uintptr_t len);
void     kprintf(const char *fmt, ...);
void     panic(unsigned long pid, int32_t trap, long err);

#endif /* __KERN_UTIL_H__ */

