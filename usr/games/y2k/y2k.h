#ifndef __Y2K_Y2K_H__
#define __Y2K_Y2K_H__

#include <games/dice.h>
#include <zero/param.h>
#include <zero/cdecl.h>

/*
 * role { rocker, solo, hacker, tech, medtech, media, cop, corp, fixer, nomad }
 * special { charisma[lead], solo[combat], hacks[(interface = usr|sys|code)],
 *	     tech, medtech, media, cop, corp, fixer, nomad }
 * char { pnt(random = 9 * d10(), fast = max(1d10, 2), stat),
 * stat { 2to10(brain, reflex, cool, tech, fortuna, looks, move, empathy) }
 */
/* cbp is global; nplr is # of players */

/* game administration */
#define USERLOAD       1
#define USERADD        2
#define USERDEL        3
#define USERBLOCK      4
#define USERKICK       5
#define USERBANIP      6
/* game actions */
#define GAMESAVE       0
#define GAMELOAD       1
/* directions */
#define DIRSOUTH       0
#define DIRSW          1
#define DIRWEST        2
#define DIRNW          3
#define DIRNORTH       4
#define DIRNE          5
#define DIREAST        6
#define DIRSE          7
/* movements */
#define MOVEUP         0
#define MOVEDOWN       1
#define MOVELEFT       2
#define MOVERIGHT      3
#define MOVEFW         4 /* forward */
#define MOVEBW         5 /* backwards */

/* character types */
#define CHARROCKER     0 /* rockerboy */
#define CHARSOLO       1 
#define CHARNETRUN     2 /* netrunner */
#define CHARTECH       3 /* techie */
#define CHARMEDIA      4
#define CHARCOP        5
#define CHARCORP       6 /* corporate */
#define CHARFIXER      7
#define CHARNOMAD      8
#define CHARADMIN      9
#define CHARHACKER    10
#define CHARNTYPE     11
/* character actions */
#define OPWALK         0
#define OPINSTALL      1
#define OPLOOK         2 
#define OPTALK         3
#define OPLISTEN       4
#define OPSMELL        5
#define OPTASTE        6
#define OPUSE          7
#define OPATTACK       8
#define OPCHAT         9
/* chat actions */
#define CHATMSG        0
#define CHATDO         1

/* statistics indices */
#define STATINTEL      0 /* intelligence */
#define STATREF        1 /* reflexes */
#define STATCOOL       2
#define STATTECH       3
#define STATLUCK       4
#define STATATTR       5
#define STATMOVE       6
#define STATEMPAT      7
#define STATNVAL       8
/* computer equipment */
#define DECKMOBILE     0 /* mobile device */
#define DECKDESK       1 /* desktop computer */
#define DECKSERVER     2 /* server computer */
#define DECKSUPER      3 /* super computer */
/* computer features */
#define DECKSCALAR     0 /* scalar; speed */
#define DECKFLOAT      1 /* float; speed */
#define DECKGFX        2 /* graphics; speed */
#define DECKNET        3 /* network; speed */
/* transportation */
#define TRANSWALK      0 /* feet */
#define TRANSBIKE      1 /* bicycle */
#define TRANSMOTO      2 /* motorbike */
#define TRANSCAR       3 /* car */
#define TRANSPLANE     4 /* airplane */
#define TRANSBALL      5 /* balloon */
#define TRANSJET       6 /* jet engine airplane */
#define TRANSSHUT      7 /* shuttle */
/* WARFARE */
/* vehicles */
#define WARMAN         0 /* soldier */
#define WARTANK        1 /* panzer/tank */
#define WARBOAT        2 /* boat */
#define WARSHIP        3 /* ship */
#define WARPLANE       4 /* fighter/bomber/transport airplane */
#define WARHELI        5 /* helicopter */
/* weapons */
#define WARPISTOL      0
#define WARRIFLE       1
#define WARMCGUN       2 /* machine gun */
#define WARGRENADE     3 /* grenade */
#define WARBOMB        4 /* bomb */
#define WARNUKE        5 /* nuclear warhead */
/* other hardware */
#define WARRADAR
#define WARSATEL
/* location */
#define LOCDIG         0 /* underground */
#define LOCGROUND      1 /* ground level */
#define LOCSKY         2 /* sky; airplanes, helicopters */
#define LOCSPACE       3 /* space; shuttles, missiles, satellites */
/* room types */
#define ROOMBASE       0 /* basement */
#define ROOMBAR        1 /* bar */
#define ROOMREST       2 /* restaurant */
#define ROOMBATH       3 /* bathroom */
#define ROOMBED        4 /* bedroom */
#define ROOMST         5 /* street */
#define ROOMHIW        6 /* highway */
#define ROOMALLEY      7 /* alley */
#define ROOMHIWAY      8 /* highway */
#define ROOMDATAC      9 /* data center */
#define ROOMTEMPLE     10
#define ROOMNTYPE      11
/* items */
#define ITEMSCROLL      1
#define ITEMFOOD        2
#define ITEMBEVER       3 
#define ITEMROCK        4
#define ITEMBAT         5
#define ITEMCLUB        6
#define ITEMAXE         7
#define ITEMTOMAHAWK    8
#define ITEMKATANA      9
#define ITEMHEADARMOR  10
#define ITEMCHESTARMOR 11
#define ITEMHANDARMOR  12
#define ITEMFOOTARMOR  13
#define ITEMALTAR      14
#define ITEMMACH       15
#define ITEMNTYPE      16

#define Y2KMAPW        80
#define Y2KMAPH        24
struct map {
    long          w;
    long          h;
    struct room **rtab;
};

struct room {
    long          type;
    long          w;
    long          h;
    long          nitem;
    struct itemq *iqtab;
};

#define plrcancarry(plr, ip) ((ip)->wtug <= 1000000 * (plr)->body)
#define plrcanlift(cp, ip)  ((ip)->wtug <= 4000000 * (plr)->body)
struct item {
    long         type;
    long         wtug;  /* micrograms */
    long         val;   /* value in currency */
    long         param; /* type-specific parameter */
    struct item *prev;  /* previous item in list */
    struct item *next;  /* next item in list */
};

struct itemq {
    volatile long  lk;
    long           nitem;
    struct item   *head;
    struct item   *tail;
};

#define MOVERUN  0	/* move / 3 */
#define MOVELEAP 1	/* move / 4 */
struct plr {
    long hitpt;
    long intel;		/* INTEL */
    long ref;		/* REF */
    long cool;		/* COOL */
    long tech;		/* TECH */
    long luck;		/* LUCK */
    long attr;		/* ATTR */
    long move;		/* MOVE */
    long empat;		/* EMPATHY */
    long body;		/* BODY */
    long run;
    long leap;
    long stab[STATNVAL];
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
struct mach {
    char *name;
    long  type;
    long  fpu;
    long  alu;
    long  mem;
    long  gfx;
};

struct tech {
    long usr;
    long sys;
    long code;
    long mach;
    long ice;
};

struct y2k {
    long         nplr;
    struct plr **plrtab;
    long         mapw;
    long         maph;
    struct map  *map;
};

#endif /* __Y2K_Y2K_H__ */

