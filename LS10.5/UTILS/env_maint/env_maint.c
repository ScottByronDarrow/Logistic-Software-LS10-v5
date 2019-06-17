/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: env_maint.c,v 5.3 2001/08/09 09:26:53 scott Exp $
|  Program Name  : (env_maint.c)
|  Program Desc  : (Environment Variable Maintenance)
|---------------------------------------------------------------------|
|  Date Written  : (12/05/88)      | Author       : Roger Gibbison    |
|---------------------------------------------------------------------|
| $Log: env_maint.c,v $
| Revision 5.3  2001/08/09 09:26:53  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:58:39  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:20:06  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: env_maint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/env_maint/env_maint.c,v 5.3 2001/08/09 09:26:53 scott Exp $";

#include 	<pslscr.h>
#include	<license2.h>
#include	<pinn_env.h>
#include	<ml_utils_mess.h>

struct	DES_REC	des_rec;
struct	LIC_REC	lic_rec;

struct	{
	char	dummy [11];
} local_rec;

	extern	int		envMaintOption;

	PinnEnv	envRec;

char	filename [100];

static	struct	var	vars [] =
{
	{1, LIN, "env_name",	 7, 25, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Variable Name", " ",
		 NE, NO,  JUSTLEFT, "", "", envRec.env_name},
	{1, LIN, "env_value",	 8, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Variable Value", " ",
		YES, NO,  JUSTLEFT, "", "", envRec.env_value},
	{1, LIN, "env_desc",	 9, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Variable Description", " ",
		YES, NO,  JUSTLEFT, "", "", envRec.env_desc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*===========================
| Local function prototypes |
===========================*/
int		spec_valid	 (int);
int		ReadEnviron	 (char *);
void	SrchEnviron	 (char *);
void	Update		 (void);
int		heading		 (int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char	*xptr = getenv ("PSL_ENV_NAME");
	char	*sptr = getenv ("PROG_PATH");

	sprintf (filename,"%s/BIN/LOGISTIC", (sptr != (char *)0) ? sptr : "/usr/ver9.10");
	if (xptr)
		strcpy (filename, xptr);

	envMaintOption	=	TRUE;

	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();

	ser_msg (lc_check (&des_rec, &lic_rec), &lic_rec, TRUE);

	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/
	swide ();

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (prog_exit == 0)
	{
		/*----------------------------------
		| Reset Control Flags              |
		----------------------------------*/
		entry_exit	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		heading (1);
		entry (1);

		if (restart || prog_exit)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		Update ();
	}
	clear ();
	rset_tty ();
	crsr_on ();
	snorm ();
	return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("env_name"))
	{
		if (SRCH_KEY)
		{
			SrchEnviron (temp_str);
			return (EXIT_SUCCESS);
		}

		if (ReadEnviron (temp_str))
			entry_exit = 1;
		else
			sprintf (envRec.env_name,"%-15.15s",temp_str);
			
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
ReadEnviron (
 char *	e_name)
{
	int	fd = open_env ();
	
	sprintf (err_str,"%-15.15s",e_name);

	cc = RF_READ (fd, (char *) &envRec);
	while (!cc && strcmp (err_str,envRec.env_name))
		cc = RF_READ (fd, (char *) &envRec);

	close_env (fd);
	return (!strcmp (err_str,envRec.env_name));
}

void
SrchEnviron (
 char *	key_val)
{
	int	fd = open_env ();

	work_open ();
	save_rec ("#Variable Name","#Variable Value");
	cc = RF_READ (fd, (char *) &envRec);
	while (!cc)
	{
		if (!strncmp (envRec.env_name,key_val,strlen (key_val)))
		{
			cc = save_rec (envRec.env_name,envRec.env_value);
			if (cc)
				break;
		}
		cc = RF_READ (fd, (char *) &envRec);
	}
	close_env (fd);
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	ReadEnviron (temp_str);
}

void
Update (
 void)
{
	put_env (envRec.env_name,envRec.env_value,envRec.env_desc);
}

/*=========================
| Display Screen Heading  |
=========================*/
int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);

	swide ();
	clear ();
	rv_pr (ML (mlUtilsMess027),49,0,1);

	line_at (1,0,132);
	print_at (4,10,ML (mlUtilsMess004),filename);

	box (0,6,132,3);
	line_at (20,0,132);
	line_at (22,0,132);
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}
