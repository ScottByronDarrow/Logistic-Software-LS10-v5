/*
 *	Hex print program - reads nominated file (std input default)
 *	and writes characters out in hex and ascii (if printable) on
 *	the std output. Note buffers 1 line at a time due to need to print
 *	ascii characters at right hand margin.
 $Id: hex.c,v 5.1 2001/08/09 09:26:58 scott Exp $

 $Log: hex.c,v $
 Revision 5.1  2001/08/09 09:26:58  scott
 Updated to add FinishProgram () function

 Revision 5.0  2001/06/19 08:22:54  robert
 LS10-5.0 New Release as of 19 JUNE 2001

 Revision 4.0  2001/03/09 02:43:52  scott
 LS10-4.0 New Release as at 10th March 2001

 Revision 3.0  2000/10/10 12:24:16  gerry
 Revision No. 3 Start
 <after Rel-10102000>

 Revision 2.0  2000/07/15 09:15:18  gerry
 Forced Revision No Start 2.0 Rel-15072000

 Revision 1.3  1999/11/16 08:11:35  scott
 Update for warnings due to usage of -Wall flags.

 */

#define	CCMAIN
char	*PNAME = "$RCSfile: hex.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/hex/hex.c,v 5.1 2001/08/09 09:26:58 scott Exp $";

#include <pslscr.h>
#define  BUF	16

int	lcase = 0,
	mask = 0377,
	page_brk = 0,
	hexcnt = 0;

	char hex[17] = "0123456789ABCDEF",
         dispchar[17];

	long	bytecnt;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
int copy (char *s);
int hex_line (char *buffer, int len);

int
main (
 int                argc,
 char*              argv[])
{
	char	c;
	char	*cp;
	int	printed = 0;

	while (argc > 1)
	{
		switch (*argv[1])
		{
		case '-': /* option string */
			cp = argv[1];
			while ((c = *++cp))
			{
				switch(c)
				{
				case 'l':
					lcase = 1;
					break;
				case 'h':
					hexcnt = 1;
					break;
				case '7':
					mask = 0177;
					break;
				case 'p':
					page_brk = 1;
					break;
				default:
					break;
				}
			}
			if (lcase)
				strcpy (hex,"0123456789abcdef");
			break;

		default: /* other args */
			copy (argv[1]);
			printed++;
		}
		argc--;
		argv++;
	}
	if (printed == 0)
		copy ((char*)0);
    return (EXIT_SUCCESS);
}
	
int
copy (
 char*              s)
{
	char	buffer[BUF + 1];
		int	length;
		int	fd = 0;

	bytecnt = 0L;
	if (s)
		fd = open(s, 0);
	if (fd == -1)
	{
		fprintf(stderr,"hex: can't open file %s \n",s);
		return(1);
	}
	do
	{
		length = read(fd,buffer,BUF);
		if (length < 1)
			continue;
		hex_line (buffer,length);
		bytecnt += (long) length;
	} while (length > 0);

	fprintf(stdout,"\n\n");
	if (s)
		close(fd);

	return(0);
}	

int
hex_line (
 char*              buffer,
 int                len)
{
	char	c;
	int	a,i;

	if (hexcnt)
		fprintf(stdout,"%9.9lx:",bytecnt);
	else
		fprintf(stdout,"%09ld:",bytecnt);

	if ( page_brk != 0 && bytecnt % 512 == 0L)
		fprintf(stdout,"-");
	else
		fprintf(stdout," ");
	for (i = 0;i < len; i++)
	{
		c = buffer[i];
		a = buffer[i];
		a = a & mask;
		if (a < 32 || a > 126)
			c = '.';	 /* unprintable char. */

		fprintf(stdout,"%c%c ",hex[( a / 16)],hex[( a % 16)]);
		dispchar[i] = c;
	}
	dispchar[len] = '\0';
	if (len < BUF)
	{
		for (i = len; i < BUF; i++) 
			fprintf(stdout,"   ");
	}
	fprintf (stdout,"   %s\n",dispchar);
	return(0);	
}
