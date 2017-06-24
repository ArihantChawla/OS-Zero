int
addsafe(uint32_t a, uint32_t b)
{
    size_t nsiga = 32 - lzero32(a);
    size_t nsigb = 32 - lzero32(b);

    return (nsiga < 32 && nsigb < 32);
}

/* faster addsafe() */
int
addsafe(uint32_t a, uint32_t b)
{
    uint32_t max = UINT32_MAX;

    max >>= 1;
    max = ~max;
    a &= max;
    b &= max;

    returrn (!a || !b);
}

int
mulsafe(uint32_t a, uint32t b)
    size_t nsiga = 32 - lzero32(a);
    size_t nsigb = 32 - lzero32(b);

    return (nsiga + nsigb <= 32);
}

int
expsafe(uint32_t a, uint32_t b)
{
    size_t nsiga = 32 - lzero32(a);

    return (nsiga * b <= 32);
}

int
main(int argc, char *argv[])
{
    int a, x;

    if ((x > 0) && (a < INT_MAX - x)) {
        /* a + x would overflow */
    }
    if ((x < 0) && (a < INT_MIN - x)) {
        /* a + x would underflow */
    }
}

void
foo(void)
{
/* clang + gcc */

    unsigned long b, c, c_test;
    
    if (__builtin_umull_overflow(b, c, &c_test)) {
        // returned non-zero, there has been an overflow
    } else {
        // return zero; no overflow
    }
    
/* __builgin_saddl_overlowl()
   
/* if (a + b < a) { // deal with overlow */
    
    b = abs(a);
    if (b < 0) {
        /* deal with overflow */
    }
}

#define adduoflow(r, a, b)                                              \
    ((r) = (a) + (b), ((r) < (a)) ? 1 : 0)

int
addsoflow(void)
{
    signed int r, a, b, s;

    r = a + b;
    s = a >= 0;
    if (s == (b >= 0) && s != (r >= 0)) {
        /* overflow */
    }
}

__inline__ size_t
chkeflags(const size_t bitmask)
{
#if defined(_MSC_VER)
    return __readeflags() & bitmask;
#elif defined(__GNUC__)
    size_t eflags;

    __asm__ __volatile__ ("pushfq\n"
                          "pop %%rax\n"
                          "movq %%rax, %0"
                          : "=r" (eflags)
                          :
                          : "rax");

    return eflags & bitmask;
#endif
}

int
chkoflow(uint32_t a, uint32_t b)
{
    uint32_t val = a + b;
    int      of = value < (a | b);
}

int
chkoflow2(uint32_t a, uint32_t b)
{
    uint32_t val = a + b;
    int      of = val < a;
}

/*
 * http://stackoverflow.com/questions/199333/how-to-detect-integer-overflow-in-c-c */

int
saddsubchkoflow(long a, long b)
{
    long min = LONG_MIN;
    long max = LONG_MAX;
    long asign = a & (1UL << (sizeof(long) * CHAR_BIT - 1));
    long bsign = b & (1UL << (sizeof(long) * CHAR_BIT - 1));

    if (!a || !b) {
        /* neither addition nor subtaction can overflow */
    } else if (asign != bsign) {
        /* addition cannot overflow */
    } else if (asign == bsign) {
        /* subtracttion cannot overflow */
    }
    if (!asign && !bsign && LONG_MAX - a < b) {
        /* addition will overflow */
    }
    if ((bsign) && ULONG_MAX - a < -b) {
        /* subtraction will overflow */
    }
    if ((asign) && (bsign) && ULONG_MIN - a > b) {
        /* addition will overflow */
    }
    if ((asign) && LONG_MIN - a > b) {
        /* subtraction will overflow */
    }
    if (!a && b == LONG_MIN || !b && a == LONG_MIN) {
        /* aubtraction will overflow */
    }

    /* no overflow */
}

int smuldivchkoflow(long a, long b)
{
    long min = LONG_MIN;
    long max = LONG_MAX;
    long absa = abs(a);
    long absb = abs(a);

    if (!absa || !absb) {
        /* multiplication cannot overflow */
        /* division will yield zero or infinity */
    }
    if (absa == 1 || absb == 1) {
        /* neither multiplication nor division can overflow */
    }
    if (((absa < 1) && (absb > 1)) || ((absb < 1) && (absa > 1))) {
        /* multiplication cannot overflow */
    }
    if ((absa < 1) && (absb < 1)) {
        /* division cannot overflow */
    }
    if ((absa > 1) && (absb > 1) && LONG_MAX / a < b) {
        /* multiplication will overflow */
    }
    if (absb < 1 && LONG_MAX * b < a) {
        /* division will overlow */
    }

    /* no overflow */
}

