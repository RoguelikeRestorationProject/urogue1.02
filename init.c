/*
    init.c  -  global variable initializaton
   
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


char *rainbow[NCOLORS] = {
	"red",		"blue",		"green",	"yellow",
	"black",	"brown",	"orange",	"pink",
	"purple",	"grey",		"white",	"silver",
	"gold",		"violet",	"clear",	"vermilion",
	"ecru",		"turquoise",	"magenta",	"amber",
	"topaz",	"plaid",	"tan",		"tangerine",
	"aquamarine",	"scarlet",	"khaki",	"crimson",
	"indigo",	"beige",	"lavender",	"saffron",
} ;

char *sylls[NSYLLS] = {
    "a", "ab", "ag", "aks", "ala", "an", "ankh", "app", "arg", "arze",
    "ash", "ban", "bar", "bat", "bek", "bie", "bin", "bit", "bjor",
    "blu", "bot", "bu", "byt", "comp", "con", "cos", "cre", "dalf",
    "dan", "den", "do", "e", "eep", "el", "eng", "er", "ere", "erk",
    "esh", "evs", "fa", "fid", "for", "fri", "fu", "gan", "gar",
    "glen", "gop", "gre", "ha", "he", "hyd", "i", "ing", "ion", "ip",
    "ish", "it", "ite", "iv", "jo", "kho", "kli", "klis", "la", "lech",
    "man", "mar", "me", "mi", "mic", "mik", "mon", "mung", "mur",
    "nej", "nelg", "nep", "ner", "nes", "nes", "nih", "nin", "o", "od",
    "ood", "org", "orn", "ox", "oxy", "pay", "pet", "ple", "plu", "po",
    "pot", "prok", "re", "rea", "rhov", "ri", "ro", "rog", "rok", "rol",
    "sa", "san", "sat", "see", "sef", "seh", "shu", "ski", "sna",
    "sne", "snik", "sno", "so", "sol", "sri", "sta", "sun", "ta",
    "tab", "tem", "ther", "ti", "tox", "trol", "tue", "turs", "u",
    "ulk", "um", "un", "uni", "ur", "val", "viv", "vly", "vom", "wah",
    "wed", "werg", "wex", "whon", "wun", "xo", "y", "yot", "yu",
    "zant", "zap", "zeb", "zim", "zok", "zon", "zum",
} ;

char *stones[NSTONES] = {
	"agate",		"alexandrite",		"amethyst",
	"azurite",		"carnelian",		"chrysoberyl",
	"chrysoprase",		"citrine",		"diamond",
	"emerald",		"garnet",		"hematite",
	"jacinth",		"jade",			"kryptonite",
	"lapus lazuli",		"malachite",		"moonstone",
	"obsidian",		"olivine",		"onyx",
	"opal",			"pearl",		"peridot",
	"quartz",		"rhodochrosite",	"ruby",
	"sapphire",		"sardonyx",		"serpentine",
	"spinel",		"tiger eye",		"topaz",
	"tourmaline",		"turquoise",
} ;

char *wood[NWOOD] = {
	"avocado wood",	"balsa",	"banyan",	"birch",
	"cedar",	"cherry",	"cinnibar",	"dogwood",
	"driftwood",	"ebony",	"eucalyptus",	"hemlock",
	"ironwood",	"mahogany",	"manzanita",	"maple",
	"oak",		"pine",		"redwood",	"rosewood",
	"teak",		"walnut",	"zebra wood", 	"persimmon wood",
} ;

char *metal[NMETAL] = {
	"aluminium",	"bone",		"brass",	"bronze",
	"copper",	"chromium",	"iron",		"lead",
	"magnesium",	"pewter",	"platinum",	"steel",
	"tin",		"titanium",	"zinc",
} ;

/*
 * init_player:
 *	roll up the rogue
 */
void
init_player(void)
{
    int special = rnd(10) ;

    pstats.s_lvl = 1 ;
    pstats.s_exp = 0L ;

    if (char_type == -1) {
	/* See what type character will be */
	wclear(hw) ;
	touchwin(hw) ;
	mvwaddstr(hw,2,0,"[1] Fighter\n[2] Magician\n[3] Cleric\n[4] Thief") ;
	mvwaddstr(hw, 0, 0, "What character class do you desire? ") ;
	draw(hw) ;
	char_type = ((readchar(hw) & 0177) - '0') ;
	while (char_type < 1 || char_type > 4) {
	    mvwaddstr(hw,0,0,"Please enter a character type between 1 and 4: ");
	    draw(hw) ;
	    char_type = ((readchar(hw) & 0177) - '0') ;
	}
	char_type-- ;
    }
    player.t_ctype = char_type ;
    player.t_quiet = 0;
    max_stats.s_hpt = pstats.s_hpt = 15 + rnd(5);
    pstats.s_str = 8 + rnd(5) ;
    pstats.s_intel = 8 + rnd(5) ;
    pstats.s_wisdom = 8 + rnd(5) ;
    pstats.s_dext = 8 + rnd(5) ;
    pstats.s_const = 10 + rnd(8) ;
    pstats.s_charisma = 8 + rnd(4) ;
    strcpy(pstats.s_dmg,"1d4");
    pstats.s_arm = 10 ;

    /* Now for the special ability */
    switch (char_type) {
	when C_FIGHTER:
	    pstats.s_str = (special == 4) ? 18 : 16 ;
	    pstats.s_const += rnd(4);
	    strcpy(pstats.s_dmg,"2d4") ;
	    pstats.s_arm = 8;
	when C_MAGICIAN:
	    pstats.s_intel = (special == 4) ? 18 : 16 ;
	when C_CLERIC:
	    pstats.s_wisdom = (special == 4) ? 18 : 16 ;
	    pstats.s_str += rnd(4);
	when C_THIEF:
	    pstats.s_dext = (special == 4) ? 18 : 16 ;
    }
    if (pstats.s_const > 15)
	pstats.s_hpt += pstats.s_const - 15;
    max_stats = pstats ;
    pack = NULL ;
    resurrect = pstats.s_const;
    pstats.s_carry = totalenc();
}

