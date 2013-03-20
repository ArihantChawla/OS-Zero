#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <zero/trix.h>
//#include <zero/randk.h>
#include <dice.h>
#include <cpunk_character.h>
#include <cpunk_spell.h>

long cpunkheal(struct cpunkcharacter *healer, struct cpunkcharacter *target);
long cpunkfreeze(struct cpunkcharacter *healer, struct cpunkcharacter *target);
long cpunkresurrect(struct cpunkcharacter *healer, struct cpunkcharacter *target);

#define cpunkspellprint(spell, target, val)                             \
    printf(spellstr[(spell)], target, val)
    
typedef long (*spellfunc_t)(struct cpunkcharacter *, struct cpunkcharacter *);
spellfunc_t _spellfuncs[CPUNK_NSPELL]
= {
    NULL,
    cpunkheal,
    cpunkfreeze,
    cpunkresurrect
};

const char *spellstr[CPUNK_NSPELL]
= {
    NULL,
    "%s healed by %ld\n",
    "%s frozen for %ld turns\n",
    "%s resurrected with %ld hitpoints and energy\n",
};

long
cpunkheal(struct cpunkcharacter *healer, struct cpunkcharacter *target)
{
    long power = 0;

    power = diceroll(DICE_1D20);
    if (healer->race == CPUNK_RACE_WIZARD) {
        power += (healer->exp >> 8) & 0xff;
    }
    power += (healer->magic >> 8) & 0xff;
    target->hit = max(target->maxhit, target->hit + power);
    target->nrg = max(target->maxnrg, target->nrg + power);

    return power;
}

long
cpunkfreeze(struct cpunkcharacter *healer, struct cpunkcharacter *target)
{
    long nturn = 0;

    if (healer->race == CPUNK_RACE_WIZARD) {
        nturn += (healer->exp >> 8) & 0xff;
    }
    nturn += (healer->magic >> 8) & 0xff;

    return nturn;
}

long
cpunkresurrect(struct cpunkcharacter *healer, struct cpunkcharacter *target)
{
    long power = 0;

    if (target->hit) {

        return 0;
    }

    power = diceroll(DICE_1D12);
    if (healer->race == CPUNK_RACE_WIZARD) {
        power += (healer->exp >> 8) & 0xff;
    }
    power += (healer->magic >> 8) & 0xff;
    target->hit = max(target->maxhit, power);
    target->nrg = max(target->maxnrg, power);

    return power;
}

long
cpunkspell(long spell,
           struct cpunkcharacter *caster,
           struct cpunkcharacter *target)
{
    long power = 0;

    power = _spellfuncs[spell](caster, target);
    if (power) {
        cpunkspellprint(spell, target, power);
    }

    return power;
}

