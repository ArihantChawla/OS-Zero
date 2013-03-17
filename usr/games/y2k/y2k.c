#include <stdlib.h>

/* NOTE: this file is based on the book Cyberpunk 2020. */

/*
 * Y2K Game
 * --------
 *
 * The game, based on a short story by the same name Y2K, plans to implement
 * a cyberpunk-themed rogue-like dungeon/room exploring game. Instead of using
 * AD&D rules, I plan to implement Cyberpunk 2020 features in the game where
 * feasible.
 */

/*
 * role { rocker, solo, hacker, tech, medtech, media, cop, corp, fixer, nomad }
 * special { charisma[lead], solo[combat], hacks[(interface = usr|sys|code)],
 *	     tech, medtech, media, cop, corp, fixer, nomad }
 * char { pnt(random = 9 * d10(), fast = max(1d10, 2), stat),
 * stat { 2to10(brain, reflex, cool, tech, fortuna, looks, move, empathy) }
 */
/* cbp is global; nplr is # of players */

#define USERLOAD   1
#define USERADD    2
#define USERDEL    3
#define USERBLOCK  4
#define USERKICK   5
#define USERBANIP  6
/* game actions */
#define GAMESAVE   0
#define GAMELOAD   1
/* directions */
#define DIRSOUTH   0
#define DIRSW      1
#define DIRWEST    2
#define DIRNW      3
#define DIRNORTH   4
#define DIRNE      5
#define DIREAST    6
#define DIRSE      7
/* movements */
#define MOVEUP     0
#define MOVEDOWN   1
#define MOVELEFT   2
#define MOVERIGHT  3
#define MOVEFW     4 /* forward */
#define MOVEBW     5 /* backwards */

/* character types */
#define CHARROCKER 0 /* rockerboy */
#define CHARSOLO   1
#define CHARNETRUN 2 /* netrunner */
#define CHARTECH   3 /* techie */
#define CHARMEDIA  4
#define CHARCOP    5
#define CHARCORP   6 /* corporate */
#define CHARFIXER  7
#define CHARNOMAD  8
/* character actions */
#define WALK       0
#define LOOK       1 
#define TALK       2
#define LISTEN     3
#define SMELL      4
#define USE        5
#define ATTACK     6
#define CHAT       7
/* chat actions */
#define CHATMSG    0
#define CHATDO     1
/* statistics indices */
#define STATINT    0
#define STATREF    1
#define STATCL     2
#define STATTECH   3
#define STATLLK    4
#define STATATT    5
#define STATMA     6
#define STATMOVE   7
#define STATEMPAT  8
/* computer equipment */
#define DECKMOBILE 0 /* mobile device */
#define DECKDESK   1 /* desktop computer */
#define DECKSERVER 2 /* server computer */
#define DECKSUPER  3 /* super computer */
/* computer features */
#define DECKSCALAR 0 /* scalar; speed */
#define DECKFLOAT  1 /* float; speed */
#define DECKGFX    2 /* graphics; speed */
#define DECKNET    3 /* network; speed */
/* transportation */
#define TRANSWALK  0 /* feet */
#define TRANSBIKE  1 /* bicycle */
#define TRANSMOTO  2 /* motorbike */
#define TRANSCAR   3 /* car */
#define TRANSPLANE 4 /* airplane */
#define TRANSBALL  5 /* balloon */
#define TRANSSHUT  6 /* shuttle */
/* WARFARE */
/* vehicles */
#define WARMAN     0 /* soldier */
#define WARTANK    1 /* panzer/tank */
#define WARBOAT    2 /* boat */
#define WARSHIP    3 /* ship */
#define WARPLANE   4 /* fighter/bomber/transport airplane */
#define WARHELI    5 /* helicopter */
/* weapons */
#define WARPISTOL  0
#define WARRIFLE   1
#define WARMCGUN   2 /* machine gun */
#define WARGRENADE 3 /* grenade */
#define WARBOMB    1 /* bomb */
#define WARNUKE    2 /* nuclear warhead */
/* other hardware */
#define WARRADAR
#define WARSATEL
/* location */
#define LOCGROUND  0
#define LOCSKY     1 /* sky; airplanes, helicopters */
#define LOCSPACE   2 /* space; shuttles, missiles, satellites */
/* room types */
#define ROOMBASE   0 /* basement */
#define ROOMBAR    3 /* bar */
#define ROOMREST   4 /* restaurant */
#define ROOMBATH   5 /* bathroom */
#define ROOMBED    6 /* bedroom */
#define ROOMST     7 /* street */
#define ROOMHIW    8 /* highway */
#define ROOMALLEY  7 /* alley */

