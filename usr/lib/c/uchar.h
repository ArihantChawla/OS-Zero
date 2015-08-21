#ifndef __UCHAR_H__
#define __UCHAR_H__

#include <stddef.h>
#include <stdint.h>
#include <wchar.h>

#define __STDC_UTF_16__ 1
#define __STDC_UTF_32__ 1

typedef uint16_t char16_t;
typedef uint32_t char32_t;

#if (!__KERNEL__)

size_t mbrtoc16(char16_t *__restrict ptr16,
                const char *__restrict str,
                size_t n,
                mbstate_t *__restrict state);
size_t c16rtomb(char *__restrict str,
                char16_t c16,
                mbstate_t *__restrict state);
size_t mbrtoc32 (char32_t *__restrict ptr32,
		 const char *__restrict str,
		 size_t n,
		 mbstate_t *__restrict state);
size_t c32rtomb (char *__restrict str,
		 char32_t c32,
		 mbstate_t *__restrict state);

#endif /* !__KERNEL__ */

#endif /* __UCHAR_H__ */

