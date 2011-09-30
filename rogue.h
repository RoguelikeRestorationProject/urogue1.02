/*
 * Rogue definitions and variable declarations
 *
 */
#include "tunable.h"
#include "mdport.h"

#define reg	register	/* register abbr.	*/
#define NOOP(x) (x += 0)
#define CCHAR(x) ( (x & A_CHARTEXT) )

#define MAXDAEMONS 60

#define NCOLORS         32
#define NSTONES         35
#define NWOOD           24
#define NMETAL          15
#define NSYLLS          159

/*
 * Maximum number of different things
 */
#define	MAXROOMS	9
#define	MAXTHINGS	9
#define	MAXOBJ		9
#define	MAXPACK		23
#define	MAXTREAS	30	/* number monsters/treasure in treasure room */
#define	MAXTRAPS	30	/* max traps per level */
#define	MAXTRPTRY	16	/* attempts/level allowed for setting traps */
#define	MAXDOORS	4	/* Maximum doors to a room */
#define	MAXPRAYERS	12	/* Maximum number of prayers for cleric */
#define	MAXSPELLS	21	/* Maximum number of spells (for magician) */
#define	NUMMONST	124	/* Current number of monsters */
#define NUMUNIQUE	17	/* number of UNIQUE creatures */
#define	NLEVMONS	2	/* Number of new monsters per level */
#define MAXPURCH	8	/* max purchases per trading post visit */
#define LINELEN		80	/* characters in a buffer */

/* These defines are used by get_play.c */
#define	I_STR		0
#define	I_INTEL		1
#define	I_WIS		2
#define	I_DEX		3
#define	I_WELL		4
#define	I_APPEAL	5
#define	I_HITS		6
#define	I_ARM		7
#define	I_WEAP		8
#define	I_CHAR		9
#define I_WEAPENCH	10
#define	MAXPATT		11	/* Total Number of above defines. */
#define	MAXPDEF		4	/* Maximum number of pre-defined chars */

/* Movement penalties */
#define BACKPENALTY 3
#define SHOTPENALTY 2		/* In line of sight of missile */
#define DOORPENALTY 1		/* Moving out of current room */

/*
 * stuff to do with encumberance
 */
#define NORMENCB	1500	/* normal encumberance */
#define F_OK		 0	/* have plenty of food in stomach */
#define F_HUNGRY	 1	/* player is hungry */
#define F_WEAK		 2	/* weak from lack of food */
#define F_FAINT		 3	/* fainting from lack of food */

/*
 * return values for get functions
 */
#define	NORM	0	/* normal exit */
#define	QUIT	1	/* quit option setting */
#define	MINUS	2	/* back up one option */

/* 
 * The character types
 */
#define	C_FIGHTER	0
#define	C_MAGICIAN	1
#define	C_CLERIC	2
#define	C_THIEF		3
#define	C_MONSTER	4

/*
 * values for games end
 */
#define SCOREIT -1
#define KILLED 	 0
#define CHICKEN  1
#define WINNER   2
#define TOTAL    3

/*
 * definitions for function step_ok:
 *	MONSTOK indicates it is OK to step on a monster -- it
 *	is only OK when stepping diagonally AROUND a monster
 */
#define MONSTOK 1
#define NOMONST 2

/*
 * used for ring stuff
 */
#define LEFT_1	 0
#define LEFT_2	 1
#define LEFT_3	 2
#define LEFT_4	 3
#define RIGHT_1	 4
#define RIGHT_2	 5
#define RIGHT_3	 6
#define RIGHT_4	 7

/*
 * All the fun defines
 */
#define next(ptr) (*ptr).l_next
#define prev(ptr) (*ptr).l_prev
#define ldata(ptr) (*ptr).l_data
#define inroom(rp, cp) (\
    (cp)->x <= (rp)->r_pos.x + ((rp)->r_max.x - 1) && (rp)->r_pos.x <= (cp)->x \
 && (cp)->y <= (rp)->r_pos.y + ((rp)->r_max.y - 1) && (rp)->r_pos.y <= (cp)->y)
#define winat(y, x) (mvwinch(mw, y, x)==' '?(mvwinch(stdscr, y, x)&A_CHARTEXT):(winch(mw)&A_CHARTEXT))
#define debug if (wizard) msg
#define RN (((seed = seed*11109+13849) & 0x7fff) >> 1)
#define unc(cp) (cp).y, (cp).x
#define cmov(xy) move((xy).y, (xy).x)
#define DISTANCE(y1, x1, y2, x2) ((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1))
#define OBJPTR(what)	(struct object *)((*what).l_data)
#define THINGPTR(what)	(struct thing *)((*what).l_data)
#define when break;case
#define otherwise break;default
#define until(expr) while(!(expr))
#define ce(a, b) ((a).x == (b).x && (a).y == (b).y)
#define draw(window) wrefresh(window)
#define hero player.t_pos
#define pstats player.t_stats
#define max_stats player.maxstats
#define pack player.t_pack
#define attach(a, b) _attach(&a, b)
#define detach(a, b) _detach(&a, b)
#define free_list(a) _free_list(&a)
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#define on(thing, flag) \
    (((thing).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] & flag) != 0)
#define off(thing, flag) \
    (((thing).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] & flag) == 0)
#define turn_on(thing, flag) \
    ((thing).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] |= (flag & ~FLAGMASK))
#define turn_off(thing, flag) \
    ((thing).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] &= ~flag)
#ifndef	CTRL
#define CTRL(ch) (ch & 037)
#endif
#define ALLOC(x) calloc(x,1)
#define FREE(x) free((char *) x)
#define	EQSTR(a, b, c)	(strncmp(a, b, c) == 0)
#define GOLDCALC (rnd(50 + 10 * level) + 2)
#define ISRING(h, r) (cur_ring[h] != NULL && cur_ring[h]->o_which == r)
#define ISWEARING(r)	(ISRING(LEFT_1, r) || ISRING(LEFT_2, r) ||\
			 ISRING(LEFT_3, r) || ISRING(LEFT_4, r) ||\
			 ISRING(RIGHT_1, r) || ISRING(RIGHT_2, r) ||\
			 ISRING(RIGHT_3, r) || ISRING(RIGHT_4, r))
