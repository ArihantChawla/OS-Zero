/*
 * register.h - x86 register interface interface for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_INTERNAL_REGISTER_H
#define EARTHQUAKE_INTERNAL_REGISTER_H

#define X86_EAX_INTEGER_REGISTER    0
#define X86_EBX_INTEGER_REGISTER    1
#define X86_ECX_INTEGER_REGISTER    2
#define X86_EDX_INTEGER_REGISTER    3
#define X86_ESI_INTEGER_REGISTER    4
#define X86_EDI_INTEGER_REGISTER    5
#define X86_EBP_INTEGER_REGISTER    6
#define X86_ESP_INTEGER_REGISTER    7
#define X86_INTEGER_REGISTERS       8

#define X86_CS_SEGMENT_REGISTER     0
#define X86_DS_SEGMENT_REGISTER     1
#define X86_SS_SEGMENT_REGISTER     2
#define X86_ES_SEGMENT_REGISTER     3
#define X86_FS_SEGMENT_REGISTER     4
#define X86_GS_SEGMENT_REGISTER     5
#define X86_SEGMENT_REGISTERS       6

/*
 * TODO: Missing some in here?
 */
#define X86_EFLAGS_SPECIAL_REGISTER 0
#define X86_EIP_SPECIAL_REGISTER    1
#define X86_CR0_SPECIAL_REGISTER    2
#define X86_CR1_SPECIAL_REGISTER    3
#define X86_CR2_SPECIAL_REGISTER    4
#define X86_CR3_SPECIAL_REGISTER    5
#define X86_CR4_SPECIAL_REGISTER    6
#define X86_MXCSR_SPECIAL_REGISTER  7
#define X86_SPECIAL_REGISTERS       8

#define X86_MM0_MMX_REGISTER        0
#define X86_MM1_MMX_REGISTER        1
#define X86_MM2_MMX_REGISTER        2
#define X86_MM3_MMX_REGISTER        3
#define X86_MM4_MMX_REGISTER        4
#define X86_MM5_MMX_REGISTER        5
#define X86_MM6_MMX_REGISTER        6
#define X86_MM7_MMX_REGISTER        7
#define X86_MMX_REGISTERS           8

#define X86_FPU_DATA_REGISTERS      8
#define X86_FPU_SPECIAL_REGISTERS   6

#define X86_XMM0_XMM_REGISTER       0
#define X86_XMM1_XMM_REGISTER       1
#define X86_XMM2_XMM_REGISTER       2
#define X86_XMM3_XMM_REGISTER       3
#define X86_XMM4_XMM_REGISTER       4
#define X86_XMM5_XMM_REGISTER       5
#define X86_XMM6_XMM_REGISTER       6
#define X86_XMM7_XMM_REGISTER       7
#define X86_XMM_REGISTERS           8

/*
 * Internal flags structure for register allocation etc.
 */
struct Ex86registerflags {
    uint8_t integer[X86_INTEGER_REGISTERS];
    uint8_t segment[X86_SEGMENT_REGISTERS];
    uint8_t special[X86_SPECIAL_REGISTERS];
    uint8_t mmx[X86_MMX_REGISTERS];
    uint8_t fpudata[X86_FPU_DATA_REGISTERS];
    uint8_t fpuspecial[X86_FPU_SPECIAL_REGISTERS];
    uint8_t xmm[X86_XMM_REGISTERS];
};

struct Ex86registers {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;

    uint32_t cs;
    uint32_t ds;
    uint32_t ss;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;

    uint32_t eflags;
    uint32_t eip;
    uint32_t cr0;
    uint32_t cr1;
    uint32_t cr2;
    uint32_t cr3;
    uint32_t cr4;
    uint32_t mxcsr;

    uint64_t mm0;
    uint64_t mm1;
    uint64_t mm2;
    uint64_t mm3;
    uint64_t mm4;
    uint64_t mm5;
    uint64_t mm6;
    uint64_t mm7;

    f128_t xmm0;
    f128_t xmm1;
    f128_t xmm2;
    f128_t xmm3;
    f128_t xmm4;
    f128_t xmm5;
    f128_t xmm6;
    f128_t xmm7;
};

#endif /* EARTHQUAKE_INTERNAL_REGISTER_H */

