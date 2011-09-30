/*
 * $Log:	lav.c,v $
 * Revision 1.1  84/11/10  14:45:41  mikec
 * Initial revision
 * 
 *
 */

/*
 *
 *	lav.c -- Print the load average.
 *		lav must be setuid root (or appropriote uid) that can read
 *		/dev/kmem.  On some systems, such as ours, permissions to
 *		kmem are turned off for security reasons.
 *
 */

#include 	<nlist.h>
#include 	<stdio.h>

struct	nlist nl[] = {
	{ "_avenrun" },
	{ "" },
};

int	kmem;
double	avenrun[3];

main()
{
	register int i;

	if ((kmem = open("/dev/kmem", 0)) < 0) {
		fprintf(stderr, "No kmem\n");
		exit(1);
	}

	nlist("/vmunix", nl);

	if (nl[0].n_type==0) {
		fprintf(stderr, "No namelist\n");
		exit(1);
	}

	/*
	 * Print 1, 5, and 15 minute load averages.
	 * (Found by looking in kernel for avenrun).
	 */

	lseek(kmem, (long)nl[0].n_value, 0);
	read(kmem, avenrun, sizeof(avenrun));
	for (i = 0; i < (sizeof(avenrun)/sizeof(avenrun[0])); i++) {
		printf("%.2f ", avenrun[i]);
	}
	printf("\n");
}