#define newgrp() ++group
#define o_charges o_ac
#define ISMULT(type) (type == FOOD)
#define isrock(ch) ((ch == WALL) || (ch == '-') || (ch == '|'))
#define is_stealth(tp) \
    (rnd(25) < (tp)->t_stats.s_dext || (tp == &player && ISWEARING(R_STEALTH)))
#define mi_wght mi_worth

/*
 * Ways to die
 */
#define	D_PETRIFY	-1
#define	D_ARROW		-2
#define	D_DART		-3
#define	D_POISON	-4
#define	D_BOLT		-5
#define	D_SUFFOCATION	-6
#define	D_POTION	-7
#define	D_INFESTATION	-8
#define D_DROWN		-9
#define D_FALL		-10
#define D_FIRE		-11

/*
 * Things that appear on the screens
 */
#define	WALL		' '
#define	PASSAGE		'#'
#define	DOOR		'+'
#define	FLOOR		'.'
#define VPLAYER 	'@'
#define IPLAYER 	'_'
#define	POST		'^'
#define LAIR		'('
#define RUSTTRAP	';'
#define	TRAPDOOR	'>'
#define	ARROWTRAP	'{'
#define	SLEEPTRAP	'$'
#define	BEARTRAP	'}'
#define	TELTRAP		'~'
#define	DARTTRAP	'`'
#define POOL		'"'
#define MAZETRAP	'\\'
#define FIRETRAP	'<'
#define POISONTRAP	'['
#define ARTIFACT	','
#define	SECRETDOOR	'&'
#define	STAIRS		'%'
#define	GOLD		'*'
#define	POTION		'!'
#define	SCROLL		'?'
#define	MAGIC		'$'
#define	BMAGIC		'>'	/*	Blessed	magic	*/
#define	CMAGIC		'<'	/*	Cursed	magic	*/
#define	FOOD		':'
#define	WEAPON		')'
#define	ARMOR		']'
#define	RING		'='
#define	STICK		'/'
#define	CALLABLE	-1
#define	MARKABLE	-2

/*
 * Various constants
 */
#define	MAXAUTH		10		/* Let's be realistic! */
#define	PASSWD		rpass.rp_pass
#define PASSWDSEED	rpass.rp_pkey
#define	BEARTIME	3
#define	SLEEPTIME	4
#define	FREEZETIME	6
#define	HEALTIME	30
#define	HOLDTIME	2
#define	CHILLTIME	(roll(2, 4))
#define	SMELLTIME	20
#define	STONETIME	8
#define	SICKTIME	10
#define	STPOS		0
#define	WANDERTIME	70
#define HEROTIME	20
#define	BEFORE		1
#define	AFTER		2
#define	HUHDURATION	20
#define	SEEDURATION	850
#define	CLRDURATION	15
#define GONETIME	200
#define	PHASEDURATION	300
#define	HUNGERTIME	1300
#define	MORETIME	150
#define	STINKTIME	6
#define	STOMACHSIZE	2000
#define	ESCAPE		27
#define LINEFEED	10
#define CARRIAGE_RETURN 13
#define	BOLT_LENGTH	10
#define	MARKLEN		20

/*
 * Save against things
 */
#define	VS_POISON		0
#define	VS_PARALYZATION		0
#define	VS_DEATH		0
#define	VS_PETRIFICATION	1
#define	VS_WAND			2
#define	VS_BREATH		3
#define	VS_MAGIC		4

/*
 * attributes for treasures in dungeon
 */
#define ISCURSED     	       01
#define ISKNOW      	       02
#define ISPOST		       04	/* object is in a trading post */
#define	ISMETAL     	      010
#define ISPROT		      020	/* object is protected */
#define ISBLESSED     	      040
#define ISZAPPED             0100	/* weapon has been charged by dragon */
#define ISVORPED	     0200	/* vorpalized weapon */
#define ISSILVER	     0400	/* silver weapon */
#define ISPOISON	    01000	/* poisoned weapon */
#define CANRETURN	    02000	/* weapon returns if misses */
#define ISOWNED		    04000	/* weapon returns always */
#define ISLOST		   010000	/* weapon always disappears */
#define ISMISL      	   020000
#define ISMANY     	   040000
#define CANBURN           0100000	/* burns monsters */
#define ISHOLY		  0200000       /* double damage on undead monsters */
/*
 * Various flag bits
 */
#define ISDARK	    	       01
#define ISGONE	    	       02
#define	ISTREAS     	       04
#define ISFOUND     	      010
#define ISTHIEFSET	      020
/*
 * 1st set of creature flags (this might include player)
 */
#define ISBLIND     	       01
#define	ISINWALL     	       02
#define ISRUN	    	       04
#define	ISFLEE	    	      010
#define ISINVIS     	      020
#define ISMEAN      	      040
#define ISGREED     	     0100
#define CANSHOOT     	     0200
#define ISHELD      	     0400
#define ISHUH       	    01000
#define ISREGEN     	    02000
#define CANHUH      	    04000
#define CANSEE      	   010000
#define HASFIRE	    	   020000
#define ISSLOW	    	   040000
#define ISHASTE     	  0100000
#define ISCLEAR     	  0200000
#define CANINWALL   	  0400000
#define ISDISGUISE 	 01000000
#define CANBLINK   	 02000000
#define CANSNORE   	 04000000
#define HALFDAMAGE	010000000
#define	CANSUCK		020000000
#define	CANRUST		040000000
#define	CANPOISON      0100000000
#define	CANDRAIN       0200000000
#define ISUNIQUE       0400000000
#define	STEALGOLD     01000000000
#define	STEALMAGIC    02000000000
#define	CANDISEASE    04000000000

/* 
 * Second set of flags 
 */
