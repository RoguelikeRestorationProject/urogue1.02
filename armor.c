/*
    armor.c  -  This file contains misc functions for dealing with armor
   
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

#include <curses.h>
#include "rogue.h"

/*
 * wear:
 *	The player wants to wear something, so let him/her put it on.
 */
void
wear(void)
{
    register struct linked_list *item;
    register struct object *obj;

    if (cur_armor != NULL)
    {
	addmsg("You are already wearing some");
	if (!terse)
	    addmsg(".  You'll have to take it off first");
	addmsg("!");
	endmsg();
	after = FALSE;
	return;
    }

    /* What does player want to wear? */
    if ((item = get_item("wear", ARMOR)) == NULL)
	return;

    obj = (struct object *) ldata(item);
    if (obj->o_type != ARMOR) {
	 msg("You can't wear that!");
	 return;
    }
    waste_time();
    addmsg(terse ? "W" : "You are now w");
    msg("earing %s.", armors[obj->o_which].a_name);
    cur_armor = obj;
    obj->o_flags |= ISKNOW;
}

/*
 * take_off:
 *	Get the armor off of the players back
 */
void
take_off(void)
{
    register struct object *obj;

    if ((obj = cur_armor) == NULL)
    {
	msg("%s wearing armor!", terse ? "Not" : "You aren't");
	return;
    }
    if (!dropcheck(cur_armor))
	return;
    cur_armor = NULL;
    addmsg(terse ? "Was" : "You used to be");
    msg(" wearing %c) %s.", pack_char(obj), inv_name(obj, TRUE));
	if (on(player, STUMBLER)) {
		msg("Your foot feels a lot better now.");
		turn_off(player, STUMBLER);
	}
}

/*
 * waste_time:
 *	Do nothing but let other things happen
 */
void
waste_time(void)
{
    if (inwhgt)			/* if from wghtchk then done */
	return;
    do_daemons(BEFORE);
    do_fuses(BEFORE);
    do_daemons(AFTER);
    do_fuses(AFTER);
}
