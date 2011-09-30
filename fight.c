/*
    fight.c  -  All the fighting gets done here
   
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
#include <ctype.h>
#include "rogue.h"
#include "mach_dep.h"

/* 
 * This are the beginning experience levels for all players
 * all further experience levels are computed by muliplying by 2
 */
static int e_levels[4] = {
	113,	/* Fighter */
	135,	/* Magician */
	87,	/* Cleric */
	72,	/* Thief */
};

struct matrix att_mat[5] = {
/* Base		Max_lvl,	Factor,		Offset,		Range */
{  10,		17,		2,		1,		2 },
{  9,		21,		2,		1,		5 },
{  10,		19,		2,		1,		3 },
{  10,		21,		2,		1,		4 },
{   7,		25,		2,		0,		2 }
};

void
do_fight(int y, int x, int tothedeath)
{
    if (!tothedeath)
	if (pstats.s_hpt < max_stats.s_hpt/3) {
	    if (!terse)
		msg("That's not wise.");
	    after = fighting = FALSE;
	    return;
	}
    if (isalpha(winat(hero.y+y, hero.x+x))) {
	after = fighting = TRUE;
	do_move(y, x);
    } else {
	if (fighting == FALSE)
	    msg("Nothing there.");
	after = fighting = FALSE;
    }
}

/*
 * fight:
 *	The player attacks the monster.
 */
int
fight(coord *mp, struct object *weap, int thrown)
{
    register struct thing *tp;
    register struct linked_list *item;
    register int did_hit = TRUE;

    /*
     * Find the monster we want to fight
     */
    if ((item = find_mons(mp->y, mp->x)) == NULL) {
	debug("Fight what @ %d,%d", mp->y, mp->x);
	return 0;
    }
    tp = (struct thing *) ldata(item);
    /*
     * Since we are fighting, things are not quiet so no healing takes
     * place.
     */
    player.t_quiet = 0;
    tp->t_quiet = 0;
    runto(mp, &hero);
    /*
     * Let him know it was really a mimic (if it was one).
     */
    if (on(*tp, ISDISGUISE) && (tp->t_type != tp->t_disguise) &&
	off(player, ISBLIND))
    {
	msg("Wait! That's a %s!", monsters[tp->t_index].m_name);
	turn_off(*tp, ISDISGUISE);
	did_hit = thrown;
    }
    if (on(*tp, CANSURPRISE) && off(player, ISBLIND)) {
	msg("Wait! There's a %s!", monsters[tp->t_index].m_name);
	turn_off(*tp, CANSURPRISE);
	did_hit = thrown;
    }

    if (did_hit)
    {
	register char *mname;

	did_hit = FALSE;
	mname = (on(player, ISBLIND)) ? "it" : monsters[tp->t_index].m_name;
	if (!can_blink(tp) &&
	    (off(*tp, MAGICHIT) || (weap != NULL && (weap->o_hplus > 0 || weap->o_dplus > 0))) &&
	    (off(*tp, BMAGICHIT) || (weap != NULL && (weap->o_hplus > 1 || weap->o_dplus > 1))) &&
	    roll_em(&player, tp, weap, thrown, cur_weapon))
	{
	    did_hit = TRUE;
	    tp->t_wasshot = TRUE;

		if (thrown) {
			if (weap != NULL && weap->o_type == WEAPON
					&& weap->o_which == GRENADE) {
				msg("BOOOM!");
				aggravate();
			}
			thunk(weap, mname);
		}
	    else
		hit(NULL, mname);

	    /* If the player hit a rust monster, he better have a + weapon */
	    if (on(*tp, CANRUST)) {
		if (!thrown && (weap != NULL) &&
		    (weap->o_flags & ISMETAL) &&
		    !(weap->o_flags & ISPROT) &&
			!(weap->o_flags & ISSILVER) &&
		    (weap->o_hplus < 1) && (weap->o_dplus < 1)) {
		    if (rnd(100) < 50) weap->o_hplus--;
		    else weap->o_dplus--;
		    msg(terse ? "Your %s weakens!"
			  : "Your %s appears to be weaker now!",
		        weaps[weap->o_which].w_name);   
		}
		else if (!thrown && weap != NULL && (weap->o_flags & ISMETAL))
		    msg(terse ? "" : "The rust vanishes from your %s!",
			weaps[weap->o_which].w_name);   
	    }
		
	    /* flammable monsters die from burning weapons */
	    if ( thrown && on(*tp, CANBBURN) && 
			(weap != NULL && weap->o_flags & CANBURN) &&
			!save_throw(VS_WAND, tp)) {
		msg("The %s vanishes in a ball of flame.", 
			monsters[tp->t_index].m_name);
		killed(item, TRUE, TRUE);
	    }

	    /* fireproof monsters laugh at you when burning weapon hits */
	    if ( thrown && on(*tp, NOFIRE) && (weap != NULL && weap->o_flags & CANBURN)) 
		msg("The %s laughs as the %s bounces.", 
			monsters[tp->t_index].m_name,
			weaps[weap->o_which].w_name);

	    /* If the player hit something that shrieks, wake the dungeon */
	    if (on(*tp, CANSHRIEK)) {
		turn_off(*tp, CANSHRIEK);
		if (on(player, CANHEAR)) {
		    msg("You are stunned by the %s's shriek.", mname);
		    no_command += 4 + rnd(8);
		}
		else if (off(player, ISDEAF))
		    msg("The %s emits a piercing shriek.", mname);
		else
		    msg("The %s seems to be trying to make some noise.", mname);
		aggravate();
		if (rnd(wizard ? 3 : 39) == 0 && cur_armor != NULL 
		    && cur_armor->o_which == CRYSTAL_ARMOR) {

		    register struct linked_list *item;
		    register struct object *obj = NULL;

		    for (item = pack; item != NULL; item = next(item)) {
			obj = (struct object *) ldata(item);
			if (obj == cur_armor)
			    break;
		    }
		    if (item == NULL) {
			debug("Can't find crystalline armor being worn.");
		    }
		    else {
			msg("Your armor shatters from the shriek.");
			cur_armor = NULL;
			detach(pack, item);
			freeletter(item);
			discard(item);
			inpack--;
		    }
		}
	    }

	    /* If the player hit something that can surprise, it can't now */
	    if (on(*tp, CANSURPRISE)) turn_off(*tp, CANSURPRISE);

	    /* If the player hit something that can summon, it will try to */
	    if (on(*tp, CANSUMMON) && rnd(40) < tp->t_stats.s_lvl) {
	        register char *helpname;
	        register int which, i;

		turn_off(*tp, CANSUMMON);
	        msg("The %s summons help!", mname);
	        helpname = monsters[tp->t_index].m_typesum;
	        for (which=1; which<NUMMONST; which++) {
		     if (strcmp(helpname, monsters[which].m_name) == 0)
		         break;
	        }
	        if (which >= NUMMONST)
		     debug("Couldn't find summoned one.");

		/* summoned monster was genocided */
		if (which < NUMMONST && 
		     !monsters[which].m_normal &&
		     !monsters[which].m_wander) {
		     msg("The %s becomes very annoyed at you!", mname);
		     if (on(*tp, ISSLOW))
			turn_off(*tp, ISSLOW);
		     else
			turn_on(*tp, ISHASTE);
		}
		else
	             for (i=0; i<monsters[tp->t_index].m_numsum; i++)
			creat_mons(&player, which, FALSE);
	    }

	    /* Can the player confuse? */
	    if (on(player, CANHUH) && !thrown)
	    {
		msg("Your hands stop glowing red!");
		msg("The %s appears confused.", mname);
		turn_on(*tp, ISHUH);
		turn_off(player, CANHUH);
	    }
	    /* Merchants just disappear if hit */
	    /* increases prices and curses objects from now on though */
	    if (on(*tp, CANSELL)) {
		msg("The %s disappears with his wares in a flash.",mname);
		killed(item, FALSE, FALSE);
		aggravate();
		luck++;
	    }

	    else if (tp->t_stats.s_hpt <= 0)
		killed(item, TRUE, TRUE);

	    /* If the monster is fairly intelligent and about to die, it
	     * may turn tail and run.
	     */
	    else if ((tp->t_stats.s_hpt < max(10, tp->maxstats.s_hpt/10)) &&
		     (rnd(25) < tp->t_stats.s_intel)) {
		turn_on(*tp, ISFLEE);

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
	    }
	}
	else {
	    if (thrown) 
		bounce(weap, mname);
	    else
		miss(NULL, mname);
	}
    }
    count = 0;
    return did_hit;
}

