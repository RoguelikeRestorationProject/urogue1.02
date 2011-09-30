/*
    rip.c  -  File for the fun ends. Death or a total win
   
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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include "mach_dep.h"
#include "rogue.h"

static char *rip[15] = {		
"                       __________",
"                      /          \\",
"                     /    REST    \\",
"                    /      IN      \\",
"                   /     PEACE      \\",
"                  /                  \\",
"                  |                  |",
"                  |                  |",
"                  |    killed by     |",
"                  |                  |",
"                  |       1980       |",
"                 *|     *  *  *      | *",
"         ________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______",
    0
};

/*
 * death:
 *	Do something really fun when he dies
 */
void
death(int monst)
{
    register char **dp = rip, *killer; 	/**replace w rip**/
    register struct tm *lt;
    time_t date;
    char buf[80];
    struct tm *localtime();

    if (ISWEARING(R_RESURRECT)) {
	int die = TRUE;

	if (resurrect-- == 0) 
	    msg("You've run out of lives.");
	else if (!save_resurrect(ring_value(R_RESURRECT)))
	    msg("Your attempt to return from the grave fails.");
	else {
	    register struct linked_list *item;
	    register struct object *obj;
	    register int rm, flags;
	    coord pos;

	    die = FALSE;
	    msg("You feel sudden warmth and then nothingness.");
	    teleport();
	    if (ring_value(R_RESURRECT) > 1 && rnd(10)) {
		pstats.s_hpt = 2 * pstats.s_const;
		pstats.s_const = max(pstats.s_const - 1, 3);
	    }
	    else {
		item = pack;
		while (item != NULL) {
		    obj = (struct object *) ldata(item);
		    flags = obj->o_flags;
		    obj->o_flags &= ~ISCURSED;
		    dropcheck(obj);
		    obj->o_flags = flags;
		    detach(pack, item);
		    freeletter(item);
		    attach(lvl_obj, item);
		    inpack--;
		    if (obj->o_type == ARTIFACT)
			has_artifact &= ~(1 << obj->o_which);
		    do {
			rm = rnd_room();
		        rnd_pos(&rooms[rm], &pos);
		    } until (winat(pos.y, pos.x) == FLOOR);
		    mvaddch(pos.y, pos.x, obj->o_type);
		    obj->o_pos = pos;
		    item = pack;
		}
		pstats.s_hpt = pstats.s_const;
		pstats.s_const = max(pstats.s_const - roll(2,2), 3);
	    }
	    pstats.s_lvl = max(pstats.s_lvl, 1);
	    no_command += 2 + rnd(4);
	    if (on(player, ISHUH))
		lengthen(unconfuse, rnd(8)+HUHDURATION);
	    else
		fuse(unconfuse, 0, rnd(8)+HUHDURATION, AFTER);
	    turn_on(player, ISHUH);
	    light(&hero);
	}
	if (!die)
		wmove(cw, (int)mpos, 0);
		waddstr(cw, morestr);
		wrefresh(cw);
		wait_for(cw, ' ');
	    return;
	}
    time(&date);
    lt = localtime(&date);
    clear();
    move(8, 0);
    while (*dp)
	printw("%s\n", *dp++);
    mvaddstr(14, 28-(((int)strlen(whoami)+1)/2), whoami);
    sprintf(buf, "%d Points", pstats.s_exp );
    mvaddstr(15, 28-(((int)strlen(buf)+1)/2), buf);
    killer = killname(monst);
    mvaddstr(17, 28-(((int)strlen(killer)+1)/2), killer);
    mvaddstr(18, 26, (sprintf(prbuf, "%4d", 1900+lt->tm_year), prbuf));
    move(LINES-1, 0);
    idenpack();
    refresh();
    score(pstats.s_exp, KILLED, monst);
    exit(0);
}

/*
 * score -- figure score and post it.
 */

