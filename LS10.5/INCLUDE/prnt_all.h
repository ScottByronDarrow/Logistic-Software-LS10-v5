#ifndef PRNT_ALL_H
#define PRNT_ALL_H

void prnt_all (int, long, char *, char *);

void
prnt_all (
 int	lpno,
 long	hhco_hash,
 char *	mode,
 char *	type_flag)
{
	int	c	=	0;
	static int	print_all = 0;
	char	*sptr;

	if (!print_all)
	{
		sptr = chk_env("LINE_UP");
		if (sptr != (char *)0)
		{
			c = atoi(sptr);
			print_all = !(c);
		}

		fprintf(pout,"%d\n",lpno);
		fprintf(pout,"%s\n",mode);
		fflush(pout);

		if (print_all)
		{
			fprintf(pout,"0\n");
			fflush(pout);
			return;
		}
	}

	do
	{
		fprintf(pout,"%ld\n",hhco_hash);
		fflush(pout);
		if (!print_all)
		{
			switch (type_flag[0])
			{
			case	'I':
			case	'i':
				strcpy(err_str,"Reprint Invoice (for lineup) <Y/N> ? ");
				break;

			case	'C':
			case	'c':
				strcpy(err_str,"Reprint Credit Note (for lineup) <Y/N> ? ");
				break;

			case	'P':
			case	'p':
				strcpy(err_str,"Reprint Packing Slip (for lineup) <Y/N> ? ");
				break;

			case	'R':
			case	'r':
				strcpy(err_str,"Reprint Remittance (for lineup) <Y/N> ? ");
				break;

			case	'S':
			case	's':
				strcpy(err_str,"Reprint Statement (for lineup) <Y/N> ? ");
				break;

			case	'X':
			case	'x':
				strcpy(err_str,"Reprint Inspection Slip (for lineup) <Y/N> ? ");
				break;

			default:
				return;
			}

			sleep(2);
			clear();
			c = prmptmsg(err_str,"YyNn",26,1);
			if (mode[0] == 'M' && (c == 'N' || c == 'n'))
			{
				fprintf(pout,"0\n");
				fflush(pout);
			}
			clear();
		}
	} while (!print_all && (c == 'Y' || c == 'y'));
	print_all = 1;
}
#endif /*PRN_ALL_H*/