/*
 * attack:
 *	The monster attacks the player
 */
int
attack(struct thing *mp, struct object *weapon, int thrown)
{
    register char *mname;
    register int did_hit = FALSE;

    /* If the monster is in a wall, it cannot attack */
    if (on(*mp, ISINWALL)) 
	return(FALSE);

    /* If two monsters start to gang up on our hero, stop fight mode
     */
    if (fighting) {
	if (beast == NULL)
	    beast = mp;
	else if (beast != mp)
	    fighting = FALSE;
    }

    /*
     * Since this is an attack, stop running and any healing that was
     * going on at the time.
     */
    running = FALSE;
    player.t_quiet = 0;
    mp->t_quiet = 0;

    if (on(*mp, ISDISGUISE) && off(player, ISBLIND))
	turn_off(*mp, ISDISGUISE);
    mname = on(player, ISBLIND) ? "the monster" : monsters[mp->t_index].m_name;
    if (roll_em(mp, &player, weapon, thrown, wield_weap(weapon, mp)))
    {
	did_hit = TRUE;

	if (thrown) m_thunk(weapon, mname);
	else hit(mname, NULL);

	if (pstats.s_hpt <= 0) {
	    death(mp->t_index);	/* Bye bye life ... */
	    return TRUE;
	}

	/*
	 * surprising monsters appear after they shoot at you 
	 */
	if (thrown) {
	    if (on(*mp, CANSURPRISE)) 
		turn_off(*mp, CANSURPRISE);
	}
	if (!thrown) {
	    /*
	     * If a vampire hits, it may take half your hit points
	     */
	    if (on(*mp, CANSUCK) && 
		!save(VS_MAGIC - 
		    ((cur_armor != NULL && cur_armor->o_which == MITHRIL) ?
			-5 : 0))) {
		if (pstats.s_hpt == 1) {
		    death(mp->t_index);
		    return TRUE;
		}
		else {
		    pstats.s_hpt /= 2;
		    msg("You feel your life force being drawn from you.");
		}
	    }

	    /*
	     * monsters hitting hard can shatter crystalline armor
	     * or cause it to begin ringing if they are strong enough
	     */
	    if (cur_armor != NULL && cur_armor->o_which == CRYSTAL_ARMOR) {
		if (rnd(mp->t_stats.s_str + (cur_armor->o_ac/2)) > 20) {

		    register struct linked_list *item;
		    register struct object *obj = NULL;

		    for (item = pack; item != NULL; item = next(item)) {
			obj = (struct object *) ldata(item);
			if (obj == cur_armor)
			    break;
		    }
		    if (item == NULL) {
			debug("Can't find crystalline armor being worn.");
		    }
		    else {
			msg("Your armor is shattered by the blow.");
			cur_armor = NULL;
			detach(pack, item);
			freeletter(item);
			discard(item);
			inpack--;
		    }
		}
		else if (rnd(mp->t_stats.s_str) > 15) {
		    msg("Your armor rings from the blow.");
		    aggravate();
		}
	    }

	    /*
	     * Stinking monsters make player weaker (to hit)
	     */
	    if (on(*mp, CANSTINK)) {
		turn_off(*mp, CANSTINK);
		if (!save(VS_POISON)) {
		    if (on(player, CANSCENT)) {
			msg("You pass out from the stench of the %s.", mname);
			no_command += 4 + rnd(8);
		    }
		    else if (off(player, ISUNSMELL))
			msg("The stench of the %s sickens you.", mname);
		    if (on(player, HASSTINK)) 
			lengthen(unstink, STINKTIME);
		    else {
			turn_on(player, HASSTINK);
			fuse(unstink, 0, STINKTIME, AFTER);
		    }
		}
	    }

	    /*
	     * Chilling monster reduces strength each time unless you are
	     * wearing crystal armor
	     */
	    if (on(*mp, CANCHILL) && 
		(cur_armor == NULL || cur_armor->o_which != CRYSTAL_ARMOR)) {

		msg("You cringe at the %s's chilling touch.", mname);
		if (!ISWEARING(R_SUSABILITY)) {
		    chg_str(-1, FALSE, TRUE);
		    if (lost_str == 0)
			fuse(res_strength, 0, CHILLTIME, AFTER);
		    else lengthen(res_strength, CHILLTIME);
		}
	    }

	    /*
	     * itching monsters reduce dexterity (temporarily)
	     */
	    if (on(*mp, CANITCH) && !save(VS_POISON)) {
		msg("The claws of the %s scratch you!", mname);
		if(ISWEARING(R_SUSABILITY)) {
		    msg("The scratch has no effect.");
		}
		else {
		    msg("You feel a burning itch.");
		    turn_on(player, HASITCH);
		    chg_dext(-1, FALSE, TRUE);
		    fuse(un_itch, 0, roll(4,6), AFTER);
		}
	    }


	    /*
	     * If a hugging monster hits, it may SQUEEEEEEEZE
	     * unless you are wearing crystal armor
	     */
	    if (on(*mp, CANHUG) && 
		(cur_armor == NULL || cur_armor->o_which != CRYSTAL_ARMOR)) {
		if (roll(1,20) >= 18 || roll(1,20) >= 18) {
		    msg("The %s squeezes you against itself.", mname);
		    if ((pstats.s_hpt -= roll(2,8)) <= 0) {
			death(mp->t_index);
			return TRUE;
		    }
		}
	    }

	    /*
	     * If a disease-carrying monster hits, there is a chance the
	     * player will catch the disease
	     */
	    if (on(*mp, CANDISEASE) &&
		(rnd(pstats.s_const) < mp->t_stats.s_lvl) &&
		off(player, HASDISEASE)) {

		if (ISWEARING(R_HEALTH)) 
		    msg("The wound heals quickly.");
		else {
		    turn_on(player, HASDISEASE);
		    fuse(cure_disease, 0, roll(4,4) * SICKTIME, AFTER);
		    msg(terse ? "You have been diseased."
			: "You have contracted a disease!");
		}
	    }

	    /*
	     * If a rust monster hits, you lose armor
	     */
	    if (on(*mp, CANRUST)) {
		if (cur_armor != NULL &&
		    cur_armor->o_which != LEATHER &&
		    cur_armor->o_which != PADDED_ARMOR &&
		    cur_armor->o_which != CRYSTAL_ARMOR &&
		    cur_armor->o_which != MITHRIL &&
		    !(cur_armor->o_flags & ISPROT) &&
		    cur_armor->o_ac < pstats.s_arm+1) {
		    msg(terse ? "Your armor weakens!"
		        : "Your armor appears to be weaker now. Oh my!");
		    cur_armor->o_ac++;
		}
		else if (cur_armor != NULL && (cur_armor->o_flags & ISPROT) &&
            cur_armor->o_which != LEATHER &&
            cur_armor->o_which != PADDED_ARMOR &&
            cur_armor->o_which != CRYSTAL_ARMOR &&
            cur_armor->o_which != MITHRIL)
		    
			msg(terse ? "" : "The rust vanishes instantly!");
	    }

	    /* If a surprising monster hit you, you can see it now */
	    if (on(*mp, CANSURPRISE)) 
		turn_off(*mp, CANSURPRISE);

	    /*
	     * If an infesting monster hits you, you get a parasite or rot
	     */
	    if (on(*mp, CANINFEST) && rnd(pstats.s_const) < mp->t_stats.s_lvl) {
		if (ISWEARING(R_HEALTH)) 
		    msg("The wound quickly heals.");
		else {
		    turn_off(*mp, CANINFEST);
		    msg(terse ? "You have been infested."
			: "You have contracted a parasitic infestation!");
		    infest_dam++;
		    turn_on(player, HASINFEST);
		}
	    }

	    /*
	     * Ants have poisonous bites
	     */
	    if (on(*mp, CANPOISON) && !save(VS_POISON)) {
		if (ISWEARING(R_SUSABILITY))
		    msg(terse ? "Sting has no effect."
			      : "A sting momentarily weakens you.");
		else {
		    chg_str(-1, FALSE, FALSE);
		    msg(terse ? "A sting has weakened you." :
		    "You feel a sting in your arm and now feel weaker.");
		}
	    }

	    /*
	     * Cause fear by touching
	     */
	    if (on(*mp, TOUCHFEAR)) {
		turn_off(*mp, TOUCHFEAR);
		if (!save(VS_WAND - 
		    ((cur_armor != NULL && cur_armor->o_which == MITHRIL) ?
			 -5 : 0)) 
		    && !(on(player, ISFLEE) 
		    && (player.t_dest == &mp->t_pos))) {
				if (off(player, SUPERHERO)) {
					turn_on(player, ISFLEE);
					player.t_dest = &mp->t_pos;
					msg("The %s's touch terrifies you.", mname);
				}
				else
					msg("The %s's touch feels cold and clammy.", mname);
		}
	    }

	    /*
	     * Suffocating our hero
	     */
	    if (on(*mp, CANSUFFOCATE) && (rnd(100) < 15) &&
		(find_slot(suffocate) == NULL)) {
		turn_on(*mp, DIDSUFFOCATE);
		msg("The %s is beginning to suffocate you.", mname);
		fuse(suffocate, 0, roll(4,2), AFTER);
	    }

	    /*
	     * Turning to stone
	     */
	    if (on(*mp, TOUCHSTONE)) {
		turn_off(*mp, TOUCHSTONE);
		if (on(player, CANINWALL))
			msg("The %s's touch has no effect.", mname);
		else {
		    if (!save(VS_PETRIFICATION) && rnd(100) < 3) {
		        msg("Your body begins to solidify.");
		        msg("You are turned to stone !!! --More--");
		        wait_for(msgw,' ');
			death(D_PETRIFY);
			return TRUE;
		    }
		    else {
			msg("The %s's touch stiffens your limbs.", mname);
			no_command = rnd(STONETIME) + 2;
		    }
		}
	    }

	    /*
	     * Wraiths might drain energy levels
	     */
	    if ((on(*mp, CANDRAIN) || on(*mp, DOUBLEDRAIN)) && rnd(100) < 15) {
		lower_level(mp->t_index);
		if (on(*mp, DOUBLEDRAIN)) 
		    lower_level(mp->t_index);
		turn_on(*mp, DIDDRAIN);  
	    }

	    /*
	     * Violet fungi and others stop the poor guy from moving
	     */
	    if (on(*mp, CANHOLD) && off(*mp, DIDHOLD) 
			&& !ISWEARING(R_FREEDOM)) {
		turn_on(player, ISHELD);
		turn_on(*mp, DIDHOLD);
		hold_count++;
	    }

	    /*
	     * Sucker will suck blood and run
	     */
	    if (on(*mp, CANDRAW)) {
		turn_off(*mp, CANDRAW);
		turn_on(*mp, ISFLEE);
		msg("The %s sates itself with your blood.", mname);
		if ((pstats.s_hpt -= 12) <= 0) {
		    death(mp->t_index);
		    return TRUE;
		}
	    }

	    /*
	     * Bad smell will force a reduction in strength
	     */
	    if (on(*mp, CANSMELL)) {
		turn_off(*mp, CANSMELL);
		if (save(VS_MAGIC - 
			((cur_armor != NULL && cur_armor->o_which == MITHRIL) ?
				 -5 : 0)) 
			|| ISWEARING(R_SUSABILITY))
		    msg("You smell an unpleasant odor.");
		else {
		    int odor_str = -(rnd(6)+1);

		    if (on(player, CANSCENT)) {
			msg("You pass out from a foul odor.");
			no_command += 4 + rnd(8);
		    }
		    else if (off(player, ISUNSMELL))
			msg("You are overcome by a foul odor.");
		    if (lost_str == 0) {
			chg_str(odor_str, FALSE, TRUE);
			fuse(res_strength, 0, SMELLTIME, AFTER);
		    }
		    else 
			lengthen(res_strength, SMELLTIME);
		}
	    }

	    /*
	     * Paralyzation
	     */
	    if (on(*mp, CANPARALYZE)) {
		turn_off(*mp, CANPARALYZE);
		if (!save(VS_PARALYZATION) && no_command == 0) {
		    if (on(player, CANINWALL))
			msg("The %s's touch has no effect.", mname);
		    else {
			msg("The %s's touch paralyzes you.", mname);
			no_command = FREEZETIME;
		    }
		}
	    }

	    /*
	     * Rotting
	     */
	    if (on(*mp, CANROT)) {
		turn_off(*mp, CANROT);
		turn_on(*mp, DOROT);
	    }

	    if (on(*mp, STEALGOLD)) {
		/*
		 * Leperachaun steals some gold
		 */
		register int lastpurse;
		register struct linked_list *item;
		register struct object *obj;

		lastpurse = purse;
		purse -= GOLDCALC;
		if (!save(VS_MAGIC - 
			((cur_armor != NULL && cur_armor->o_which == MITHRIL) ?
				 -5 : 0)))
		    purse -= GOLDCALC + GOLDCALC + GOLDCALC + GOLDCALC;
		if (purse < 0)
		    purse = 0;
		if (purse != lastpurse) {
		    msg("Your purse feels lighter.");

		    /* Give the gold to the thief */
		    for (item=mp->t_pack; item != NULL; item=next(item)) {
			obj = (struct object *) ldata(item);
			if (obj->o_type == GOLD) {
			    obj->o_count += lastpurse - purse;
			    break;
			}
		    }

		    /* Did we do it? */
		    if (item == NULL) {	/* Then make some */
			item = new_item(sizeof *obj);
			obj = (struct object *) ldata(item);
			obj->o_type = GOLD;
			obj->o_count = lastpurse - purse;
			obj->o_hplus = obj->o_dplus = 0;
			strcpy(obj->o_damage, "0d0");
			strcpy(obj->o_hurldmg, "0d0");
			obj->o_ac = 11;
			obj->o_group = 0;
			obj->o_flags = 0;
			obj->o_mark[0] = '\0';
			obj->o_pos = mp->t_pos;

			attach(mp->t_pack, item);
		    }
		}

		if (rnd(2))
			turn_on(*mp, ISFLEE);
		turn_on(*mp, ISINVIS);
	    }

	    if (on(*mp, STEALMAGIC)) {
		register struct linked_list *list, *steal;
		register struct object *obj;
		register int worth = 0;

		/*
		 * Nymph's steal a magic item, look through the pack
		 * and pick out one we like.
		 */
		steal = NULL;
		for (list = pack; list != NULL; list = next(list))
		{
		    obj = (struct object *) ldata(list);
			if (rnd(33) == 0) {
				if (obj->o_flags & ISBLESSED)
					obj->o_flags &= ~ISBLESSED;
				else
					obj->o_flags |= ISCURSED;
				msg("You feel nimble fingers reach into your pack.");
			}
		    if (((obj != cur_armor && obj != cur_weapon &&
			obj != cur_ring[LEFT_1] && obj != cur_ring[LEFT_2] &&
			obj != cur_ring[LEFT_3] && obj != cur_ring[LEFT_4] &&
			obj != cur_ring[RIGHT_1] && obj != cur_ring[RIGHT_2] &&
			obj != cur_ring[RIGHT_3] && obj != cur_ring[RIGHT_4] &&
			!(obj->o_flags & ISPROT) && is_magic(obj))
				|| level > 95)
			&& get_worth(obj) > worth) {
			    steal = list;
			    worth = get_worth(obj);
		    }
		}
		if (steal != NULL)
		{
		    register struct object *obj;

		    obj = (struct object *) ldata(steal);
		    if (obj->o_count > 1 && obj->o_group == 0)
		    {
			register int oc;
			register struct linked_list *nitem;
			register struct object *op;

			oc = --(obj->o_count);
			obj->o_count = 1;
			nitem = new_item(sizeof *obj);
			op = (struct object *) ldata(nitem);
			*op = *obj;
			msg("She stole %s!", inv_name(obj, TRUE));
			obj->o_count = oc;
			attach(mp->t_pack, nitem);
		    }
		    else
		    {
			msg("She stole %s!", inv_name(obj, TRUE));
			obj->o_flags &= ~ISCURSED;
			dropcheck(obj);
			detach(pack, steal);
			freeletter(steal);
			attach(mp->t_pack, steal);
		        inpack--;
			if (obj->o_type == ARTIFACT)
			    has_artifact &= ~(1 << obj->o_which);
		    }
		    if (obj->o_flags & ISOWNED) {
			turn_on(*mp, NOMOVE);
			msg("The %s is transfixed by your ownership spell.",
				mname);
		    }
			if (rnd(2))
				turn_on(*mp, ISFLEE);
            turn_on(*mp, ISINVIS);
		    updpack(FALSE);
		}
	    }
	}
    }
    else {
	/* If the thing was trying to surprise, no good */
	if (on(*mp, CANSURPRISE)) turn_off(*mp, CANSURPRISE);

	if (on(*mp, DOROT)) {
	    msg(terse ? "You feel weaker."
		      : "Your skin crawls and you feel weaker.");

	    pstats.s_hpt -= 2;
	    if (pstats.s_hpt <= 0) {
		death(mp->t_index);	/* Bye bye life ... */
		return TRUE;
	    }
	}
	else if (thrown) 
	    m_bounce(weapon, mname);
	else 
	    miss(mname, NULL);
    }
    if (fight_flush)
	flushinp();
    count = 0;
    status(FALSE);
    return(did_hit);
}