/* VARARGS2 */
void
score(int amount, int flags, int monst)
{
    static struct sc_ent {
	int sc_score;
	char sc_name[76];
	int sc_gold;
	int sc_flags;
	int sc_level;
	int sc_uid;
	int sc_artifacts;
	int sc_monster;
    } top_ten[10];
    register struct sc_ent *scp;
    register int i;
    register struct sc_ent *sc2;
    register FILE *outf;
    register char *killer;
    register int prflags = 0;
	char *username;
    static char *reason[] = {
	"killed",
	"quit",
	"a winner",
	"a total winner"
    };
    char *packend;

    signal(SIGINT, byebye);
    if (flags != WINNER && flags != TOTAL && flags != SCOREIT) {
	if (flags == CHICKEN)
	    packend = "when you quit";
	else
	    packend = "at your untimely demise";
	mvaddstr(LINES - 1, 0, retstr);
	refresh();
	getstr(prbuf);
	showpack(packend);
    }

    /*
     * Open file and read list
     */

    if (fd_score == NULL)
	return;

    outf = fd_score;

    for (scp = top_ten; scp < &top_ten[10]; scp++)
    {
	scp->sc_score = 0L;
	for (i = 0; i < 76; i++)
	    scp->sc_name[i] = (char) rnd(255);
	scp->sc_gold = 0L;
	scp->sc_flags = RN;
	scp->sc_level = RN;
	scp->sc_monster = RN;
	scp->sc_artifacts = 0;
	scp->sc_uid = RN;
    }

    signal(SIGINT, SIG_DFL);
    if (flags != SCOREIT)
    {
	mvaddstr(LINES - 1, 0, retstr);
	refresh();
	fflush(stdout);
	getstr(prbuf);
    }
    if (wizard) {
	if (strncmp(prbuf, "edit", 4) == 0)
	    prflags = 1;
	else if (strncmp(prbuf, "force", 5) == 0)
	    prflags = 2;
    }

    encread((char *) top_ten, sizeof top_ten, fd_score);
    /*
     * Insert player in list if need be
     */
    if (!waswizard)
    {
	struct	sc_ent	*sc3;
	int uid = md_getuid();
	for (scp = top_ten; scp < &top_ten[10]; scp++) {
	    if (amount > scp->sc_score)
		break;
	    else if(scp->sc_uid == uid)
		scp = &top_ten[9];
	}
	if (scp < &top_ten[10])
	{
	    for(sc3 = scp; sc3 < &top_ten[10]; sc3++)
		if(sc3->sc_uid == uid){
		    for (sc2 = sc3; sc2 < &top_ten[9]; sc2++)
			*sc2 = *(sc2 + 1);
		    top_ten[9].sc_score = 0L;
		    for (i = 0; i < 76; i++)
			top_ten[9].sc_name[i] = (char) rnd(255);
		    top_ten[9].sc_gold = 0L;
		    top_ten[9].sc_flags = RN;
		    top_ten[9].sc_level = RN;
		    top_ten[9].sc_monster = RN;
		    top_ten[9].sc_artifacts = 0;
		    top_ten[9].sc_uid = RN;
		    sc3--;
		}
	    for (sc2 = &top_ten[9]; sc2 > scp; sc2--)
		*sc2 = *(sc2-1);
	    scp->sc_score = amount;
	    scp->sc_gold = purse;
	    scp->sc_uid = md_getuid();
	    strcpy(scp->sc_name, whoami);
		username = md_getusername();
	    if (username == NULL) {
		sprintf(prbuf, " (%d)", scp->sc_uid);
		strcat(scp->sc_name, prbuf);
	    }
	    else {
		sprintf(prbuf, " (%s)", username);
		if (strcmp(scp->sc_name, whoami))
		    strcat(scp->sc_name, prbuf);
	    }
	    sprintf(prbuf, ", Level %d %s", pstats.s_lvl, 
			cnames[player.t_ctype][min(pstats.s_lvl,11) - 1]);
	    strcat(scp->sc_name, prbuf);
	    scp->sc_flags = flags;
	    if (flags == WINNER || flags == TOTAL)
		scp->sc_level = max_level;
	    else
		scp->sc_level = level;
	    scp->sc_monster = monst;
	    scp->sc_artifacts = picked_artifact;
	}
    }
    if (flags != SCOREIT) {
	clear();
	refresh();
	endwin();
    }
    /*
     * Print the list
     */
    printf("\nTop Ten Adventurers:\nRank\tScore\tGold\t\tName\n");
    for (scp = top_ten; scp < &top_ten[10]; scp++) {

	if (scp->sc_score) {
	    printf("%d\t%d\t%d\t%s:\n", (int)(scp - top_ten) + 1,
		scp->sc_score, scp->sc_gold, scp->sc_name);
	    if (scp->sc_artifacts) {
		char things[60];
		int  i;
		int first = TRUE;

		things[0] = '\0';
		for (i = 0; i <= MAXARTIFACT; i++) {
		    if (scp->sc_artifacts & (1 << i)) {
			if (strlen(things))
			    strcat(things, ", ");
			if (first) {
			    strcat(things, "retrieved ");
			    first = FALSE;
			}
			if (55 - strlen(things) < strlen(arts[i].ar_name)) {
			    printf("\t\t\t%s\n", things);
			    things[0] = '\0';
		        }
			strcat(things, arts[i].ar_name);
		    }
		}
		if (strlen(things)) 
		    printf("\t\t\t%s,", things);
		putchar('\n');
	    }
	    printf("\t\t\t%s on level %d", 
		reason[scp->sc_flags], scp->sc_level);
	    if (scp->sc_flags == 0) {
		printf(" by");
		killer = killname(scp->sc_monster);
		printf(" %s", killer);
	    }
	    if (prflags == 1)
	    {
		fflush(stdout);
		fgets(prbuf,80,stdin);
		if (prbuf[0] == 'd')
		{
		    for (sc2 = scp; sc2 < &top_ten[9]; sc2++)
			*sc2 = *(sc2 + 1);
		    top_ten[9].sc_score = 0L;
		    for (i = 0; i < 76; i++)
			top_ten[9].sc_name[i] = (char) rnd(255);
		    top_ten[9].sc_gold = 0L;
		    top_ten[9].sc_flags = RN;
		    top_ten[9].sc_level = RN;
		    top_ten[9].sc_monster = RN;
		    scp--;
		}
	    }
	    else if (prflags == 2)
	    {
		if (scp == top_ten)
	            scp->sc_artifacts = 1;
		  
		printf(". (zapped)\n");
	    }
	    else
		printf(".\n");
	}
    }
    fseek(outf, 0L, 0);
    /*
     * Update the list file
     */
    encwrite((char *) top_ten, sizeof top_ten, outf);
    fclose(outf);
}