#define HASDISEASE   010000000001
#define CANSUFFOCATE 010000000002
#define DIDSUFFOCATE 010000000004
#define BOLTDIVIDE   010000000010
#define BLOWDIVIDE   010000000020
#define NOCOLD	     010000000040
#define	TOUCHFEAR    010000000100
#define BMAGICHIT    010000000200
#define NOFIRE	     010000000400
#define NOBOLT	     010000001000
#define CARRYGOLD    010000002000
#define CANITCH      010000004000
#define HASITCH	     010000010000
#define DIDDRAIN     010000020000
#define WASTURNED    010000040000
#define CANSELL	     010000100000
#define CANBLIND     010000200000
#define CANBBURN     010000400000
#define ISCHARMED    010001000000
#define CANSPEAK     010002000000
#define CANFLY       010004000000
#define ISFRIENDLY   010010000000
#define CANHEAR	     010020000000
#define ISDEAF	     010040000000
#define CANSCENT     010100000000
#define ISUNSMELL    010200000000
#define WILLRUST     010400000000
#define WILLROT      011000000000
#define SUPEREAT     012000000000
#define PERMBLIND    014000000000

/* 
 * Third set of flags 
 */
#define MAGICHIT     020000000001
#define CANINFEST    020000000002
#define HASINFEST    020000000004
#define NOMOVE	     020000000010
#define CANSHRIEK    020000000020
#define CANDRAW	     020000000040
#define CANSMELL     020000000100
#define CANPARALYZE  020000000200
#define CANROT	     020000000400
#define ISSCAVENGE   020000001000
#define DOROT	     020000002000
#define CANSTINK     020000004000
#define HASSTINK     020000010000
#define ISSHADOW     020000020000
#define CANCHILL     020000040000
#define	CANHUG	     020000100000
#define CANSURPRISE  020000200000
#define CANFRIGHTEN  020000400000
#define CANSUMMON    020001000000
#define TOUCHSTONE   020002000000
#define LOOKSTONE    020004000000
#define CANHOLD      020010000000
#define DIDHOLD      020020000000
#define DOUBLEDRAIN  020040000000
#define ISUNDEAD     020100000000
#define BLESSMAP     020200000000
#define BLESSGOLD    020400000000
#define BLESSMONS    021000000000
#define BLESSMAGIC   022000000000
#define BLESSFOOD    024000000000

/* 
 * Fourth set of flags 
 */
#define CANBRANDOM   030000000001	/* Types of breath */
#define CANBACID     030000000002
#define CANBFIRE     030000000004
#define CANBBOLT     030000000010
#define CANBGAS      030000000020
#define CANBICE      030000000040
#define CANBPGAS     030000000100	/* Paralyze gas */
#define CANBSGAS     030000000200	/* Sleeping gas */
#define CANBSLGAS    030000000400	/* Slow gas */
#define CANBFGAS     030000001000	/* Fear gas */
#define CANBREATHE   030000001777	/* Can it breathe at all? */
#define STUMBLER     030000002000
#define POWEREAT     030000004000
#define ISELECTRIC   030000010000
#define HASOXYGEN    030000020000
#define POWERDEXT    030000040000
#define POWERSTR     030000100000
#define POWERWISDOM  030000200000
#define POWERINTEL   030000400000
#define POWERCONST   030001000000
#define SUPERHERO    030002000000
#define ISUNHERO     030004000000

#define ISREADY      040000000001
#define ISDEAD       040000000002

/* Masks for choosing the right flag */
#define FLAGMASK     030000000000
#define FLAGINDEX    000000000003
#define FLAGSHIFT    30

/* 
 * Mask for cancelling special abilities 
 * The flags listed here will be the ones left on after the
 * cancellation takes place
 */
#define CANC0MASK (	ISBLIND		| ISINWALL	| ISRUN		| \
			ISFLEE		| ISMEAN	| ISGREED	| \
			CANSHOOT	| ISHELD	| ISHUH		| \
			ISSLOW		| ISHASTE	| ISCLEAR	| \
			ISUNIQUE	| CARRYGOLD )
#define CANC1MASK (	HASDISEASE	| DIDSUFFOCATE	| CARRYGOLD 	| \
			HASITCH		| CANSELL 	| CANBBURN	| \
			CANSPEAK	| CANFLY	| ISFRIENDLY)
#define CANC2MASK (	HASINFEST	| NOMOVE	| ISSCAVENGE	| \
			DOROT		| HASSTINK	| DIDHOLD	| \
			ISUNDEAD )
#define CANC3MASK (	CANBREATHE )

/* types of things */
#define TYP_POTION	0
#define TYP_SCROLL	1
#define TYP_FOOD	2
#define TYP_WEAPON	3
#define TYP_ARMOR	4
#define TYP_RING	5
#define TYP_STICK	6
#define TYP_ARTIFACT	7
#define	NUMTHINGS	8

/*
 * Potion types
 */
#define	P_CLEAR		0
#define	P_ABIL		1
#define	P_SEEINVIS	2
#define	P_HEALING	3
#define	P_MFIND		4
#define	P_TFIND		5
#define	P_RAISE		6
#define	P_HASTE		7
#define	P_RESTORE	8
#define	P_PHASE		9
#define P_INVIS		10
#define P_SMELL		11
#define P_HEAR		12
#define P_SHERO		13
#define P_DISGUISE	14
#define	MAXPOTIONS	15

/*
 * Scroll types
 */
#define	S_CONFUSE	0
#define	S_MAP		1
#define	S_LIGHT		2
#define	S_HOLD		3
#define	S_SLEEP		4
#define	S_ALLENCH	5
#define	S_IDENT		6
#define	S_SCARE		7
#define	S_GFIND		8
#define	S_TELEP		9
#define	S_CREATE	10
#define	S_REMOVE	11
#define	S_PETRIFY	12
#define	S_GENOCIDE	13
#define	S_CURING	14
#define S_MAKEIT	15
#define S_PROTECT	16
#define S_NOTHING	17
#define S_SILVER	18
#define S_OWNERSHIP	19
#define S_FOODFIND	20
#define S_ELECTRIFY     21
#define	MAXSCROLLS	22

/*
 * Weapon types
 */
