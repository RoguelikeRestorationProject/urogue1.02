/*
    encumb.c  -  Stuff to do with encumberence
   
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

/*
 * updpack:
 *	Update his pack weight and adjust fooduse accordingly
 */
void
updpack(int getmax)
{

	reg int topcarry, curcarry;

	if (getmax)
	    pstats.s_carry = totalenc();	/* get total encumb */
	curcarry = packweight();		/* get pack weight */
	topcarry = pstats.s_carry / 5;		/* 20% of total carry */
	if(curcarry > 4 * topcarry) {
	    if(rnd(100) < 80)
		foodlev = 3;			/* > 80% of pack */
	} else if(curcarry > 3 * topcarry) {
	    if(rnd(100) < 60)
		foodlev = 2;			/* > 60% of pack */
	} else
	    foodlev = 1;			/* <= 60% of pack */
	pstats.s_pack = curcarry;		/* update pack weight */
}


/*
 * packweight:
 *	Get the total weight of the hero's pack
 */
int
packweight(void)
{
	reg struct object *obj;
	reg struct linked_list *pc;
	reg int weight;

	weight = 0;
	for(pc = pack ; pc != NULL ; pc = next(pc)) {
	    obj = OBJPTR(pc);
	    weight += itemweight(obj) * obj->o_count;
	}
	if(ISWEARING(R_CARRYING))
	    weight -= (ring_value(R_CARRYING) * weight) / 4;
	if(weight < 0)		/* in case of artifacts and stuff */
	     weight = 0;

	return(weight);
}


/*
 * itemweight:
 *	Get the weight of an object
 */
int
itemweight(struct object *wh)
{
	reg int weight;
	reg int ac;

	weight = wh->o_weight;		/* get base weight */
	switch(wh->o_type) {
	    case ARMOR:
		/*
		 * subtract 20% for each enchantment
		 * this will add weight for negative items
		 */
		ac = armors[wh->o_which].a_class - wh->o_ac;
		weight = ((weight*5) - (weight*ac)) / 5;
		if (weight < 0) weight = 0;
	    when WEAPON:
		if ((wh->o_hplus + wh->o_dplus) > 0)
			weight /= 2;
	}
	if(wh->o_flags & ISCURSED)
		weight += weight / 5;	/* 20% more for cursed */
	else if(wh->o_flags & ISBLESSED)
		weight -= weight / 5;	/* 20% less for blessed */
	return(weight);
}


/*
 * playenc:
 *	Get hero's carrying ability above norm
 */
int
playenc(void)
{
	return ((pstats.s_str-8)*50);
}


/*
 * totalenc:
 *	Get total weight that the hero can carry
 */
int
totalenc(void)
{
	reg int wtotal;

	wtotal = NORMENCB + playenc();
	switch(hungry_state) {
		case F_OK:
		case F_HUNGRY:	;			/* no change */
		when F_WEAK:	wtotal -= wtotal / 10;	/* 10% off weak */
		when F_FAINT:	wtotal /= 2;		/* 50% off faint */
	}
	return(wtotal);
}



/*
 * whgtchk:
 *	See if the hero can carry his pack
 */
void
wghtchk(void)
{
	reg int dropchk, err = TRUE;
	reg int ch;

	inwhgt = TRUE;
	if (pstats.s_pack > pstats.s_carry) {
	    ch = CCHAR( mvwinch(stdscr, hero.y, hero.x) );
	    if((ch != FLOOR && ch != PASSAGE)) {
		extinguish(wghtchk);
		fuse(wghtchk,(void *)TRUE,1,AFTER);
		inwhgt = FALSE;
		return;
	    }
	    extinguish(wghtchk);
	    msg("Your pack is too heavy for you.");
	    do {
		dropchk = drop(NULL);
		if(dropchk == 0) {
		    mpos = 0;
		    msg("You must drop something.");
		}
		if(dropchk == TRUE)
		    err = FALSE;
	    } while(err);
	}
	inwhgt = FALSE;
}


/*
 * hitweight:
 *	Gets the fighting ability according to current weight
 * 	This returns a  +1 hit for light pack weight
 * 			 0 hit for medium pack weight
 *			-1 hit for heavy pack weight
 */
int
hitweight(void)
{
	return(2 - foodlev);
}
