#ifndef __ZPU_CONF_H__
#define __ZPU_CONF_H__

#define ZPUIREGSIZE 4   // integer register size in bytes        
#define ZPURAT      1   // enable or disable rational operations
#define ZPUSIMD     1   // enable or disable SIMD operations

#define ZPUCORESIZE (512 * 1024 * 1024) // emulator memory size

#define OPNOP    OP_NOP
#define ARGNONE  ARG_NONE
#define ARGINDIR ARG_INDIR
#define ARGINDEX ARG_INDEX
#define ARGIMMED ARG_IMMED
#define ARGADR   ARG_ADR
#define ARGREG   ARG_REG
#define reg1     src
#define reg2     dest
#define arg1t    sflg
#define arg2t    dflg
#define size     argsz

#endif /* __ZPU_CONF_H__ */