#define MACE		0		/* mace */
#define SWORD		1		/* long sword */
#define BOW		2		/* short bow */
#define ARROW		3		/* arrow */
#define DAGGER		4		/* dagger */
#define ROCK		5		/* rocks */
#define TWOSWORD	6		/* two-handed sword */
#define SLING		7		/* sling */
#define DART		8		/* darts */
#define CROSSBOW	9		/* crossbow */
#define BOLT		10		/* crossbow bolt */
#define SPEAR		11		/* spear */
#define TRIDENT		12		/* trident */
#define SPETUM		13		/* spetum */
#define BARDICHE	14 		/* bardiche */
#define SPIKE		15		/* short pike */
#define BASWORD		16		/* bastard sword */
#define HALBERD		17		/* halberd */
#define BATTLEAXE	18		/* battle axe */
#define SILVERARROW     19              /* silver arrows */
#define HANDAXE		20		/* hand axe */
#define CLUB		21		/* club */
#define FLAIL		22		/* flail */
#define GLAIVE		23		/* glaive */
#define GUISARME	24		/* guisarme */
#define HAMMER		25		/* hammer */
#define JAVELIN		26		/* javelin */
#define MSTAR		27		/* morning star */
#define PARTISAN	28		/* partisan */
#define PICK		29		/* pick */
#define LPIKE		30		/* long pike */
#define SCIMITAR	31		/* scimitar */
#define BULLET		32		/* sling bullet */
#define QSTAFF		33		/* quarter staff */
#define BRSWORD		34		/* broad sword */
#define SHSWORD		35		/* short sword */
#define SHIRIKEN	36		/* shurikens */
#define BOOMERANG	37		/* boomerangs */
#define MOLOTOV		38		/* molotov cocktails */
#define CLAYMORE	39		/* claymore sword */
#define CRYSKNIFE	40		/* crysknife */
#define FOOTBOW		41		/* footbow */
#define FBBOLT		42		/* footbow bolt */
#define MACHETE		43		/* machete */
#define LEUKU		44		/* leuku */
#define TOMAHAWK	45		/* tomahawk */
#define PERTUSKA	46		/* pertuska */
#define SABRE		47		/* sabre */
#define CUTLASS		48		/* cutlass sword */
#define GRENADE     49      /* grenade */
#define MAXWEAPONS	50		/* types of weapons */
#define NONE		100		/* no weapon */

/*
 * Armor types
 */
#define	LEATHER		0
#define	RING_MAIL	1
#define	STUDDED_LEATHER	2
#define	SCALE_MAIL	3
#define	PADDED_ARMOR	4
#define	CHAIN_MAIL	5
#define	SPLINT_MAIL	6
#define	BANDED_MAIL	7
#define	PLATE_MAIL	8
#define	PLATE_ARMOR	9
#define	MITHRIL		10
#define	CRYSTAL_ARMOR   11
#define	MAXARMORS	12

/*
 * Ring types
 */
#define	R_PROTECT	0
#define	R_ADDSTR	1
#define	R_SUSABILITY	2
#define	R_SEARCH	3
#define	R_SEEINVIS	4
#define	R_ALERT		5
#define	R_AGGR		6
#define	R_ADDHIT	7
#define	R_ADDDAM	8
#define	R_REGEN		9
#define	R_DIGEST	10
#define	R_TELEPORT	11
#define	R_STEALTH	12
#define	R_ADDINTEL	13
#define	R_ADDWISDOM	14
#define	R_HEALTH	15
#define R_VREGEN	16
#define R_LIGHT		17
#define R_DELUSION	18
#define R_CARRYING	19
#define R_ADORNMENT	20
#define R_LEVITATION	21
#define R_FIRERESIST	22
#define R_COLDRESIST	23
#define R_ELECTRESIST	24
#define R_RESURRECT	25
#define R_BREATHE	26
#define R_FREEDOM	27
#define R_WIZARD	28
#define R_TELCONTROL	29
#define	MAXRINGS	30

/*
 * Rod/Wand/Staff types
 */

#define	WS_LIGHT	0
#define	WS_HIT		1
#define	WS_ELECT	2
#define	WS_FIRE		3
#define	WS_COLD		4
#define	WS_POLYMORPH	5
#define	WS_MISSILE	6
#define	WS_SLOW_M	7
#define	WS_DRAIN	8
#define	WS_CHARGE	9
#define	WS_TELMON	10
#define	WS_CANCEL	11
#define WS_CONFMON	12
#define WS_ANNIH	13
#define WS_ANTIM	14
#define WS_PARALYZE	15
#define WS_MDEG		16
#define WS_NOTHING	17
#define WS_INVIS	18
#define WS_BLAST    19
#define	MAXSTICKS	20

/*
 * Food types
 */

#define FD_RATION	0
#define FD_FRUIT	1
#define FD_CRAM		2
#define FD_CAKES	3
#define FD_LEMBA	4
#define FD_MIRUVOR	5
#define	MAXFOODS	6

/*
 * Artifact types
 */

#define TR_PURSE	0
#define TR_PHIAL	1
#define TR_AMULET	2
#define TR_PALANTIR	3
#define TR_CROWN	4
#define TR_SCEPTRE	5
#define TR_SILMARIL	6
#define TR_WAND		7
#define MAXARTIFACT	8

/*
 * Artifact flags
 */

#define ISUSED	       01
#define ISACTIVE       02

/*
 * Now we define the structures and types
 */

struct delayed_action {
	int d_type;
	void (*d_func)();
	void *d_arg;
	int d_time;
};

/*
 * level types
 */
typedef enum {
	NORMLEV,	/* normal level */
	POSTLEV,	/* trading post level */
	MAZELEV,	/* maze level */
	THRONE		/* unique monster's throne room */
} LEVTYPE;

/*
 * Help list
 */

struct h_list {
    int h_ch;
    char *h_desc;
};

/*
 * Coordinate data type
 */
typedef struct {
    int x;
    int y;
} coord;

/*
 * Linked list data type
 */
struct linked_list {
    struct linked_list *l_next;
    struct linked_list *l_prev;
    char *l_data;			/* Various structure pointers */
    char l_letter;			/* Letter for inventory */
};

/*
 * Stuff about magic items
 */

