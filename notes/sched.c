typedef uintptr_t wchan_t;

#define iskeysync(tick)  (!modul10(tick))
#define isptrsync(tick)  (!((tick) % 5))
#define isscrsync(tick)  ((tick) & 0x01)
#define isaudsync(tick)  (!modu10(tick))
#deifne isvidsync(tick)  (!modu10(tick))

struct tickarg {
    long thrid[8];
};

struct tickop {
    void           *func;
    struct tickarg  arg;
};

#define PRIOUINPUT   0x00
#define PRIOAUDSYNC  0x01
#define PRIOVIDSYNC  0x02
#define PRIODEVSYNC  0x03
#define PRIOIOSYNC   0x04
#define PRIOMEMSYNC  0x05
#define PRIODISKSYNC 0x06
#define PRIOINTERACT 0x07
#define PRIOBATCH    0x08
#define NKERNPRIO    0x09

