/*
    save.c  -  save and restore routines
   
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
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <io.h>
#endif
#include "rogue.h"

int sigdie = -1;

int
save_game(void)
{
    register int c;
    char buf[LINELEN];

    /*
     * get file name
     */
    mpos = 0;
    if (file_name[0] != '\0')
    {
	msg("Save file (%s)? ", file_name);
	do
	{
	    c = readchar(msgw) & 0177;
	    if (c == ESCAPE) return(0);
	} while (c != 'n' && c != 'N' && c != 'y' && c != 'Y');
	mpos = 0;
	if (c == 'y' || c == 'Y')
	{
	    msg("File name: %s", file_name);
	    goto gotfile;
	}
    }

    for(;;)
    {
	msg("File name: ");
	mpos = 0;
	buf[0] = '\0';
	if (get_str(buf, msgw) == QUIT)
	{
	    msg("");
	    return FALSE;
	}
	strcpy(file_name, buf);
gotfile:
    if (save_file(file_name) != 0)
		msg(strerror(errno));
	else
		break;
	}

    return TRUE;
}

/*
 * automatically save a file.  This is used if a HUP signal is
 * recieved
 */
void
auto_save(int sig)
{
    /*
     * Code added by Bruce Dautrich on 4/11/84 to
     * increase probability of saving game if two 
     * signals are sent to game. Requires define
     * in rogue.h of a variable called sigdie
     */

    if (sigdie == -1) {
	/*This is first signal*/
	sigdie=sig;
	signal(sig,SIG_IGN);
	goto got1sig;
    } else {
	/*This is second signal*/
	signal(sig,SIG_IGN);
    }

got1sig:
	save_file(file_name);
    exit(1);
}

/*
 * write the saved game on the file
 */
int
save_file(char *file_name)
{
    wmove(cw, LINES-1, 0);
    draw(cw);
    return (rs_save_file(file_name));
}

/*This is only until all have used this rogue version 1.2 */
int
restore(char *file, char **envp)
{
    extern char **environ;
    struct stat sbuf2;

    makesure();

    /*
     * Reset the effective uid & gid to the real ones.
     */
	md_normaluser();

    if (strcmp(file, "-r") == 0)
	file = file_name;

    /*
     * Set the new terminal and make sure we aren't going to a smaller screen.
     */

    initscr();

    cw = newwin(LINES, COLS, 0, 0);
    mw = newwin(LINES, COLS, 0, 0);
    hw = newwin(LINES, COLS, 0, 0);
    msgw = newwin(4, COLS, 0, 0);
    keypad(cw,1);
    keypad(msgw,1);

	mpos = 0;
    mvwprintw(cw, 0, 0, "%s: %s", file, ctime(&sbuf2.st_mtime));

    /*
     * defeat multiple restarting from the same place
     */
    if (!wizard) {
	stat(file, &sbuf2);
	if (sbuf2.st_nlink != 1) {
	    printf("Cannot restore from a linked file\n");
	    return FALSE;
	}
    }

	endwin();

    if (rs_restore_file(file) != 0)
    {
		if (strcmp(oversion, version) != 0)
		{
			fprintf(stderr, "Save Game Version: %s\nReal Game Version: %s\n", oversion, version);
			fprintf(stderr, "Sorry, saved game is out of date.\n");
		}

		printf("\nCannot restore file\n");
        return(FALSE);
    }

    if (COLS < oldcol || LINES < oldline) {
        fprintf(stderr, "Cannot restart the game on a smaller screen.\n");
        return FALSE;
    }

	if (strcmp(oversion, version) != 0)
    {
		fprintf(stderr, "Save Game Version: %s\nReal Game Version: %s\n", oversion, version);
		fprintf(stderr, "Sorry, saved game is out of date.\n");
		return FALSE;
    }

	wrefresh(cw);
    environ = envp;
    strcpy(file_name, file);
    setup();
    clearok(curscr, TRUE);
    touchwin(cw);
    md_srand(md_getpid());
    playit();
    /*NOTREACHED*/
	return(0);
}

/*****************************************************************
 *
 *           encwrite, encread: encoded read/write routines
 *
 * The encwrite/encread routines implement a synchronous stream Vernam
 * cipher using a linear congruential random number generator to
 * simulate a one-time pad.  The random seed is encoded and stored in
 * the file using data diffusion (putword,getword).
 *
 * Usage:
 *	encwrite (start, size, outf);
 *	char *start; int size; FILE *outf;
 *
 *	encread (start, size, infd);
 *	char *start; int size; int infd;
 *
 * HISTORY
 * 03-Mar-85  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Modified for UltraRogue.  Allowed multiple encwrites to a file by
 *	removing lseek calls and computing checksum while reading.
 *
 * 20-Dec-82  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Created as a replacement for the encwrite/encread in
 *	Rogue 5.2, which are very easily broken.
 *
 *****************************************************************/

