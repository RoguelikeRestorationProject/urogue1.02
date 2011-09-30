/*
    rings.c  -  routines dealing specifically with rings
   
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

#include <stdlib.h>
#include <string.h>
#include "curses.h"
#include "rogue.h"

void
ring_on(void)
{
    register struct object *obj;
    register struct linked_list *item;
    register int ring;
    char buf[LINELEN];

    item = get_item("put on", RING);
    /*
     * Make certain that it is somethings that we want to wear
     */
    if (item == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != RING) {
	msg("You can't put that on!");
	return;
    }

    /*
     * find out which hand to put it on
     */
    if (is_current(obj))
	return;

    if      (cur_ring[LEFT_1] == NULL) ring = LEFT_1;
    else if (cur_ring[LEFT_2] == NULL) ring = LEFT_2;
    else if (cur_ring[LEFT_3] == NULL) ring = LEFT_3;
    else if (cur_ring[LEFT_4] == NULL) ring = LEFT_4;
    else if (cur_ring[RIGHT_1] == NULL) ring = RIGHT_1;
    else if (cur_ring[RIGHT_2] == NULL) ring = RIGHT_2;
    else if (cur_ring[RIGHT_3] == NULL) ring = RIGHT_3;
    else if (cur_ring[RIGHT_4] == NULL) ring = RIGHT_4;
    else
    {
	if (terse) 
	    msg("Wearing enough rings.");
	else
	    msg("You already have on eight rings.");
	return;
    }
    cur_ring[ring] = obj;

    /*
     * Calculate the effect it has on the poor guy.
     */
    switch (obj->o_which)
    {
	when R_ADDSTR:
	    pstats.s_str += obj->o_ac;
	when R_ADDHIT:
	    pstats.s_dext += obj->o_ac;
	when R_ADDINTEL:
	    pstats.s_intel += obj->o_ac;
	when R_ADDWISDOM:
	    pstats.s_wisdom += obj->o_ac;
	when R_SEEINVIS:
	    if (off(player, PERMBLIND)) {
		turn_on(player, CANSEE);
		msg("Your eyes begin to tingle.");
		sight();
		light(&hero);
		mvwaddch(cw, hero.y, hero.x, PLAYER);
	    }
	when R_AGGR:
	    aggravate();
	when R_CARRYING:
	    updpack(FALSE);
	when R_LEVITATION:
	    msg("You begin to float in the air!");
	when R_LIGHT: {
		if(roomin(&hero) != NULL) {
			light(&hero);
			mvwaddch(cw, hero.y, hero.x, PLAYER);
		}
	    }
    }
    status(FALSE);
    if (r_know[obj->o_which] && r_guess[obj->o_which])
    {
	free(r_guess[obj->o_which]);
	r_guess[obj->o_which] = NULL;
    }
    else if (!r_know[obj->o_which] && 
	     askme && 
	     (obj->o_flags & ISKNOW) == 0 &&
	     r_guess[obj->o_which] == NULL) {
	mpos = 0;
	msg(terse ? "Call it: " : "What do you want to call it? ");
	if (get_str(buf, msgw) == NORM)
	{
	    r_guess[obj->o_which] = new(strlen(buf) + 1);
	    strcpy(r_guess[obj->o_which], buf);
	}
    }
	msg("");
}

void
ring_off(void)
{
    register struct object *obj;
    register struct linked_list *item;

    if (cur_ring[LEFT_1] == NULL && cur_ring[LEFT_2] == NULL &&
	cur_ring[LEFT_3] == NULL && cur_ring[LEFT_4] == NULL &&
	cur_ring[RIGHT_1] == NULL && cur_ring[RIGHT_2] == NULL &&
	cur_ring[RIGHT_3] == NULL && cur_ring[RIGHT_4] == NULL) {
	if (terse)
	    msg("No rings.");
	else
	    msg("You aren't wearing any rings.");
	return;
    }
    else
	if ((item = get_item("Remove", RING)) == NULL)
	    return;
    mpos = 0;
    obj = OBJPTR(item);
    if (obj == NULL) {
	msg("%s wearing such a ring.", terse ? "Not" : "You are not");
	return;
    }
    if (obj->o_which == R_LEVITATION)
	msg("You float gently to the ground.");
    if (dropcheck(obj))
	msg("Was wearing %s.", inv_name(obj, TRUE));
    updpack(FALSE);
}

/*
 * how much food does this ring use up?
 */
int
ring_eat(int hand)
{
    if (cur_ring[hand] == NULL)
	return 0;
    switch (cur_ring[hand]->o_which) {
	case R_REGEN:
	case R_VREGEN:
	    return rnd(pstats.s_lvl > 10 ? 10 : pstats.s_lvl);
	case R_ALERT:
	case R_SUSABILITY:
	    return 1;
	case R_SEARCH:
	    return (rnd(100) < 33);
	case R_DIGEST: {
	    register int ac = cur_ring[hand]->o_ac;

	    if (ac < 0 && rnd(1 - (ac/3)) == 0)
		return (-ac + 1);
	    else if (rnd((ac/2) + 2) == 0) {
		return (-1 - ac);
	   }
	}
    }
    return 0;
}

/*
 * print ring bonuses
 */
char *
ring_num(struct object *obj)
{
    static char buf[5];

    if (!(obj->o_flags & ISKNOW))
	return "";
    switch (obj->o_which)
    {
	when R_PROTECT:
	case R_ADDSTR:
	case R_ADDDAM:
	case R_ADDHIT:
	case R_ADDINTEL:
	case R_ADDWISDOM:
	case R_CARRYING:
	case R_VREGEN:
	case R_RESURRECT:
	case R_TELCONTROL:
	    buf[0] = ' ';
	    strcpy(&buf[1], num(obj->o_ac, 0));
	when R_DIGEST:
	    buf[0] = ' ';
	    strcpy(&buf[1], num(obj->o_ac < 0 ? obj->o_ac : obj->o_ac - 1, 0));
	when R_AGGR:
	case R_LIGHT:
	case R_TELEPORT:
	    if (obj->o_flags & ISCURSED)
		return " cursed";
	    else
		return "";
	otherwise:
	    return "";
    }
    return buf;
}


/* Return the effect of the specified ring */

int
ring_value(int type)
{
    int result = 0;

    if (ISRING(LEFT_1, type))  result += cur_ring[LEFT_1]->o_ac;
    if (ISRING(LEFT_2, type)) result += cur_ring[LEFT_2]->o_ac;
    if (ISRING(LEFT_3, type)) result += cur_ring[LEFT_3]->o_ac;
    if (ISRING(LEFT_4, type)) result += cur_ring[LEFT_4]->o_ac;
    if (ISRING(RIGHT_1, type)) result += cur_ring[RIGHT_1]->o_ac;
    if (ISRING(RIGHT_2, type))  result += cur_ring[RIGHT_2]->o_ac;
    if (ISRING(RIGHT_3, type)) result += cur_ring[RIGHT_3]->o_ac;
    if (ISRING(RIGHT_4, type))  result += cur_ring[RIGHT_4]->o_ac;
    return(result);
}
