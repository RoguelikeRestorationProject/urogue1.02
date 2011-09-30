/*
    misc.c  -  all sorts of miscellaneous routines
   
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
#include <ctype.h>

/*
 * tr_name:
 *	print the name of a trap
 */

char *
tr_name(int ch)
{
    register char *s = "";

    switch (ch)
    {
	when TRAPDOOR:
	    s = terse ? "A trapdoor." : "You found a trapdoor.";
	when BEARTRAP:
	    s = terse ? "A beartrap." : "You found a beartrap.";
	when SLEEPTRAP:
	    s = terse ? "A sleeping gas trap.":"You found a sleeping gas trap.";
	when ARROWTRAP:
	    s = terse ? "An arrow trap." : "You found an arrow trap.";
	when TELTRAP:
	    s = terse ? "A teleport trap." : "You found a teleport trap.";
	when DARTTRAP:
	    s = terse ? "A dart trap." : "You found a poison dart trap.";
	when POOL:
	    s = terse ? "A shimmering pool." : "You found a shimmering pool.";
	when MAZETRAP:
	    s = terse ? "A maze entrance." : "You found a maze entrance.";
	when FIRETRAP:
	    s = terse ? "A fire trap." : "You found a fire trap.";
	when POISONTRAP:
	    s = terse ? "A poison pool trap." : "You found a poison pool trap.";
	when LAIR:
	    s = terse ? "A monster lair." : "You found a monster lair.";
	when RUSTTRAP:
	    s = terse ? "A rust trap." : "You found a rust trap.";
    }
    return s;
}

/*
 * Look:
 *	A quick glance all around the player
 */
