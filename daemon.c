/*
    daemon.c  - Contains functions for dealing with things that happen in the future.
   
    UltraRogue
    Copyright (C) 1985 Herb Chong
    All rights reserved.
    
    Based on "Advanced Rogue"
    Copyright (C) 1982, 1984, 1985 Michael Morgan, Ken Dalka and AT&T
    All rights reserved.

    Based on "Rogue: Exploring the Dungeons of Doom"
    Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
    All rights reserved.
    
    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include "curses.h"
#include "rogue.h"

#define EMPTY 0
#define DAEMON -1

#define _X_ { EMPTY }

struct delayed_action d_list[MAXDAEMONS] = {
	_X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
	_X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
	_X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
	_X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
	_X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
	_X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
};
int demoncnt = 0;	/* number of active daemons */


/*
 * d_slot:
 *	Find an empty slot in the daemon/fuse list
 */
struct delayed_action *
d_slot()
{
	reg int i;
	reg struct delayed_action *dev;

	for (i = 0, dev = d_list; i < MAXDAEMONS; i++, dev++)
		if (dev->d_type == EMPTY)
			return dev;
	return NULL;
}


/*
 * find_slot:
 *	Find a particular slot in the table
 */
struct delayed_action *
find_slot(void (*func)())
{
	reg int i;
	reg struct delayed_action *dev;

	for (i = 0, dev = d_list; i < MAXDAEMONS; i++, dev++)
		if (dev->d_type != EMPTY && func == dev->d_func)
			return dev;
	return NULL;
}


/*
 * daemon:
 *	Start a daemon, takes a function.
 */
void
start_daemon(void (*func)(), void *arg, int type)
{
	reg struct delayed_action *dev;

	dev = d_slot();
	if (dev != NULL) {
		dev->d_type = type;
		dev->d_func = func;
		dev->d_arg = arg;
		dev->d_time = DAEMON;
		demoncnt += 1;			/* update count */
	}
}


/*
 * kill_daemon:
 *	Remove a daemon from the list
 */
void
kill_daemon(void (*func)())
{
	reg struct delayed_action *dev;

	if ((dev = find_slot(func)) == NULL)
		return;
	/*
	 * Take it out of the list
	 */
	dev->d_type = EMPTY;
	demoncnt -= 1;			/* update count */
}


/*
 * do_daemons:
 *	Run all the daemons that are active with the current flag,
 *	passing the argument to the function.
 */
void
do_daemons(int flag)
{
	reg struct delayed_action *dev;

	/*
	 * Loop through the devil list
	 */
	for (dev = d_list; dev <= &d_list[MAXDAEMONS-1]; dev++)
	/*
	 * Executing each one, giving it the proper arguments
	 */
		if (dev->d_type == flag && dev->d_time == DAEMON)
			(*dev->d_func)(dev->d_arg);
}


/*
 * fuse:
 *	Start a fuse to go off in a certain number of turns
 */
void
fuse(void (*func)(), void *arg, int time, int type)
{
	reg struct delayed_action *wire;

	wire = d_slot();
	if (wire != NULL) {
		wire->d_type = type;
		wire->d_func = func;
		wire->d_arg = arg;
		wire->d_time = time;
		demoncnt += 1;			/* update count */
	}
}


/*
 * lengthen:
 *	Increase the time until a fuse goes off
 */
void
lengthen(void (*func)(), int xtime)
{
	reg struct delayed_action *wire;

	if ((wire = find_slot(func)) == NULL)
		return;
	wire->d_time += xtime;
}


/*
 * extinguish:
 *	Put out a fuse
 */
void
extinguish(void (*func)())
{
	reg struct delayed_action *wire;

	if ((wire = find_slot(func)) == NULL)
		return;
	wire->d_type = EMPTY;
	demoncnt -= 1;
}


/*
 * do_fuses:
 *	Decrement counters and start needed fuses
 */
void
do_fuses(int flag)
{
	reg struct delayed_action *wire;

	/*
	 * Step though the list
	 */
	for (wire = d_list; wire <= &d_list[MAXDAEMONS-1]; wire++) {
	/*
	 * Decrementing counters and starting things we want.  We also need
	 * to remove the fuse from the list once it has gone off.
	 */
	    if(flag == wire->d_type && wire->d_time > 0	&&
	      --wire->d_time == 0) {
		wire->d_type = EMPTY;
		(*wire->d_func)(wire->d_arg);
		demoncnt -= 1;
	    }
	}
}


/*
 * activity:
 *	Show wizard number of daemons and memory blocks used
 */
void
activity()
{
	msg("Daemons = %d : Memory Items = %d : Memory Used = %ld",
	    demoncnt,total,md_memused());
}
