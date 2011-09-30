/*
    sticks.c  -  Functions to implement the various sticks one might find
	             while wandering around the dungeon. Read a scroll and let 
				 it happen
   
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
#include <string.h>
#include <ctype.h>
#include "rogue.h"

void
fix_stick(struct object *cur)
{
    if (strcmp(ws_type[cur->o_which], "staff") == 0) {
	cur->o_weight = 100;
	cur->o_charges = 5 + rnd(10);
	strcpy(cur->o_damage, "2d3");
	switch (cur->o_which) {
	    when WS_HIT:
		cur->o_hplus = 3;
		cur->o_dplus = 3;
		strcpy(cur->o_damage,"2d8");
	    when WS_LIGHT:
		cur->o_charges = 20 + rnd(10);
	    }
    }
    else {
	strcpy(cur->o_damage, "1d3");
	cur->o_weight = 60;
	cur->o_charges = 3 + rnd(5);
	switch (cur->o_which) {
	    when WS_HIT:
		cur->o_hplus = 3;
		cur->o_dplus = 3;
		strcpy(cur->o_damage,"1d8");
	    when WS_LIGHT:
		cur->o_charges = 10 + rnd(10);
	    }
    }
    strcpy(cur->o_hurldmg,"1d1");

}

void
do_zap(int gotdir, int which, int blessed)
{
    register struct linked_list *item, *nitem;
    register struct object *obj = NULL, *nobj;
    register struct room *rp;
    register struct thing *tp;
    register int y, x;
    int cursed, is_stick;

    cursed = FALSE;
    is_stick = FALSE;

    if (which == 0) {
	if ((item = get_item("zap with", STICK)) == NULL)
	    return;
	obj = OBJPTR(item);
	if (obj->o_type != STICK && !(obj->o_flags & ISZAPPED)) {
	    msg("You can't zap with that!");
	    return;
	}
	if (obj->o_type != STICK)
	    which = WS_ELECT;
	else
	    which = obj->o_which;
	ws_know[which] = TRUE;
	if (obj->o_charges < 1)
	{
	    msg("Nothing happens.");
	    return;
	}
	obj->o_charges--;
	if (!gotdir)
	    do {
		delta.y = rnd(3) - 1;
		delta.x = rnd(3) - 1;
	    } while (delta.y == 0 && delta.x == 0);

	cursed = (obj->o_flags & ISCURSED) != 0;
	blessed = (obj->o_flags & ISBLESSED) != 0;
	is_stick = TRUE;
    }
    switch (which)
    {
	when WS_LIGHT:
	    /*
	     * Reddy Kilowat wand.  Light up the room
	     */
	    blue_light(blessed, cursed);
	when WS_DRAIN:
	    /*
	     * Take away 1/2 of hero's hit points, then take it away
	     * evenly from the monsters in the room or next to hero
	     * if he is in a passage (but leave the monsters alone
	     * if the stick is cursed)
	     */
	    if (pstats.s_hpt < 2) {
		msg("You are too weak to use it.");
		return;
	    }
	    if (cursed)
		pstats.s_hpt /= 2;
	    else if ((rp = roomin(&hero)) == NULL)
		drain(hero.y-1, hero.y+1, hero.x-1, hero.x+1);
	    else
		drain(rp->r_pos.y, rp->r_pos.y+rp->r_max.y,
		    rp->r_pos.x, rp->r_pos.x+rp->r_max.x);
	when WS_POLYMORPH:
	case WS_TELMON:
	case WS_CANCEL:
	{
	    register int monster, oldch;
	    register int rm;

	    y = hero.y;
	    x = hero.x;
	    while (shoot_ok(winat(y, x))) {
		y += delta.y;
		x += delta.x;
	    }
	    if ( x >= 0 && x < COLS && y >= 1 && y < LINES - 2 &&
			isalpha(monster = CCHAR( mvwinch(mw, y, x) ))) {
		item = find_mons(y, x);
		tp = THINGPTR(item);
		/* if the monster gets the saving throw, leave the case */
		if (save_throw(VS_MAGIC, tp)) {
		    msg("Nothing happens.");
		    break;
		}

		/* Unhold player */
		if (on(*tp, DIDHOLD)) {
		    turn_off(*tp, DIDHOLD);
		    if (--hold_count <= 0) 
	 	    {
			hold_count = 0;
			turn_off(player, ISHELD);
		    }
		}
		/* unsuffocate player */
		if (on(*tp, DIDSUFFOCATE)) {
		    turn_off(*tp, DIDSUFFOCATE);
		    extinguish(suffocate);
		}

		if (which == WS_POLYMORPH) {
		    detach(mlist, item);
		    oldch = tp->t_oldch;
		    delta.y = y;
		    delta.x = x;
		    new_monster(item, rnd(NUMMONST-1) + 1, &delta, FALSE);
		    monster = tp->t_type;
		    if (off(*tp, ISRUN))
			runto(&delta, &hero);
		    if (isalpha(mvwinch(cw, y, x)))
			mvwaddch(cw, y, x, monster);
		    tp->t_oldch = oldch;
		    msg(terse ? "A new %s!" : "You have created a new %s!",
			monsters[tp->t_index].m_name);
		}
		else if (which == WS_CANCEL) {
		    tp->t_flags[0] &= CANC0MASK;
		    tp->t_flags[1] &= CANC1MASK;
		    tp->t_flags[2] &= CANC2MASK;
		    tp->t_flags[3] &= CANC3MASK;
		}
		else { /* A teleport stick */
		    if (cursed) {	/* Teleport monster to player */
			if ((y == (hero.y + delta.y)) &&
			    (x == (hero.x + delta.x)))
				msg("Nothing happens.");
			else {
			    tp->t_pos.y = hero.y + delta.y;
			    tp->t_pos.x = hero.x + delta.x;
			}
		    }
		    else if (blessed) {	/* Get rid of monster */
			killed(item, FALSE, FALSE);
			return;
		    }
		    else {
			register int i=0;

			do {	/* Move monster to another room */
			    rm = rnd_room();
			    rnd_pos(&rooms[rm], &tp->t_pos);
			} until(winat(tp->t_pos.y,tp->t_pos.x) ==
			        FLOOR || i++>500);
		    }

		    /* Now move the monster */
		    if (isalpha(mvwinch(cw, y, x)))
			mvwaddch(cw, y, x, tp->t_oldch);
		    tp->t_dest = &hero;
		    turn_on(*tp, ISRUN);
		    turn_off(*tp, ISDISGUISE);
		    mvwaddch(mw, y, x, ' ');
		    mvwaddch(mw, tp->t_pos.y, tp->t_pos.x, monster);
		    if (tp->t_pos.y != y || tp->t_pos.x != x)
			tp->t_oldch = CCHAR( mvwinch(cw, tp->t_pos.y, tp->t_pos.x) );
		}
	    }
	}
	when WS_MISSILE:
	{
	    static struct object bolt =
	    {
		'*' , {0, 0}, "", 0, "", "1d4" , 0, 0, 100, 1
	    };

	    sprintf(bolt.o_hurldmg, "%dd4", pstats.s_lvl);
	    do_motion(&bolt, delta.y, delta.x, &player);
	    if (!hit_monster(unc(bolt.o_pos), &bolt, &player))
	       msg("The missile vanishes with a puff of smoke.");
	}
	when WS_HIT:
	{
	    register int ch;
	    struct object strike; /* don't want to change sticks attributes */

	    delta.y += hero.y;
	    delta.x += hero.x;
	    ch = winat(delta.y, delta.x);
	    if (isalpha(ch))
	    {
		strike = *obj;
		strike.o_hplus  = 6;
		if (strcmp(ws_type[which], "staff") == 0)
		    strcpy(strike.o_damage,"3d8");
		else
		    strcpy(strike.o_damage,"2d8");
		fight(&delta, &strike, FALSE);
	    }
	}
	case WS_SLOW_M:
	    y = hero.y;
	    x = hero.x;
	    while (shoot_ok(winat(y, x)))
	    {
		y += delta.y;
		x += delta.x;
	    }
	    if ( x >= 0 && x < COLS && y >= 1 && y < LINES - 2 &&
			isalpha(mvwinch(mw, y, x))) {
		item = find_mons(y, x);
		tp = THINGPTR(item);
		if (cursed)
		{
		    if (on(*tp, ISSLOW))
			turn_off(*tp, ISSLOW);
		    else
			turn_on(*tp, ISHASTE);
		}
		else if (blessed) {
		    turn_off(*tp, ISRUN);
		    turn_on(*tp, ISHELD);
		    return;
		}
		else {
		    if (on(*tp, ISHASTE))
			turn_off(*tp, ISHASTE);
		    else
			turn_on(*tp, ISSLOW);
		    tp->t_turn = TRUE;
		}
		delta.y = y;
		delta.x = x;
		runto(&delta, &hero);
	    }
	when WS_CHARGE:
	    if (ws_know[WS_CHARGE] != TRUE && is_stick)
		msg("This is a wand of charging.");
	    if ((nitem = get_item("charge", STICK)) != NULL) {
		nobj = OBJPTR(nitem);
		if ((++nobj->o_charges == 1) && (nobj->o_which == WS_HIT))
		    fix_stick(nobj);
	    }
	when WS_ELECT:
	case WS_FIRE:
	case WS_COLD:
	    {
		char *name;
		int damage;

		if (which == WS_ELECT)
		    name = "lightning bolt";
		else if (which == WS_FIRE)
		    name = "flame";
		else
		    name = "ice";

		if (which == WS_ELECT && obj != NULL && obj->o_charges > 50)
		    damage = roll(6+rnd(6),6+rnd(3));
		else
		    damage = roll(6,6);

		shoot_bolt(&player, hero, delta, TRUE, D_BOLT, name, damage);
	    }
	when WS_ANTIM: {
	    reg int m1, m2, x1, y1;
	    reg int ch;
	    reg struct linked_list *ll;
	    reg struct thing *lt;

	    y1 = hero.y;
	    x1 = hero.x;
	    do {
		y1 += delta.y;
		x1 += delta.x;
		ch = winat(y1,x1);
	    } while (ch == PASSAGE || ch == FLOOR);
	    for (m1 = x1 - 1 ; m1 <= x1 + 1 ; m1++) {
		for(m2 = y1 - 1 ; m2 <= y1 + 1 ; m2++) {
		    ch = winat(m2,m1);
		    if (m1 == hero.x && m2 == hero.y)
			continue;
		    if (ch != ' ') {
			ll = find_obj(m2,m1);
			if (ll != NULL) {
			    detach(lvl_obj,ll);
			    discard(ll);
			}
			ll = find_mons(m2,m1);
			if (ll != NULL) {
			    lt = THINGPTR(ll);
			    if (on(*lt, CANSELL)) {
				luck++;
				aggravate();
			    }
			    if (off(*lt, CANINWALL)) {
				check_residue(lt);
				detach(mlist,ll);
				attach(monst_dead,ll);
				turn_on(*lt, ISDEAD);
				mvwaddch(mw,m2,m1,' ');
			    }
			}
			mvaddch(m2,m1,' ');
			mvwaddch(cw,m2,m1,' ');
		    }
		}
	    }
	    touchwin(cw);
	    touchwin(mw);
	}
	when WS_CONFMON:
	    y = hero.y;
	    x = hero.x;
	    while (shoot_ok(winat(y, x)))
	    {
		y += delta.y;
		x += delta.x;
	    }
	    if ( x >= 0 && x < COLS && y >= 1 && y < LINES - 2 &&
			isalpha(mvwinch(mw, y, x))) {
		item = find_mons(y, x);
		tp = THINGPTR(item);
		if (save_throw(VS_MAGIC, tp))
		     msg("Nothing happens.");
		else
		     turn_on (*tp, ISHUH);
		delta.y = y;
		delta.x = x;
		runto(&delta, &hero);
	    }
	when WS_PARALYZE:
	    y = hero.y;
	    x = hero.x;
	    while (shoot_ok(winat(y, x)))
	    {
		y += delta.y;
		x += delta.x;
	    }
	    if ( x >= 0 && x < COLS && y >= 1 && y < LINES - 2 &&
			isalpha(mvwinch(mw, y, x))) {
		item = find_mons(y, x);
		tp = THINGPTR(item);
		if (save_throw(VS_WAND, tp))
		    msg("Nothing happens.");
		else {
		    tp->t_no_move = FREEZETIME;
		}
	    }
	when WS_MDEG:
	    y = hero.y;
	    x = hero.x;
	    while (shoot_ok(winat(y, x)))
	    {
		y += delta.y;
		x += delta.x;
	    }
	    if ( x >= 0 && x < COLS && y >= 1 && y < LINES - 2 &&
			isalpha(mvwinch(mw, y, x))) {
		item = find_mons(y, x);
		tp = THINGPTR(item);
		if (cursed) {
		     tp->t_stats.s_hpt *= 2;
		     msg("The %s appears to be stronger now!", 
			monsters[tp->t_index].m_name);
		}
		else {
		     if (on(*tp, CANSELL)) {
			luck++;
			aggravate();
		     }
		     tp->t_stats.s_hpt /= 2;
		     msg("The %s appears to be weaker now.", 
			monsters[tp->t_index].m_name);
		}
		delta.y = y;
		delta.x = x;
		runto(&delta, &hero);
	        if (tp->t_stats.s_hpt < 1)
		     killed(item, TRUE, TRUE);
	    }
	when WS_ANNIH:
	    y = hero.y;
	    x = hero.x;
	    while (shoot_ok(winat(y, x))) {
		y += delta.y;
		x += delta.x;
	    }
	    if ( x >= 0 && x < COLS && y >= 1 && y < LINES - 2 &&
			isalpha(mvwinch(mw, y, x))) {
		item = find_mons(y, x);
		tp = THINGPTR(item);
		if (cursed) {
		    register int m1, m2;
		    coord mp;
		    struct linked_list *titem;
		    int ch;
		    struct thing *th;
		    for (m1=tp->t_pos.x-1 ; m1 <= tp->t_pos.x+1 ; m1++) {
			for(m2=tp->t_pos.y-1 ; m2<=tp->t_pos.y+1 ; m2++) {
			    ch = winat(m2,m1);
			    if (shoot_ok(ch) && ch != PLAYER) {
				mp.x = m1;	/* create it */
				mp.y = m2;
				titem = new_item(sizeof(struct thing));
				new_monster(titem,tp->t_index,&mp,FALSE);
				th = THINGPTR(titem);
				turn_on (*th, ISMEAN);
				runto(&mp,&hero);
			    }
			}
		    }
		    delta.y = y;
		    delta.x = x;
		    turn_on (*tp, ISMEAN);
		    runto(&delta, &hero);
		}
		else { /* if its a UNIQUE it might still live */
		    tp = THINGPTR(item);
		    if (on(*tp, ISUNIQUE) && save_throw(VS_MAGIC, tp)) {
			tp->t_stats.s_hpt /= 2;
			if (tp->t_stats.s_hpt < 1) {
			     killed(item, FALSE, TRUE);
			     msg("You have disintegrated the %s.", 
				    monsters[tp->t_index].m_name);
			}
			else {
			    delta.y = y;
			    delta.x = x;
			    runto(&delta, &hero);
			    msg("The %s appears wounded.",
				monsters[tp->t_index].m_name);
			}
		    }
		    else {
			if (on(*tp, CANSELL)) {
			    luck++;
			    aggravate();
			}
			msg("You have disintegrated the %s.", 
				monsters[tp->t_index].m_name);
			killed (item, FALSE, TRUE);
		    }
		}
	    }
	when WS_NOTHING:
	    msg("Nothing happens.");
	when WS_INVIS: {
	    if (cursed) {
		register int x1, y1, x2, y2;
		int zapped = FALSE;
		if ((rp = roomin(&hero)) == NULL) {
		    x1 = max(hero.x - 1, 0);
		    y1 = max(hero.y - 1, 0);
		    x2 = min(hero.x + 1, COLS - 1);
		    y2 = min(hero.y + 1, LINES - 3);
		}
		else {
		    x1 = rp->r_pos.x;
		    y1 = rp->r_pos.y;
		    x2 = rp->r_pos.x + rp->r_max.x;
		    y2 = rp->r_pos.y + rp->r_max.y;
		}
		for (item = mlist; item != NULL; item = next(item)) {
		    tp = THINGPTR(item);
		    if (tp->t_pos.x >= x1 && tp->t_pos.x <= x2 &&
		    	tp->t_pos.y >= y1 && tp->t_pos.y <= y2) {
			turn_on(*tp, ISINVIS);
			turn_on(*tp, ISRUN);
			turn_off(*tp, ISDISGUISE);
			runto(&tp->t_pos, &hero);
			zapped = TRUE;
		    }
		}
		if (zapped)
		    msg("The monsters seem to have all disappeared.");
		else
		    msg("Nothing happens.");
	    }
	    else {
		y = hero.y;
		x = hero.x;
		while (shoot_ok(winat(y, x)))
		{
		    y += delta.y;
		    x += delta.x;
		}
		if ( x >= 0 && x < COLS && y >= 1 && y < LINES - 2 &&
			isalpha(mvwinch(mw, y, x))) {
		    item = find_mons(y, x);
		    tp = THINGPTR(item);
		    if (blessed) {
			turn_off(*tp, ISINVIS);
			turn_off(*tp, ISSHADOW);
			msg("The %s appears.", monsters[tp->t_index].m_name);
		    }
		    else {
			turn_on(*tp, ISINVIS);
			msg("The %s disappears.", 
				monsters[tp->t_index].m_name);
		    }
		}
		else
		    msg("Nothing happens.");
	    }
	    light(&hero);
	}
    when WS_BLAST: {
		register int ch;
        register struct linked_list *item, *ip;
        register struct object *obj;
        register struct trap *trp;

        item = spec_item(WEAPON, GRENADE, 0, 0);
        obj = (struct object *) ldata(item);
        obj->o_count = 1;
        obj->o_flags |= ISKNOW;
        msg("BOOOM!");
        aggravate();
        obj->o_pos.x = hero.x;
        obj->o_pos.y = hero.y;
        for ( ; ; ) {
            obj->o_pos.y += delta.y;
            obj->o_pos.x += delta.x;
            if (!ce(obj->o_pos, hero) &&
                    cansee(unc(obj->o_pos)) &&
                    mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ') {
                mvwaddch(cw, obj->o_pos.y, obj->o_pos.x,
                            show(obj->o_pos.y, obj->o_pos.x));
            }
            if (shoot_ok(ch = winat(obj->o_pos.y, obj->o_pos.x))
                    && ch != DOOR && !ce(obj->o_pos, hero)) {
                if (cansee(unc(obj->o_pos)) &&
						ntraps + 1 < MAXTRAPS + MAXTRAPS &&
                        mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ') {
					mvwaddch(cw, obj->o_pos.y, obj->o_pos.x, TRAPDOOR);
                    draw(cw);
                }
                if (isatrap(ch)) {
                    trp = trap_at(obj->o_pos.y, obj->o_pos.x);
                    if (trp != NULL) {
                        trp->tr_type = TRAPDOOR;
                        trp->tr_flags |= ISFOUND;
                        trp->tr_show = TRAPDOOR;
                    }
                }
                else if (isalpha(ch))
                    hit_monster(unc(obj->o_pos), obj, &player);
                else if ((ch == FLOOR || ch == PASSAGE)
						&& ntraps + 1 < MAXTRAPS + MAXTRAPS) {
					mvaddch(obj->o_pos.y, obj->o_pos.x, TRAPDOOR);
                    traps[ntraps].tr_type = TRAPDOOR;
                    traps[ntraps].tr_flags = ISFOUND;
                    traps[ntraps].tr_show = TRAPDOOR;
                    traps[ntraps].tr_pos.y = obj->o_pos.y;
                    traps[ntraps++].tr_pos.x = obj->o_pos.x;
                }
                else if (ch == POTION || ch == SCROLL || ch == FOOD
                        || ch == WEAPON || ch == RING || ch == ARMOR
                        || ch == STICK || ch == ARTIFACT || ch == GOLD) {
                    if ((ip = find_obj(obj->o_pos.y, obj->o_pos.x))
						    != NULL) {
						detach(lvl_obj, ip);
                        discard(ip);
                    }
                    mvaddch(obj->o_pos.y, obj->o_pos.x,
							roomin(&obj->o_pos) == NULL ? PASSAGE : FLOOR);
                }
                continue;
            }
            break;
        }
        discard(item);
    }
	otherwise:
	    msg("What a bizarre schtick!");
    }
}

