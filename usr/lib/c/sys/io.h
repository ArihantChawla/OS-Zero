#ifndef __SYS_IO_H__
#define __SYS_IO_H__

#define _iodelay()  __asm__ __volatile__ ("outb %%al, $0x80\n")

static __inline__
unsigned char inb(int port)
{
    unsigned char ret = 0;

    __asm__ ("inb %1, %b0\n" : "=a" (ret) : "Nd" (port));

    return ret;
    
}

static __inline__
unsigned char inb_p(int port)
{
    unsigned char ret = 0;

    __asm__ ("inb %1, %b0\n" : "=a" (ret) : "Nd" (port));
    _iodelay();

    return ret;
    
}

static __inline__
unsigned short inw(int port)
{
    unsigned short ret = 0;

    __asm__ ("inw %1, %w0\n" : "=a" (ret) : "Nd" (port));

    return ret;
    
}

static __inline__
unsigned short inw_p(int port)
{
    unsigned short ret = 0;

    __asm__ ("inw %1, %w0\n" : "=a" (ret) : "Nd" (port));
    _iodelay();

    return ret;
    
}

static __inline__
unsigned int inl(int port)
{
    unsigned int ret = 0;

    __asm__ ("inl %1, %0\n" : "=a" (ret) : "Nd" (port));

    return ret;
    
}

static __inline__
unsigned int inl_p(int port)
{
    unsigned int ret = 0;

    __asm__ ("inl %1, %0\n" : "=a" (ret) : "Nd" (port));
    _iodelay();

    return ret;
    
}

#define outb(b, p)                                                      \
    __asm__ __volatile__ ("outb %b0, %w1\n" : : "a" (b), "Nd" (p))
#define outw(w, p)                                                      \
    __asm__ __volatile__ ("outw %w0, %w1\n" : : "a" (w), "Nd" (p))
#define outl(l, p)                                                      \
    __asm__ __volatile__ ("outl %0, %w1\n" : : "a" (l), "Nd" (p))

/* delayed I/O */
#define outb_p(b, p) outb(b, p); _iodelay())
#define outw_p(w, p) outw(w, p); _iodelay()
#define outl_p(l, p) outl(l, p); _iodelay()

#endif /* __SYS_IO_H__ */