struct magic_item {
    char *mi_name;
    int mi_prob;
    int mi_worth;
    int mi_curse;
    int mi_bless;
};

/*
 * Room structure
 */
struct room {
    coord r_pos;			/* Upper left corner */
    coord r_max;			/* Size of room */
    unsigned int r_flags;	/* Info about the room */
    int r_fires;			/* Number of fires in room */
    int r_nexits;			/* Number of exits */
    coord r_exit[MAXDOORS];	/* Where the exits are */
};

/*
 * Initial artifact stats
 */
struct init_artifact {
	char *ar_name;		/* name of the artifact */
	int ar_level;		/* first level where it appears */
	int ar_rings;		/* number of ring effects */
	int ar_potions;		/* number of potion effects */
	int ar_scrolls;		/* number of scroll effects */
	int ar_wands;		/* number of wand effects */
	int ar_worth;		/* gold pieces */
	int ar_weight;		/* weight of object */
};

/*
 * Artifact attributes
 */
struct artifact {
	unsigned int ar_flags;		/* general flags */
	unsigned int ar_rings;		/* ring effects flags */
	unsigned int ar_potions;	/* potion effects flags */
	unsigned int ar_scrolls;	/* scroll effects flags */
	unsigned int ar_wands;		/* wand effects flags */
	struct linked_list *t_art; /* linked list pointer */
};

/*
 * Array of all traps on this level
 */
struct trap {
    int tr_type;			/* What kind of trap */
    int tr_show;			/* Where disguised trap looks like */
    coord tr_pos;			/* Where trap is */
    unsigned int tr_flags;			/* Info about trap (i.e. ISFOUND) */
};

/*
 * Structure describing a fighting being
 */
struct stats {
    int s_str;			/* Strength */
    int s_intel;			/* Intelligence */
    int s_wisdom;			/* Wisdom */
    int s_dext;				/* Dexterity */
    int s_const;			/* Constitution */
    int s_charisma;			/* Charisma */
    int s_exp;				/* Experience */
    int s_lvl;				/* Level of mastery */
    int s_arm;				/* Armor class */
    int s_hpt;				/* Hit points */
    int s_pack;				/* current weight of his pack */
    int s_carry;			/* max weight he can carry */
    char s_dmg[30];			/* String describing damage done */
};

/*
 * Structure describing a fighting being (monster at initialization)
 */
struct mstats {
    int s_str;				/* Strength */
    int s_exp;				/* Experience */
    int s_lvl;				/* Level of mastery */
    int s_arm;				/* Armor class */
    char *s_hpt;			/* Hit points */
    char *s_dmg;			/* String describing damage done */
};

/*
 * Structure for monsters and player
 */
struct thing {
    int  t_turn;			/* If slowed, is it a turn to move */
    int  t_wasshot;			/* Was character shot last round? */
    int  t_type;			/* What it is */
    int t_disguise;			/* What mimic looks like */
    int t_oldch;			/* Character that was where it was */
    int t_ctype;			/* Character type */
    int t_index;			/* Index into monster table */
    int t_no_move;			/* How long the thing can't move */
    int t_quiet;			/* used in healing */
    int t_doorgoal;			/* What door are we heading to? */
    coord t_pos;			/* Position */
    coord t_oldpos;			/* Last position */
    coord *t_dest;			/* Where it is running to */
    unsigned int t_flags[4];		/* State word */
    struct linked_list *t_pack;		/* What the thing is carrying */
    struct stats t_stats;		/* Physical description */
    struct stats maxstats;		/* maximum(or initial) stats */
	int t_reserved;
};

/*
 * Array containing information on all the various types of monsters
 */
struct monster {
    char *m_name;			/* What to call the monster */
    int m_carry;			/* Probability of carrying something */
    int m_normal;			/* Does monster exist? */
    int m_wander;			/* Does monster wander? */
    int m_appear;			/* What does monster look like? */
    char *m_intel;			/* Intelligence range */
    unsigned int m_flags[10];	/* Things about the monster */
    char *m_typesum;			/* type of creature can he summon */
    int m_numsum;			/* how many creatures can he summon */
    int m_add_exp;			/* Added experience per hit point */
    struct mstats m_stats;		/* Initial stats */
};

/*
 * Structure for a thing that the rogue can carry
 */

struct object {
    int o_type;				/* What kind of object it is */
    coord o_pos;			/* Where it lives on the screen */
    char *o_text;			/* What it says if you read it */
    int  o_launch;			/* What you need to launch it */
    char o_damage[8];			/* Damage if used like sword */
    char o_hurldmg[8];			/* Damage if thrown */
    int o_count;			/* Count for plural objects */
    int o_which;			/* Which object of a type it is */
    int o_hplus;			/* Plusses to hit */
    int o_dplus;			/* Plusses to damage */
    int o_ac;				/* Armor class */
    unsigned int o_flags;			/* Information about objects */
    int o_group;			/* Group number for this object */
    int o_weight;			/* weight of this object */
    char o_mark[MARKLEN];		/* Mark the specific object */
    struct artifact art_stats;		/* substructure for artifacts */
};
/*
 * weapon structure
 */
struct init_weps {
    char *w_name;		/* name of weapon */
    char *w_dam;		/* hit damage */
    char *w_hrl;		/* hurl damage */
    int  w_launch;		/* need to launch it */
    int  w_flags;		/* flags */
    int  w_wght;		/* weight of weapon */
    int  w_worth;		/* worth of this weapon */
};

/*
 * armor structure 
 */
struct init_armor {
	char *a_name;		/* name of armor */
	int  a_prob;		/* chance of getting armor */
	int  a_class;		/* normal armor class */
	int  a_worth;		/* worth of armor */
	int  a_wght;		/* weight of armor */
};

struct matrix {
    int base;			/* Base to-hit value (AC 10) */
    int max_lvl;		/* Maximum level for changing value */
    int factor;			/* Amount base changes each time */
    int offset;			/* What to offset level */
    int range;			/* Range of levels for each offset */
};

struct spells {
    int s_which;		/* which scroll or potion */
    int s_cost;		/* cost of casting spell */
    int s_type;		/* scroll or potion */
    int  s_blessed;		/* is the spell blessed? */
};