/*
 * drain:
 *	Do drain hit points from player shtick
 */

void
drain(int ymin, int ymax, int xmin, int xmax)
{
    register int i, j, count;
    register struct thing *ick;
    register struct linked_list *item;

    /*
     * First count how many things we need to spread the hit points among
     */
    count = 0;
    for (i = ymin; i <= ymax; i++)
	for (j = xmin; j <= xmax; j++)
	    if (isalpha(mvwinch(mw, i, j)))
		count++;
    if (count == 0)
    {
	msg("You have a tingling feeling.");
	return;
    }
    count = pstats.s_hpt / count;
    pstats.s_hpt /= 2;
    /*
     * Now zot all of the monsters
     */
    for (i = ymin; i <= ymax; i++)
	for (j = xmin; j <= xmax; j++)
	    if (isalpha(mvwinch(mw, i, j)) &&
	        ((item = find_mons(i, j)) != NULL))
	    {
		ick = THINGPTR(item);
		if (on(*ick, CANSELL)) {
		    luck++;
		    aggravate();
		}
		if ((ick->t_stats.s_hpt -= count) < 1)
		    killed(item, (cansee(i, j) && !on(*ick, ISINVIS)), TRUE);
	    }
}

/*
 * charge a wand for wizards.
 */
char *
charge_str(struct object *obj)
{
    static char buf[20];

    if (!(obj->o_flags & ISKNOW))
	buf[0] = '\0';
    else if (terse)
	sprintf(buf, " [%d]", obj->o_charges);
    else if (obj->o_charges == 1)
	sprintf(buf, " [%d charge]", obj->o_charges);
    else
	sprintf(buf, " [%d charges]", obj->o_charges);
    return buf;
}


