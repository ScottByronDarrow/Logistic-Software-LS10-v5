/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( warning.c      )                                 |
|  Program Desc  : ( Warn User If Option Selected is The One They )   |
|                  ( Want.                                        )   |
|---------------------------------------------------------------------|
|  Access files  :  comm,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written : (10/05/86)       | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
|  Date Modified : (10/05/86)      | Modified  by : Scott B. Darrow.  |
|  Date Modified : (23/05/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (13/09/97)      | Modified  by : Leah Manibog.	  |
|  Date Modified : (03/09/1999)    | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  Comments                                                           |
|  (23/05/92)    : Updated for compatability with Risc/Os 5.0         |
|                : Also, generally tidied up.                         |
|  (13/09/97)    : Updated for Multilingual Conversion.               |
|  (03/09/1999)  : Ported to ANSI standards.                          |
|                                                                     |
| $Log: warning.c,v $
| Revision 5.2  2002/05/02 03:17:53  scott
| Updated to use prompt message.
|
| Revision 5.1  2001/08/09 05:13:58  scott
| Updated to use FinishProgram ();
|
| Revision 5.0  2001/06/19 08:09:13  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:29  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:29  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:39  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  2000/03/06 21:21:43  cam
| Changes for GVision compatibility.  When compiled under GVision, use prmptmsg
| instead of getkey.
|
| Revision 1.7  1999/12/06 01:47:29  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.6  1999/11/11 09:15:48  scott
| Updated for missing translation
|
| Revision 1.5  1999/09/17 07:27:20  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.4  1999/09/16 04:11:43  scott
| Updated from Ansi Project
|
| Revision 1.3  1999/06/15 02:36:58  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: warning.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/warning/warning.c,v 5.2 2002/05/02 03:17:53 scott Exp $";

#define	NO_SCRGEN
#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_menu_mess.h>

#define	LF	'\012'

	/*=====================================
	| File comm	{System Common file}. |
	=====================================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_cc_short"}
	};

	int comm_no_fields = 7;

	struct
	{
		int		termno;
		char	tco_no [3];
		char	tco_short [16];
		char	test_no [3];
		char	test_short [16];
		char	tcc_no [3];
		char	tcc_short [10];
	} comm_rec;

	char	*comm	= "comm",
			*data	= "data";

/*============================
| Local function prototypes  |
============================*/
void	ReadComm	(void);
int		PaintScreen	(char *);
void	PrintWords	(char *, int);
int		GetAnswer	(void);


/*==========================
| Main processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	if (argc != 2)
	{
		print_at (0, 0, mlMenuMess704, argv [0]);
		return (EXIT_FAILURE);
	}

	init_scr ();
	set_tty ();

	ReadComm ();

	if (PaintScreen (argv[1]))
		return (EXIT_SUCCESS);
	else
		return (EXIT_FAILURE);
}

/*==================
| Read Common File |
==================*/
void
ReadComm (
 void)
{
	abc_dbopen (data);

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	abc_dbclose (data);
}

/*===============
| Paint Screen. |
===============*/
int
PaintScreen (
 char *	comment)
{
	clear ();
	crsr_off ();
	box (0, 0, 79, 21);
	line_at (20,1,78);

	print_at (21,2, ML(mlStdMess038), comm_rec.tco_no, comm_rec.tco_short);
	print_at (21,25, ML(mlStdMess039), comm_rec.test_no, comm_rec.test_short);
	print_at (21,35, ML(mlStdMess099), comm_rec.tcc_no, comm_rec.tcc_short);
	PrintWords (ML(mlMenuMess236), 7);

	PrintWords (comment, 13);
	crsr_on ();

	return (GetAnswer ());
}

void
PrintWords (
 char *	comment,
 int	l_no)
{
	int	ll,
		l;

	strcpy (err_str, comment);
	ll = strlen (comment);
	l = (80 - ll) / 2;
	box (l - 2, l_no - 1, ll + 4, 1);
	rv_pr (ML (err_str), l, l_no, 1);
}

int
GetAnswer (void)
{
	int	key;

	key = prmptmsg (ML("Enter 'Y' to continue, or 'N' to abort"), "YyNn", 20, 18);
	if (key == 'Y' || key == 'y')
	{
		clear ();
		print_at (0,0, ML(mlStdMess035));
		fflush (stdout);
		rset_tty ();
		return (TRUE);
	}
	else
	{
		clear ();
		print_at (0,0, ML(mlStdMess035));
		fflush (stdout);
		rset_tty ();
		return (FALSE);
	}
}
