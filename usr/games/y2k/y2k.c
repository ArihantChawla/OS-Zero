#include <stdlib.h>
#include <y2k/y2k.h>

/* NOTE: this file is based on the book Cyberpunk 2020. */

/*
 * Y2K Game
 * --------
 *
 * The game, based on a short story by the same name Y2K, plans to implement
 * a cyberpunk-themed rogue-like dungeon/room exploring game. Instead of using
 * AD&D rules, I plan to implement Cyberpunk 2020 features in the game where
 * feasible.
 *
 * Features
 * --------
 * - networked multiplayer gameplay
 * - program-on-program combat in Redcode; SEE http://www.corewars.org/
 *   - implementation of MARS
 *   - winner takes control of host machine; think about prizes
 * - adventure based on rolling [D20] dice
 * - support for human-lead stories later
 */

static struct mach machatr[MACHNTYPE]
= {
    PAD,
    STD,
    LIME,
    MINT,
    FLAME,
    FIRE,
    LITE,
    NOVA
};

struct item *
y2kmkitem(long type)
{
    type %= ITEMNTYPE;
    item = calloc(1, sizeof(struct item));
}

void
y2kadditem(struct room *room, long x, long y, struct item *item)
{
    struct itemq *itemq = &room->iqtab[x * room->w + y];
    struct item  *head;
    struct item  *tail;

    mtxlk(&itemq->lk);
    head = itemq->head;
    tail = itemq->tail;
    if (!head) {
        itemq->head = item;
        itemq->tail = item;
    } else {
        item->prev = tail;
        tail->next = item;
        itemq->tail = item;
    }
    itemq->nitem++;
    mtxunlk(&itemq->lk);
}

struct item *
y2krmitem(struct room *room, long x, long y)
{
    struct item  *ret = NULL;
    struct itemq *itemq = &room->iqtab[x * room->w + y];
    struct item  *head;
    struct item  *tail;

    mtxlk(&itemq->lk);
    head = itemq->head;
    tail = itemq->tail;
    if (tail == head) {
        itemq->tail = NULL;
        itemq->head = NULL;
        ret = head;
    } else if (tail) {
        tail->prev->next = NULL;
        itemq->tail = tail->prev;
        ret = tail;
    }
    if (ret) {
        itemq->nitem--;
    }
    mtxunlk(&itemq->lk);

    return ret;
}

struct room *
y2kmkroom(long w, long h, long nitem)
{
    struct room  *room = malloc(sizeof(struct room));
    struct itemq *iqtab = calloc(w * h, sizeof(struct itemq));
    struct item  *item;
    long          type = rand() % (ROOMNTYPE - 1);
    long          l = nitem;

    room->type = type;
    room->w = w;
    room->h = h;
    room->nitem = nitem;
    while (l--) {
        item = y2kmkitem(rand());
        y2kadditem(room, rand() % w, rand() % h, item);
    }

    return room;
}

struct map *
y2kinitmap(long w, long h)
{
    struct map  *map = malloc(sizeof(struct map));
    struct room *room;
    long         l;

    map->w = w;
    map->h = h;
    map->rtab = calloc(w * h, sizeof(struct room *));
    while (w--) {
        for (l = 0 ; l < h ; l++) {
        }
    }
}

struct plr *
y2kmkplr(long type)
{
    struct plr *plr	= malloc(sizeof(struct plr));
    long        val;
    long        l;
    long        d;

    type %= CHARNTYPE;
    /* hit points */
    val ^= val; /* zero */
    for (l = 0 ; l < 9 ; ) {
        d = diceroll(10);
        val += d;
        l++;
    }
    plr->hitpt = val;
    /* statistics */
    for (l = 0 ; l < STATNVAL ; ) {
        d = diceroll(10);
        plr->stab[l] = d;
        l++;
    }
    /* run and leap */
    plr->run = plr->stab[STATMOVE] / 3;
    plr->leap = plr->stab[STATMOVE] / 4;

    return plr;
}

void
y2kstart(struct y2k *y2k, long nplr)
{
    struct timeval tv;
    long           l;

    y2k->map = y2kinitmap(Y2KMAPW, Y2KMAPH);
    gettimeofday(&tv, NULL);
    srand(tv.n_sec);
    y2k->plrtab = malloc(nplr * sizeof(struct plr *));
    if (y2k->plrtab) {
        for (l = 0 ; l < nplr ; l++) {
            y2k->plrtab[l] = y2kmkplr(l);
        }
    } else {
        free(y2k->map);
    }
}

void
y2kwalk(long plrid, struct room *room, long dir)
{
    ;
}

