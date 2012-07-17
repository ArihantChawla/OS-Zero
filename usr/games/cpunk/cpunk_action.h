#ifndef __CPUNK_ACTION_H__
#define __CPUNK_ACTION_H__

#include <cpunk_combat.h>

#define CPUNK_NO_ACTION        0        // do nothing
#define CPUNK_ACTION_USE       1        // use an object from inventory
#define CPUNK_ACTION_LOOK      2        // look (at room or object)
#define CPUNK_ACTION_TOUCH     3        // touch an object
#define CPUNK_ACTION_MAP       4        // look at the level map
#define CPUNK_ACTION_OPEN      5        // open (door, ...)
#define CPUNK_ACTION_CLOSE     6        // close (door, ...)
#define CPUNK_ACTION_PICK      7        // pick an item up
#define CPUNK_ACTION_DROP      8
#define CPUNK_ACTION_INVENTORY 9        // see your possessions
#define CPUNK_ACTION_LOAD      10       // load code into VM
#define CPUNK_ACTION_EDIT      11       // edit code
#define CPUNK_ACTION_SAVE      12       // save code
#define CPUNK_ACTION_DUMP      13       // dump memory footprint
#define CPUNK_ACTION_COMBAT    14       // combat with others

#endif /* __CPUNK_ACTION_H__ */

