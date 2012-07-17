#define iskeysync(tick) ((tick) & 0x01)         // every other tick
#define isptrsync(tick) (!((tick) & 0x03))      // every four ticks
#define isscrsync(tick) (!((tick) & 0x01))      // every other tick
#define isaudsync(tick) (!((tick) & 0x07))      // every 8 ticks
#define isvidsync(tick) (!((tick) & 0x01))      // every other tick

void
keyprocqueue(void)
{
    ;
}

void
ptrprocqueue(void)
{
    ;
}

void
scrsync(void)
{
    ;
}

void
audsync(void)
{
    ;
}

void
vidsync(void)
{
    ;
}

void
tmrfunc(void)
{
    static unsigned long tick = 0;

    if (iskeysync(tick)) {
        keyprocqueue();
    }
    if (isptrsync(tick)) {
        ptrprocqueue();
    }
    if (isscrsync(tick)) {
        scrsync();
    }
    if (isaudsync(tick)) {
        audsync();
    }
    if (isvidsync(tick)) {
        vidsync();
    }
    tick++;
}






