void
total_winner(void)
{
    register struct linked_list *item;
    register struct object *obj;
    register int worth, oldpurse;
    register int c;
    struct linked_list *bag = NULL;

    clear();
    standout();
    addstr("                                                               \n");
    addstr("  @   @               @   @           @          @@@  @     @  \n");
    addstr("  @   @               @@ @@           @           @   @     @  \n");
    addstr("  @   @  @@@  @   @   @ @ @  @@@   @@@@  @@@      @  @@@    @  \n");
    addstr("   @@@@ @   @ @   @   @   @     @ @   @ @   @     @   @     @  \n");
    addstr("      @ @   @ @   @   @   @  @@@@ @   @ @@@@@     @   @     @  \n");
    addstr("  @   @ @   @ @  @@   @   @ @   @ @   @ @         @   @  @     \n");
    addstr("   @@@   @@@   @@ @   @   @  @@@@  @@@@  @@@     @@@   @@   @  \n");
    addstr("                                                               \n");
    addstr("     Congratulations, you have made it to the light of day!    \n");
    standend();
    addstr("\nYou have joined the elite ranks of those who have escaped the\n");
    addstr("Dungeons of Doom alive.  You journey home and sell all your loot at\n");
    addstr("a great profit and are admitted to the fighters guild.\n");
    mvaddstr(LINES - 1, 0, spacemsg);
    refresh();
    wait_for(cw, ' ');
    clear();
    idenpack();
    oldpurse = purse;
    mvaddstr(0, 0, "   Worth  Item");
    for (c = 'a', item = pack; item != NULL; c++, item = next(item)) {
	obj = (struct object *) ldata(item);
	worth = get_worth(obj);
	purse += worth;
	if (obj->o_type == ARTIFACT && obj->o_which == TR_PURSE) 
	    bag = obj->art_stats.t_art;
	mvprintw(c - 'a' + 1, 0, "%c) %8d  %s", c, 
		worth, inv_name(obj, FALSE));
    }
    if (bag != NULL) {
	mvaddstr(LINES - 1, 0, morestr);
	refresh();
	wait_for(cw, ' ');
	clear();
	mvprintw(0, 0, "Contents of the Magic Purse of Yendor:\n");
	for (c = 'a', item = bag; item != NULL; item = next(item)) {
	    obj = OBJPTR(item);
	    worth = get_worth(obj);
	    whatis(item);
	    purse += worth;
	    mvprintw(c - 'a' + 1, 0, "%c) %8d %s\n", c, 
			worth, inv_name(obj, FALSE));
	}
    }
    mvprintw(c - 'a' + 1, 0,"   %6d  Gold Pieces          ", oldpurse);
    refresh();
    if (has_artifact == 255) 
	score(pstats.s_exp, TOTAL, 0);
    else
	score(pstats.s_exp, WINNER, 0);
    exit(0);
}

