#ifndef __STRING_H__
#define __STRING_H__

#if !defined(__KERNEL__)

#include <stddef.h>

void * memcpy(void *dest, const void *src, size_t n);
void * memmove(void *dest, void *src, size_t n);
char * strcpy(char *dest, const char *src);
char * strncpy(char *dest, const char *src, size_t n);
char * strcat(char *dest, const char *src);
char * strncat(char *dest, const char *src, size_t n);
int    memcmp(const void *ptr1, const void *ptr2, size_t n);
int    strcmp(const char *str1, const char *str2);
int    strcoll(const char *str1, const char *str2);
int    strncmp(const char *str1, const char *str2, size_t n);
size_t strxfrm(char *str1, char *str2, size_t n);
void * memchr(const void *ptr, int ch, size_t n);
void * strchr(const char *str, int ch);
size_t strcspn(const char *str1, const char *str2);
char * strpbrk(const char *str1, const char *str2);
char * strrchr(const char *str, int ch);
size_t strspn(const char *str1, const char *str2);
char * strstr(const char *str1, const char *str2);
char * strtok(char *str1, const char *str2);
void * memset(void *ptr, int ch, size_t n);
char * strerror(int errnum);
size_t strlen(const char *str);

#endif /* !defined(__KERNEL__) */

#endif /* __STRING_H__ */

