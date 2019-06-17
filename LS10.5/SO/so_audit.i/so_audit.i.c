/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_audit.i.c,v 5.3 2002/07/17 09:58:02 scott Exp $
|  Program Name  : (so_audit.i.c) 
|  Program Desc  : (Input For Invoice Audit Print)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: so_audit.i.c,v $
| Revision 5.3  2002/07/17 09:58:02  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:20:36  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:50:48  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:18:35  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/23 03:25:06  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_audit.i.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_audit.i/so_audit.i.c,v 5.3 2002/07/17 09:58:02 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <ml_so_mess.h>
#include <ml_std_mess.h>

/*================================================================
| Special fields and flags  ################################## . |
================================================================*/
extern	int	TruePosition;
extern	int	EnvScreenOK;

#include	"schema"

struct commRecord	comm_rec;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	batch [6];
	char	back [2];
	char	backDesc [11];
	char	onite [2];
	char	oniteDesc [11];
	int		printerNo;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "batch",	 5, 2, CHARTYPE,
		"UUUUU", "          ",
		" ", "", "Batch Number    ", " ALL for All Batches ",
		YES, NO,  JUSTLEFT, "", "", local_rec.batch},
	{1, LIN, "printerNo",	 7, 2, INTTYPE,
		"NN", "          ",
		"", "1", "Printer No      ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{1, LIN, "back",	 9, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background      ", "",
		YES, NO, JUSTRIGHT, "NY", "", local_rec.back},
	{1, LIN, "backDesc",	 9, 22, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onite",	 10, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight       ", "",
		YES, NO, JUSTRIGHT, "NY", "", local_rec.onite},
	{1, LIN, "oniteDesc",	 10, 22, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "NY", "", local_rec.oniteDesc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*======================= 
| Function Declarations |
=======================*/
void 	RunProgram 			(void);
void 	shutdown_prog 		(void);
void	CloseDB 			(void);
int  	heading 			(int);
int  	spec_valid 			(int);


/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv [])
{

	EnvScreenOK		=	FALSE;
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	init_scr ();
	set_tty (); 
	set_masks ();
	init_vars (1);

	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

   	entry_exit 	= FALSE;
   	prog_exit 	= FALSE;
   	restart 	= FALSE;
	search_ok 	= TRUE;
	init_vars (1);

	heading (1);
	entry (1);
    if (prog_exit)  {
		shutdown_prog ();
        return (EXIT_SUCCESS);
    }

	heading (1);
	scn_display (1);
	edit (1);
	prog_exit = 1;

	if (!restart) 
	    RunProgram ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
RunProgram (
 void)
{
	char	printerString [3];

	sprintf (printerString, "%02d", local_rec.printerNo);

	shutdown_prog ();

	if (local_rec.onite [0] == 'Y')
	{
		if (fork () == 0)
			execlp ("ONIGHT",
				"ONIGHT",
				"so_audit",
				printerString,
				"B",
				local_rec.batch,
				"Print Invoice Audit ", (char *)0);
	}
	else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
			execlp ("so_audit",
				"so_audit",
				printerString,
				"B",
				local_rec.batch, (char *)0);
	}
	else 
	{
		execlp ("so_audit",
			"so_audit",
			printerString,
			"B",
			local_rec.batch, (char *)0);
	}
}

/*========================	
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
 int field)
{
	/*------------------------
	| Validate Batch Number. |
	------------------------*/
	if (LCHECK ("batch")) 
	{
		strcpy (local_rec.batch, zero_pad (local_rec.batch, 5));

		DSP_FLD ("batch");
		return (EXIT_SUCCESS);
	}

	/*-----------------
	| Validate lpno . |
	-----------------*/
	if (LCHECK ("printerNo")) 
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back")) 
	{
		strcpy (local_rec.backDesc,
			 (local_rec.back [0] == 'Y') ? ML ("Yes") : ML ("No "));

		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onite")) 
	{
		strcpy (local_rec.oniteDesc,
			 (local_rec.onite [0] == 'Y') ? ML ("Yes") : ML ("No "));

		DSP_FLD ("oniteDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*======================== 
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_dbclose ("data");
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		/*----------------------
		| Invoice Audit Report |
		----------------------*/
		print_at (0,22, ML (mlSoMess039)); 
		rv_off ();
		fflush (stdout);
		line_at (1,0,80);
		
		line_at (6,1,79);
		line_at (8,1,79);
		if (scn == 1)
			box (0,4,80,6);

		line_at (19,0,80);
		print_at (20,0, ML (mlStdMess038), comm_rec.co_no,  comm_rec.co_name);
		print_at (21,0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
		line_at (22,0,80);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
