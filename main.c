/*
    main.c  -  Rogue
   
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
#include <stdio.h>
#include <string.h>
#include <curses.h>
#include <signal.h>
#include <time.h>
#include "mach_dep.h"
#include "rogue.h"
#ifdef SIGALRM
#include <unistd.h>
#endif

static int num_checks;		/* times we've gone over in checkout() */
static double avec[3];		/* load average vector */

FILE *fd_score = NULL;		/* file descriptor the score file */

int
main(int argc, char *argv[], char **envp)
{
    register char *env;
    register struct linked_list *item;
    register struct object *obj;
	char *pagerbase;
    int lowtime, wpt = 0, i, j, hpadd, dmadd;
    int alldone, predef;
    time_t now;
 
    md_init();

    areuok(PERMOK);				/***another void here ***/
#ifdef SIGQUIT
	signal(SIGQUIT, SIG_IGN);
#endif
    /*
     * get home and options from environment
     */
    strncpy(home,md_gethomedir(),LINELEN);

    /* Get default save file */
    strcpy(file_name, home);
    strcat(file_name, "rogue.save");

    /* Get default score file */
    strcpy(score_file, SCOREDIR);

    /*
     * Grab a file descriptor to the score file before the
     * effective uid and gid are reset to the real ones.
     */
    if ( (fd_score = fopen(score_file, "r+")) == NULL)
		fd_score = fopen(score_file, "w+");

    if ((env = getenv("SROGUEOPTS")) != NULL)
	parse_opts(env);
    if (env == NULL || whoami[0] == '\0')
		strucpy(whoami, md_getusername(), strlen(md_getusername()));
	
	if (whoami[0] == '\0')
	{
	    printf("Say, who the hell are you?\n");
	    printf("You can't run without a name.\n");
	    md_sleep(4);
	    exit(1);
	}

    if (env == NULL || fruit[0] == '\0') {
	static char *funfruit[] = {
		"candleberry", "caprifig", "dewberry", "elderberry",
		"gooseberry", "guanabana", "hagberry", "ilama", "imbu",
		"jaboticaba", "jujube", "litchi", "mombin", "pitanga",
		"prickly pear", "rambutan", "sapodilla", "soursop",
		"sweetsop", "whortleberry"
	};

	md_srand(md_getpid()+255);
	strcpy(fruit, funfruit[rnd(sizeof(funfruit)/sizeof(funfruit[0]))]);
     }

     /* put a copy of fruit in the right place */
     fd_data[1].mi_name = fruit;

    /*
     * check for print-score option
     */
    if (argc == 2 && strcmp(argv[1], "-s") == 0)
    {
	waswizard = TRUE;
	score(0, SCOREIT, 0);
	exit(0);
    }

    /*
     * check for news option
     */
    if (argc == 2 && strcmp(argv[1], "-n") == 0) {
        pagerbase = strrchr(PAGER, '/')+1;
        md_exec(PAGER, pagerbase, NEWS);
        exit(0);
    }

    /*
     * Check to see if he is a wizard
     */
    makesure();
    if (argc >= 2 && argv[1][0] == '\0')
	if (strcmp(PASSWD, md_crypt(md_getpass("Wizard's password: "), PASSWDSEED)) == 0)
	{
		if (canwizard)
		{
#ifdef SIGQUIT
			signal(SIGQUIT, SIG_DFL);
#endif
		    wizard = TRUE;
		    argv++;
		    argc--;
		}
		else
			printf("Well that's odd...\n");
	}

    if( !author() && !wizard && !holiday())
	{
	static char *niceties[] = { "juicy", "fresh", "nice" ,"sweet" };

	md_srand(md_getpid()+rnd(127));
	printf("Sorry, %s, but you can't play during working hours.\n", whoami);
	printf("Meanwhile, why not enjoy a %s %s?\n",niceties[rnd(4)], fruit);
	md_sleep(4);
	exit(1);
	}
    if (too_much() && !wizard)
    {
	if(!author())
	{
	    static char *niceties[] = { "juicy", "fresh", "nice" ,"sweet" };

	    md_srand(md_getpid()+rnd(127));
	    printf("Sorry, %s, but the system is too loaded now.\n", whoami);
	    printf("Try again later.  Meanwhile, why not enjoy a %s %s?\n",
	        niceties[rnd(4)], fruit);
	    md_sleep(4);
	    exit(1);
	}
	else
	{
		printf("The system load is rather heavy, %s...\n",whoami);
		printf("However I'll let you play anyway!\n");
		printf("Please note that there are %d users!\n",ucount());
		md_sleep(2);
	}
    }

    if (argc == 2 && argv[1][0] != '\0') {
	if (!restore(argv[1], envp))/* Note: restore returns on error only */
	    exit(1);
    }
    lowtime = (int) time(&now);
    dnum = (wizard && getenv("SEED") != NULL ?
	atoi(getenv("SEED")) :
	lowtime + md_getpid());
    if (wizard)
	printf("Hello %s, welcome to dungeon #%d", whoami, dnum);
    else
	printf("Hello %s, just a moment while I dig the dungeon...", whoami);
    fflush(stdout);
    seed = dnum;
    md_srand(seed);

    init_things();			/* Set up probabilities of things */
    init_fd();				/* Set up food probabilities */
    init_colors();			/* Set up colors of potions */
    init_stones();			/* Set up stone settings of rings */
    init_materials();			/* Set up materials of wands */
    initscr();				/* Start up cursor package */
    init_names();			/* Set up names of scrolls */
    setup();

    /*
     * Reset the effective uid & gid to the real ones.
     */
    md_normaluser();

    /*
     * Set up windows
     */
    cw = newwin(LINES, COLS, 0, 0);
    mw = newwin(LINES, COLS, 0, 0);
    hw = newwin(LINES, COLS, 0, 0);
    msgw = newwin(4, COLS, 0, 0);
    keypad(cw,1);
    keypad(msgw,1);

    predef = geta_player();
    waswizard = wizard;
re_roll:
    if(!predef)
	init_player();			/* Roll up the rogue */
    else
	goto get_food;			/* Using a pre-rolled rogue */
    /*
     * Give the rogue his weaponry.  
     */
    alldone = FALSE;
    do {
	int ch = 0;
	i = rnd(16);	/* number of acceptable weapons */
	switch(i) {
	    case 0: ch = 25; wpt = MACE;
	    when 1: ch = 25; wpt = SWORD;
	    when 2: ch = 15; wpt = TWOSWORD;
	    when 3: ch = 10; wpt = SPEAR;
	    when 4: ch = 20; wpt = TRIDENT;
	    when 5: ch = 20; wpt = SPETUM;
	    when 6: ch = 20; wpt = BARDICHE;
	    when 7: ch = 15; wpt = SPIKE;
	    when 8: ch = 15; wpt = BASWORD;
	    when 9: ch = 20; wpt = HALBERD;
	    when 10:ch = 20; wpt = BATTLEAXE;
	    when 11:ch = 20; wpt = GLAIVE;
	    when 12:ch = 20; wpt = LPIKE;
	    when 13:ch = 20; wpt = BRSWORD;
	    when 14:ch = 20; wpt = CRYSKNIFE;
	    when 15:ch = 20; wpt = CLAYMORE;
	}
	if(rnd(100) < ch) {		/* create this weapon */
	    alldone = TRUE;
	}
    } while(!alldone);
    hpadd = rnd(2) + 1;
    dmadd = rnd(2) + 1;
    if (player.t_ctype == C_FIGHTER) {
	if (rnd(100) > 50)
	    wpt = TWOSWORD;
	else
	    wpt = CLAYMORE;
	hpadd = hpadd - 1;
    }
    /*
     * Find out what the armor is.
     */
    i = rnd(100) + 1;
    j = 0;
    while (armors[j].a_prob < i)
	j++;
    /*
     * See if this rogue is acceptable to the player.
     */
    if(!puta_player(j,wpt,hpadd,dmadd))
 	goto re_roll;
    /*
     * It's OK. Add this stuff to the rogue's pack.
     */
    item = spec_item(WEAPON, wpt, hpadd, dmadd);
    obj = (struct object *) ldata(item);
    obj->o_flags |= ISKNOW;
    add_pack(item, TRUE);
    cur_weapon = obj;
    /*
     * And his suit of armor
     */
    item = spec_item(ARMOR, j, 0, 0);
    obj = (struct object *) ldata(item);
    obj->o_flags |= ISKNOW;
    obj->o_weight = armors[j].a_wght;
    add_pack(item, TRUE);
    cur_armor = obj;
    /*
     * Give him some food too
     */
get_food:
    item = spec_item(FOOD, 0, 0, 0);
    obj = OBJPTR(item);
    obj->o_weight = things[TYP_FOOD].mi_wght;
    add_pack(item, TRUE);
    new_level(NORMLEV);			/* Draw current level */
    /*
     * Start up daemons and fuses
     */
    start_daemon(doctor, &player, AFTER);
    fuse(swander, 0, WANDERTIME, AFTER);
    start_daemon(stomach, 0, AFTER);
    start_daemon(runners, 0, AFTER);
    playit();
    return(0);
}

