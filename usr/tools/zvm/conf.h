#ifndef __ZVM_CONF_H__
#define __ZVM_CONF_H__

/*
 * GUI configuration
 * -----------------
 * ZVMEFL    - use Enlightenment Foundation Libraries; Edje, Evas, Ecore,
 *             Elementary etc.
 * ZVMXORG   - use Xorg window port - HAZARD: not implemented yet 
 * ZVMXCB    - use X C Bindings window port - HAZARD: not implemented yet 
 */
#ifndef ZVMEFL
#define ZVMEFL     0
#endif
#ifndef ZVMXORG
#define ZVMXORG    0
#endif
#ifndef ZVMXCB
#define ZVMXCB     0
#endif

/* FIXME: virtual memory hasn't been implemented yet */
#ifndef ZVMVIRTMEM
#define ZVMVIRTMEM 0
#endif

#ifndef ZAS32BIT
#define ZAS32BIT   1
#endif
#ifndef ZASALIGN
#define ZASALIGN   1
#endif

#endif /* __ZVM_CONF_H__ */