void
look(int wakeup)
{
    register int x, y;
    register int ch;
	register int och;
    register int oldx, oldy;
    register int inpass, horiz, vert, do_light = FALSE, do_blank = FALSE;
    register int passcount = 0;
    register struct room *rp;
    register int ey, ex;

    /* Are we moving vertically or horizontally? */
    if (runch == 'h' || runch == 'l') horiz = TRUE;
    else horiz = FALSE;
    if (runch == 'j' || runch == 'k') vert = TRUE;
    else vert = FALSE;

    getyx(cw, oldy, oldx);	/* Save current position */

    /* Blank out the floor around our last position and check for
     * moving out of a corridor in a maze.
     */
    if (oldrp != NULL && (oldrp->r_flags & ISDARK) &&
	!(oldrp->r_flags & HASFIRE) && off(player, ISBLIND))
	do_blank = TRUE;
    for (x = player.t_oldpos.x - 1; x <= player.t_oldpos.x + 1; x++)
	for (y = player.t_oldpos.y - 1; y <= player.t_oldpos.y + 1; y++) {
	    ch = show(y, x);
	    if (do_blank && (y != hero.y || x != hero.x) && ch == FLOOR)
		mvwaddch(cw, y, x, ' ');
		
	    /* Moving out of a corridor? */
	    if (levtype == MAZELEV &&
		(ch != '|' && ch != '-') &&      /* Not a wall */
		((vert && x != player.t_oldpos.x && y==player.t_oldpos.y) ||
		 (horiz && y != player.t_oldpos.y && x==player.t_oldpos.x)))
		do_light = TRUE;	/* Just came to a turn */
	}

    inpass = ((rp = roomin(&hero)) == NULL); /* Are we in a passage? */

    /* Are we coming out of a wall into a corridor in a maze? */
    och = show(player.t_oldpos.y, player.t_oldpos.x);
    ch = show(hero.y, hero.x);
  if (levtype == MAZELEV && (rp != NULL) && (och == '|' || och == '-' || och == SECRETDOOR) &&
	(ch != '|' && ch != '-' && ch != SECRETDOOR))
	do_light = off(player, ISBLIND); /* Light it up if not blind */

    /* Look around the player */
    ey = hero.y + 1;
    ex = hero.x + 1;
    for (x = hero.x - 1; x <= ex; x++)
	if (x >= 0 && x < COLS) for (y = hero.y - 1; y <= ey; y++)
	{
	    if (y <= 0 || y >= LINES - 2)
		continue;
	    if (isalpha(mvwinch(mw, y, x)))
	    {
		register struct linked_list *it;
		register struct thing *tp;

		if (wakeup)
		    it = wake_monster(y, x);
		else
		    it = find_mons(y, x);
		if (it == NULL) 
		    continue;
		tp = (struct thing *) ldata(it);
		tp->t_oldch = CCHAR( mvinch(y, x) );
		if (isatrap(tp->t_oldch)) {
		    register struct trap *trp = trap_at(y, x);

		    tp->t_oldch = (trp->tr_flags & ISFOUND) ? tp->t_oldch
							    : trp->tr_show;
		}
		if (tp->t_oldch == FLOOR && (rp->r_flags & ISDARK)
		    && !(rp->r_flags & HASFIRE) && off(player, ISBLIND))
			tp->t_oldch = ' ';
	    }

	    /*
	     * Secret doors show as walls
	     */
	    if ((ch = show(y, x)) == SECRETDOOR)
		ch = secretdoor(y, x);
	    /*
	     * Don't show room walls if he is in a passage and
	     * check for maze turns
	     */
	    if (off(player, ISBLIND))
	    {
		if ((y == hero.y && x == hero.x)
		    || (inpass && (ch == '-' || ch == '|')))
			continue;

		/* Are we at a crossroads in a maze? */
		if (levtype == MAZELEV &&  (rp != NULL) &&
		    (ch != '|' && ch != '-') &&      /* Not a wall */
		    ((vert && x != hero.x && y == hero.y) ||
		     (horiz && y != hero.y && x == hero.x)))
		    do_light = TRUE;	/* Just came to a turn */
	    }
	    else if (y != hero.y || x != hero.x)
		continue;

	    wmove(cw, y, x);
	    waddch(cw, ch);
	    if (door_stop && !firstmove && running)
	    {
		switch (runch)
		{
		    when 'h':
			if (x == ex)
			    continue;
		    when 'j':
			if (y == hero.y - 1)
			    continue;
		    when 'k':
			if (y == ey)
			    continue;
		    when 'l':
			if (x == hero.x - 1)
			    continue;
		    when 'y':
			if ((x + y) - (hero.x + hero.y) >= 1)
			    continue;
		    when 'u':
			if ((y - x) - (hero.y - hero.x) >= 1)
			    continue;
		    when 'n':
			if ((x + y) - (hero.x + hero.y) <= -1)
			    continue;
		    when 'b':
			if ((y - x) - (hero.y - hero.x) <= -1)
			    continue;
		}
		switch (ch)
		{
		    case DOOR:
			if (x == hero.x || y == hero.y)
			    running = FALSE;
			break;
		    case PASSAGE:
			if (x == hero.x || y == hero.y)
			    passcount++;
			break;
		    case FLOOR:
			/* Stop by new passages in a maze (floor next to us) */
			if ((levtype == MAZELEV) &&
			    ((horiz && x == hero.x && y != hero.y) ||
			     (vert && y == hero.y && x != hero.x)))
				running = FALSE;
		    case '|':
		    case '-':
		    case ' ':
			break;
		    default:
			running = FALSE;
			break;
		}
	    }
	}
    if (door_stop && !firstmove && passcount > 1)
	running = FALSE;

   /* Do we have to light up the area (just stepped into a new corridor)? */
   if (do_light && wakeup && (rp != NULL) && 	/* wakeup will be true on a normal move */
	!(rp->r_flags & ISDARK) &&   /* We have some light */
	!ce(hero, player.t_oldpos))  /* Don't do anything if we didn't move */
	    light(&hero);

    mvwaddch(cw, hero.y, hero.x, PLAYER);
    wmove(cw, oldy, oldx);
    if (wakeup) {
	player.t_oldpos = hero; /* Don't change if we didn't move */
	oldrp = rp;
    }
}

/*
 * secret_door:
 *	Figure out what a secret door looks like.
 */
int
secretdoor(int y, int x)
{
    register int i;
    register struct room *rp;
    register coord *cpp;
    static coord cp;

    cp.y = y;
    cp.x = x;
    cpp = &cp;
    for (rp = rooms, i = 0; i < MAXROOMS; rp++, i++)
	if (inroom(rp, cpp))
	{
	    if (y == rp->r_pos.y || y == rp->r_pos.y + rp->r_max.y - 1)
		return('-');
	    else
		return('|');
	}

    return('p');
}

/*
 * find_obj:
 *	find the unclaimed object at y, x
 */

struct linked_list *
find_obj(int y, int x)
{
    register struct linked_list *obj;
    register struct object *op;

    for (obj = lvl_obj; obj != NULL; obj = next(obj))
    {
	op = (struct object *) ldata(obj);
	if (op->o_pos.y == y && op->o_pos.x == x)
		return obj;
    }
    return NULL;
}

/*
 * eat:
 *	He wants to eat something, so let him try
 */
