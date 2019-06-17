/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_rf_disp.c,v 5.2 2001/08/09 09:19:43 scott Exp $
|  Program Desc  : (RF Stock status.                              )   |
-----------------------------------------------------------------------
| $Log: sk_rf_disp.c,v $
| Revision 5.2  2001/08/09 09:19:43  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:40  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:17:17  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:38:36  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:21:02  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 1.2  2000/08/15 07:57:08  scott
| Updated from testing
|
| Revision 1.1  2000/08/15 02:10:20  scott
| New files - RF stock display
|
 */
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_rf_disp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_rf_disp/sk_rf_disp.c,v 5.2 2001/08/09 09:19:43 scott Exp $";

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>
#include 	"schema"

struct	commRecord	comm_rec;
struct	ccmrRecord	ccmr_rec;
struct	inccRecord	incc_rec;
struct	inmrRecord	inmr_rec;


extern	int		TruePosition;
/*============
| File Names |
============*/
static char
	*data	= "data";

/*=========
| Globals |
==========*/
/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	item_no [17];
	char	desc [20];
	float	SOH;
	float	SOO;
	float	AVL;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "item_no", 2, 1, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no}, 
	{1, LIN, "desc", 3, 1, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.desc}, 
	{1, LIN, "SOH",	4, 1, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "SOH : ", "",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.SOH},
	{1, LIN, "SOO",	5, 1, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "SOO : ", "",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.SOO},
	{1, LIN, "AVL",	6, 1, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", "0.00", "AVL : ", "",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.AVL},
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*======================
| Function Declaration |
======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
int  	spec_valid 			(int);
int  	heading 			(int);
void	RfErrorLine 		(char *);

#include <FindInmr.h>

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	init_scr 	();	
	set_tty 	();
	set_masks 	();		
	init_vars 	(1);

	OpenDB ();

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		if (restart)
			continue;

		/*-----------------
		| Update records. |
		-----------------*/
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec ("ccmr", &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (ccmr);
	abc_fclose (incc);
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("item_no"))
	{
		if (SRCH_KEY)
		{
	 		InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
			
		cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			RfErrorLine  (ML ("NOT ON FILE"));
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.desc, "%-18.18s", inmr_rec.description);
		strcpy (local_rec.item_no, inmr_rec.item_no);
		DSP_FLD ("desc");
		DSP_FLD ("item_no");

		incc_rec.hhcc_hash	=	ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		cc = find_rec ("incc", &incc_rec, COMPARISON, "r");
		if (cc)
		{
			RfErrorLine  (ML ("NOT IN W/H"));
			return (EXIT_FAILURE);
		}
		local_rec.SOH	=	incc_rec.closing_stock;
		local_rec.SOO	=	incc_rec.on_order;
		local_rec.AVL	=	incc_rec.closing_stock	-	
							incc_rec.committed - 
							incc_rec.backorder;
		DSP_FLD ("SOH");
		DSP_FLD ("SOO");
		DSP_FLD ("AVL");

		PauseForKey (8,3,ML ("%RHIT ANY KEY"),0);
		
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}


int
heading (
 int scn)
{
	if (restart) 
	{
		abc_unlock (inmr);
		return (EXIT_SUCCESS);
	}
	
	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	rv_pr (ML ("STOCK SCAN DISPLAY"), 1, 1, 1);
	box (0,0,20,8);

	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

void
RfErrorLine 
(
	char	*rfMessage)
{
	print_at (8,1, "%R%-18.18s", rfMessage);
	sleep (sleepTime);
	print_at (8,1, "%-18.18s", " ");
}