struct  real_pass {
	char	rp_pass[20];
	char	rp_pkey[2];
} ;

void waste_time(void); /* armor.c */
void wear(void);
void take_off(void);

void new_artifact(int which, struct object *cur); /* artifact.c */
int make_artifact(void);
void apply(void);
int possessed(int artifact);
void do_minor(struct object *tr);
void do_major();
void do_phial(void);
void do_palantir(void);
void do_silmaril(void);
void do_amulet(void);
void do_bag(struct object *obj);
void do_sceptre(void);
void do_wand(void);
void do_crown(void);
void add_bag(struct linked_list **bag);
struct linked_list *get_bag(struct linked_list **bag);
void bag_inventory(struct linked_list *list);
int bag_char(struct object *obj, struct linked_list *bag);
void bagletter(struct linked_list *item);
void delbagletter(struct linked_list *item);
int is_carrying(int artifact);

int cansee(int y, int x); /* chase.c */
void runto(coord *runner, coord *spot);
int diag_ok(coord *sp, coord *ep, struct thing *flgptr);
struct room *roomin(coord *cp);
struct linked_list *find_mons(int y, int x);
void runners(void);
int can_blink(struct thing *tp);
coord *can_shoot(coord *er, coord *ee);
struct linked_list *get_hurl(struct thing *tp);
int straight_shot(int ery, int erx, int eey, int eex, coord *shooting);
void do_chase(struct thing *th, int flee);
int chase(struct thing *tp, coord *ee, int flee, int *mdead);

void quit(int sig); /* command.c */
void command(void);
void call(int mark);
void shell(void);
void d_level(void);
void u_level(void);
void help(void);
void identify(void);
void search(int is_thief);

void extinguish(void (*func)()); /* daemon.c */
struct delayed_action *find_slot(void (*func)());
void lengthen(void (*func)(), int xtime);
void fuse(void (*func)(), void *arg, int time, int type);
void start_daemon(void (*func)(), void *arg, int type);
void kill_daemon(void (*func)());
void do_daemons(int flag);
void do_fuses(int flag);
void activity();

void unsee(void); /* daemons.c */
void unconfuse(void);
void unscent(void);
void sight(void);
void res_strength(void);
void nohaste(void);
void noslow(void);
void suffocate(void);
void stomach(void);
void cure_disease(void);
void un_itch(void);
void appear(void);
void unelectrify(void);
void unshero(void);
void unbhero(void);
void unxray(void);
void undisguise(void);
void shero(void);
void unphase(void);
void unclrhead(void);
void unstink(void);
void hear(void);
void unhear(void);
void scent(void);
void rollwand(void);
void swander(void);
void doctor(struct thing *tp);

void updpack(int getmax); /* encumb.c */
int itemweight(struct object *wh);
int totalenc(void);
int hitweight(void);
int packweight(void);
int playenc(void);
void wghtchk(void);

int fight(coord *mp, struct object *weap, int thrown); /* fight.c */
int attack(struct thing *mp, struct object *weapon, int thrown);
int save_throw(int which, struct thing *tp);
void killed(struct linked_list *item, int pr, int points);
int save(int which);
int is_magic(struct object *obj);
void raise_level(void);
void check_level(void);
int swing(int class, int at_lvl, int op_arm, int wplus);
void remove_monster(coord *mp, struct linked_list *item);
struct object *wield_weap(struct object *weapon, struct thing *mp);
void m_thunk(struct object *weap, char *mname);
void bounce(struct object *weap, char *mname);
void m_bounce(struct object *weap, char *mname);
void do_fight(int y, int x, int tothedeath);
int roll_em(struct thing *att_er, struct thing *def_er, struct object *weap, int hurl, struct object *cur_weapon);
int hung_dam(void);
void hit(char *er, char *ee);
void miss(char *er, char *ee);
int dext_plus(int dexterity);
int dext_prot(int dexterity);
int str_plus(int str);
int add_dam(int str);
void thunk(struct object *weap, char *mname);
char *prname(char *who, int upper);

int geta_player(void); /* get_play.c */
int puta_player(int arm, int wpt, int hpadd, int dmadd);

void init_things(void); /* init.c */
void init_fd(void);
void init_colors(void);
void init_names(void);
void init_stones(void);
void init_materials(void);
void badcheck(char *name, struct magic_item *magic, int bound);
void init_player(void);

void doadd(char *fmt, va_list ap); /* io.c */
void msg(char *fmt, ...);
void addmsg(char *fmt, ...);
void endmsg(void);
int readchar(WINDOW *win);
void restscr(WINDOW *scr);
void wait_for(WINDOW *win, int ch);
int shoot_ok(int ch);
void status(int display);
int step_ok(int y, int x, int can_on_monst, struct thing *flgptr);
void dbotline(WINDOW *scr,char *message);
void show_win(WINDOW *scr, char *message);
void ministat(void);

void _detach(struct linked_list **list, struct linked_list *item); /* list.c */
void _attach(struct linked_list **list, struct linked_list *item);
void _free_list(struct linked_list **ptr);
void discard(struct linked_list *item);
struct linked_list *new_item(int size);
char *new(size_t size);
struct linked_list *creat_item(void);


int rnd(int range); /* main.c */
int roll(int number, int sides);
int makesure(void);
void setup(void);
void playit(void);
void fatal(char *s);
void areuok(char *file);
int author(void);
int too_much(void);
int holiday(void);
int ucount(void);
void endit(void);
void checkout(int sig);
void chmsg(char *fmt, ...);
void loadav(double *avg);
void tstp(int sig);

void do_maze(void); /* maze.c */
int maze_view(int y, int x);
char *moffset(int y, int x);
char *foffset(int y, int x);
int findcells(int y, int x);
void rmwall(int newy, int newx, int oldy, int oldx);
void crankout(void);
void draw_maze(void);
 