void
eat(void)
{
    register struct linked_list *item;
    register struct object *obj;
    register int amount;

    if ((item = get_item("eat", FOOD)) == NULL)
	return;
    obj = (struct object *) ldata(item);
    switch (obj->o_which) {
	when FD_RATION:
	    amount = HUNGERTIME + rnd(400) - 200;
	    if (rnd(100) > 70) {
		msg("Yuk, this food tastes awful.");
		pstats.s_exp++;
		check_level();
	    }
	    else
		msg("Yum, that tasted good.");
	when FD_FRUIT:
	    amount = 200 + rnd(HUNGERTIME);
	    msg("My, that was a yummy %s.", fruit);
	when FD_CRAM:
	    amount = rnd(HUNGERTIME / 2) + 600;
	    msg("The cram tastes dry in your mouth.");
	when FD_CAKES:
	    amount = (HUNGERTIME / 3) + rnd(600);
	    msg("Yum, the honey cakes tasted good.");
	when FD_LEMBA:
	    amount = (HUNGERTIME / 2) + rnd(900);
	    quaff(P_HEALING, FALSE);
	when FD_MIRUVOR:
	    amount = (HUNGERTIME / 3) + rnd(500);
	    quaff(P_HEALING, FALSE);
	    quaff(P_RESTORE, FALSE);
	otherwise:
	    msg("What a strange thing to eat!");
	    amount = HUNGERTIME;
    }
    food_left += amount;
    if (obj->o_flags & ISBLESSED) {
	food_left += 2 * amount;
	msg("You have a tingling feeling in your mouth.");
    }
    else if (food_left > STOMACHSIZE) {
	food_left = STOMACHSIZE;
	msg("You feel satiated and too full to move.");
	no_command = HOLDTIME;
    }
    hungry_state = F_OK;
    del_pack(item);
    updpack(TRUE);
    if (obj == cur_weapon)
	cur_weapon = NULL;
}

/*
 * Used to modify the player's strength
 * it keeps track of the highest it has been, just in case
 */
void
chg_str(int amt, int both, int lost)
{
    register int ring_str;		/* ring strengths */
    register struct stats *ptr;		/* for speed */
	int oldstr;

    ptr = &pstats;
    ring_str = ring_value(R_ADDSTR) + (on(player, POWERSTR) ? 10 : 0) +
			(on(player, SUPERHERO) ? 10 : 0);
    ptr->s_str -= ring_str;
	oldstr = ptr->s_str;
    ptr->s_str += amt;

    if (ptr->s_str < 3) {
	ptr->s_str = 3;
    }
    else if (ptr->s_str > 25)
	ptr->s_str = 25;

    if (both)
	max_stats.s_str += amt;
    if (lost)
	lost_str -= (ptr->s_str - oldstr);

    ptr->s_str += ring_str;
    if (ptr->s_str < 0)
	ptr->s_str = 0;

    updpack(TRUE);
}

/*
 * Used to modify the player's dexterity
 * it keeps track of the highest it has been, just in case
 */
void
chg_dext(int amt, int both, int lost)
{
    register int ring_dext;		/* ring strengths */
    register struct stats *ptr;		/* for speed */
	register int olddext;

    ptr = &pstats;
    ring_dext = ring_value(R_ADDHIT) + (on(player, POWERDEXT) ? 10 : 0) +
			(on(player, SUPERHERO) ? 5 : 0);
    ptr->s_dext -= ring_dext;
	olddext = ptr->s_dext;
    ptr->s_dext += amt;

    if (ptr->s_dext < 3) {
	ptr->s_dext = 3;
    }
    else if (ptr->s_dext > 25)
	ptr->s_dext = 25;

    if (both)
	max_stats.s_dext += amt;
    if (lost)
	lost_dext -= (ptr->s_dext - olddext);

    ptr->s_dext += ring_dext;
    if (ptr->s_dext < 0)
	ptr->s_dext = 0;
}

/*
 * add_haste:
 *	add a haste to the player
 */
void
add_haste(int blessed)
{
    int hasttime;

    if (blessed) hasttime = 10;
    else hasttime = 6;

    if (on(player, ISSLOW)) { /* Is person slow? */
	extinguish(noslow);
	noslow();

	if (blessed) hasttime = 4;
	else return;
    }

    if (on(player, ISHASTE)) {
	msg("You faint from exhaustion.");
	no_command += rnd(hasttime);
	lengthen(nohaste, rnd(hasttime) + (roll(1,4) * hasttime));
    }
    else {
	turn_on(player, ISHASTE);
	fuse(nohaste, 0, roll(hasttime, hasttime), AFTER);
    }
}

/*
 * aggravate:
 *	aggravate all the monsters on this level
 */