/*
 * swing:
 *	returns true if the swing hits
 */
int
swing(int class, int at_lvl, int op_arm, int wplus)
{
    register int res = rnd(20)+1;
    register int need;

    need = att_mat[class].base -
	   att_mat[class].factor *
	   ((min(at_lvl, att_mat[class].max_lvl) -
	    att_mat[class].offset)/att_mat[class].range) +
	   (10 - op_arm);
    if (need > 20 && need <= 25) need = 20;

    return (res+wplus >= need);
}

/*
 * check_level:
 *	Check to see if the guy has gone up a level.
 */
void
check_level(void)
{
    register int i, j, add = 0;
    register int exp;
    int nsides = 0;

    i = 0;
    exp = e_levels[player.t_ctype];
    while (exp <= pstats.s_exp) {
	i++;
	exp *= 2L;
    }
    if (++i > pstats.s_lvl) {
	switch (player.t_ctype) {
	    when C_FIGHTER:	nsides = 12;
	    when C_MAGICIAN:	nsides = 4;
	    when C_CLERIC:	nsides = 8;
	    when C_THIEF:	nsides = 6;
	}

	/* Take care of multi-level jumps */
	for (j=0; j < (i-pstats.s_lvl); j++)
	    add += max(1, roll(1,nsides) + const_bonus());
	max_stats.s_hpt += add;
	if ((pstats.s_hpt += add) > max_stats.s_hpt)
	    pstats.s_hpt = max_stats.s_hpt;
	msg("Welcome, %s, to level %d.",
	    cnames[player.t_ctype][min(i-1, 10)], i);
	spell_power = 0; /* A new round of spells */
    }
    pstats.s_lvl = i;
}

