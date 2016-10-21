#include <kern/conf.h>
#include <zero/param.h>
#include <kern/cpu.h>
#include <kern/mem/vm.h>
#if (SMP)
#include <stdint.h>
#include <kern/unit/ia32/mp.h>
#else
#include <kern/unit/x86/boot.h>
#endif

