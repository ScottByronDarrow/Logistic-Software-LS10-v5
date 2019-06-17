/*========================================================================
| This routine prints a message to the screen for use by ALL processing  |
| streams.                                                               |
========================================================================*/
#include <signal.h>

#ifndef	FORE_GND
#define	FORE_GND	0
#endif
int		foregnd;
void	dsp_screen (char *,char *, char*);
void	sig_off (void);

void
dsp_screen (char *dsp_desc, char *cono, char *coname)
{
	int		desc_len;
	extern	int	_wide;
	
	if (foreground () || FORE_GND) 
	{
		strcpy (err_str,  dsp_desc);
		init_scr();
		clear();
		crsr_off();
		box(0,0,(_wide) ? 130 : 79,21);
		desc_len = (( ((_wide) ? 130 : 80) - (int) strlen(dsp_desc)) / 2);
		print_at (8, desc_len, "%s", ML (err_str));
		move(desc_len,9);
		line((int) strlen(dsp_desc) + 1);
		move(1,20); line((_wide) ? 128 : 78);
		print_at (21,1, "Company No : %s - %s", cono,coname);
		fflush(stdout);
	}
	else
		sig_off();

	return;
}

void
sig_off (void)
{
	signal (SIGHUP, SIG_IGN);
	signal (SIGINT, SIG_IGN);
	signal (SIGQUIT, SIG_IGN);
}
