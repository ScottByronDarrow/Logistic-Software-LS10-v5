/*=====================================================================
|  Copyright (C) 1996 - 2001 Logistic Software Limited   .            |
|=====================================================================|
| $Id: del_quote.c,v 5.2 2001/08/09 08:44:37 scott Exp $
|  Program Name  : (del_quote.c)
|  Program Desc  : (Quotaion Inquiry Display)
|---------------------------------------------------------------------|
|  Author        : Elena Cuaresma  | Date Written  : 25/10/1995       |
|---------------------------------------------------------------------|
| $Log: del_quote.c,v $
| Revision 5.2  2001/08/09 08:44:37  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:38:15  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:42  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/05/02 08:57:11  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to perform routine maintenance to ensure standards are maintained.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: del_quote.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/QT/qt_del_quote/del_quote.c,v 5.2 2001/08/09 08:44:37 scott Exp $";

#define	X_OFF	20
#define	Y_OFF	3
#include	<pslscr.h>
#include	<get_lpno.h>
#include 	<qt_status.h>
#include 	<ml_std_mess.h>
#include 	<ml_qt_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct qtlnRecord	qtln_rec;
struct qtlpRecord	qtlp_rec;
struct qthrRecord	qthr_rec;

	char	*data = "data";

	char	branchNo [3];
	int		envVarDbCo 		= 0,
			envVarDbFind	= 0;

#include	<time.h>

/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	dummy [11];
	char	del_cancelled [2];
	char	del_onhold [2];
	char	del_won [2];
	char	del_lost [2];
	char	del_no_ord [2];
	char	del_convert [2];
	long	del_date;
	int	lpno;
} local_rec;


static struct	var vars [] ={
	{1, LIN, "dt_effective",	4, 35, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Delete Inactive Quotations from  :", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.del_date},

	{1, LIN, "onhold",	8, 35, CHARTYPE,
		"U", "          ",
		" ", "N", "45   On Hold            : ", "Enter Y(es) or N(o) ",
		NA, NO,  JUSTLEFT, "YN", "", local_rec.del_onhold},

	{1, LIN, "cancelled",	10, 35, CHARTYPE,
		"U", "          ",
		" ", "N", "80   Cancelled          : ", "Enter Y(es) or N(o) ",
		NA, NO,  JUSTLEFT, "YN", "", local_rec.del_cancelled},
	{1, LIN, "won",	12, 35, CHARTYPE,
		"U", "          ",
		" ", "N", "90   Won                : ", "Enter Y(es) or N(o) ",
		NA, NO,  JUSTLEFT, "YN", "", local_rec.del_won},
	{1, LIN, "lost",	14, 35, CHARTYPE,
		"U", "          ",
		" ", "N", "91   Lost               : ", "Enter Y(es) or N(o) ",
		NA, NO,  JUSTLEFT, "YN", "", local_rec.del_lost},
	{1, LIN, "no_order",	16, 35, CHARTYPE,
		"U", "          ",
		" ", "N", "92   No Order to Place  : ", "Enter Y(es) or N(o) ",
		NA, NO,  JUSTLEFT, "YN", "", local_rec.del_no_ord},
	{1, LIN, "convert",	18, 35, CHARTYPE,
		"U", "          ",
		" ", "N", "95   Converted to Order : ", "Enter Y(es) or N(o) ",
		NA, NO,  JUSTLEFT, "YN", "", local_rec.del_convert},

	{0,LIN,"",0,0,INTTYPE,
		"A","          ",
		" ","", "dummy"," ",
		YES,NO, JUSTRIGHT," "," ",local_rec.dummy}
};

/*============================
| Function prototype         |
============================*/
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		 	(void);
int		spec_valid		(int);
int		Process			(void);
int		DeleteLine 	 	(void);
int		heading			(int);
void	FlashMess		(int, char *, int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{

	SETUP_SCR (vars);
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();          /*  get into raw mode		*/
	clear ();
	set_masks ();		/*  setup print using masks	*/
	init_vars (1);		/*  set default values		*/

	envVarDbCo  	= atoi (get_env ("DB_CO"));
	envVarDbFind	= atoi (get_env ("DB_FIND"));
	strcpy (branchNo, (!envVarDbCo) ? " 0" : comm_rec.est_no);

	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	
	while (prog_exit == 0)
	{
		search_ok 	= TRUE;
		entry_exit 	= TRUE;
		restart 	= FALSE;
		prog_exit 	= FALSE; 
		init_vars (1); 
		heading (1);
		entry (1);
		while (prog_exit == 0 && restart == 0)
		{
			if (entry_exit)
			{
				FLD ("onhold") 		= NO;
				FLD ("cancelled") 	= NO;
				FLD ("won") 		= NO;
				FLD ("lost") 		= NO;
				FLD ("no_order") 	= NO;
				FLD ("convert") 	= NO;
				heading (1);
				scn_display (1);
				edit (1);
				if (!restart)
					Process ();

				restart = 1;
			}
		}
	} 
	shutdown_prog ();   
	return (EXIT_SUCCESS);
}

/*========================	
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB (void)
{
	abc_dbopen (data);

	open_rec (qthr, qthr_list, QTHR_NO_FIELDS, "qthr_id_no2");
	open_rec (qtln, qtln_list, QTLN_NO_FIELDS, "qtln_id_no");
	open_rec (qtlp, qtlp_list, QTLP_NO_FIELDS, "qtlp_id_no");
}

void
CloseDB (void)
{
	abc_fclose (qthr);
	abc_dbclose (data);
}

int
spec_valid (
 int	field)
{
	if (LCHECK ("dt_effective"))
	{
		strcpy (local_rec.del_onhold, "N");
		strcpy (local_rec.del_cancelled, "N");
		strcpy (local_rec.del_won, "N");
		strcpy (local_rec.del_lost, "N");
		strcpy (local_rec.del_no_ord, "N");
		strcpy (local_rec.del_convert, "N");
		DSP_FLD ("dt_effective");
		DSP_FLD ("onhold");
		DSP_FLD ("cancelled");
		DSP_FLD ("won");
		DSP_FLD ("lost");
		DSP_FLD ("no_order");
		DSP_FLD ("convert");
		entry_exit = TRUE;
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("onhold"))
	{
		if (prog_status == ENTRY && FLD ("onhold") == NA)
				return (EXIT_SUCCESS);

		DSP_FLD ("onhold");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("cancelled"))
	{
		if (prog_status == ENTRY && FLD ("cancelled") == NA)
			return (EXIT_SUCCESS);

		DSP_FLD ("cancelled");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("won"))
	{
		if (prog_status == ENTRY && FLD ("won") == NA)
				return (EXIT_SUCCESS);

		DSP_FLD ("won");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("lost"))
	{
		if (prog_status == ENTRY && FLD ("lost") == NA)
				return (EXIT_SUCCESS);

		DSP_FLD ("lost");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("no_order"))
	{
		if (prog_status == ENTRY && FLD ("no_order") == NA)
				return (EXIT_SUCCESS);

		DSP_FLD ("no_order");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("convert"))
	{
		if (prog_status == ENTRY && FLD ("convert") == NA)
				return (EXIT_SUCCESS);

		DSP_FLD ("convert");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
Process (void)
{
	struct  tm	*localtime (const time_t *),
				*tme;
	long	dt_effective;
	char	time_str [11];

	sprintf (err_str, ML (mlQtMess048));
	FlashMess (21,err_str, 4);
	memset (&qthr_rec, 0, sizeof (qthr_rec));
	strcpy (qthr_rec.co_no, comm_rec.co_no);
	strcpy (qthr_rec.br_no, "  ");
	strcpy (qthr_rec.quote_no, "        ");
	cc = find_rec (qthr, &qthr_rec, GTEQ, "r");
	while (!cc && !strcmp (qthr_rec.co_no, comm_rec.co_no))
	{
		tme = localtime (&qthr_rec.timestamp);

		sprintf (time_str, "%02d/%02d/%04d", 
						tme->tm_mday,
						tme->tm_mon + 1,
						tme->tm_year);

		dt_effective = StringToDate (time_str);

		if (dt_effective <= local_rec.del_date)
		{
			if (local_rec.del_onhold [0] == 'Y' && 
				!strcmp (qthr_rec.status, "45"))
					DeleteLine (); 

			if (local_rec.del_cancelled [0] == 'Y' && 
				!strcmp (qthr_rec.status, "80"))
					DeleteLine (); 

			if (local_rec.del_won [0] == 'Y' &&
				!strcmp (qthr_rec.status, "90"))
					DeleteLine (); 

			if (local_rec.del_lost [0] == 'Y' && 
				!strcmp (qthr_rec.status, "91"))
					DeleteLine (); 

			if (local_rec.del_no_ord [0] == 'Y' && 
				!strcmp (qthr_rec.status, "92"))
					DeleteLine (); 

			if (local_rec.del_convert [0] == 'Y' && 
				!strcmp (qthr_rec.status, "95"))
				 	DeleteLine (); 

		}
		cc = find_rec (qthr, &qthr_rec, NEXT, "r");
		continue;
	}
	return (EXIT_SUCCESS);
}

int
DeleteLine (void)
{
	char	cmd [100];
	char	directory [101];
	char	*sptr;

	sptr	= getenv ("PROG_PATH");

#ifndef GVISION
	sprintf (directory, "%s/BIN/PRT", (sptr == (char *)0) ? "/usr/ver9" : sptr);
#else	/* GVISION */
	sprintf (directory, "%s\\BIN\\PRT", (sptr == (char *)0) ? "\\GVision\\RunTime\\ver9" : sptr);
#endif	/* GVISION */

	/*--------------
	| Delete File. |
	--------------*/
#ifndef GVISION
	sprintf (cmd, "rm %s/%-14.14s", directory, qthr_rec.prt_name);
	sys_exec (cmd);
#else	/* GVISION */
	sprintf (cmd, "%s\\%-14.14s", directory, qthr_rec.prt_name);
	remove (cmd);
#endif	/* GVISION */

	qtln_rec.hhqt_hash = qthr_rec.hhqt_hash;
	qtln_rec.line_no   = 0;
	cc = find_rec (qtln, &qtln_rec, GTEQ, "r");
	while (!cc && qtln_rec.hhqt_hash == qthr_rec.hhqt_hash)
	{
		cc = abc_delete (qtln);
		if (cc)
			file_err (cc, qtln, "DBDELETE"); 

		cc = find_rec (qtln, &qtln_rec, NEXT, "r");
	}

	qtlp_rec.hhqt_hash = qthr_rec.hhqt_hash;
	qtlp_rec.line_no   = 0;
	cc = find_rec (qtlp, &qtlp_rec, GTEQ, "r");
	while (!cc && qtlp_rec.hhqt_hash == qthr_rec.hhqt_hash)
	{
		cc = abc_delete (qtlp);
		if (cc)
			file_err (cc, qtlp, "DBDELETE"); 

		cc = find_rec (qtlp, &qtlp_rec, NEXT, "r");
	}

 	cc = abc_delete (qthr);
	if (cc)
		file_err (cc, qthr, "DBDELETE"); 

	return (TRUE);
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
	
		move (0,1);
		line (80);

		rv_pr (ML (mlQtMess050),28,0,1);

		box (0,2,80,17);

		print_at (6, 2, ML (mlQtMess049));

		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

void
FlashMess (
 int 	x_pos,
 char *	mess,
 int	MessDelay)
{
	int		i;
	
	for (i = 0; i < MessDelay; i++)
	{
		move (0,23);
		rv_pr (err_str, (80 - strlen (mess)) /2, x_pos, (i % 2));
		sleep (sleepTime);
	}
	move (0,x_pos); cl_line ();
}	