/*
 * roll_em:
 *	Roll several attacks
 */
int
roll_em(struct thing *att_er, struct thing *def_er, struct object *weap, int hurl, struct object *cur_weapon)
{
    register struct stats *att, *def;
    register char *cp, *pcp;
    register int ndice, nsides, nplus, def_arm;
    register int did_hit = FALSE;
    register int prop_hplus, prop_dplus;

    /* Get statistics */
    att = &att_er->t_stats;
    def = &def_er->t_stats;

    prop_hplus = prop_dplus = 0;
    if (weap == NULL)
	cp = att->s_dmg;
    else if (hurl)
	if ((weap->o_flags&ISMISL) && cur_weapon != NULL &&
	  cur_weapon->o_which == weap->o_launch)
	{
	    cp = weap->o_hurldmg;
	    prop_hplus = cur_weapon->o_hplus;
	    prop_dplus = cur_weapon->o_dplus;
	}
	else
	    cp = (weap->o_flags&ISMISL ? weap->o_damage : weap->o_hurldmg);
    else
    {
	cp = weap->o_damage;
	/*
	 * Drain a staff of striking
	 */
	if (weap->o_type == STICK && weap->o_which == WS_HIT
	    && weap->o_charges == 0)
		{
		    strcpy(weap->o_damage, "0d0");
		    weap->o_hplus = weap->o_dplus = 0;
		}
    }
    for (;;)
    {
	int damage;
	int hplus = prop_hplus + (weap == NULL ? 0 : weap->o_hplus);
	int dplus = prop_dplus + (weap == NULL ? 0 : weap->o_dplus);

	/* Is attacker weak? */
	if (on(*att_er, HASSTINK)) hplus -= 2;

	/* Code changed by Bruce Dautrich 4/4/84 to fix bug */
	if (att_er == &player)	/* Is it the player? */
	{
	    hplus += hitweight();	/* adjust for encumberence */
	    dplus += hung_dam();	/* adjust damage for hungry player */
	    dplus += ring_value(R_ADDDAM);
	}
	ndice = atoi(cp);
	if ((cp = strchr(cp, 'd')) == NULL)	/* strchr was index */
	    break;
	nsides = atoi(++cp);
	if ((pcp = strchr(cp, '+')) != NULL) { 
		cp = pcp;
		nplus = atoi(++cp);
	}
	else nplus = 0;

	if (def == &pstats)
	{
	    if (cur_armor != NULL)
		def_arm = cur_armor->o_ac;
	    else
		def_arm = def->s_arm;
	    def_arm -= ring_value(R_PROTECT);
	}
	else
	    def_arm = def->s_arm;
	if ((weap != NULL && weap->o_type == WEAPON &&
		(weap->o_flags & ISSILVER) && 
		!save_throw(VS_MAGIC,def_er)) ||
	    swing(att_er->t_ctype, att->s_lvl, def_arm-dext_prot(def->s_dext),
		  hplus+str_plus(att->s_str)+dext_plus(att->s_dext)))
	{
	    register int proll;

	    proll = roll(ndice, nsides);
	    if (ndice + nsides > 0 && proll < 1)
		debug("Damage for %dd%d came out %d.", ndice, nsides, proll);
	    damage = dplus + proll + nplus + add_dam(att->s_str);

	    /* Check for half damage monsters */
	    if (on(*def_er, HALFDAMAGE) && weap != NULL &&
		!((weap->o_flags & CANBURN) && on(*def_er, CANBBURN)))
		damage /= 2;

	    /* undead get twice damage from silver weapons */
	    if (on(*def_er, ISUNDEAD) && 
			weap != NULL && weap->o_flags & ISSILVER) 
		damage *= 2;

	    /* Check for fireproof monsters */
	    if (on(*def_er, NOFIRE) && weap != NULL && 
			(weap->o_flags & CANBURN))
		damage = 0;

	    /* Check for poisoned weapons */
	    if ((weap != NULL) && (weap->o_flags & ISPOISON) 
			&& off(*def_er, ISUNDEAD)
			&& !save_throw(VS_POISON, def_er)) {
		damage = (def->s_hpt / 2) + 5;
		debug("Defender was hit by poison.");
	    }

	    /* Check for no-damage and division */
	    if (on(*def_er, BLOWDIVIDE) &&
			!((weap != NULL) && (weap->o_flags & CANBURN))) {
		damage = 0;
		creat_mons(def_er, def_er->t_index, FALSE);
	    }

		damage = max(0, damage);
	    def->s_hpt -= damage;	/* Do the damage */
		debug("Hit %s for %d (%d) ",
            monsters[def_er->t_index].m_name, damage, def_er->t_stats.s_hpt);
        if (att_er == &player && ISWEARING(R_VREGEN)) {
            damage = (ring_value(R_VREGEN) * damage) / 3;
            pstats.s_hpt = min(max_stats.s_hpt, pstats.s_hpt + damage);
        }

	    did_hit = TRUE;
	}
	if (cp == NULL || ((cp = strchr(cp, '/')) == NULL))
	    break;
	cp++;
    }
    return did_hit;
}

