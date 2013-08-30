#include <mjolnir/mjol.h>

typedef void mjolcmdfunc(struct mjolgame *);

static mjolcmdfunc *cmdfunctab[256];

void
mjolidentify(struct mjolgame *game)
{
    ;
}

void
mjolmoveleft(struct mjolgame *game)
{
    ;
}

void
mjolmovedown(struct mjolgame *game)
{
    ;
}

void
mjolmoveup(struct mjolgame *game)
{
    ;
}

void
mjolmoveright(struct mjolgame *game)
{
    ;
}

void
mjolmoveupleft(struct mjolgame *game)
{
    ;
}

void
mjolmoveupright(struct mjolgame *game)
{
    ;
}

void
mjolmovedownleft(struct mjolgame *game)
{
    ;
}

void
mjolmovedownright(struct mjolgame *game)
{
    ;
}

void
mjolinitcmd(void)
{
    cmdfunctab[MJOL_CMD_IDENTIFY] = mjolidentify;
    cmdfunctab[MJOL_CMD_MOVE_LEFT] = mjolmoveleft;
    cmdfunctab[MJOL_CMD_MOVE_DOWN] = mjolmovedown;
    cmdfunctab[MJOL_CMD_MOVE_UP] = mjolmoveup;
    cmdfunctab[MJOL_CMD_MOVE_RIGHT] = mjolmoveright;
    cmdfunctab[MJOL_CMD_MOVE_UPLEFT] = mjolmoveupleft;
    cmdfunctab[MJOL_CMD_MOVE_UPRIGHT] = mjolmoveupright;
    cmdfunctab[MJOL_CMD_MOVE_DOWNLEFT] = mjolmovedownleft;
    cmdfunctab[MJOL_CMD_MOVE_DOWNRIGHT] = mjolmovedownright;

    return;
}

void
mjoldocmd(struct mjolgame *game, int ch)
{
    ;
}
