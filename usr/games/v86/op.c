#include <stdint.h>

struct v86vmop v86optab[V86_OPERATIONS]
= {
    { 0, NULL },        // V86_NOP
    { 1, v86not },      // V86_NOT
    { 2, v86and },      // V86_AND
    { 2, v86or },       // V86_OR
    { 2, v86xor },      // V86_XOR
    { 2, v86shl },      // V86_SHL
    { 2, v86shr },      // V86_SHR
    { 2, v86shr },      // V86_SAR
    { 2, v86add },      // V86_ADD
    { 2, v86sub },      // V86_SUB
    { 2, v86cmp },      // V86_CMP
    { 2, v86mul },      // V86_MUL
    { 2, v86div },      // V86_DIV
    { 1, v86jmp },      // V86_JMP
    { 1, v86ljmp },     // V86_LJMP
    { 1, v86jz },       // V86_JZ
    { 1, v86nz },       // V86_JNZ
    { 1, v86jc },       // V86_JC
    { 1, v86jnc },      // V86_JNC
    { 1, v86jo },       // V86_JO
    { 1, v86jno },      // V86_JNO
    { 1, v86jle },      // V86_JLE
    { 1, v86jgt },      // V86_JGT
    { 1, v86jge },      // V86_JGE
    { 1, v86call },     // V86_CALL
    { 1, v86ret },      // V86_RET
    { 2, v86ldr },      // V86_LDR
    { 2, v86str },      // V86_STR
    { 1, v86push },     // V86_PUSH
    { 0, v86pusha },    // V86_PUSHA
    { 1, v86pop },      // V86_POP
    { 0, v86popa },     // V86_POPA
    { 2, v86in },       // V86_IN
    { 2, v86out },      // V86_OUT
    { 2, v86hlt }       // V86_HLT
};