/*
 * prname:
 *	The print name of a combatant
 */

char *
prname(char *who, int upper)
{
    static char tbuf[LINELEN];

    *tbuf = '\0';
    if (who == 0)
	strcpy(tbuf, "you"); 
    else if (on(player, ISBLIND))
	strcpy(tbuf, "the monster");
    else
    {
	strcpy(tbuf, "the ");
	strcat(tbuf, who);
    }
    if (upper)
	*tbuf = (char) toupper(*tbuf);
    return tbuf;
}

/*
 * hit:
 *	Print a message to indicate a succesful hit
 */
void
hit(char *er, char *ee)
{
    register char *s = "";

    if (fighting)
	return;
    addmsg(prname(er, TRUE));
    if (terse)
	s = " hit";
    else
	switch (rnd(4))
	{
	    when 0: s = " scored an excellent hit on ";
	    when 1: s = " hit ";
	    when 2: s = (er == 0 ? " have injured " : " has injured ");
	    when 3: s = (er == 0 ? " swing and hit " : " swings and hits ");
	}
    addmsg(s);
    if (!terse)
	addmsg(prname(ee, FALSE));
    addmsg(".");
    endmsg();
}

/*
 * miss:
 *	Print a message to indicate a poor swing
 */
void
miss(char *er, char *ee)
{
    register char *s = "";

    if (fighting)
	return;
    addmsg(prname(er, TRUE));
    switch (terse ? 0 : rnd(4))
    {
	when 0: s = (er == 0 ? " miss" : " misses");
	when 1: s = (er == 0 ? " swing and miss" : " swings and misses");
	when 2: s = (er == 0 ? " barely miss" : " barely misses");
	when 3: s = (er == 0 ? " don't hit" : " doesn't hit");
    }
    addmsg(s);
    if (!terse)
	addmsg(" %s", prname(ee, FALSE));
    addmsg(".");
    endmsg();
}