/*
 * endit:
 *	Exit the program abnormally.
 */
void
endit(void)
{
    fatal("Ok, if you want to exit that badly, I'll have to allow it\n");
}

/*
 * fatal:
 *	Exit the program, printing a message.
 */
void
fatal(char *s)
{
    clear();
    move(LINES-2, 0);
    printw("%s", s);
    draw(stdscr);
    endwin();
    printf("\n");	/* So the cursor doesn't stop at the end of the line */
    exit(0);
}

/*
 * rnd:
 *	Pick a very random number.
 */
int
rnd(int range)
{
	return (range == 0 ? 0 : md_rand() % range);
}

/*
 * roll:
 *	roll a number of dice
 */
int
roll(int number, int sides)
{
    register int dtotal = 0;

    while(number--)
	dtotal += rnd(sides)+1;
    return dtotal;
}


/*
 * handle stop and start signals
 */

#ifdef SIGTSTP
void
tstp(int sig)
{
    NOOP(sig);

    mvcur(0, COLS - 1, LINES - 1, 0);
    endwin();
    fflush(stdout);
    signal(SIGTSTP, SIG_IGN);
    kill(0, SIGTSTP);
    signal(SIGTSTP, tstp);
    crmode();
    noecho();
    clearok(curscr, TRUE);
    touchwin(cw);
    draw(cw);
    flushinp();
} 
#endif

