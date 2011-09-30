/*
    options.c  -  This file has all the code for the option command.
   
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

 /*
  * I would rather this command were not necessary, but
  * it is the only way to keep the wolves off of my back.
  *
  */
 
#include <string.h>
#include <stdio.h>
#include "mach_dep.h"
#include "curses.h"
#include <ctype.h>
#include "rogue.h"
 
#define	NUM_OPTS	(sizeof optlist / sizeof (OPTION))
 
 
 /*
  * description of an option and what to do with it
  */
 struct optstruct {
     char	*o_name;	/* option name */
     char	*o_prompt;	/* prompt for interactive entry */
     void		*o_opt;		/* pointer to thing to set */
     void		(*o_putfunc)(void *vp, WINDOW *win);	/* function to print value */
     int		(*o_getfunc)(void *vp, WINDOW *win);	/* function to get value interactively */
 };
 
 typedef struct optstruct	OPTION;
 
 OPTION	optlist[] = {
     {"terse",	 "Terse output (terse): ",
 		 &terse,	put_bool,	get_bool	},
     {"flush",	 "Flush typeahead during battle (flush): ",
 		 &fight_flush,	put_bool,	get_bool	},
     {"jump",	 "Show position only at end of run (jump): ",
 		 &jump,		put_bool,	get_bool	},
     {"step",	"Do inventories one line at a time (step): ",
 		&slow_invent,	put_bool,	get_bool	},
     {"askme",	"Ask me about unidentified things (askme): ",
 		&askme,		put_bool,	get_bool	},
     {"name",	 "Name (name): ",
 		whoami,		put_str,	get_str		},
     {"fruit",	 "Fruit (fruit): ",
 		fruit,		put_str,	get_str		},
     {"file",	 "Save file (file): ",
 		file_name,	put_str,	get_str		},
     {"score",	 "Score file (score): ",
 		score_file,	put_str,	get_str		},
     {"class",	"Character class (class): ",
 		&char_type,	put_abil,	get_abil	}
 };
 
 /*
  * print and then set options from the terminal
  */
 void
 option(void)
 {
     register OPTION	*op;
     register int	retval;
 
     wclear(hw);
     touchwin(hw);
     /*
      * Display current values of options
      */
     for (op = optlist; op <= &optlist[NUM_OPTS-1]; op++)
     {
 	waddstr(hw, op->o_prompt);
 	(*op->o_putfunc)(op->o_opt, hw);
 	waddch(hw, '\n');
     }
     /*
      * Set values
      */
     wmove(hw, 0, 0);
     for (op = optlist; op <= &optlist[NUM_OPTS-1]; op++)
     {
 	waddstr(hw, op->o_prompt);
 	retval = (*op->o_getfunc)(op->o_opt, hw);
 	if (retval)
	{
 	    if (retval == QUIT)
 		break;
 	    else if (op > optlist) {	/* MINUS */
 		wmove(hw, (int)(op - optlist) - 1, 0);
 		op -= 2;
 	    }
 	    else	/* trying to back up beyond the top */
 	    {
 		putchar('\007');
 		wmove(hw, 0, 0);
 		op--;
 	    }
         }
     }
     /*
      * Switch back to original screen
      */
     mvwaddstr(hw, LINES-1, 0, spacemsg);
     draw(hw);
     wait_for(hw, ' ');
     clearok(cw, TRUE);
     touchwin(cw);
     after = FALSE;
 }
 
 /*
  * put out a boolean
  */
 void
 put_bool(void *b, WINDOW *win)
 {
     waddstr(win, *(int *)b ? "True" : "False");
 }
 
 /*
  * put out a string
  */
 void
 put_str(void *str, WINDOW *win)
 {
     waddstr(win, (char *)str);
 }
 
 /*
  * print the character type
  */
 void
 put_abil(void *ability, WINDOW *win)
 {
     char *abil;
 
     switch (*(int *)ability) {
 	case C_FIGHTER:
 	    abil = "Fighter";
 	    break;
 	case C_MAGICIAN:
 	    abil = "Magic User";
 	    break;
 	case C_CLERIC:
 	    abil = "Cleric";
 	    break;
 	case C_THIEF:
 	    abil = "Thief";
 	    break;
 	default:
 	    abil = "??";
     }
     waddstr(win, abil);
 }
 
 
 /*
  * allow changing a boolean option and print it out
  */
 int
 get_bool(void *vp, WINDOW *win)
 {
	 int *bp = (int *) vp;
     register int oy, ox;
     register int op_bad;
 
     op_bad = TRUE;
     getyx(win, oy, ox);
     waddstr(win, *bp ? "True" : "False");
     while(op_bad)	
     {
 	wmove(win, oy, ox);
 	draw(win);
 	switch (readchar(win))
 	{
 	    case 't':
 	    case 'T':
 		*bp = TRUE;
 		op_bad = FALSE;
 		break;
 	    case 'f':
 	    case 'F':
 		*bp = FALSE;
 		op_bad = FALSE;
 		break;
 	    case '\n':
 	    case '\r':
 		op_bad = FALSE;
 		break;
 	    case '\033':
 	    case '\007':
 		return QUIT;
 	    case '-':
 		return MINUS;
 	    default:
 		mvwaddstr(win, oy, ox + 10, "(T or F)");
 	}
     }
     wmove(win, oy, ox);
     wclrtoeol(win);
     waddstr(win, *bp ? "True" : "False");
     waddch(win, '\n');
     return NORM;
 }
 
 /*
  * set a string option
  */
 int
 get_str(void *vp, WINDOW *win)
 {
     char *opt = (char *) vp;
     register char *sp;
     register int c, oy, ox;
     char buf[LINELEN];
     
	 *opt = 0;
     
	 draw(win);
     getyx(win, oy, ox);
     /*
      * loop reading in the string, and put it in a temporary buffer
      */
     for (sp = buf;
 	(c = readchar(win)) != '\n'	&& 
 	c != '\r'			&& 
 	c != '\033'			&& 
 	c != '\007'			&&
 	sp < &buf[LINELEN-1];
 	wclrtoeol(win), draw(win))
     {
 	if (c == -1)
 	    continue;
 	else if (c == md_erasechar()) /* process erase character */
 	{
 	    if (sp > buf)
 	    {
 		register size_t i;
 
 		sp--;
 		for (i = strlen(unctrl(*sp)); i; i--)
 		    waddch(win, '\b');
 	    }
 	    continue;
 	}
 	else if (c == md_killchar())  /* process kill character */
 	{
 	    sp = buf;
 	    wmove(win, oy, ox);
 	    continue;
 	}
 	else if (sp == buf)
	{
 	    if (c == '-' && win == hw)	/* To move back a line in hw */
 		break;
 	    else if (c == '~')
 	    {
 		strcpy(buf, home);
 		waddstr(win, home);
 		sp += strlen(home);
 		continue;
 	    }
 	}
	*sp++ = (char) c;
 	waddstr(win, unctrl(c));
     }
     *sp = '\0';
     if (sp > buf)	/* only change option if something has been typed */
 	strucpy(opt, buf, strlen(buf));
     wmove(win, oy, ox);
     waddstr(win, opt);
     waddch(win, '\n');
     draw(win);
     if (win == cw)
 	mpos += (int)(sp - buf);
     if (c == '-')
 	return MINUS;
     else if (c == '\033' || c == '\007')
 	return QUIT;
     else
 	return NORM;
 }
 
 /*
  * The ability field is read-only
  */
 int
 get_abil(void *vp, WINDOW *win)
 {
	 int *abil = (int *) vp;
     register int oy, ox, ny, nx;
     register int op_bad;
 
     op_bad = TRUE;
     getyx(win, oy, ox);
     put_abil(abil, win);
     getyx(win, ny, nx);
     while(op_bad)	
     {
 	wmove(win, oy, ox);
 	draw(win);
 	switch (readchar(win))
 	{
 	    case '\n':
 	    case '\r':
 		op_bad = FALSE;
 		break;
 	    case '\033':
 	    case '\007':
 		return QUIT;
 	    case '-':
 		return MINUS;
 	    default:
 		mvwaddstr(win, ny, nx + 5, "(no change allowed)");
 	}
     }
     wmove(win, ny, nx + 5);
     wclrtoeol(win);
     wmove(win, ny, nx);
     waddch(win, '\n');
     return NORM;
 }
 
 
 /*
  * parse options from string, usually taken from the environment.
  * the string is a series of comma seperated values, with booleans
  * being stated as "name" (true) or "noname" (false), and strings
  * being "name=....", with the string being defined up to a comma
  * or the end of the entire option string.
  */
 void
 parse_opts(char *str)
 {
     register char *sp;
     register OPTION *op;
     register int len;
 
     while (*str)
     {
 	/*
 	 * Get option name
 	 */
 	for (sp = str; isalpha(*sp); sp++)
 	    continue;
 	len = (int)(sp - str);
 	/*
 	 * Look it up and deal with it
 	 */
 	for (op = optlist; op <= &optlist[NUM_OPTS-1]; op++)
 	    if (EQSTR(str, op->o_name, len))
 	    {
 		if (op->o_putfunc == put_bool)	/* if option is a boolean */
 		    *(int *)op->o_opt = TRUE;
 		else				/* string option */
 		{
 		    register char *start;
 		    char value[80];
 
 		    /*
 		     * Skip to start of string value
 		     */
 		    for (str = sp + 1; *str == '='; str++)
 			continue;
 		    if (*str == '~')
 		    {
 			strcpy((char *) value, home);
 			start = (char *) value + strlen(home);
 			while (*++str == '/')
 			    continue;
 		    }
 		    else
 			start = (char *) value;
 		    /*
 		     * Skip to end of string value
 		     */
 		    for (sp = str + 1; *sp && *sp != ','; sp++)
 			continue;
 		    strucpy(start, str, (int)(sp - str));
 
 		    /* Put the value into the option field */
 		    if (op->o_putfunc != put_abil) 
 			strcpy(op->o_opt, value);
 
 		    else if (*(int *)op->o_opt == -1) { /* Only init ability once */
 			register size_t len = strlen(value);
 
 			if (isupper(value[0])) value[0] = (char) tolower(value[0]);
 			if (EQSTR(value, "fighter", len))
 				*(int *)op->o_opt = C_FIGHTER;
 			else if (EQSTR(value, "magic", min(len, 5)))
 				*(int *)op->o_opt = C_MAGICIAN;
 			else if (EQSTR(value, "cleric", len))
 				*(int *)op->o_opt = C_CLERIC;
 			else if (EQSTR(value, "thief", len))
 				*(int *)op->o_opt = C_THIEF;
 		    }
 		}
 		break;
 	    }
 	    /*
 	     * check for "noname" for booleans
 	     */
 	    else if (op->o_putfunc == put_bool
 	      && EQSTR(str, "no", 2) && EQSTR(str + 2, op->o_name, len - 2))
 	    {
 		*(int *)op->o_opt = FALSE;
 		break;
 	    }
 
 	/*
 	 * skip to start of next option name
 	 */
 	while (*sp && !isalpha(*sp))
 	    sp++;
 	str = sp;
     }
 }
 
 /*
  * copy string using unctrl for things
  */
 void
 strucpy(char *s1, char *s2, size_t len)
 {
     const char *sp;
 
     while (len--)
     {
 	strcpy(s1, (sp = unctrl(*s2++)));
 	s1 += strlen(sp);
     }
     *s1 = '\0';
 }
