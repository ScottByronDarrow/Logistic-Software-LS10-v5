/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: lrp_envinp.c,v 5.2 2001/08/09 09:29:44 scott Exp $
|  Program Name  : (lrp_envinp.c)
|  Program Desc  : (LRP - Environment Variable Maintenance)
|---------------------------------------------------------------------|
| $Log: lrp_envinp.c,v $
| Revision 5.2  2001/08/09 09:29:44  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:26  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:18  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:25  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/01/30 05:32:01  scott
| Update to add app.schema
|
| Revision 3.0  2000/10/10 12:15:25  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:58:36  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.13  1999/12/06 01:34:16  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/17 06:40:10  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.11  1999/11/12 08:00:20  scott
| Updated due to changes on erratic. LRP_ERRATIC has changed.
|
| Revision 1.10  1999/11/03 00:22:17  scott
| Updated to change environment FF_ to LRP_
|
| Revision 1.9  1999/10/27 07:32:57  scott
| Updated for -Wall warnings + modifications for ASL on percentage error.
|
| Revision 1.8  1999/10/13 21:32:55  scott
| General cleanup after ansi project
|
| Revision 1.7  1999/09/17 07:26:35  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.6  1999/09/16 09:20:39  scott
| Updated from Ansi Project
|
| Revision 1.5  1999/06/15 07:27:02  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lrp_envinp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_envinp/lrp_envinp.c,v 5.2 2001/08/09 09:29:44 scott Exp $";

#include 	<pslscr.h>
#include	<license2.h>
#include	<pinn_env.h>
#include	<ml_utils_mess.h>

struct	DES_REC	des_rec;
struct	LIC_REC	lic_rec;


PinnEnv	env_rec;

char	filename[100];
char	EnvComments[11][71];

