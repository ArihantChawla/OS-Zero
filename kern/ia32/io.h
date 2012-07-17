#ifndef __IA32_IO_H__
#define __IA32_IO_H__

#include <stdint.h>

static __inline__
uint8_t inb(uint16_t port)
{
    uint32_t ret = 0;

    __asm__ ("inb %1, %b0\n" : "=a" (ret) : "Nd" (port));

    return (uint8_t)ret;
    
}
static __inline__
uint16_t inw(uint16_t port)
{
    uint32_t ret = 0;

    __asm__ ("inw %1, %w0\n" : "=a" (ret) : "Nd" (port));

    return (uint16_t)ret;
    
}
static __inline__
uint32_t inl(uint16_t port)
{
    uint32_t ret = 0;

    __asm__ ("inl %1, %0\n" : "=a" (ret) : "Nd" (port));

    return ret;
    
}
#define outb(b, p)                                                      \
    __asm__ __volatile__ ("outb %b0, %w1\n" : : "a" (b), "Nd" (p))
#define outw(w, p)                                                      \
    __asm__ __volatile__ ("outw %w0, %w1\n" : : "a" (w), "Nd" (p))
#define outl(l, p)                                                      \
    __asm__ __volatile__ ("outl %0, %w1\n" : : "a" (l), "Nd" (p))
/* delayed I/O */
#define _iodelay()  __asm__ __volatile__ ("outb %%al, $0x80\n")
#define outb_p(b, p) outb(b, p); _iodelay()
#define outw_p(w, p) outw(w, p); _iodelay()
#define outl_p(l, p) outl(l, p); _iodelay()

#endif /* __IA32_IO_H__ */

