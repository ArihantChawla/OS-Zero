/*
 * main.h - main header file for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_MAIN_H
#define EARTHQUAKE_MAIN_H

//#include <Earthquake/version.h>
#include <compile.h>
#include <debug.h>
#include <constants.h>
#include <macros.h>
#include <protos.h>
#include <util.h>

#if (CPU_X86)
#   include <arch/x86/main.h>
#endif

#include <internal/main.h>

#endif /* EARTHQUAKE_MAIN_H */

