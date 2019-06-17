#define CCMAIN
char	*PNAME = "$RCSfile: caret.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/caret/caret.c,v 5.1 2001/08/09 09:26:46 scott Exp $";

#include	<pslscr.h>
#include	<errno.h>
#include	<ml_utils_mess.h>
#include	<stdio.h>

#define	TRUE	1
#define	FALSE	0
FILE	*finput;
FILE	*fout;

#define	PLUS	( c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'H' )
#define	LINE	( c == 'G' || c == 'I' || c == 'J' )
#define	PIPE   	( c == 'K' || c == 'L' || c == 'E' || c == 'F')
#define	MISC	( c == '1' || c == '6' )

/*===========================
| Local function prototypes |
===========================*/
void	process	(int print_line);


int
main (
 int	argc,
 char *	argv [])
{
	int	i;
	int	line_no;

	if (argc < 2)
	{
		/*printf("Usage : %s [-n] <filenames>\007\n",argv[1]);*/
		print_at(0,0,ML(mlUtilsMess713),argv[1]);
		return (EXIT_FAILURE);
	}

	line_no = (!strcmp(argv[1],"-n"));

	for (i = (line_no) ? 2 : 1;i < argc;i++)
	{
		if ((finput = fopen(argv[i],"r")) == NULL)
		{
			sprintf(err_str,"Error in %s during (FOPEN)",argv[i]);
			sys_err(err_str,errno,"caret.c");
		}

		if ((fout = fopen("caret.out","w")) == NULL)
			sys_err("Error in caret.out during (FOPEN)",errno,"caret.c");
		
		fprintf(fout,"FILENAME : %s\n\n",argv[i]);
		
		/*print_at(0,0,"Caret : %s\n\n",argv[i]);*/
		print_at(0,0,ML(mlUtilsMess066),argv[i]);
		fflush(stdout);
		process(line_no);
		fclose(finput);
		fflush(fout);
		fclose(fout);
	}

	return (EXIT_SUCCESS);
}

void
process (
 int	print_line)
{
	int	c;
	int	lastc = 0;
	int	line_no = 1;
	int	inside_twin = FALSE;

	while ((c = getc(finput)) != EOF)
	{
		if ( c == '^' && lastc == '^' )
			inside_twin = ( inside_twin == TRUE ) ? FALSE : TRUE;
		
		if (lastc == '\n')
		{
			inside_twin = FALSE;
			if (print_line)
				fprintf(fout,"\n%03d  ",line_no++);
			else
				putc('\n',fout);

			if (c != '^')
				putc(c,fout);
		}
		else
		{
			if (line_no == 1 && print_line)
				fprintf(fout,"%03d  ",line_no++);

			if (c != '^' && c != '\n')
			{
				if ( inside_twin || lastc == '^')
				{
					if ( PLUS )
						putc('+',fout);

					else if ( LINE )
						putc('-',fout);

					else if ( PIPE )
						putc('|',fout);

					else if ( !MISC )
						putc(c,fout);
				}
				else
					putc(c,fout);
			}
		}
		lastc = c;
	}
}
