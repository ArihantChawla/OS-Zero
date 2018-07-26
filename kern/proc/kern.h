#ifndef __KERN_PROC_KERN_H__
#define __KERN_PROC_KERN_H__

#include <kern/proc/proc.h>

/* system-assigned process IDs */
#define TASKKERN     0  // main kernel; context switches, system calls, ...
#define TASKINIT     1  // init process; the mother of all processes
#define TASKEVD      2  // event daemon; manage events such as user input ones
#define TASKBUFD     3  // buffer daemon; HID, audio, video, net, disk
#define TASKPAGED    4  // page daemon; page replacement; aging, LRU-queue
#define TASKIDLE     5  // idle process; maintenance tasks
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