/*
 * save_throw:
 *	See if a creature save against something
 */
int
save_throw(int which, struct thing *tp)
{
    register int need;

    need = 14 + which - tp->t_stats.s_lvl / 2;
    return (roll(1, 20) >= need);
}
/*
 * save:
 *	See if he saves against various nasty things
 */
int
save(int which)
{
    return save_throw(which, &player);
}

/*
 * dext_plus:
 *	compute to-hit bonus for dexterity
 */
int
dext_plus(int dexterity)
{
    return ((dexterity-10)/3);
}


/*
 * dext_prot:
 *	compute armor class bonus for dexterity
 */
int
dext_prot(int dexterity)
{
    return ((dexterity-9)/2);
}
/*
 * str_plus:
 *	compute bonus/penalties for strength on the "to hit" roll
 */
int
str_plus(int str)
{
    return((str-10)/3);
}

/*
 * add_dam:
 *	compute additional damage done for exceptionally high or low strength
 */
int
add_dam(int str)
{
    return((str-9)/2);
}

/*
 * hung_dam:
 *	Calculate damage depending on players hungry state
 */
int
hung_dam(void)
{
	reg int howmuch = 0;

	switch(hungry_state) {
		case F_OK:
		case F_HUNGRY:	howmuch = 0;
		when F_WEAK:	howmuch = -1;
		when F_FAINT:	howmuch = -2;
	}
	return howmuch;
}

