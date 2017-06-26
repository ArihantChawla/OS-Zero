#ifndef __ZERO_RCU_H__
#define __ZERO_RCU_H__

void rcusetptr(src, dest);
ptr  rcuderef(src);
/* rcureadlk() and rcureadunlk() never spin or block */
void rcureadlk();
void rcureadunlk();

/*
 * rcureadlk();
 * p = rcuderef(ptr);
 * if (p) {
 *     dostuff(p->a, p->b, p->c);
 * }
 * rcureadunlk();
 */

/*
 * list routines example
 * publish              retract         subscribe               wait
 * -------              -------         ---------               ----
 * rcusetptr()          rcusetptr(NULL) rcuderef()              rcusync()
 * listaddrcu()         listdelrcu()    listforeach()
 */

/* rcusync(): cpuforeach(sys) run(cpu) */
/* rcu classic readers are not allowed to sleep or block */

#endif /* __ZERO_RCU_H__ */