int is_current(struct object *obj); /* misc.c */
void chg_str(int amt, int both, int lost);
void chg_dext(int amt, int both, int lost);
void aggravate(void);
void look(int wakeup);
void add_haste(int blessed);
int get_dir(void);
char *tr_name(int ch);
int secretdoor(int y, int x);
struct linked_list *find_obj(int y, int x);
void eat(void);
char *vowelstr(char *str);
void listenfor(void);

void new_monster(struct linked_list *item, int type, coord *cp, int max_monster); /* monsters.c */
void check_residue(struct thing *tp);
void genocide(void);
int randmonster(int wander, int no_unique);
void wanderer(void);
struct linked_list *wake_monster(int y, int x);
int id_monst(int monster);
void sell(struct thing *tp);

void light(coord *cp); /* move.c */
int show(int y, int x);
int blue_light(int blessed, int cursed);
void do_run(int ch);
void corr_move(int dy, int dx);
void do_move(int dy, int dx);
int be_trapped(struct thing *th, coord *tc);
void dip_it(void);
coord *rndmove(struct thing *who);
int isatrap(int ch);
struct trap *trap_at(int y, int x);
void set_trap(struct thing *tp, int y, int x);

int rnd_room(void); /* new_level.c */
void new_level(LEVTYPE ltype);
void put_things(LEVTYPE ltype);
void do_throne(void);

int get_str(void *opt, WINDOW *win); /* options.c */
void option(void);
void put_bool(void *b, WINDOW *win);
void put_str(void *str, WINDOW *win);
int get_bool(void *vp, WINDOW *win);
int get_abil(void *vp, WINDOW *win);
void parse_opts(char *str);
void strucpy(char *s1, char *s2, size_t len);
void put_abil(void *ability, WINDOW *win);
 
void freeletter(struct linked_list *item); /* pack.c */
void idenpack(void);
int add_pack(struct linked_list *item, int silent);
int inventory(struct linked_list *list, int type);
void pick_up(int ch);
void picky_inven(void);
struct linked_list *get_item(char *purpose, int type);
void del_pack(struct linked_list *what);
void cur_null(struct object *op);
void getletter(struct linked_list *item);
int pack_char(struct object *obj);
void show_floor(void);
  
void do_passages(void); /* passages.c */
void conn(int r1, int r2);
void door(struct room *rm, coord *cp);
 
int const_bonus(void); /* player.c */
void gsense(void);
void affect(void);
void cast(void);
void steal(void);
void pray(void);
   
void quaff(int which, int blessed); /* potions.c */
void add_intelligence(int cursed);
void add_wisdom(int cursed);
void add_dexterity(int cursed);
void add_const(int cursed);
void add_strength(int cursed);
void lower_level(int who);
void res_dexterity(void);
void res_wisdom(void);
void res_intelligence(void);
 
int ring_value(int type); /* rings.c */
void ring_on(void);
int ring_eat(int hand);
char *ring_num(struct object *obj);
void ring_off(void);

void death(int monst); /* rip.c */
void byebye(int sig);
void score(int amount, int flags, int monst);
void showpack(char *howso);
void total_winner(void);
char *killname(int monst);
int save_resurrect(int bonus);

void rnd_pos(struct room *rp, coord *cp); /* rooms.c */
void draw_room(struct room *rp);
void do_rooms(void);
void vert(int cnt);
void horiz(int cnt);

int save_game(void); /* save.c */
void auto_save(int sig);
int save_file(char *file_name);
size_t encwrite (void *start, size_t size, FILE *outf);
size_t encread (void *start, size_t size, FILE *infd);
int restore(char *file, char **envp);
size_t putword (int word, FILE *file);
int getword (FILE *fd);

void read_scroll(int which, int blessed); /* scrolls.c */
int creat_mons(struct thing *person, int monster, int report);
int is_r_on(struct object *obj);

int rs_save_file(const char *file_name); /* state.c */
int rs_restore_file(const char *file_name);

void fix_stick(struct object *cur); /* sticks.c */
char *charge_str(struct object *obj);
void do_zap(int gotdir, int which, int blessed);
void drain(int ymin, int ymax, int xmin, int xmax);
int shoot_bolt(struct thing *shooter, coord start, coord dir, int get_points, int reason, char *name, int damage);

int dropcheck(struct object *op); /* things.c */
int drop(struct linked_list *item);
char *inv_name(struct object *obj, int drop);
struct linked_list *spec_item(int type, int which, int hit, int damage);
int pick_one(struct magic_item *magic, int nitems);
int extras(void);
char *blesscurse(int flags);
struct linked_list *new_thing(void);

void do_post(void); /* trader.c */
int price_it(void);
void buy_it(void);
void sell_it(void);
int open_market(void);
char *typ_name(struct object *obj);
int get_worth(struct object *obj);
void trans_line(void);

int fall(struct linked_list *item, int pr); /* weapons.c */
void init_weapon(struct object *weap, int type);
void missile(int ydelta, int xdelta, struct linked_list *item, struct thing *tp);
void do_motion(struct object *obj, int ydelta, int xdelta, struct thing *tp);
int fallpos(coord *pos, coord *newpos, int passages);
int hit_monster(int y, int x, struct object *obj, struct thing *tp);
void wield(void);
char *num(int n1, int n2);

void makemon(void); /* wizard.c */
int getbless(void);
int teleport(void);
int passwd(void);
void whatis(struct linked_list *what);
void create_obj(int which_item, int which_type, int cursed);

/*
 * Now all the global variables
 */
