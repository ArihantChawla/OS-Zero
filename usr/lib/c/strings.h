#ifndef __STRINGS_H__
#define __STRINGS_H__

#include <features.h>

#if !defined(__size_t_defined)
#include <share/size.h>
#endif

#if !defined(__KERNEL__)

int   bcmp(const void *str1, const void *str2, size_t nbyte);
void  bcopy(const void *src, const void *dest, size_t nbyte);
void  bzero(void *adr, size_t nbyte);
int   ffs(int i);
char *index(const char *str, int ch);
char *rindex(const char *str, int ch);
int   strcasecmp(const char *str1, const char *str2);
int   strncasecmp(const char *str1, const char *str2, size_t nbyte);

#endif /* !defined(__KERNEL__) */

#endif /* __STRINGS_H__ */