/*
 * raise_level:
 *	The guy just magically went up a level.
 */
void
raise_level(void)
{
    if (pstats.s_exp < e_levels[player.t_ctype]/2)
	pstats.s_exp = e_levels[player.t_ctype];
    else pstats.s_exp *= 2;
    check_level();
}

/*
 * thunk:
 *	A missile hits a monster
 */
void
thunk(struct object *weap, char *mname)
{
    if (fighting)
	return;
    if (weap != NULL && weap->o_type == WEAPON)
	msg("The %s hits %s.",weaps[weap->o_which].w_name,prname(mname, FALSE));
    else
	msg("You hit %s.", prname(mname, FALSE));
}

/*
 * mthunk:
 *	 A missile from a monster hits the player
 */
void
m_thunk(struct object *weap, char *mname)
{
    if (fighting)
	return;
    if (weap != NULL && weap->o_type == WEAPON)
	msg("%s's %s hits you.", prname(mname, TRUE), weaps[weap->o_which].w_name);
    else
	msg("%s hits you.", prname(mname, TRUE));
}

/*
 * bounce:
 *	A missile misses a monster
 */
void
bounce(struct object *weap, char *mname)
{
    if (fighting)
	return;
    if (weap != NULL && weap->o_type == WEAPON)
	msg("The %s misses %s.",weaps[weap->o_which].w_name,prname(mname,FALSE));
    else
	msg("You missed %s.", prname(mname, FALSE));
}