void
setup(void)
{
#ifdef SIGHUP
	signal(SIGHUP, auto_save);
#endif
#ifdef SIGINT
    signal(SIGINT, quit);
#endif
#ifdef SIGTSTP
    signal(SIGTSTP, tstp);
#endif

    if (!author())
    {
#ifdef SIGALRM
	signal(SIGALRM, checkout);
	alarm(CHECKTIME * 60);
#endif
	num_checks = 0;
    }
    crmode();				/* Cbreak mode */
    noecho();				/* Echo off */
}

/*
 * playit:
 *	The main loop of the program.  Loop until the game is over,
 * refreshing things and looking at the proper times.
 */

void
playit(void)
{
    /*
     * set up defaults for slow terminals
     */

    char_type = player.t_ctype;

    player.t_oldpos = hero;
    oldrp = roomin(&hero);
    after = TRUE;
    while (playing)
	command();			/* Command execution */
    endit();
}

/*
 * see if the system is being used too much for this game
 */
int
too_much(void)
{
	loadav(avec);
	return (avec[2] > (MAXLOAD / 10.0)) || (ucount() > MAXUSERS);
}

/*
 * author:
 *	See if a user is an author of the program
 *	or a user with the author's permission.
 */
int
author(void)
{
	int	id;
	int	i;

	id = md_getuid();

	for(i=0;i<MAXAUTH;i++)
		if(id == auth_or[i])
			return TRUE;
	return FALSE;
}

int
makesure(void)
{
    FILE *fd;
	int	i;

	canwizard = FALSE;
	if ((fd = fopen(PASSCTL, "r")) == NULL) return (FALSE);

	for ( i=0; i<20;i++)
		rpass.rp_pass[i] = 0;
	for ( i=0; i<2;i++)
		rpass.rp_pkey[i] = 0;

	encread(rpass.rp_pkey, sizeof(rpass.rp_pkey), fd);
	encread(rpass.rp_pass, sizeof(rpass.rp_pass), fd);

	if((*(rpass.rp_pkey) != 0) && (*(rpass.rp_pass) != 0))
	    canwizard = TRUE;

	fclose(fd);
	return(FALSE);
}

