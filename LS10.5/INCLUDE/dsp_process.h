#ifndef	MOD
#define	MOD	100
#endif
#ifndef	FORE_GND
#define	FORE_GND	0
#endif

void	finish_prog(void);
void	dsp_process (char *, char *);

/*=========================================================================
| This routine is used to print a single character on the screen to show  |
| the user that the processing stream is doing something.                 |
| Program needs to be passed the following.                               |
=========================================================================*/
void
dsp_process (char *pro_desc, char *pro_field)
{
	static	int	proc_numb = 0;
	extern	int	_wide;
	if (foreground() || FORE_GND) 
	{
		if ((proc_numb++ == 0 || proc_numb % MOD == 0) && strlen(pro_field) > 1)
		{
			box(19,11,(_wide) ? 65 : 44,1);
			print_at (12,21,"(%s) %-16.16s",pro_desc, pro_field);
			fflush(stdout);
		}
	}
}

void
finish_prog (void)
{
	if (foreground())
	{
		clear();
		snorm();
		rset_tty();
		crsr_on();
	}
}
