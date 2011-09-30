/*
    trader.c  -  Anything to do with trading posts
   
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
#include <string.h>
#include <ctype.h>
#define o_tradevalue art_stats.ar_flags

/*
 * do_post:
 *	Put a trading post room and stuff on the screen
 */
void
do_post(void)
{
	coord tp;
	reg int i, worth;
	reg struct room *rp;
	reg struct object *op;
	reg struct linked_list *ll;

	for (rp = rooms; rp < &rooms[MAXROOMS]; rp++) {
	    rp->r_nexits = 0;			/* no exits */
	    rp->r_flags = ISGONE;		/* kill all rooms */
	}
	rp = &rooms[0];				/* point to only room */
	rp->r_flags = 0;			/* this room NOT gone */
	rp->r_max.x = 40;
	rp->r_max.y = 10;			/* 10 * 40 room */
	rp->r_pos.x = (COLS - rp->r_max.x) / 2;	/* center horizontal */
	rp->r_pos.y = 1;			/* 2nd line */
	draw_room(rp);				/* draw the only room */
	i = roll(4,10);				/* 10 to 40 items */
	for (; i > 0 ; i--) {			/* place all the items */
	    ll = new_thing();		/* get something */
	    attach(lvl_obj, ll);
	    op = OBJPTR(ll);
	    switch(luck) {
		when 0: break;
		when 1:
			if (rnd(3) == 0) {
			    op->o_flags |= ISCURSED;
			    op->o_flags &= ~ISBLESSED;
			}
		otherwise:
			if (rnd(luck)) {
			    op->o_flags |= ISCURSED;
			    op->o_flags &= ~ISBLESSED;
			}
	    }
	    op->o_flags |= (ISPOST | ISKNOW);	/* object in trading post */
	    do {
		rnd_pos(rp,&tp);
	    } until (CCHAR( mvinch(tp.y, tp.x) ) == FLOOR);
	    op->o_pos = tp;
		worth = get_worth(op);
		if (worth <= 0 && luck > 0)
			worth = rnd(luck * 100) + 50;
		else if (worth < 25)
			worth = 25;
		worth *= luck + 3;                          /* slightly expensive */
		worth = (worth/2) + (roll(6,worth)/6);      /* and randomized */
		op->o_tradevalue = worth;
	    mvaddch(tp.y,tp.x,op->o_type);
	}
	trader = 0;
	wmove(cw,12,0);
	waddstr(cw,"Welcome to Friendly Fiend's Flea Market\n\r");
	waddstr(cw,"=======================================\n\r");
	waddstr(cw,"$: Prices object that you stand upon.\n\r");
	waddstr(cw,"#: Buys the object that you stand upon.\n\r");
	waddstr(cw,"%: Trades in something in your pack for gold.\n\r");
	trans_line();
}

/*
 * price_it:
 *	Price the object that the hero stands on
 */
int
price_it(void)
{
	static char *bargain[] = {
	    "great bargain",
	    "quality product",
	    "exceptional find",
	};
	reg struct linked_list *item;
	reg struct object *obj;
	reg int worth;
	reg char *str;

	if (!open_market())		/* after buying hours */
	    return FALSE;
	if ((item = find_obj(hero.y,hero.x)) == NULL)
	    return FALSE;
	obj = OBJPTR(item);
	worth = obj->o_tradevalue;
	if (worth < 0) {
	    msg("That's not for sale.");
	    return FALSE;
	}
	str = typ_name(obj);
	msg("That %s is a %s for only %d pieces of gold.",str,
	    bargain[rnd(3)],worth);
	curprice = worth;		/* save price */
	strcpy(curpurch,str);		/* save item */
	return TRUE;
}

/*
 * buy_it:
 *	Buy the item on which the hero stands
 */
void
buy_it(void)
{
	reg int wh;

	if (purse <= 0) {
	    msg("You have no money.");
	    return;
	}
	if (curprice < 0) {		/* if not yet priced */
	    wh = price_it();
	    if (!wh)			/* nothing to price */
		return;
	    msg("Do you want to buy it? ");
	    do {
		wh = readchar(msgw);
		if (isupper(wh))
		    wh = tolower(wh);
		if (wh == ESCAPE || wh == 'n') {
		    msg("");
		    return;
		}
	    } until(wh == 'y');
	}
	mpos = 0;
	if (curprice > purse) {
	    msg("You can't afford to buy that %s!",curpurch);
	    return;
	}
	/*
	 * See if the hero has done all his transacting
	 */
	if (!open_market())
	    return;
	/*
	 * The hero bought the item here
	 */
	mpos = 0;
	wh = add_pack(NULL,FALSE);	/* try to put it in his pack */
	if (wh) {			/* he could get it */
	    purse -= curprice;		/* take his money */
	    ++trader;			/* another transaction */
	    trans_line();		/* show remaining deals */
	    curprice = -1;		/* reset stuff */
	    curpurch[0] = '\0';
	}
}

/*
 * sell_it:
 *	Sell an item to the trading post
 */