char *
killname(int monst)
{
    static char mons_name[80];

    if (monst >= 0) {
	switch (monsters[monst].m_name[0]) {
	    case 'a':
	    case 'e':
	    case 'i':
	    case 'o':
	    case 'u':
		sprintf(mons_name, "an %s", monsters[monst].m_name);
		break;
	    default:
		sprintf(mons_name, "a %s", monsters[monst].m_name);
	}
	return(mons_name);
    }
    else
	switch (monst)
	{
	    case D_ARROW:
		return "an arrow";
	    case D_DART:
		return "a dart";
	    case D_BOLT:
		return "a bolt";
	    case D_POISON:
		return "poison";	/* Cursed healing potion */
	    case D_POTION:
		return "a cursed potion";
	    case D_PETRIFY:
		return "petrification";
	    case D_SUFFOCATION:
		return "suffocation";
	    case D_INFESTATION:
		return "a parasite";
	    case D_DROWN:
		return "drowning";
	    case D_FALL:
		return "falling";
	    case D_FIRE:
		return "slow boiling in oil";
	}

	return "";
}

/*
 * showpack:
 *	Display the contents of the hero's pack
 */
void
showpack(char *howso)
{
	reg char *iname;
	reg int cnt, worth, ch, oldpurse;
	reg struct linked_list *item;
	reg struct object *obj;
	struct linked_list *bag = NULL;

	cnt = 1;
	clear();
	mvprintw(0, 0, "Contents of your pack %s:\n",howso);
	ch = 'a';
	oldpurse = purse;
	purse = 0;
	for (item = pack; item != NULL; item = next(item)) {
		obj = OBJPTR(item);
		worth = get_worth(obj);
		whatis(item);
		purse += worth;
		if (obj->o_type == ARTIFACT && obj->o_which == TR_PURSE)
			bag = obj->art_stats.t_art;
		iname = inv_name(obj, FALSE);
		mvprintw(cnt, 0, "%c) %s\n",ch,iname);
		ch += 1;
		if (++cnt > LINES - 5 && next(item) != NULL) {
			cnt = 1;
			mvaddstr(LINES - 1, 0, morestr);
			refresh();
			wait_for(cw, ' ');
			clear();
		}
	}
	if (bag != NULL) {
		mvaddstr(LINES - 1, 0, morestr);
		refresh();
		wait_for(cw, ' ');
		clear();
		cnt = 1;
		ch = 'a';
		mvprintw(0, 0, "Contents of the Magic Purse of Yendor %s:\n",
			howso);
		for (item = bag; item != NULL; item = next(item)) {
			obj = OBJPTR(item);
			worth = get_worth(obj);
			whatis(item);
			purse += worth;
			mvprintw(cnt, 0, "%c) %s\n", ch, inv_name(obj, FALSE));
			ch += 1;
			if (++cnt > LINES - 5 && next(item) != NULL) {
				cnt = 1;
				mvaddstr(LINES - 1, 0, morestr);
				refresh();
				wait_for(cw, ' ');
				clear();
			}
		}
	}
	mvprintw(cnt + 1,0,"Carrying %d gold pieces", oldpurse);
	mvprintw(cnt + 2,0,"Carrying objects worth %d gold pieces", purse);
	purse += oldpurse;
	refresh();
}

void 
byebye(int sig)
{
	NOOP(sig);

    if (!isendwin()) {
        clear();
	endwin();
    }
    printf("\n");
    exit(0);
}

/*
 * save_resurrect:
 *      chance of resurrection according to modifed D&D probabilities
 */
int
save_resurrect(int bonus)
{
	register int need, adjust;

	adjust = pstats.s_const + bonus - luck;
	if (adjust > 17)
		return TRUE;
	else if (adjust < 14)
		need = 5 * (adjust + 5);
	else
		need = 90 + 2 * (adjust - 13);
	return (roll(1, 100) < need);
}

