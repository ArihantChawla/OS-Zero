#ifndef __CPUNK_SPELL_H__
#define __CPUNK_SPELL_H__

#define CPUNK_NO_SPELL        0
#define CPUNK_SPELL_HEAL      1
#define CPUNK_SPELL_FREEZE    2
#define CPUNK_SPELL_RESURRECT 3
#define CPUNK_SPELL_LIGHTNING 4
#define CPUNK_SPELL_SMOKE     5
#define CPUNK_SPELL_TELEPORT  6
#define CPUNK_NSPELL          7
struct cpunkspell {
    long type;
    long power;
};

#endif /* __CPUNK_SPELL_H__ */

