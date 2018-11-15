#ifndef __KERN_UTIL_H__
#define __KERN_UTIL_H__

#include <stdint.h>
#include <zero/cdefs.h>

#define    kbcopy(src, dest, len) kmemcpy(dest, src, len)
#define    krewind(frm)           krewind2(fmr, NULL)

void       kbzero(void *adr, uintptr_t len);
void       kmemset(void *adr, int byte, uintptr_t len);
void       kmemcpy(void *dest, const void *src, uintptr_t len);
intptr_t   kmemcmp(const void *ptr1, const void *ptr2, uintptr_t len);
intptr_t   kstrcmp(const char *str1, const char *str2);
long       kstrncpy(char *dest, const char *src, uintptr_t len);
void     * kstrtok(void *ptr, int ch);
void       kpanic(int32_t trap, long err, void *frame);
void       kbacktrace(void **buf, int size, long syms);

#endif /* __KERN_UTIL_H__ */

