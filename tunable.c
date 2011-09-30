/*
    tunable.c  -  Machine dependent (tunable) parametesafe.rs.
   
    UltraRogue
    Copyright (C) 1985 Herb Chong
    All rights reserved.
    
    See the file LICENSE.TXT for full copyright and licensing information.
*/

/*
 * Wizard's identity.
 */
int	UID		=	97;			/* his user id */
char	LOGIN[]		=	"games";		/* his login name */

/*
 * Special files.
 */

/*
 * File containing uids of people considered authosafe.rs.
 */
char	PERMOK[]	=	"permok";

/*
 * File containing wizard's password and password key.
 */ 
char	PASSCTL[]	=	"passctl";

/*
 * File where news is kept
 */

char	NEWS[]		=	"news";

/*
 * Directory where the scores are stored.
 */
char	SCOREDIR[]	=	"urogue_roll";

/*
 * Location of lav program to print load averages
 */
char	LOADAV_PROG[]	=	"";

/*
 * Location of utmp
 */
char	UTMP[]		=	"";

/*
 * Name of player definitions file
 */
char	ROGDEFS[]	=	".rog_defs";

/*
 * Name of pager routine for news and motd
 */
char	PAGER[]		=	"/bin/cat";

/*
 * Load control constants
 */
int	MAXLOAD		=	50;	/* 10 * max 15 minute load average */
int	MAXUSERS	=	15;	/* max users at start */
int	HOLIDAY		=	0;	/* enable prime shift restrictions */
int	CHECKTIME	=	5;	/* minutes between load checks */
