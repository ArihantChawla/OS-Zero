#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zero/randk.h>
#include <zero/trix.h>
#include <dice.h>
#include <cpunk_character.h>

const char *racetab[]
= {
    NULL,
    "human",
    "cyborg",
    "alien",
    "robot",
    "wizard"
};

void
cpunkprintchar(struct cpunkcharacter *player)
{
    fprintf(stderr, "CHARACTER SHEET\n");
    fprintf(stderr, "---------------\n");
    fprintf(stderr, "name %s\n", player->name);
    fprintf(stderr, "race %s\n", racetab[player->race]);
    fprintf(stderr, "intelligence %ld\n", player->intel);
    fprintf(stderr, "hit %ld\n", player->hit);
    fprintf(stderr, "maxhit %ld\n", player->maxhit);
    fprintf(stderr, "nrg %ld\n", player->nrg);
    fprintf(stderr, "maxnrg %ld\n", player->maxnrg);
    fprintf(stderr, "experience %ld\n", player->exp);
    fprintf(stderr, "charm %ld\n", player->charm);
    fprintf(stderr, "karma %ld\n", player->karma);
    fprintf(stderr, "stamina %ld\n", player->stam);
    fprintf(stderr, "magic %ld\n", player->magic);
    fprintf(stderr, "code %ld\n", player->code);
    fprintf(stderr, "crypt %ld\n", player->crypt);
    fprintf(stderr, "reveng %ld\n", player->reveng);
    fprintf(stderr, "ice %ld\n", player->ice);
    fprintf(stderr, "martial %ld\n", player->mart);
}

struct cpunkcharacter *
cpunknewchar(char *name, long race)
{
    struct cpunkcharacter *newchar = malloc(sizeof(struct cpunkcharacter));
    
    if (name) {
        strncpy(newchar->name, name, CPUNK_NAME_LEN);
    }
    newchar->race = race;
    newchar->intel = diceroll(DICE_1D20);
    newchar->maxhit = diceroll(DICE_1D20);
    newchar->hit = max(diceroll(DICE_1D20), newchar->maxhit);
    newchar->maxnrg = diceroll(DICE_1D20);
    newchar->nrg = max(diceroll(DICE_1D20), newchar->maxnrg);
    newchar->exp = 0;
    newchar->charm = diceroll(DICE_1D20);
    newchar->karma = diceroll(DICE_1D20);
    newchar->stam = diceroll(DICE_1D20);
    newchar->magic = diceroll(DICE_1D20);
    newchar->code = diceroll(DICE_1D20);
    newchar->crypt = diceroll(DICE_1D20);
    newchar->reveng = diceroll(DICE_1D20);
    newchar->ice = diceroll(DICE_1D20);
    newchar->mart = diceroll(DICE_1D20);

    return newchar;
};

