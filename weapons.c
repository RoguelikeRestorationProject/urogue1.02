/*
    weapons.c  -  Functions for dealing with problems brought about by weapons
   
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

#include <string.h>
#include "curses.h"
#include <ctype.h>
#include "rogue.h"

/*
 * missile:
 *	Fire a missile in a given direction
 */
void
missile(int ydelta, int xdelta, struct linked_list *item, struct thing *tp)
{
	register struct object *obj ;
	register struct linked_list *nitem ;

	/*
	* Get which thing we are hurling
	*/
	if (item == NULL) {
		return ;
	}
	obj = (struct object *) ldata(item) ;
	if (!dropcheck(obj) || is_current(obj)) {
		return ;
	}
	/*
	* Get rid of the thing. If it is a non-multiple item object, or
	* if it is the last thing, just drop it. Otherwise, create a new
	* item with a count of one.
	*/
	if (obj->o_count < 2) {
		detach(tp->t_pack, item) ;
		if (tp->t_pack == pack) {
			inpack-- ;
			freeletter(item);
		}
	} else {
		obj->o_count-- ;
		nitem = (struct linked_list *) new_item(sizeof *obj) ;
		obj = (struct object *) ldata(nitem) ;
		*obj = *((struct object *) ldata(item)) ;
		obj->o_count = 1 ;
		item = nitem ;
	}
	if (obj->o_type == ARTIFACT)
		has_artifact &= ~(1 << obj->o_which);
	if (obj->o_type == SCROLL && obj->o_which == S_SCARE) {
		if (obj->o_flags & ISBLESSED)
		    obj->o_flags &= ~ISBLESSED;
		else 
		    obj->o_flags |= ISCURSED;
	}
	updpack (FALSE);
	do_motion(obj, ydelta, xdelta, tp) ;
	/*
	* AHA! Here it has hit something. If it is a wall or a door,
	* or if it misses (combat) the monster, put it on the floor
	*/
	if (!hit_monster(unc(obj->o_pos), obj, tp)) {
		if (obj->o_type == WEAPON && obj->o_which == GRENADE) {
			register struct room *rp;
            static coord fpos;

            msg("BOOOM!");
            aggravate();
            rp = roomin(&obj->o_pos);
            if (ntraps + 1 < MAXTRAPS + MAXTRAPS &&
					fallpos(&obj->o_pos, &fpos, TRUE)) {
				mvaddch(fpos.y, fpos.x, TRAPDOOR);
                traps[ntraps].tr_type = TRAPDOOR;
                traps[ntraps].tr_flags = ISFOUND;
                traps[ntraps].tr_show = TRAPDOOR;
                traps[ntraps].tr_pos.y = fpos.y;
                traps[ntraps++].tr_pos.x = fpos.x;
                light(&hero);
            }
            discard(item);
        }
	    else if (obj->o_flags & ISLOST) {
		if (obj->o_type == WEAPON)
		    addmsg("The %s", weaps[obj->o_which].w_name);
		else
		    addmsg(inv_name(obj, TRUE));
		msg(" vanishes in a puff of greasy smoke.");
		discard(item);
	    }
	    else {
		if (fall(item, TRUE))
		if (obj->o_flags & CANRETURN)
		    msg("You have %s.", inv_name(obj, TRUE));
	    }
	}
	else if (obj->o_flags & ISOWNED) {
		add_pack(item, TRUE);
		msg("You have %s.", inv_name(obj, TRUE));
	}
	mvwaddch(cw, hero.y, hero.x, PLAYER) ;
}

/*
 * do the actual motion on the screen done by an object traveling
 * across the room
 */
void
do_motion(struct object *obj, int ydelta, int xdelta, struct thing *tp)
{

	/*
	* Come fly with us ...
	*/
	obj->o_pos = tp->t_pos ;
	for ( ; ;) {
		register int ch ;
		/*
		* Erase the old one
		*/
		if (!ce(obj->o_pos, tp->t_pos) &&
		    cansee(unc(obj->o_pos)) &&
		    mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ') {
			mvwaddch(cw, obj->o_pos.y, obj->o_pos.x, show(obj->o_pos.y, obj->o_pos.x)) ;
		}
		/*
		* Get the new position
		*/
		obj->o_pos.y += ydelta ;
		obj->o_pos.x += xdelta ;
		if (shoot_ok(ch = winat(obj->o_pos.y, obj->o_pos.x)) && ch != DOOR && !ce(obj->o_pos, hero)) {
			/*
			* It hasn't hit anything yet, so display it
			* If it alright.
			*/
			if (cansee(unc(obj->o_pos)) &&
			    mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ') {
				mvwaddch(cw, obj->o_pos.y, obj->o_pos.x, obj->o_type) ;
				draw(cw) ;
			}
			continue ;
		}
		/* don't want objects to end up in wall, or fall through
		 * so back it up.
		 */
		if (ch == '|' || ch == '-' || ch == SECRETDOOR || ch == ' ')
			{
			obj->o_pos.y -= ydelta ;
			obj->o_pos.x -= xdelta ;
			}
			
		break ;
	}
}

/*
 * fall:
 *	Drop an item someplace around here.
 */