/*
 * shoot_bolt fires a bolt from the given starting point in the
 * 	      given direction
 */

int
shoot_bolt(struct thing *shooter, coord start, coord dir, int get_points, int reason, char *name, int damage)
{
    register int dirch, ch;
    register int used, change;
    register int y, x;
    coord pos;
    coord spotpos[BOLT_LENGTH];
    int ret_val = FALSE;	/* True if monster gets killed */

    switch (dir.y + dir.x) {
	when 0: dirch = '/';
	when 1: case -1: dirch = (dir.y == 0 ? '-' : '|');
	when 2: case -2: dirch = '\\';
    }
    pos.y = start.y + dir.y;
    pos.x = start.x + dir.x;
    used = FALSE;
    change = FALSE;
    for (y = 0; y < BOLT_LENGTH && !used; y++)
    {
	ch = winat(pos.y, pos.x);
	spotpos[y] = pos;

	/* Are we at hero? */
	if (ce(pos, hero)) goto at_hero;

	switch (ch)
	{
	    case SECRETDOOR:
	    case '|':
	    case '-':
	    case ' ':
		if (dirch == '-' || dirch == '|') {
		    dir.y = -dir.y;
		    dir.x = -dir.x;
		}
		else switch (ch) {
		    case '|':
		    case '-':
		    case SECRETDOOR:
			{
			    register struct room *rp;

			    rp = roomin(&pos);
			    if (pos.y == rp->r_pos.y ||
				pos.y == rp->r_pos.y + rp->r_max.y - 1) {
				dir.y = -dir.y;
				change ^= TRUE;
			    }
			    if (pos.x == rp->r_pos.x ||
				pos.x == rp->r_pos.x + rp->r_max.x - 1) {
				dir.x = -dir.x;
				change ^= TRUE;
			    }
			}
			break;
		    default:	/* A wall */
			{
			    int chy = CCHAR( mvinch(pos.y-dir.y, pos.x+dir.x) ),
				 chx = CCHAR( mvinch(pos.y+dir.y, pos.x-dir.x) );

			    if (chy != WALL && chy != SECRETDOOR &&
				chy != '-' && chy != '|') {
				dir.y = -dir.y;
				change = TRUE;
			    }
			    else if (chx != WALL && chx != SECRETDOOR &&
				     chx != '-' && chx != '|') {
				    dir.x = -dir.x;
				    change = TRUE;
				}
			    else {
				dir.y = -dir.y;
				dir.x = -dir.x;
			    }
			}
		}

		/* Do we change how the bolt looks? */
		if (change) {
		    change = FALSE;
		    if (dirch == '\\') dirch = '/';
		    else if (dirch == '/') dirch = '\\';
		}

		y--;
		msg("The %s bounces!", name);
		break;
	    default:
		if (isalpha(ch)) {
		    register struct linked_list *item;
		    register struct thing *tp;
		    register char *mname;

		    item = find_mons(unc(pos));
		    if (item == NULL) {
			debug("Can't find monster %c @ %d %d.", ch, unc(pos));
			continue;
		    }
		    tp = THINGPTR(item);
		    mname = monsters[tp->t_index].m_name;

		    if (!save_throw(VS_MAGIC, tp)) {
			if (on(*tp, ISDISGUISE) &&
			    (tp->t_type != tp->t_disguise) &&
			    off(player, ISBLIND)) {
			    msg("Wait! That's a %s!", mname);
			    turn_off(*tp, ISDISGUISE);
			}

			tp->t_wasshot = TRUE;

			if (on(*tp, CANSELL)) {
			    luck++;
			    aggravate();
			}

			/* Hit the monster -- does it do damage? */
			if ((strcmp(name, "ice") == 0 && on(*tp, NOCOLD)) ||
			    (strcmp(name, "ice") == 0 && on(*tp, ISUNDEAD)) ||
			    (strcmp(name,"flame")== 0 && on(*tp, NOFIRE)) ||
			    (strcmp(name,"lightning bolt")==0 && 
							on(*tp,NOBOLT))) {
			    msg("The %s has no effect on the %s.", name,
				on(player, ISBLIND) ? "monster" : mname);
			}

			else if ((strcmp(name, "lightning bolt") == 0) &&
				 on(*tp, BOLTDIVIDE)) {
				if (creat_mons(tp, tp->t_index, FALSE))
				    msg("The %s divides the %s.", name, mname);
				else msg("The %s has no effect on the %s.",
					name, on(player, ISBLIND) ? 
					"monster" : mname);
			}

			else if ((strcmp(name, "flame") == 0) &&
				on(*tp, CANBBURN)) {
			    msg("The %s is burned to death by the flame.",
					mname);
			    killed(item, FALSE, get_points);
			    ret_val = TRUE;
			}

			else if ((tp->t_stats.s_hpt -= damage) <= 0) {
			    msg("The %s kills the %s.", name,
				on(player, ISBLIND) ? "monster" : mname);
			    killed(item, FALSE, get_points);
			    ret_val = TRUE;
			}

			else {
			    msg("The %s hits the %s.", name,
				on(player, ISBLIND) ? "monster" : mname);
			    if (get_points) runto(&pos, &hero);
			}
			used = TRUE;
		    }
		    else if (isalpha(show(pos.y, pos.x))) {
			if (terse)
			    msg("%s misses.", name);
			else
			    msg("The %s whizzes past the %s.", name,
				on(player, ISBLIND) ? "monster" : mname);
			if (get_points) runto(&pos, &hero);
		    }
		}
		else if (pos.y == hero.y && pos.x == hero.x) {
at_hero: 
			running = FALSE;
		    if (!save(VS_MAGIC-(cur_armor != NULL && cur_armor->o_which==MITHRIL ? -5:0))) {
			if (terse)
			    msg("The %s hits you.", name);
			else
			    msg("You are hit by the %s.", name);
			if (cur_armor != NULL && 
				cur_armor->o_which == CRYSTAL_ARMOR &&
	 			(strcmp(name, "acid") == 0)) {
				damage = 0;
				msg("The acid splashes harmlessly against your armor!");
		       	}
		    	else if (((cur_armor != NULL &&
				    cur_armor->o_which == CRYSTAL_ARMOR) ||
			     	ISWEARING(R_ELECTRESIST)) &&
				(strcmp(name, "lightning bolt") == 0)) {
				damage = 0;
				if ( rnd(100) < 75 
					&& cur_weapon != NULL
					&& shooter != &player) {
				    cur_weapon->o_flags |= ISZAPPED;
				    cur_weapon->o_charges += (10 + rnd(15));
				}
				if (cur_weapon != NULL)
				    msg("Your armor and %s are covered with dancing blue lights!", weaps[cur_weapon->o_which].w_name);
				else
				    msg("Your armor is covered with dancing blue lights!");
		       	}
			else if(ISWEARING(R_FIRERESIST) &&
				(strcmp(name, "flame") == 0)) {
				damage = 0;
				msg("It flickers a moment and then fades.");
			}
			else if(ISWEARING(R_COLDRESIST) &&
				(strcmp(name, "ice") == 0)) {
				damage = 0;
				msg("It cracks and quickly melts.");
			}
			if ((pstats.s_hpt -= damage) <= 0) {
			    death(reason);
			    return(FALSE);
			}
			used = TRUE;

			/* Check for gas with special effects */
			if (!save(VS_BREATH) && !ISWEARING(R_BREATHE)) {
			    if (strcmp(name, "nerve gas") == 0) {
				if (no_command == 0) {
				    msg("The nerve gas paralyzes you.");
				    no_command = FREEZETIME;
				}
			    }
			    else if (strcmp(name, "sleeping gas") == 0) {
				if (no_command == 0) {
				    msg("The sleeping gas puts you to sleep.");
				    no_command = SLEEPTIME;
				}
			    }
			    else if (strcmp(name, "slow gas") == 0
				    && !ISWEARING(R_FREEDOM)) {
				msg("You feel yourself moving %sslower.",
					on(player, ISSLOW) ? "even " : "");
				if (on(player, ISSLOW))
				    lengthen(noslow, rnd(10) + 4);
				else {
				    turn_on(player, ISSLOW);
				    player.t_turn = TRUE;
				    fuse(noslow, 0, rnd(10)+4, AFTER);
				}
			    }
			    else if (strcmp(name, "fear gas") == 0) {
				if (!(on(player, ISFLEE) &&
				      (player.t_dest == &shooter->t_pos)) &&
				    (shooter != &player)) {
					if (off(player, SUPERHERO)) {
						turn_on(player, ISFLEE);
						player.t_dest = &shooter->t_pos;
						msg("The fear gas terrifies you.",
							monsters[shooter->t_index].m_name);
					}
					else
						msg("The fear gas has no effect.");
				}
			    }
			}
		    }
		    else
			msg("The %s whizzes by you.", name);
		}
		mvwaddch(cw, pos.y, pos.x, dirch);
		draw(cw);
	}
	pos.y += dir.y;
	pos.x += dir.x;
    }
    for (x = 0; x < y; x++)
	if ( spotpos[x].y >= 0 && spotpos[x].x >= 0 &&
			spotpos[x].y < LINES && spotpos[x].x < COLS)
	    mvwaddch(cw, spotpos[x].y, spotpos[x].x,
			show(spotpos[x].y, spotpos[x].x));

    return(ret_val);
}