#define CYBMAPW 8
#define CYBMAPH 16
struct map {
    long          w;
    long          h;
    struct room **rtab;
};

struct room {
    long         type;
    long         nitem;
    struct item *itab;
};

#define pcancarry(cp, ip) ((ip)->wtug <= 10000000 * (cp)->body)
#define pcanlift(cp, ip)  ((ip)->wtug <= 40000000 * (cp)->body)
struct item {
    long type;
    long wtug;	/* micrograms */
    long val;	/* value in currency */
    long param;	/* type-specific parameter */
};

#define MOVERUN  0	/* move / 3 */
#define MOVELEAP 1	/* move / 4 */
struct cybplr {
    long cpnt;
    long stab[0];
    long intel;		/* INT */
    long reflx;		/* REF */
    long cool;		/* CL */
    long tech;		/* TECH */
    long luck;		/* LK */
    long attr;		/* ATT */
    long move;		/* MA */
    long empat;		/* EMPATHY */
    long body;		/* BT */
    long mtab[0];
    long run;
    long leap;
};

#define MACHPAD   0
#define MACHSTD   1
#define MACHLIME  2
#define MACHMINT  3
#define MACHFLAME 4
#define MACHFIRE  5
#define MACHLITE  6
#define MACHNOVA  7
#define MACHDUAL  8
#define MACHNTYPE 9
#define PAD       { "PAD",   MACHPAD,   10,  10,  15,  10  }
#define STD       { "STD",   MACHSTD,   10,  10,  20,  20  }
#define LIME      { "LIME",  MACHLIME,  15,  20,  20,  20  }
#define MINT      { "MINT",  MACHMINT,  20,  30,  40,  35  }
#define FLAME     { "FLAME", MACHFLAME, 40,  60,  50,  50  }
#define FIRE      { "FIRE",  MACHFIRE,  50,  70,  60,  60  }
#define LITE      { "LITE",  MACHLITE,  75,  80,  80,  75  }
#define NOVA      { "NOVA",  MACHNOVA,  100, 100, 100, 90  }
#define DUAL      { "DUAL",  MACHDUAL,  150, 170, 100, 100 }
struct cybmach {
    char *name;
    long  type;
    long  fpu;
    long  alu;
    long  mem;
    long  gfx;
};

struct cybtech {
    long usr;
    long sys;
    long code;
    long mach;
    long ice;
};

#define roll(x)   (rand() ((x) + 1))
#define rolld10() roll(10)

static struct cybmach machatr[MACHNTYPE]
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
static struct plr **plrtab;
static struct map  *cybmap;

struct room *
mkroom(long nitem)
{
    struct room *room = malloc(sizeof(struct room));
    long         type = rand() % (ROOMNTYPES - 1);
    long         l = nitem;

    room->type = type;
    room->nitem = nitem;
    while (l) {
        long 
    }

    return room;
}

struct map *
cybinitmap(long w, long h)
{
    struct map  *map = malloc(sizeof(struct map));
    struct room *room;
    long         l;

    map->w = w;
    map->h = h;
    map->rtab = calloc(w * h * sizeof(struct room *));
    while (w--) {
        for (l = 0 ; l < h ; l++) {
        }
    }
}

struct plr *
mkplr(long id)
{
    struct plr *plr	= malloc(sizeof(struct plr));
    long        val;
    long        l;
    long        d;

    /* character points */
    val ^= val; /* zero */
    for (l = 0 ; l < 9 ; ) {
        d = d10();
        if (d < 2) {
            continue;
        }
        val += d;
        l++;
    }
    plr->cp = val;
    /* statistics */
    for (l = 0 ; l < 9 ; ) {
        d = d10();
        if (d < 2) {
            continue;
        }
        plr->stab[l] = d;
        l++;
    }
    /* run and leap */
    plr->run = plr->stab[STATMOVE] / 3;
    plr->leap = plr->stab[STATMOVE] / 4;

    return plr;
}

void
cybstart(struct cb *cbp, long nplr)
{
    struct timeval tv;
    long           l;

    cybmap = cybinitmap();
    gettimeofday(&tv, NULL);
    srand(tv.n_sec);
    plrtab = malloc(nplr * sizeof(struct plr *));
    for (l = 0 ; l < nplr ; l++) {
        plrtab[l] = mkplr(l);
    }
}

#define WALK       0
#define INSTALL    1
#define LOOK       2 
#define TALK       3
#define LISTEN     4
#define SMELL      5
#define TASTE      6
#define USE        7
#define ATTACK     8
#define CHAT       9

void
walk(long plrid, struct room *room, long dir)
{
    ;
}

