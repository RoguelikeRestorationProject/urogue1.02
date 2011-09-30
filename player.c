/*
    player.c  -  This file contains functions for dealing with special player
	             abilities
   
    UltraRogue
    Copyright (C) 1985 Herb Chong
    All rights reserved.
    
    Based on "Advanced Rogue"
    Copyright (C) 1982, 1984, 1985 Michael Morgan, Ken Dalka and AT&T
    All rights reserved.
   
    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include "curses.h"
#include "rogue.h"
 
 /* Constitution bonus */
 
 int
 const_bonus(void)	/* Hit point adjustment for changing levels */
 {
     if (pstats.s_const > 6 && pstats.s_const <= 12) 
 	return(0);
     if (pstats.s_const > 12) 
 	return(pstats.s_const-12);
     if (pstats.s_const > 3) 
 	return(-1);
     return(-2);
 }
 
 /* Routines for thieves */
 
 /*
  * gsense:
  *	Sense gold
  */
 void
 gsense(void)
 {
     /* Only thieves can do this */
     if (player.t_ctype != C_THIEF) {
 	msg("You seem to have no gold sense.");
 	return;
     }
 
     if (lvl_obj != NULL) {
 	struct linked_list *gitem;
 	struct object *cur;
 	int gtotal = 0;
 
 	wclear(hw);
 	for (gitem = lvl_obj; gitem != NULL; gitem = next(gitem)) {
 	    cur = (struct object *) ldata(gitem);
 	    if (cur->o_type == GOLD) {
 		gtotal += cur->o_count;
 		mvwaddch(hw, cur->o_pos.y, cur->o_pos.x, GOLD);
 	    }
 	}
 	if (gtotal) {
 	    s_know[S_GFIND] = TRUE;
 	    msg("You sense gold!");
 	    overlay(hw,cw);
 	    return;
 	}
     }
     msg("You can sense no gold on this level.");
 }
 
 
 /*
  * steal:
  *	Steal in direction given in delta
  */
 void
 steal(void)
 {
     register struct linked_list *item;
     register struct thing *tp;
     register char *mname;
     coord new_pos;
     int thief_bonus = -50;
 
     new_pos.y = hero.y + delta.y;
     new_pos.x = hero.x + delta.x;
 
     /* Anything there? */
     if (new_pos.y < 0 || new_pos.y > LINES-3 ||
 	new_pos.x < 0 || new_pos.x > COLS-1 ||
 	mvwinch(mw, new_pos.y, new_pos.x) == ' ') {
 	msg("Nothing to steal from.");
 	return;
     }
 
     if ((item = find_mons(new_pos.y, new_pos.x)) == NULL) {
 	debug("Steal from what @ %d,%d?", new_pos.y, new_pos.x);
 	return;
     }
     tp = THINGPTR(item);
     mname = monsters[tp->t_index].m_name;
 
     /* Can player steal something unnoticed? */
     if (player.t_ctype == C_THIEF) thief_bonus = 10;
 
     if (rnd(100) <
 	(thief_bonus + 2*pstats.s_dext + 5*pstats.s_lvl -
 	 5*(tp->t_stats.s_lvl - 3))) {
 	register struct linked_list *s_item, *pack_ptr;
 	int count = 0;
 
 	s_item = NULL;	/* Start stolen goods out as nothing */
 
 	/* Find a good item to take */
 	for (pack_ptr=tp->t_pack; pack_ptr != NULL; pack_ptr=next(pack_ptr))
 	    if (rnd(++count) == 0) s_item = pack_ptr;
 
 	/* if we have a merchant, get an item */
 	if (s_item == NULL && tp->t_index == NUMMONST)
 	    s_item = new_thing();
 
 
 	/* Find anything? */
 	if (s_item == NULL) {
 	    msg("The %s has nothing to steal.", mname);
 	    return;
 	}
 
 	/* Take it from monster */
 	if (tp->t_pack) detach(tp->t_pack, s_item);
 
 	/* Give it to player */
 	if (add_pack(s_item, FALSE) == FALSE) {
 	   (OBJPTR(s_item))->o_pos = hero;
 	   fall(s_item, TRUE);
 	}
 
 	/* Get points for stealing */
 	pstats.s_exp += tp->t_stats.s_exp/2;
 
 	/*
 	 * Do adjustments if player went up a level
 	 */
 	check_level();
     }
 
     else {
 	msg("Your attempt fails.");
 
 	/* Annoy monster (maybe) */
 	if (rnd(35) >= pstats.s_dext + thief_bonus) runto(&new_pos, &hero);
     }
 }
 
 
 /* Routines for clerics */
 
 void  
 pray(void)
 {
     register int i, which_prayer, avail_points;
	 register int num_prayers;
     register int nohw = FALSE;
 
     if (player.t_ctype != C_CLERIC && pstats.s_wisdom < 17) {
 	msg("You are not permitted to pray.");
 	return;
     }
 
     /* Get the number of avilable prayers */
     if (pstats.s_wisdom > 16) 
 	num_prayers = (pstats.s_wisdom - 15) / 2;
     else 
 	num_prayers = 0;
 
     if (player.t_ctype == C_CLERIC) 
 	num_prayers += pstats.s_lvl;
 
     if (player.t_ctype != C_MAGICIAN && player.t_ctype != C_CLERIC
  		&& ISWEARING(R_WIZARD))
 	num_prayers *= 2;
 
     if (num_prayers > MAXPRAYERS) 
 	num_prayers = MAXPRAYERS;
 
	 avail_points = pstats.s_wisdom * pstats.s_lvl;
	 if(ISWEARING(R_WIZARD))
		 avail_points *= 2;

     /* Prompt for prayer */
     msg("Which prayer are you offering [%d points left]? (* for list): ",
 			avail_points - spell_power);
 
     which_prayer = ((readchar(msgw) & 0177) - 'a');
     if (which_prayer == ESCAPE - 'a') {
 	after = FALSE;
 	return;
     }
     if (which_prayer >= 0 && which_prayer < num_prayers) nohw = TRUE;
 
     else if (slow_invent) {
 	register int c;
 
 	for (i=0; i<num_prayers; i++) {
 	    mvwaddch(cw, 0, 0, '[');
 	    waddch(cw, 'a' + i);
 	    wprintw(cw, ", %d/%d] A prayer for ", cleric_spells[i].s_cost,
 			avail_points - spell_power);
 	    if (cleric_spells[i].s_type == TYP_POTION)
 		waddstr(cw, p_magic[cleric_spells[i].s_which].mi_name);
 	    else if (cleric_spells[i].s_type == TYP_SCROLL)
 		waddstr(cw, s_magic[cleric_spells[i].s_which].mi_name);
 	    else if (cleric_spells[i].s_type == TYP_STICK)
 		waddstr(cw, ws_magic[cleric_spells[i].s_which].mi_name);
 	    waddstr(cw, morestr);
		wclrtoeol(cw);
 	    draw(cw);
 	    do {
 		c = readchar(cw);
 	    } while (c != ' ' && c != ESCAPE);
 	    if (c == ESCAPE)
 		break;
 	}
 	mvwaddstr(cw, 0, 0, "Which prayer are you offering? ");
	wclrtoeol(cw);
 	draw(cw);
     }
     else {
 	/* Set up for redraw */
 	msg("");
 	clearok(cw, TRUE);
 	touchwin(cw);
 
 	/* Now display the possible prayers */
 	wclear(hw);
 	touchwin(hw);
 	for (i=0; i<num_prayers; i++) {
 	    mvwaddch(hw, i+2, 0, '[');
 	    waddch(hw, 'a' + i);
 	    wprintw(hw, ", %2d] A prayer for ", cleric_spells[i].s_cost);
 	    if (cleric_spells[i].s_type == TYP_POTION)
 		waddstr(hw, p_magic[cleric_spells[i].s_which].mi_name);
 	    else if (cleric_spells[i].s_type == TYP_SCROLL)
 		waddstr(hw, s_magic[cleric_spells[i].s_which].mi_name);
 	    else if (cleric_spells[i].s_type == TYP_STICK)
 		waddstr(hw, ws_magic[cleric_spells[i].s_which].mi_name);
 	}
 	wmove(hw, 0, 0);
 	wprintw(hw, "Which prayer are you offering (%d points left)? ",
 				avail_points - spell_power);
 	draw(hw);
     }
 
     if (!nohw) {
 	which_prayer = ((readchar(hw) & 0177) - 'a');
 	while (which_prayer < 0 || which_prayer >= num_prayers) {
 	    if (which_prayer == ESCAPE - 'a') {
 		after = FALSE;
 		return;
 	    }
 	    wmove(hw, 0, 0);
 	    wclrtoeol(hw);
 	    waddstr(hw, "Please enter one of the listed prayers. ");
 	    draw(hw);
 	    which_prayer = ((readchar(hw) & 0177) - 'a');
 	}
     }
 
 
    if ((cleric_spells[which_prayer].s_cost + spell_power) > avail_points) {
 	msg("Your prayer fails.");
 	return;
     }
 
     if (nohw) 
 	msg("Your prayer has been granted.");
     else {
 	mvwaddstr(hw, 0, 0, "Your prayer has been granted.--More--");
 	wclrtoeol(hw);
 	draw(hw);
 	wait_for(hw, ' ');
     }
     if (cleric_spells[which_prayer].s_type == TYP_POTION)
 	quaff(	cleric_spells[which_prayer].s_which,
 		cleric_spells[which_prayer].s_blessed);
     else
 	read_scroll(	cleric_spells[which_prayer].s_which,
 			cleric_spells[which_prayer].s_blessed);
 
     spell_power += cleric_spells[which_prayer].s_cost;
 }
 
 
 /*
  * affect:
  *	cleric affecting undead
  */
 void 
 affect(void)
 {
     register struct linked_list *item;
     register struct thing *tp;
     register char *mname;
     coord new_pos;
 
     if (player.t_ctype != C_CLERIC) {
 	msg("Only clerics can affect undead.");
 	return;
     }
 
     new_pos.y = hero.y + delta.y;
     new_pos.x = hero.x + delta.x;
 
     /* Anything there? */
     if (new_pos.y < 0 || new_pos.y > LINES-3 ||
 	new_pos.x < 0 || new_pos.x > COLS-1 ||
 	mvwinch(mw, new_pos.y, new_pos.x) == ' ') {
 	msg("Nothing to affect.");
 	return;
     }
 
     if ((item = find_mons(new_pos.y, new_pos.x)) == NULL) {
 	debug("Affect what @ %d,%d?", new_pos.y, new_pos.x);
 	return;
     }
     tp = (struct thing *) ldata(item);
     mname = monsters[tp->t_index].m_name;
 
     if (off(*tp, ISUNDEAD) || on(*tp, WASTURNED)) goto annoy;
 
     /* Can cleric kill it? */
     if (pstats.s_lvl >= 3 * tp->t_stats.s_lvl) {
 	msg("You have destroyed the %s.", mname);
 	killed(item, FALSE, TRUE);
 	return;
     }
 
     /* Can cleric turn it? */
     if (rnd(100) + 1 >
 	 (100 * ((2 * tp->t_stats.s_lvl) - pstats.s_lvl)) / pstats.s_lvl) {
 	/* Make the monster flee */
 	turn_on(*tp, WASTURNED);	/* No more fleeing after this */
 	turn_on(*tp, ISFLEE);
 
 	/* Let player know */
 	msg("You have turned the %s.", mname);
 
 	/* get points for turning monster */
 	pstats.s_exp += tp->t_stats.s_exp/2;
 
 	/* If monster was suffocating, stop it */
 	if (on(*tp, DIDSUFFOCATE)) {
 	    turn_off(*tp, DIDSUFFOCATE);
 	    extinguish(suffocate);
 	}
 
 	/* If monster held us, stop it */
 	if (on(*tp, DIDHOLD) && (--hold_count <= 0))
	{
	    hold_count = 0;
 	    turn_off(player, ISHELD);
	}
 	turn_off(*tp, DIDHOLD);
 	return;
     }
 
     /* Otherwise -- no go */
 annoy:
     msg("You do not affect the %s.", mname);
 
     /* Annoy monster */
    if (off(*tp, WASTURNED)) runto(&new_pos, &hero);
 }
 
 
 /* Routines for magicians */
 
 void 
 cast(void)
 {
     register int i, which_spell, avail_points;
	 register int num_spells;
     register int nohw = FALSE;
 
     if (player.t_ctype != C_MAGICIAN && pstats.s_intel < 16) {
 	msg("You are not permitted to cast spells.");
 	return;
     }
 
     /* Get the number of avilable spells */
     if (pstats.s_intel >= 16) 
 	num_spells = pstats.s_intel - 15;
     else 
 	num_spells = 0;
 
     if (player.t_ctype == C_MAGICIAN) 
 	num_spells += pstats.s_lvl;
 
     if (player.t_ctype != C_MAGICIAN && player.t_ctype != C_CLERIC
  		&& ISWEARING(R_WIZARD))
 	num_spells *= 2;
 
    if (num_spells > MAXSPELLS) 
	num_spells = MAXSPELLS;

	avail_points = pstats.s_intel * pstats.s_lvl;
	if (ISWEARING(R_WIZARD))
		avail_points *= 2;

    /* Prompt for spells */
    msg("Which spell are you casting [%d points left]? (* for list): ",
			avail_points - spell_power);

    which_spell = ((readchar(msgw) & 0177) - 'a');
    if (which_spell ==  ESCAPE - 'a') {
	after = FALSE;
	return;
    }
    if (which_spell >= 0 && which_spell < num_spells) nohw = TRUE;

    else if (slow_invent) {
	register int c;

	for (i=0; i<num_spells; i++) {
	    mvwaddch(cw, 0, 0, '[');
	    waddch(cw, ('a' + i));
	    wprintw(cw, ", %d/%d] A spell of ", magic_spells[i].s_cost,
			avail_points - spell_power);
	    if (magic_spells[i].s_type == TYP_POTION)
		waddstr(cw, p_magic[magic_spells[i].s_which].mi_name);
	    else if (magic_spells[i].s_type == TYP_SCROLL)
		waddstr(cw, s_magic[magic_spells[i].s_which].mi_name);
	    else if (magic_spells[i].s_type == TYP_STICK)
		waddstr(cw, ws_magic[magic_spells[i].s_which].mi_name);
	    waddstr(cw, morestr);
		wclrtoeol(cw);
	    draw(cw);
	    do {
		c = readchar(cw);
	    } while (c != ' ' && c != ESCAPE);
	    if (c == ESCAPE)
		break;
	}
	mvwaddstr(cw, 0, 0, "Which spell are you casting? ");
	wclrtoeol(cw);
	draw(cw);
    }
    else {
	/* Set up for redraw */
	msg("");
	clearok(cw, TRUE);
	touchwin(cw);

	/* Now display the possible spells */
	wclear(hw);
	touchwin(hw);
	for (i=0; i<num_spells; i++) {
	    mvwaddch(hw, i+2, 0, '[');
	    waddch(hw, 'a' + i);
	    wprintw(hw, ", %2d] A spell of ", magic_spells[i].s_cost);
	    if (magic_spells[i].s_type == TYP_POTION)
		waddstr(hw, p_magic[magic_spells[i].s_which].mi_name);
	    else if (magic_spells[i].s_type == TYP_SCROLL)
		waddstr(hw, s_magic[magic_spells[i].s_which].mi_name);
	    else if (magic_spells[i].s_type == TYP_STICK)
		waddstr(hw, ws_magic[magic_spells[i].s_which].mi_name);
	}
	wmove(hw, 0, 0);
	wprintw(hw, "Which spell are you casting (%d points left)? ",
			avail_points - spell_power);
	draw(hw);
    }

    if (!nohw) {
	which_spell = ((readchar(hw) & 0177) - 'a');
	while (which_spell < 0 || which_spell >= num_spells) {
	    if (which_spell == ESCAPE - 'a') {
		after = FALSE;
		return;
	    }
	    msg("");
	    wmove(hw, 0, 0);
	    wclrtoeol(hw);
	    waddstr(hw, "Please enter one of the listed spells. ");
	    draw(hw);
	    which_spell = ((readchar(hw) & 0177) - 'a');
	}
    }

    if ((spell_power + magic_spells[which_spell].s_cost) > avail_points) {
	msg("Your attempt fails.");
	return;
    }
    if (nohw)
	msg("Your spell is successful.");
    else {
	mvwaddstr(hw, 0, 0, "Your spell is successful.--More--");
	wclrtoeol(hw);
	draw(hw);
	wait_for(hw, ' ');
    }
    if (magic_spells[which_spell].s_type == TYP_POTION)
        quaff(	magic_spells[which_spell].s_which,
        	magic_spells[which_spell].s_blessed);
    else if (magic_spells[which_spell].s_type == TYP_SCROLL)
        read_scroll(	magic_spells[which_spell].s_which,
        		magic_spells[which_spell].s_blessed);
    else if (magic_spells[which_spell].s_type == TYP_STICK) {
	 if (get_dir())
	      do_zap(	TRUE, 
			magic_spells[which_spell].s_which,
			magic_spells[which_spell].s_blessed);
	 else
	      return;
    }
    spell_power += magic_spells[which_spell].s_cost;
}