extern struct h_list helpstr[];
extern FILE *fd_score;
extern struct trap traps[];
extern struct room rooms[];		/* One for each room -- A level */
extern struct room *oldrp;		/* Roomin(&oldpos) */
extern struct linked_list *mlist;	/* List of monsters on the level */
extern struct linked_list *monst_dead;
extern struct thing player;		/* The rogue */
extern struct thing *beast;		/* The last monster attacking */
extern struct monster monsters[];	/* The initial monster states */
extern struct linked_list *lvl_obj;	/* List of objects on this level */
extern struct object *cur_weapon;	/* Which weapon he is weilding */
extern struct object *cur_armor;	/* What a well dresssed rogue wears */
extern struct object *cur_ring[];	/* Which rings are being worn */
extern struct magic_item things[];	/* Chances for each type of item */
extern struct magic_item s_magic[];	/* Names and chances for scrolls */
extern struct magic_item p_magic[];	/* Names and chances for potions */
extern struct magic_item r_magic[];	/* Names and chances for rings */
extern struct magic_item ws_magic[];	/* Names and chances for sticks */
extern struct magic_item fd_data[];	/* Names and chances for food */
extern struct spells magic_spells[];	/* spells for magic users */
extern struct spells cleric_spells[];	/* spells for magic users */
extern struct real_pass rpass;		/* For protection's sake! */
extern char *cnames[][11];		/* Character level names */
extern char curpurch[];			/* name of item ready to buy */
extern int PLAYER;			/* what the player looks like */
extern int def_array[MAXPDEF][MAXPATT];	/* Pre-def'd chars */
extern int resurrect;			/* resurrection counter */
extern int char_type;			/* what type of character is player */
extern int foodlev;			/* how fast he eats food */
extern int see_dist;			/* how far he can see^2 */
extern int level;			/* What level rogue is on */
extern int monslevel;			/* What level the monsters are from */
extern int trader;			/* number of purchases */
extern int curprice;			/* price of an item */
extern int purse;			/* How much gold the rogue has */
extern size_t mpos;			/* Where cursor is on top line */
extern int ntraps;			/* Number of traps on this level */
extern int no_move;			/* Number of turns held in place */
extern int no_command;			/* Number of turns asleep */
extern int inpack;			/* Number of things in pack */
extern int total;			/* Total dynamic memory bytes */
extern int lastscore;			/* Score before this turn */
extern int no_food;			/* Number of levels without food */
extern int seed;			/* Random number seed */
extern int count;			/* Number of times to repeat command */
extern int dnum;			/* Dungeon number */
extern int max_level;			/* Deepest player has gone */
extern int food_left;			/* Amount of food in hero's stomach */
extern int group;			/* Current group number */
extern int hungry_state;		/* How hungry is he */
extern int infest_dam;			/* Damage from parasites */
extern int lost_str;			/* Amount of strength lost */
extern int lost_dext;			/* amount of dexterity lost */
extern int hold_count;			/* Number of monsters holding player */
extern int trap_tries;			/* Number of attempts to set traps */
extern int spell_power;			/* Spell power left at this level */
extern int auth_or[MAXAUTH];		/* MAXAUTH priviledged players */
extern int has_artifact;		/* set for possesion of artifacts */
extern int picked_artifact;		/* set for any artifacts picked up */
extern int msg_index;			/* pointer to current message buffer */
extern int luck;			/* how expensive things to buy thing */
extern int take;			/* Thing the rogue is taking */
extern char prbuf[];			/* Buffer for sprintfs */
extern char outbuf[];			/* Output buffer for stdout */
extern int  runch;			/* Direction player is running */
extern char *s_names[];			/* Names of the scrolls */
extern char *p_colors[];		/* Colors of the potions */
extern char *r_stones[];		/* Stone settings of the rings */
extern struct init_weps weaps[];	/* weapons and attributes */
extern struct init_armor armors[];	/* armors and attributes */
extern struct init_artifact arts[];	/* artifacts and attributes */
extern char *ws_made[];			/* What sticks are made of */
extern char *release;			/* Release number of rogue */
extern char whoami[];			/* Name of player */
extern char fruit[];			/* Favorite fruit */
extern char msgbuf[10][2*BUFSIZ];	/* message buffer */
extern char *s_guess[];			/* Players guess at what scroll is */
extern char *p_guess[];			/* Players guess at what potion is */
extern char *r_guess[];			/* Players guess at what ring is */
extern char *ws_guess[];		/* Players guess at what wand is */
extern char *ws_type[];			/* Is it a wand or a staff */
extern char file_name[];		/* Save file name */
extern char score_file[];		/* Score file name */
extern char home[];			/* User's home directory */
extern WINDOW *cw;			/* Window that the player sees */
extern WINDOW *hw;			/* Used for the help command */
extern WINDOW *mw;			/* Used to store mosnters */
extern WINDOW *msgw;
extern int pool_teleport;		/* just teleported from a pool */
extern int inwhgt;			/* true if from wghtchk() */
extern int running;			/* True if player is running */
extern int fighting;			/* True if player is fighting */
extern int playing;			/* True until he quits */
extern int wizard;			/* True if allows wizard commands */
extern int after;			/* True if we want after daemons */
extern int notify;			/* True if player wants to know */
extern int fight_flush;		/* True if toilet input */
extern int terse;			/* True if we should be short */
extern int door_stop;			/* Stop running when we pass a door */
extern int jump;			/* Show running as series of jumps */
extern int slow_invent;		/* Inventory one line at a time */
extern int firstmove;			/* First move after setting door_stop */
extern int waswizard;			/* Was a wizard sometime */
extern int canwizard;			/* Will be permitted to do this */
extern int askme;			/* Ask about unidentified things */
extern int moving;			/* move using 'm' command */
extern int  s_know[];			/* Does he know what a scroll does */
extern int  p_know[];			/* Does he know what a potion does */
extern int  r_know[];			/* Does he know what a ring does */
extern int  ws_know[];			/* Does he know what a stick does */
extern int in_shell;			/* True if executing a shell */
extern coord oldpos;			/* Position before last look() call */
extern coord delta;			/* Change indicated to get_dir() */
extern char *spacemsg;
extern char *morestr;
extern char *retstr;
extern LEVTYPE levtype;
extern char *rainbow[NCOLORS];
extern char *sylls[NSYLLS];
extern char *stones[NSTONES];
extern char *metal[NMETAL];
extern char *wood[NWOOD];
extern coord ch_ret;
extern int demoncnt;
extern struct delayed_action d_list[MAXDAEMONS];
extern int between;
extern char prbuf[2*LINELEN];
extern int inbag;
extern char bag_letters[];
extern char *bag_index;
extern char *bag_end;
extern char pack_letters[];
extern char *pack_index;
extern char *pack_end;
extern char version[];
extern int oldcol;
extern int oldline;
extern char *oversion;