void
sell_it(void)
{
	reg struct linked_list *item;
	reg struct object *obj;
	reg int wo, ch;

	if (!open_market())		/* after selling hours */
	    return;

	if ((item = get_item("sell",0)) == NULL)
	    return;
	obj = OBJPTR(item);
	wo = get_worth(obj);
	if (wo <= 0) {
	    mpos = 0;
	    msg("We don't buy those.");
		if (ISWEARING(R_ADORNMENT) && rnd(7) < 3)
			msg("How about that %s ring instead?",
					r_stones[R_ADORNMENT]);
	    return;
	}
	if (wo < 25)
	    wo = 25;
	msg("Your %s is worth %d pieces of gold.",typ_name(obj),wo);
	msg("Do you want to sell it? ");
	do {
	    ch = readchar(msgw);
	    if (isupper(ch))
		ch = tolower(ch);
	    if (ch == ESCAPE || ch == 'n') {
		msg("");
		if (ISWEARING(R_ADORNMENT) && rnd(7) < 3)
			msg("How about that %s ring instead?",
					r_stones[R_ADORNMENT]);
		return;
	    }
	} until (ch == 'y');
	mpos = 0;
	if (obj->o_type != ARTIFACT)
		obj->o_tradevalue = wo;
	if (drop(item) == TRUE) {		/* drop this item */	
	    purse += wo;			/* give him his money */
	    ++trader;				/* another transaction */
	    msg("Sold %s.",inv_name(obj,TRUE));
	    trans_line();			/* show remaining deals */
	}
}

/*
 * open_market:
 *	Retruns TRUE when ok do to transacting
 */
int
open_market(void)
{
	register int maxtrans;

	maxtrans = ISWEARING(R_ADORNMENT) ? MAXPURCH + 4 : MAXPURCH;
	if (wizard || trader < maxtrans)
		return TRUE;
	else {
	    msg("The market is closed. The stairs are that-a-way.");
	    return FALSE;
	}
}

/*
 * typ_name:
 * 	Return the name for this type of object
 */
char *
typ_name(struct object *obj)
{
	static char buff[20];
	reg int wh;

	switch (obj->o_type) {
		case POTION:  wh = TYP_POTION;
		when SCROLL:  wh = TYP_SCROLL;
		when STICK:   wh = TYP_STICK;
		when RING:    wh = TYP_RING;
		when ARMOR:   wh = TYP_ARMOR;
		when WEAPON:  wh = TYP_WEAPON;
		when ARTIFACT:  wh = TYP_ARTIFACT;
		when FOOD:    wh = TYP_FOOD;
		otherwise:    wh = -1;
	}
	if (wh < 0)
		strcpy(buff,"unknown");
	else
		strcpy(buff,things[wh].mi_name);
	return (buff);
}

/*
 * get_worth:
 *	Calculate an objects worth in gold
 */
int
get_worth(struct object *obj)
{
	reg int worth, wh;

	worth = 0;
	wh = obj->o_which;
	switch (obj->o_type) {
	    when FOOD:
		worth = fd_data[wh].mi_worth;
		if (obj->o_flags & ISBLESSED)
		     worth *= 5;
	    when WEAPON:
		if (wh < MAXWEAPONS) {
		    worth = weaps[wh].w_worth;
		    worth *= obj->o_count * (2 + (4 * obj->o_hplus + 4 * obj->o_dplus));
		    if (obj->o_flags & ISSILVER)
			worth *= 2;
		    if (obj->o_flags & ISPOISON)
			worth *= 2;
		    if (obj->o_flags & ISZAPPED)
			worth += 20 * obj->o_charges;
		}
	    when ARMOR:
		if (wh < MAXARMORS) {
		    worth = armors[wh].a_worth;
		    worth *= (1 + (10 * (armors[wh].a_class - obj->o_ac)));
		}
	    when SCROLL:
		if (wh < MAXSCROLLS)
		    worth = s_magic[wh].mi_worth;
	    when POTION:
		if (wh < MAXPOTIONS)
		    worth = p_magic[wh].mi_worth;
	    when RING:
		if (wh < MAXRINGS) {
		    worth = r_magic[wh].mi_worth;
		    worth += obj->o_ac * 40;
		}
	    when STICK:
		if (wh < MAXSTICKS) {
		    worth = ws_magic[wh].mi_worth;
		    worth += 20 * obj->o_charges;
		}
	    when ARTIFACT:
		if (wh < MAXARTIFACT) 
		    worth = arts[wh].ar_worth;
	    otherwise:
		worth = 0;
	}
	if (obj->o_flags & ISPROT)	/* 300% more for protected */
	    worth *= 3;
	if (obj->o_flags &  ISBLESSED)	/* 50% more for blessed */
	    worth = worth * 3 / 2;
	if (obj->o_flags & ISCURSED)	/* half for cursed */
	    worth /= 2;
	if (obj->o_flags & (CANRETURN | ISOWNED))
	    worth *= 4;
	else if (obj->o_flags & CANRETURN)
	    worth *= 2;
	else if (obj->o_flags & ISLOST)
	    worth /= 3;
	if (worth < 0)
	    worth = 0;
	return worth;
}

/*
 * trans_line:
 *	Show how many transactions the hero has left
 */
void
trans_line(void)
{
	reg int adorned = ISWEARING(R_ADORNMENT);

	if (!wizard)
	    sprintf(prbuf,"You have %d transactions remaining.",
		max(0,(adorned ? MAXPURCH + 4 : MAXPURCH) - trader));
	else
	    sprintf(prbuf,
		"You have infinite transactions remaining oh great wizard.");
	mvwaddstr(cw,LINES - 3,0,prbuf);
}
