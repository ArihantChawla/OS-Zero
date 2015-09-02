#ifndef __ZVC_CAL_H__
#define __ZVC_CAL_H__

#include <zero/trix.h>

#define CAL_NONE          0
#define CAL_NAME_TOKEN    0x01
#define CAL_MICRO_TOKEN   0x02
#define CAL_NEWLINE_TOKEN 0x03

#define calisname(c) (bitset(calnametab, (c)))
extern uint8_t       calnametab[32];

struct caltoken {
    long             type;
    long             parm;
    char            *str;
    struct caltoken *prev;
    struct caltoken *next;
};

struct caltokenq {
    struct caltoken *head;
    struct caltoken *tail;
};

/*
 * instruction formats
 * -------------------
 * 1. 1-parcel instruction with discrete j and k fields
 * - arithmetic, logical, double shift, floating-point constant
 * - g and h fields define opcode
 * - i is result register
 * - j and k designate operand register
 * 2. 1-parcel instruction with combined j and k fields
 * - i and j
 *   - branch instruction 005
 *   - constant
 *   - B and T register block memory transfer
 *   - B and T register data transfer
 *   - single shift
 *   - mask
 * - g and h fields contain opcode
 * - i field is destination register
 * - j and k have a constant or B or T register ID
 * 3. 2-parcel instruction with combined j, k, and m fields
 * - transfer 22-bit jkm constant to an A or S register
 * - scalar memory transfers use 22-bit jkm address displacement
 *   - g is opcode
 *   - h is address index register
 *   - i is source or result register
 *   - jkm is address displacement
 * 4. 2-parcel instruction with combined i, j, k, and m fields
 *   - g and h define opcode
 *   - high-order bit of i is unused
 *   - the rest of i, j, k, and m contain a 24-bit address that allows branching
 *     to an instruction parcel; the low 2 bits of m are parcel select
 *   - special register values
 *     - if S0 and A0 are referenced in the j or k fields of certain
 *       instructions, the contents of the respective register are not used;
 *       instead, a special operand is generated. The special value is available
 *       regardless of existing A0 or S0 reservations; this use doesn't alter
 *       the value of A0 or S0. if S0 or A0 is used in the i field as the
 *       operand, the actual value of the register is provided.
 *       - Ah, h=0 -> 0
 *       - Ai, i=0 -> (A0)
 *       - Aj, j=0 -> 0
 *       - Ak, k=0 -> 1
 *       - Si, i=0 -> (S0)
 *       - Sj, j=0 -> 0
 *       - Sk, k=0 -> 2**63
 */
struct calinst {
    unsigned g: 4;
    unsigned h: 3;
    unsigned i: 3;
    unsigned j: 3;
    unsigned k: 3;
};

/* 1-parcel instruction with discrete j and k fields */
struct calinst1 {
    unsigned op:   7;
    unsigned res:  3;
    unsigned arg1: 3;
    unsigned arg2: 3;
};

/* 2. 1-parcel instruction with combined j and k fields */
struct calinst2 {
    unsigned op:   7;
    unsigned res:  3;
    unsigned arg:  6;
};

/* 2-parcel instruction with combined j, k, and m fields */
struct calinst3 {
    unsigned op:   7;
    unsigned res:  3;
    unsigned val: 22;
};

#endif /* __ZVC_CAL_H__ */