struct	{
	char	dummy[11];
	float	lrp_dflt_review;
	char	lrp_dmnd_neg[11];
	double	lrp_erratic;
	int		lrp_per_error;
	int		lrp_show_all_sup;
	char	lrp_methods[5];
	char	lrp_priority[5];
	float	lrp_cost_pvar;
	int		po_max_lines;
	int		sk_tr_max_lines;
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "LRP_DFLT_REVIEW",	 3, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0.00", "Default Review Period.                             : ", "Input the default review period for LRP system.",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.lrp_dflt_review},
	{1, LIN, "LRP_DMND_NEG",	 4, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Do you allow Negative value for Future demand?.    : ", "Input Y (es) or N (o).",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.lrp_dmnd_neg},
	{1, LIN, "LRP_ERRATIC",	 5, 2, DOUBLETYPE,
		"N.N", "          ",
		" ", "1", "Erratic demand Threshold point ?.                  : ", "Should be a value from .5 to 1 (See LRP manuals for more information.)",
		YES, NO,  JUSTLEFT, ".5", "1", (char *)&local_rec.lrp_erratic},
	{1, LIN, "LRP_METHODS",	 6, 2, CHARTYPE,
		"UUUU", "          ",
		" ", "ABCD", "Enter valid Methods to be used A-D?.               : ", "Should be set to ABCD",
		YES, NO,  JUSTLEFT, "ABCD", "", local_rec.lrp_methods},
	{1, LIN, "LRP_PRIORITY",	 7, 2, CHARTYPE,
		"U", "          ",
		" ", "1", "Is Priority based on supplier or price.            : ", "0 = Based on supplier FOB cost, 1 = based on pre-defined priority.",
		YES, NO,  JUSTLEFT, "10", "", local_rec.lrp_priority},
	{1, LIN, "LRP_COST_PVAR",	 8, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "10.00", "Percentage variance from cost                      : ", "% variance variance from cost between cheaper supplier and P1 supplier.",
		YES, NO,  JUSTLEFT, "0", "100", (char *)&local_rec.lrp_cost_pvar},
	{1, LIN, "LRP_SHOW_ALL_SU",	 9, 2, INTTYPE,
		"U", "          ",
		" ", "0", "Show all suppliers on reorder screen.              : ", "1 = Yes, show all suppliers, 0 = No, show only mu suppliers.",
		YES, NO,  JUSTLEFT, "0", "1", (char *)&local_rec.lrp_show_all_sup},
	{1, LIN, "PO_MAX_LINES",	 10, 2, INTTYPE,
		"NNNN", "          ",
		"1", "1000", "Maximum number of purchase order lines per order   : ", "Input from 1-9999.",
		YES, NO,  JUSTLEFT, "1", "9999", (char *)&local_rec.po_max_lines},
	{1, LIN, "SK_TR_MAX_LINES",	 11, 2, INTTYPE,
		"NNNN", "          ",
		" ", "1000", "Maximum number of transfer order lines per order   : ", "Input from 1-9999.",
		YES, NO,  JUSTLEFT, "1", "9999", (char *)&local_rec.sk_tr_max_lines},
	{1, LIN, "LRP_PER_ERROR",	 12, 2, INTTYPE,
		"NNN", "          ",
		" ", "100",  "The % error allowed before avge. last 3 mths used  : ", "Input from 20-100.",
		YES, NO,  JUSTLEFT, "20", "100", (char *)&local_rec.lrp_per_error},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

extern	int		TruePosition;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	Default 			 (void);
int 	spec_valid 			 (int);
int 	ReadEnvironment 	 (char *);
void 	Update 				 (void);
int 	heading 			 (int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int    argc,
 char*  argv[])
{
	char	*xptr = getenv ("PSL_ENV_NAME");
	char	*sptr = getenv ("PROG_PATH");

	TruePosition	=	TRUE;

	sprintf (filename,"%s/BIN/LOGISTIC",(sptr != (char *)0) ? sptr : "/usr/LS10.5");
	if (xptr)
		strcpy (filename, xptr);

	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (prog_exit == 0)
	{
		/*----------------------------------
		| Reset Control Flags              |
		----------------------------------*/
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);

		Default ();

		heading (1);
		scn_display (1);
		edit (1);

		if (!restart)
			Update ();

		prog_exit	=	TRUE;
	}
	clear ();
	rset_tty ();
	crsr_on ();
	snorm ();
	return (EXIT_SUCCESS);
}

void
Default (void)
{
	int	field;

	for (field = label ("LRP_DFLT_REVIEW"); 
		 field <= label ("LRP_PER_ERROR"); field++)
	{
		if (!ReadEnvironment (FIELD.label))
		{
			file_err (1, FIELD.label, "GETENV");
		}
		strcpy (EnvComments[field], env_rec.env_desc);

		switch (field)
		{
			case	0:
				local_rec.lrp_dflt_review	=	atof (env_rec.env_value);
				break;
			case	1:
				if (atoi (env_rec.env_value))
					strcpy (local_rec.lrp_dmnd_neg, "Y");
				else
					strcpy (local_rec.lrp_dmnd_neg, "N");
				break;

			case	2:
				local_rec.lrp_erratic	=	atof (env_rec.env_value);
				break;

			case	3:
				sprintf (local_rec.lrp_methods, "%-4.4s", env_rec.env_value);
				break;

			case	4:
				sprintf (local_rec.lrp_priority, "%-1.1s", env_rec.env_value);
				break;

			case	5:
				local_rec.lrp_cost_pvar	=	atof (env_rec.env_value);
				break;

			case	6:
				local_rec.lrp_show_all_sup	=	atoi (env_rec.env_value);
				break;

			case	7:
				local_rec.po_max_lines	=	atoi (env_rec.env_value);
				break;

			case	8:
				local_rec.sk_tr_max_lines	=	atoi (env_rec.env_value);
				break;

			case	9:
				local_rec.lrp_per_error	=	atoi (env_rec.env_value);
				break;
		}
	}
}

int
spec_valid (
 int    field)
{
	return (EXIT_SUCCESS);
}

int
ReadEnvironment (
 char*  e_name)
{
	int	fd = open_env ();
	
	sprintf (err_str,"%-15.15s",e_name);

	cc = RF_READ (fd, (char *) &env_rec);
	while (!cc && strcmp (err_str,env_rec.env_name))
		cc = RF_READ (fd, (char *) &env_rec);

	close_env (fd);
	return (!strcmp (err_str,env_rec.env_name));
}

void
Update (void)
{
	int	field;

	for (field = label ("LRP_DFLT_REVIEW"); 
		 field <= label ("LRP_PER_ERROR"); field++)
	{
		if (!ReadEnvironment (FIELD.label))
			continue;

		strcpy (env_rec.env_desc, EnvComments[field]);

		switch (field)
		{
			case	0:
				sprintf (env_rec.env_value, "%4.2f", local_rec.lrp_dflt_review);
				break;
			case	1:
				if (local_rec.lrp_dmnd_neg[0] == 'Y')
					strcpy (env_rec.env_value, "1");
				else
					strcpy (env_rec.env_value, "0");
				break;

			case	2:
				sprintf (env_rec.env_value, "%2.1f", local_rec.lrp_erratic);
				break;

			case	3:
				sprintf (env_rec.env_value, "%-4.4s", local_rec.lrp_methods);
				break;

			case	4:
				sprintf (env_rec.env_value, "%-1.1s", local_rec.lrp_priority);
				break;

			case	5:
				sprintf (env_rec.env_value, "%5.2f", local_rec.lrp_cost_pvar);
				break;

			case	6:
				sprintf (env_rec.env_value, "%d", local_rec.lrp_show_all_sup);
				break;

			case	7:
				sprintf (env_rec.env_value, "%d", local_rec.po_max_lines);
				break;

			case	8:
				sprintf (env_rec.env_value, "%d", local_rec.sk_tr_max_lines);
				break;

			case	9:
				sprintf (env_rec.env_value, "%d", local_rec.lrp_per_error);
				break;
		}
		put_env (env_rec.env_name,env_rec.env_value,env_rec.env_desc);
	}
}

/*=========================
| Display Screen Heading  |
=========================*/
int
heading (
 int    scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		rv_pr (ML ("LRP - Environment Setup Maintenance"),20,0,1);

		move (0,1);
		line (80);
		box (0,2,80,10);
		move (0,20);
		line (80);
		move (0,22);
		line (80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
