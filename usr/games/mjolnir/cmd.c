#include <zero/cdecl.h>
#include <zero/param.h>
#include <mjolnir/mjol.h>

extern void mjolbeep(void);

mjolcmdfunc     *mjolcmdfunctab[1024][1024] ALIGNED(CLSIZE);
mjolcmdmovefunc *mjolcmdmovefunctab[1024] ALIGNED(PAGESIZE);
uint8_t          mjolcmdismovemap[32];
uint8_t          mjolcmdhasdirmap[32];
uint8_t          mjolcmdhasargmap[32];

#if 0
struct mjolobj *
mjolidentify(struct mjolgame *game, struct mjolchr *chr)
{
    struct mjolobj *dest;

    char  *str = dest->str;
    int  (*printmsg)(const char *, ...) = game->scr->printmsg;

    if (str) {
        printmsg("%s\n", str);
    }

    return NULL;
}
#endif

struct mjolobj *
mjolmoveleft(struct mjolgame *game, struct mjolchr *chr)
{
    struct mjolobj *dest = NULL;

    return dest;
}

struct mjolobj *
mjolmovedown(struct mjolgame *game, struct mjolchr *chr)
{
    struct mjolobj *dest = NULL;

    return dest;
}

struct mjolobj *
mjolmoveup(struct mjolgame *game, struct mjolchr *chr)
{
    struct mjolobj *dest = NULL;

    return dest;
}

struct mjolobj *
mjolmoveright(struct mjolgame *game, struct mjolchr *chr)
{
    struct mjolobj *dest = NULL;

    return dest;
}

struct mjolobj *
mjolmoveupleft(struct mjolgame *game, struct mjolchr *chr)
{
    struct mjolobj *dest = NULL;

    return dest;
}

struct mjolobj *
mjolmoveupright(struct mjolgame *game, struct mjolchr *chr)
{
    struct mjolobj *dest = NULL;

    return dest;
}

struct mjolobj *
mjolmovedownleft(struct mjolgame *game, struct mjolchr *chr)
{
    struct mjolobj *dest = NULL;

    return dest;
}

struct mjolobj *
mjolmovedownright(struct mjolgame *game, struct mjolchr *chr)
{
    struct mjolobj *dest = NULL;

    return dest;
}

void
mjolinitcmd(void)
{
    mjolcmdmovefunctab[MJOL_CMD_MOVE_LEFT] = mjolmoveleft;
    mjolcmdmovefunctab[MJOL_CMD_MOVE_DOWN] = mjolmovedown;
    mjolcmdmovefunctab[MJOL_CMD_MOVE_UP] = mjolmoveup;
    mjolcmdmovefunctab[MJOL_CMD_MOVE_RIGHT] = mjolmoveright;
    mjolcmdmovefunctab[MJOL_CMD_LEFT] = mjolmoveleft;
    mjolcmdmovefunctab[MJOL_CMD_DOWN] = mjolmovedown;
    mjolcmdmovefunctab[MJOL_CMD_UP] = mjolmoveup;
    mjolcmdmovefunctab[MJOL_CMD_RIGHT] = mjolmoveright;
    mjolcmdmovefunctab[MJOL_CMD_MOVE_UPLEFT] = mjolmoveupleft;
    mjolcmdmovefunctab[MJOL_CMD_MOVE_UPRIGHT] = mjolmoveupright;
    mjolcmdmovefunctab[MJOL_CMD_MOVE_DOWNLEFT] = mjolmovedownleft;
    mjolcmdmovefunctab[MJOL_CMD_MOVE_DOWNRIGHT] = mjolmovedownright;
    mjolsetmove(MJOL_CMD_MOVE_LEFT);
    mjolsetmove(MJOL_CMD_MOVE_DOWN);
    mjolsetmove(MJOL_CMD_MOVE_UP);
    mjolsetmove(MJOL_CMD_MOVE_RIGHT);
    mjolsetmove(MJOL_CMD_LEFT);
    mjolsetmove(MJOL_CMD_DOWN);
    mjolsetmove(MJOL_CMD_UP);
    mjolsetmove(MJOL_CMD_RIGHT);
    mjolsetmove(MJOL_CMD_MOVE_UPLEFT);
    mjolsetmove(MJOL_CMD_MOVE_UPRIGHT);
    mjolsetmove(MJOL_CMD_MOVE_DOWNLEFT);
    mjolsetmove(MJOL_CMD_MOVE_DOWNRIGHT);

    mjolsetdir(MJOL_CMD_FIGHT);
    mjolsetdir(MJOL_CMD_THROW);
    mjolsetdir(MJOL_CMD_MOVE);
    mjolsetdir(MJOL_CMD_ZAP);
    mjolsetdir(MJOL_CMD_IDENTIFY_TRAP);

    mjolsetarg(MJOL_CMD_IDENTIFY);
    mjolsetarg(MJOL_CMD_THROW);
    mjolsetarg(MJOL_CMD_ZAP);
    mjolsetarg(MJOL_CMD_QUAFF);
    mjolsetarg(MJOL_CMD_READ);
    mjolsetarg(MJOL_CMD_EAT);
    mjolsetarg(MJOL_CMD_WIELD);
    mjolsetarg(MJOL_CMD_TAKE_OFF);
    mjolsetarg(MJOL_CMD_PUT_RING_ON);
    mjolsetarg(MJOL_CMD_REMOVE_RING);
    mjolsetarg(MJOL_CMD_DROP);
    mjolsetarg(MJOL_CMD_CALL);

    return;
}