void
areuok(char *file)
{
	FILE *	fd;

	if((fd = fopen(file,"r")) == NULL)
		return ;

	encread((char *)auth_or, sizeof(auth_or), fd);

	fclose(fd);
}

void
checkout(int sig)
{
	static char *msgs[] = {
	"The system is too loaded for games. Please leave in %d minutes.",
	"Please save your game.  You have %d minutes.",
	"This is your last chance. You had better leave in %d minutes.",
	};
	int checktime;

	NOOP(sig);

#ifdef SIGALRM
	signal(SIGALRM, checkout);
#endif
	if (too_much() || !holiday()) {
	    if (num_checks >= 3) {

		clear();
		move(LINES-2, 0);
		printw("You didn't listen, so now you are DEAD!!\n");
		draw(stdscr);
		endwin();
		printf("\n");	/* now do a save if possible */
	    save_file(file_name);
		exit(1);
	    }
	    checktime = CHECKTIME / (2 * (num_checks + 1));
	    chmsg(msgs[num_checks++], checktime);
#ifdef SIGALRM
	    alarm(checktime * 60);
#endif
	}
	else {
	    if (num_checks) {
		chmsg("The load has dropped. You have a reprieve.");
		num_checks = 0;
	    }
#ifdef SIGALRM
	    alarm(CHECKTIME * 60);
#endif
	}
}

/*
 * checkout()'s version of msg.  If we are in the middle of a shell, do a
 * printf instead of a msg to avoid the refresh.
 */
void
chmsg(char *fmt, ...)
{
	va_list ap;

	if (in_shell) {
		va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
		putchar('\n');
		fflush(stdout);
	}
	else {
		va_start(ap,fmt);
		doadd(fmt,ap);
		va_end(ap);
		endmsg();
	}
}

/*
 * loadav - find current load average
 */
void
loadav(double *avg)
{
	FILE * pp;
	pp = popen (LOADAV_PROG, "r");
	fscanf (pp, "%lf%lf%lf", &avg[0], &avg[1], &avg[2]);
	pclose (pp);
	return;
}

/*
 * ucount:
 *	Count the number of people on the system
 */
#ifdef _WIN32
int
ucount(void)
{
	return(1);
}
#else
#ifndef USGV4		/*Must be a 5.0 UNIX system */
#include <sys/types.h>
#include <utmp.h>
struct utmp buf;
int
ucount(void)
{
	reg struct utmp *up;
	reg FILE *utmp;
	reg int count;

	if ((utmp = fopen(UTMP, "r")) == NULL)
	    return 0;

	up = &buf;
	count = 0;
	while (fread(up, 1, sizeof (*up), utmp) > 0)
		if (buf.ut_type == USER_PROCESS)
			count++;
	fclose(utmp);
	return count;
}
#else		/* Otherwise it's a USG 3.0 or 4.0 UNIX */
#include <utmp.h>

struct utmp buf;

ucount()
{
    register struct utmp *up;
    register FILE *utmp;
    register int count;

    if ((utmp = fopen(UTMP, "r")) == NULL)
	return 0;

    up = &buf;
    count = 0;

    while (fread(up, 1, sizeof (*up), utmp) > 0)
	if (buf.ut_name[0] != '\0')
	    count++;
    fclose(utmp);
    return count;
}
#endif		/* end ifndef USGV4 */
#endif

/*
 * holiday:
 *	Returns TRUE when it is a good time to play rogue
 */
int
holiday(void)
{
	time_t now;
	reg struct tm *ntime;

	if (!HOLIDAY)
		return TRUE;

	time(&now);			/* get the current time */
	ntime = localtime(&now);
	if(ntime->tm_wday == 0 || ntime->tm_wday == 6)
		return TRUE;		/* OK on Sat & Sun */
	if(ntime->tm_hour < 9 || ntime->tm_hour >= 17)
		return TRUE;		/* OK before 9AM & after 5PM */
	return FALSE;			/* All other times are bad */
}
