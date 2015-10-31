#ifndef __KERN_PROC_KERN_H__
#define __KERN_PROC_KERN_H__

#include <kern/proc/proc.h>

/* system-assigned process IDs */
#define PROCKERN     0  // main kernel; context switches, system calls, ...
#define PROCINIT     1  // init process; the mother of all processes
#define PROCBUFD     2  // buffer daemon; HID, audio, video, net, disk
#define PROCEVD      3  // event daemon; manage events such as user input ones
#define PROCPAGED    4  // page daemon; page replacement; aging, LRU-queue
#define PROCIDLE     5  // idle process; zeroes memory etc.
#define TASKNPREDEF  6

/*
 * init.c
 * - init process; traditional SysV-style, runlevels
 */
struct idletask {
    struct physpage *zeroqueue;
    struct proc     *termqueue;
    struct proc     *dumpqueue;
    struct physpage *pageoutqueue;
    struct physpage *pageinqueue;
};

#endif /* __KERN_PROC_KERN_H__ */

