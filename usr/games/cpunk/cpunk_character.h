#ifndef __CPUNK_CHARACTER_H__
#define __CPUNK_CHARACTER_H__

#define CPUNK_NAME_LEN    16

#define CPUNK_RACE_HUMAN  1
#define CPUNK_RACE_CYBORG 2
#define CPUNK_RACE_ALIEN  3
#define CPUNK_RACE_ROBOT  4
#define CPUNK_RACE_WIZARD 5
struct cpunkcharacter {
    long race;                  // human, cyborg, alien, robot, wizard
    long intel;                 // intelligence; 0..0xffff
    long hit;                   // hit points
    long maxhit;
    long nrg;                   // energy
    long maxnrg;
    long exp;                   // experience; 0..0xffffffff
    long charm;                 // charm; 0..0xffff
    long karma;                 // karma; 0..0xffff
    long stam;                  // stamina; 0..0xffff
    long magic;                 // magic skill; 0..0xffff
    long code;                  // code skill; 0..0xffff
    long crypt;                 // crypt skill; 0..0xffff
    long reveng;                // reverse-engineering skill; 0..0xffff
    long ice;                   // ice skill; 0..0xffff
    long mart;                  // martial arts skill
    char name[CPUNK_NAME_LEN];  // character name
};

void   cpunkprintchar(struct cpunkcharacter *player);
struct cpunkcharacter * cpunknewchar(char *name, long race);

#endif /* __CPUNK_CHARACTER_H__ */

