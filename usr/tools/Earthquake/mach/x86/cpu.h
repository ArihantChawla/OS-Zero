/*
 * cpu.h - x86 CPU interface for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_ARCH_X86_CPU_H
#define EARTHQUAKE_ARCH_X86_CPU_H

/*
 * CPU types.
 */
#define EARTHQUAKE_386_CPU      0
#define EARTHQUAKE_486_CPU      1
#define EARTHQUAKE_PENTIUM_CPU  2
#define EARTHQUAKE_PENTIUM2_CPU 3
#define EARTHQUAKE_PENTIUM3_CPU 4
#define EARTHQUAKE_PENTIUM4_CPU 5

/* TODO: add amd.h, cyrix.h, transmeta.h, etc. */

/*
 * CPU flags.
 */
#define EARTHQUAKE_X86_MMX_INSTRUCTIONS   1U
#define EARTHQUAKE_X86_SSE_INSTRUCTIONS   (1U << 1)
#define EARTHQUAKE_X86_SSE2_INSTRUCTIONS  (1U << 2)
#define EARTHQUAKE_X86_3DNOW_INSTRUCTIONS (1U << 3)

struct Ex86cpu {
    cpuuword_t type;
    cpuuword_t flags;
};

#endif /* EARTHQUAKE_ARCH_X86_CPU_H */