void
aggravate(void)
{
    register struct linked_list *mi;

    for (mi = mlist; mi != NULL; mi = next(mi))
	runto(&((struct thing *) ldata(mi))->t_pos, &hero);
}

/*
 * for printfs: if string starts with a vowel, return "n" for an "an"
 */
char *
vowelstr(char *str)
{
    switch (*str)
    {
	case 'a':
	case 'e':
	case 'i':
	case 'o':
	case 'u':
	    return "n";
	default:
	    return "";
    }
}

/* 
 * see if the object is one of the currently used items
 */
int
is_current(struct object *obj)
{
    if (obj == NULL)
	return FALSE;
    if (obj == cur_armor || obj == cur_weapon || 
	obj == cur_ring[LEFT_1] || obj == cur_ring[LEFT_2] ||
	obj == cur_ring[LEFT_3] || obj == cur_ring[LEFT_4] ||
	obj == cur_ring[RIGHT_1] || obj == cur_ring[RIGHT_2] ||
	obj == cur_ring[RIGHT_3] || obj == cur_ring[RIGHT_4]) {
	msg(terse ? "In use." : "That's already in use.");
	return TRUE;
    }
    return FALSE;
}

/*
 * set up the direction co_ordinate for use in varios "prefix" commands
 */
int
get_dir(void)
{
    register char *prompt;
    register int gotit;

    if (terse) {
	prompt = "Direction? ";
    } else {
	prompt = "Which direction? ";
	msg(prompt);
    }

    do
    {
	gotit = TRUE;
	switch (readchar(msgw))
	{
	    when 'h': case'H': delta.y =  0; delta.x = -1;
	    when 'j': case'J': delta.y =  1; delta.x =  0;
	    when 'k': case'K': delta.y = -1; delta.x =  0;
	    when 'l': case'L': delta.y =  0; delta.x =  1;
	    when 'y': case'Y': delta.y = -1; delta.x = -1;
	    when 'u': case'U': delta.y = -1; delta.x =  1;
	    when 'b': case'B': delta.y =  1; delta.x = -1;
	    when 'n': case'N': delta.y =  1; delta.x =  1;
	    when ESCAPE: return FALSE;
	    otherwise:
		mpos = 0;
		msg(prompt);
		gotit = FALSE;
	}
    } until (gotit);
    if (on(player, ISHUH) && rnd(100) > 80)
	do
	{
	    delta.y = rnd(3) - 1;
	    delta.x = rnd(3) - 1;
	} while (delta.y == 0 && delta.x == 0);
    mpos = 0;
    return TRUE;
}


/*
 * Maze_view:
 *	Returns true if the player can see the specified location within
 *	the confines of a maze (within one column or row)
 */

int
maze_view(int y, int x)
{
    register int start, goal, delta, ycheck = 0, xcheck = 0, absy, absx;
    register int row;

    /* Get the absolute value of y and x differences */
    absy = hero.y - y;
    absx = hero.x - x;
    if (absy < 0) absy = -absy;
    if (absx < 0) absx = -absx;

    /* Must be within one row or column */
    if (absy > 1 && absx > 1) return(FALSE);

    if (absy <= 1) {		/* Go along row */
	start = hero.x;
	goal = x;
	row = TRUE;
	ycheck = hero.y;
    }
    else {			/* Go along column */
	start = hero.y;
	goal = y;
	row = FALSE;
	xcheck = hero.x;
    }
    if (start <= goal) delta = 1;
    else delta = -1;

    while (start != goal) {
	if (row) xcheck = start;
	else ycheck = start;
	switch (winat(ycheck, xcheck)) {
	    case '|':
	    case '-':
	    case WALL:
	    case DOOR:
	    case SECRETDOOR:
		return(FALSE);
	}
	start += delta;
    }
    return(TRUE);
}

/*
 * listen: listen for monsters less than 5 units away
 */
void
listenfor(void)
{
    register struct linked_list *item;
    register struct thing *tp;
    register int thief_bonus = -50;
    register int mcount = 0;

    if (player.t_ctype == C_THIEF)
	thief_bonus = 10;

    for (item = mlist; item != NULL; item = next(item)) {
	tp = (struct thing *) ldata(item);
	if (DISTANCE(hero.y, hero.x, tp->t_pos.y, tp->t_pos.x) < 64
	    && rnd(100) < (thief_bonus + 2*pstats.s_dext + 5*pstats.s_lvl)) {
 	    msg("You hear a%s %s nearby.", 
		vowelstr(monsters[tp->t_index].m_name),
		monsters[tp->t_index].m_name);
	    mcount ++;
	}
    }
    if (mcount == 0)
	msg("You hear nothing.");
}