/*
 * m_bounce:
	  A missile from a monster misses the player
 */
void
m_bounce(struct object *weap, char *mname)
{
    if (fighting)
	return;
    if (weap != NULL && weap->o_type == WEAPON)
	msg("%s's %s misses you.", prname(mname, TRUE), weaps[weap->o_which].w_name);
    else
	msg("%s misses you.", prname(mname, TRUE));
}

/*
 * remove a monster from the screen
 */
void
remove_monster(coord *mp, struct linked_list *item)
{
	struct thing *tp;

	tp = THINGPTR(item);
    mvwaddch(mw, mp->y, mp->x, ' ');
    mvwaddch(cw, mp->y, mp->x, tp->t_oldch);
    detach(mlist, item);
    turn_on(*tp, ISDEAD);
	attach(monst_dead, item);
}

/*
 * is_magic:
 *	Returns true if an object radiates magic
 */
int
is_magic(struct object *obj)
{
    switch (obj->o_type)
    {
	case ARMOR:
	    return obj->o_ac != armors[obj->o_which].a_class;
	when WEAPON:
	    return obj->o_hplus != 0 || obj->o_dplus != 0;
	when POTION:
	case SCROLL:
	case STICK:
	case RING:
	case ARTIFACT:
	    return TRUE;
    }
    return FALSE;
}

/*
 * killed:
 *	Called to put a monster to death
 */
void
killed(struct linked_list *item, int pr, int points)
{
    register struct thing *tp;
    register struct linked_list *pitem, *nexti;

    fighting = FALSE;
    tp = (struct thing *) ldata(item);

    if ( on(*tp, ISDEAD) )
		return; /* already dead, can't kill it twice! */

    if (pr)
    {
	addmsg(terse ? "Defeated " : "You have defeated ");
	if (on(player, ISBLIND))
	    msg("it.");
	else
	{
	    if (!terse)
		addmsg("the ");
	    msg("%s.", monsters[tp->t_index].m_name);
	}
    }

    /* Take care of any residual effects of the monster */
    check_residue(tp);

    if (points) {
	pstats.s_exp += tp->t_stats.s_exp;

	/*
	 * Do adjustments if he went up a level
	 */
	check_level();
    }

    /*
     * Empty the monsters pack
     */
    pitem = tp->t_pack;

    /*
     * Get rid of the monster.
     */
    while (pitem != NULL)
    {
	register struct object *obj;

	nexti = next(tp->t_pack);
	obj = (struct object *) ldata(pitem);
	obj->o_pos = tp->t_pos;
	detach(tp->t_pack, pitem);

	if (points) fall(pitem, FALSE);
	else discard(pitem);

	pitem = nexti;
    }

	remove_monster(&tp->t_pos, item);
}


/* Returns a pointer to the weapon the monster is wielding corresponding to
 * the given thrown weapon
 */

struct object *
wield_weap(struct object *weapon, struct thing *mp)
{
    int look_for;
    register struct linked_list *pitem;

    if (weapon == NULL)			/* BDG 1-3-87 */
        return(NULL);

    switch (weapon->o_which) {
	case BOLT:	/* Find the crossbow */
	    look_for = CROSSBOW;
	    break;
	case ARROW:	/* Find the bow */
	    look_for = BOW;
	    break;
	case SILVERARROW:	/* Find the bow */
	    look_for = BOW;
	    break;
	case ROCK:	/* find the sling */
	    look_for = SLING;
	    break;
	default:
	    return(NULL);
    }

    for (pitem=mp->t_pack; pitem; pitem=next(pitem))
	if (((struct object *) ldata(pitem))->o_which == look_for)
	    return((struct object *) ldata(pitem));

    return(NULL);
}