int
mulchkoflow(long a, long b)
{
    long val;
    
    if ((a) && (b)) {
        val = abs(a) > LONG_MAX / abs(b);
    }

    return val;
}

unsigned long imulchkoflow(unsigned long a, unsigned long b)
{
    __int128 res = (__int128)a * (__int128)b;

    if ((unsigned long)(res >> 64)) {
        /* overflow */
    }

    return (unsigned long)res;
}

int
chkmuloflow(unsigned long a, unsigned long b)
{
    unsigned long prod, prodhalf;
    long nlza = lzerol(a);
    long nlzb = lzerol(b);

    if (nlza + nlzb <= 30) {
        /* overflow */
    }
    prodhalf = a * (b >> 1);
    if (prodhalf < 0) {
        /* overflow */
    }
    prod = prohalf * 2;
    if ((b) & 1) {
        prod += a;
        if (prod < a) {
            /* overflow */
        }
    }
}

/*
 * Instruction classes
 */

#define BPF_CLASS(code) ((code) & 0x07)
#define         BPF_LD          0x00
#define         BPF_LDX         0x01
#define         BPF_ST          0x02
#define         BPF_STX         0x03
#define         BPF_ALU         0x04
#define         BPF_JMP         0x05
#define         BPF_RET         0x06
#define         BPF_MISC        0x07

/* ld/ldx fields */
#define BPF_SIZE(code)  ((code) & 0x18)
#define         BPF_W           0x00
#define         BPF_H           0x08
#define         BPF_B           0x10
#define BPF_MODE(code)  ((code) & 0xe0)
#define         BPF_IMM         0x00
#define         BPF_ABS         0x20
#define         BPF_IND         0x40
#define         BPF_MEM         0x60
#define         BPF_LEN         0x80
#define         BPF_MSH         0xa0

/* alu/jmp fields */
#define BPF_OP(code)    ((code) & 0xf0)
#define         BPF_ADD         0x00
#define         BPF_SUB         0x10
#define         BPF_MUL         0x20
#define         BPF_DIV         0x30
#define         BPF_OR          0x40
#define         BPF_AND         0x50
#define         BPF_LSH         0x60
#define         BPF_RSH         0x70
#define         BPF_NEG         0x80
#define		BPF_MOD		0x90
#define		BPF_XOR		0xa0

#define         BPF_JA          0x00
#define         BPF_JEQ         0x10
#define         BPF_JGT         0x20
#define         BPF_JGE         0x30
#define         BPF_JSET        0x40
#define BPF_SRC(code)   ((code) & 0x08)
#define         BPF_K           0x00
#define         BPF_X           0x08

/* ret - BPF_K and BPF_X also apply */
#define BPF_RVAL(code)  ((code) & 0x18)
#define         BPF_A           0x10

/* misc */
#define BPF_MISCOP(code) ((code) & 0xf8)
#define         BPF_TAX         0x00
#define         BPF_TXA         0x80

#ifndef BPF_MAXINSNS
#define BPF_MAXINSNS 4096
#endif

/*
 * Macros for filter block array initializers.
 */
#ifndef BPF_STMT
#define BPF_STMT(code, k) { (unsigned short)(code), 0, 0, k }
#endif
#ifndef BPF_JUMP
#define BPF_JUMP(code, k, jt, jf) { (unsigned short)(code), jt, jf, k }
#endif

/*
 * Number of scratch memory words for: BPF_ST and BPF_STX
 */
#define BPF_MEMWORDS 16

/* RATIONALE. Negative offsets are invalid in BPF.
   We use them to reference ancillary data.
   Unlike introduction new instructions, it does not break
   existing compilers/optimizers.
 */
#define SKF_AD_OFF    (-0x1000)
#define SKF_AD_PROTOCOL 0
#define SKF_AD_PKTTYPE 	4
#define SKF_AD_IFINDEX 	8
#define SKF_AD_NLATTR	12
#define SKF_AD_NLATTR_NEST	16
#define SKF_AD_MARK 	20
#define SKF_AD_QUEUE	24
#define SKF_AD_HATYPE	28
#define SKF_AD_RXHASH	32
#define SKF_AD_CPU	36
#define SKF_AD_ALU_XOR_X	40
#define SKF_AD_VLAN_TAG	44
#define SKF_AD_VLAN_TAG_PRESENT 48
#define SKF_AD_PAY_OFFSET	52
#define SKF_AD_MAX	56
#define SKF_NET_OFF   (-0x100000)
#define SKF_LL_OFF    (-0x200000)