# include "stdio.h"

/* Constants for key generation */
# define OFFSET		667818
# define MODULUS	894871
# define MULT		2399
# define ENCHAR		(((seed= ((seed*MULT+OFFSET)%MODULUS)) >> 10) & 0xff)

/* Constants for checksumming */
# define INITCK		1232531
# define CKMOD		6506347

struct wb { int w1, w2, w3, w4; };

/*****************************************************************
 * Encwrite: write a buffer to a file using data encryption
 *****************************************************************/
size_t
encwrite (void *vstart, size_t size, FILE *outf)
{ register int cksum = INITCK, seed, ch;
  size_t savsiz;
  char *stsav, *start = vstart;

  if ((outf == NULL) || (size == 0))
	  return(0);

  srand ((int)time (0) + md_getpid () + md_getuid ());	/* Build a random seed */
  seed = (md_rand () & 0x7fff);
  
  putword (seed, outf);				/* Write the random seed */
  putword ((int)size, outf);				/* Write the file length */

# ifdef DEBUG
  fprintf (stderr, "Encwrite: size %ld, seed %ld.\n", size, seed);
# endif

  /* Now checksum, encrypt, and write out the buffer. */
#if 0
  /* This is the old way of doing it. */
  while (size--)
  { ch = *start++ & 0xff;			/* Get the next char */
    cksum = ((cksum << 8) + ch) % CKMOD;	/* Checksum clear text */
    putc (ch ^ ENCHAR, outf);			/* Write ciphertext */
  } */
#endif
  /* And here's the new (and hopefully faster) way. */
  
  savsiz = size;
  stsav = start;
  while (size--)
  { ch = *start & 0xff;
    cksum = ((cksum << 8) + ch) % CKMOD;
    *start++ = (char)(ch ^ ENCHAR);
  }
  savsiz = fwrite(stsav, 1, savsiz, outf);

  putword (cksum, outf);			/* Write out the checksum */
  fflush (outf);				/* Flush the output */

# ifdef DEBUG
  fprintf (stderr, "Checksum is %ld.\n", cksum);
# endif
  return(savsiz);
}

/*****************************************************************
 * Encread: read a block of encrypted text from a file descriptor
 *****************************************************************/
size_t
encread (void *vstart, size_t size, FILE *infd)
{ register int cksum = INITCK, seed, ch;
  size_t length, stored, cnt;
  char *start = vstart;

  if ((infd == NULL) || (size == 0))
	  return(0);

  seed = getword (infd); 
  stored = getword (infd);  

# ifdef DEBUG
  fprintf (stderr, "Encread: size %ld, seed %ld, stored %ld.\n", 
	   size, seed, stored);
# endif
  
  if ((length = fread (start, 1, min (size, stored), infd)) > 0)
  { for (cnt = length; cnt--; ) 
    { ch = (*start++ ^= ENCHAR) & 0xff;
      cksum = ((cksum << 8) + ch) % CKMOD;
    }
  
    if ((length == stored) && (getword (infd) != cksum))
    {
# ifdef DEBUG
      fprintf (stderr, "Computed checksum %ld is wrong.\n", cksum);
# else
      fprintf (stderr, "Sorry, file has been touched.\n");
# endif
      while (length--) *--start = '\0';		/* Zero the buffer */
    }
  }
  
  return (length);
}

/*****************************************************************
 * putword: Write out an encoded int word
 *****************************************************************/

size_t
putword (int word, FILE *file)
{ struct wb w;

  w.w1 = rand ();  
  w.w2 = rand ();  
  w.w3 = rand ();  
  w.w4 = w.w1 ^ w.w2 ^ w.w3 ^ word;
  
  return fwrite ((char *) &w, sizeof (struct wb), 1, file);
}

/*****************************************************************
 * getword: Read in an encoded int word
 *****************************************************************/

int
getword (FILE *fd)
{ struct wb w;
  
  if (fread ((char *) &w, 1, sizeof (struct wb),fd) == sizeof (struct wb))
    return (w.w1 ^ w.w2 ^ w.w3 ^ w.w4);
  else
    return (0);
}