int
fall(struct linked_list *item, int pr)
{
	register struct object *obj ;
	register struct room *rp ;
	static coord fpos ;
	int ret;

	obj = (struct object *) ldata(item) ;
	rp = roomin(&hero);
	if (obj->o_flags & CANRETURN) {
	    add_pack(item, TRUE);
	    return TRUE;
	}
	else if (fallpos(&obj->o_pos, &fpos, TRUE)) {
		if (obj->o_flags & CANBURN && ntraps + 1 < MAXTRAPS + MAXTRAPS) {
		    mvaddch(fpos.y, fpos.x, FIRETRAP);
		    traps[ntraps].tr_type = FIRETRAP;
		    traps[ntraps].tr_flags = ISFOUND;
		    traps[ntraps].tr_show = FIRETRAP;
		    traps[ntraps].tr_pos.y = fpos.y;
		    traps[ntraps++].tr_pos.x = fpos.x;
		    if (rp != NULL)
			rp->r_flags &= ~ISDARK;
			discard(item);
			ret = FALSE;
		}
		else {
		    mvaddch(fpos.y, fpos.x, obj->o_type) ;
		    obj->o_pos = fpos ;
		    attach(lvl_obj, item) ;
			ret = TRUE;
		}
		if (rp != NULL &&
		    (!(rp->r_flags & ISDARK) ||
		    (rp->r_flags & HASFIRE))) {
			light(&hero) ;
			mvwaddch(cw, hero.y, hero.x, PLAYER) ;
		}
		return ret;
	}
	if (pr) {
		if (obj->o_type == WEAPON)
			addmsg("The %s", weaps[obj->o_which]);
		else
			addmsg(inv_name(obj, TRUE));
		msg(" vanishes as it hits the ground.");
	}
	discard(item) ;
	return FALSE;
}

/*
 * init_weapon:
 *	Set up the initial goodies for a weapon
 */
void
init_weapon(struct object *weap, int type)
{
	register struct init_weps *iwp ;

	iwp = &weaps[type] ;
	strcpy(weap->o_damage,iwp->w_dam);
	strcpy(weap->o_hurldmg,iwp->w_hrl) ;
	weap->o_launch = iwp->w_launch ;
	weap->o_flags = iwp->w_flags ;
	weap->o_weight = iwp->w_wght;
	if (weap->o_flags & ISMANY) {
		weap->o_count = rnd(8) + 8 ;
		weap->o_group = newgrp() ;
	} else {
		weap->o_count = 1 ;
	}
}

/*
 * Does the missile hit the monster
 */
int
hit_monster(int y, int x, struct object *obj, struct thing *tp)
{
	static coord mp ;

	mp.y = y ;
	mp.x = x ;
	if (tp == &player) {
		/* Make sure there is a monster where it landed */
		if (!isalpha(mvwinch(mw, y, x))) {
			return(FALSE) ;
		}
		return(fight(&mp, obj, TRUE)) ;
	} else {
		if (!ce(mp, hero)) {
			return(FALSE) ;
		}
		return(attack(tp, obj, TRUE)) ;
	}
}

/*
 * num:
 *	Figure out the plus number for armor/weapons
 */

char *num(int n1, int n2)
{
	static char numbuf[LINELEN] ;

	if (n1 == 0 && n2 == 0) {
		return "+0" ;
	}
	if (n2 == 0) {
		sprintf(numbuf, "%s%d", n1 < 0 ? "" : "+", n1) ;
	} else {
		sprintf(numbuf, "%s%d, %s%d", n1 < 0 ? "" : "+", n1, n2 < 0 ? "" : "+", n2) ;
	}
	return(numbuf) ;
}

/*
 * wield:
 *	Pull out a certain weapon
 */
void
wield(void)
{
	register struct linked_list *item ;
	register struct object *obj, *oweapon ;

	oweapon = cur_weapon ;
	if (!dropcheck(cur_weapon)) {
		cur_weapon = oweapon ;
		return ;
	}
	cur_weapon = oweapon ;
	if ((item = get_item("wield", WEAPON)) == NULL) {
		after = FALSE ;
		return ;
	}
	obj = (struct object *) ldata(item) ;
	if (obj->o_type != WEAPON) {
	    msg ("You can't wield that!");
	    return;
	}
	if (is_current(obj)) {
		after = FALSE ;
		return ;
	}
	if (terse) {
		addmsg("W") ;
	} else {
		addmsg("You are now w") ;
	}
	msg("ielding %s.", inv_name(obj, TRUE)) ;
	cur_weapon = obj ;
}

/*
 * pick a random position around the give (y, x) coordinates
 */
int
fallpos(coord *pos, coord *newpos, int passages)
{
	register int y, x, cnt, ch ;

	cnt = 0 ;
	for (y = pos->y - 1 ; y <= pos->y + 1 ; y++) {
		for (x = pos->x - 1 ; x <= pos->x + 1 ; x++) {
		/*
			check to make certain the spot is empty, if it is,
			put the object there, set it in the level list
			and re-draw the room if he can see it
		*/
			if (y == hero.y && x == hero.x) {
				continue ;
			}
			if (((ch = winat(y, x)) == FLOOR ||
			    (passages && ch == PASSAGE)) &&
			    rnd(++cnt) == 0) {
				newpos->y = y ;
				newpos->x = x ;
			}
		}
	}
	return (cnt != 0) ;
}