/*
 * Contains definitions and functions for dealing with things like
 * potions and scrolls
 */

/*
 * init_things
 *	Initialize the probabilities for types of things
 */
void
init_things(void)
{
    register struct magic_item *mp ;

    for (mp = &things[1] ; mp < &things[NUMTHINGS] ; mp++)
	mp->mi_prob += (mp-1)->mi_prob ;
    badcheck("things", things, NUMTHINGS) ;
}

/*
 * init_fd
 *	Initialize the probabilities for types of food
 */
void
init_fd(void)
{
    register struct magic_item *mp ;

    for (mp = &fd_data[1] ; mp < &fd_data[MAXFOODS] ; mp++)
	mp->mi_prob += (mp-1)->mi_prob ;
    badcheck("food", fd_data, MAXFOODS) ;
}

/*
 * init_colors:
 *	Initialize the potion color scheme for this time
 */

void
init_colors(void)
{
    register int i,j ;
	int used[NCOLORS];

	for (i = 0; i < NCOLORS; i++)
		used[i] = FALSE;

    for (i = 0 ; i < MAXPOTIONS ; i++)
    {
	do
	    j = rnd(NCOLORS);
	until (!used[j]) ;
	used[j] = TRUE;
	p_colors[i] = rainbow[j];
	p_know[i] = FALSE ;
	p_guess[i] = NULL ;
	if (i > 0)
		p_magic[i].mi_prob += p_magic[i-1].mi_prob ;
    }
    badcheck("potions", p_magic, MAXPOTIONS) ;
}

/*
 * init_names:
 *	Generate the names of the various scrolls
 */
void
init_names(void)
{
    register int nsyl ;
    register char *cp, *sp ;
    register int i, nwords ;

    for (i = 0 ; i < MAXSCROLLS ; i++)
    {
	cp = prbuf ;
	nwords = rnd(COLS/20) + 1 + (COLS > 40 ? 1 : 0) ;
	while(nwords--)
	{
	    nsyl = rnd(3)+1 ;
	    while(nsyl--)
	    {
		sp = sylls[rnd((sizeof sylls) / (sizeof (char *)))] ;
		while(*sp)
		    *cp++ = *sp++ ;
	    }
	    *cp++ = ' ' ;
	}
	*--cp = '\0' ;
	s_names[i] = (char *) new(strlen(prbuf)+1) ;
	s_know[i] = FALSE ;
	s_guess[i] = NULL ;
	strcpy(s_names[i], prbuf) ;
	if (i > 0)
		s_magic[i].mi_prob += s_magic[i-1].mi_prob ;
    }
    badcheck("scrolls", s_magic, MAXSCROLLS) ;
}

/*
 * init_stones:
 *	Initialize the ring stone setting scheme for this time
 */
void
init_stones(void)
{
    register int i,j ;
    int used[NSTONES];

	for (i = 0; i < NSTONES; i++)
		used[i] = FALSE;

	for (i = 0 ; i < MAXRINGS ; i++)
    {
	do
	    j = rnd(NSTONES) ;
	until (!used[j]) ;
	used[j] = TRUE;
	r_stones[i] = stones[j];
	r_know[i] = FALSE ;
	r_guess[i] = NULL ;
	if (i > 0)
		r_magic[i].mi_prob += r_magic[i-1].mi_prob ;
    }
    badcheck("rings", r_magic, MAXRINGS) ;
}

/*
 * init_materials:
 *	Initialize the construction materials for wands and staffs
 */
void
init_materials(void)
{
    register int i,j ;
	register char *str;
    int metused[NMETAL], woodused[NWOOD];

	for(i = 0; i < NWOOD; i++)
		woodused[i] = FALSE;

	for(i = 0; i < NMETAL; i++)
		metused[i] = FALSE;

    for (i = 0 ; i < MAXSTICKS ; i++)
    {
	for(;;)
	    if (rnd(100) > 50)
	    {
		j = rnd(NMETAL) ;
		if (!metused[j])
		{
			ws_type[i] = "wand" ;
			str = metal[j];
			metused[j] = TRUE;
			break;
		}
	    }
	    else
	    {
		j = rnd(NWOOD) ;
		if (!woodused[j]) 
		{
			ws_type[i] = "staff" ;
			str = wood[j];
			woodused[j] = TRUE;
			break;
		}
	    }
	
	ws_made[i] = str ;
	ws_know[i] = FALSE ;
	ws_guess[i] = NULL ;
	if (i > 0)
		ws_magic[i].mi_prob += ws_magic[i-1].mi_prob ;
    }
    badcheck("sticks", ws_magic, MAXSTICKS) ;
}

void
badcheck(char *name, struct magic_item *magic, int bound)
{
    register struct magic_item *end ;

    if (magic[bound - 1].mi_prob == 1000)
	return ;
    printf("\nBad percentages for %s:\n", name) ;
    for (end = &magic[bound] ; magic < end ; magic++)
	printf("%3d%% %s\n", magic->mi_prob, magic->mi_name) ;
    printf("[hit RETURN to continue]") ;
    fflush(stdout) ;
    while ((getchar() & 0177) != '\n')
	continue ;
}

