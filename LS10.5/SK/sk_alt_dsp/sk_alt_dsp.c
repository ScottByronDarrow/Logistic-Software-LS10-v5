/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_alt_dsp.c,v 5.3 2001/08/09 09:18:03 scott Exp $
|  Program Name  : (sk_alt_dsp.c  )                                 |
|  Program Desc  : (Display Alternate Items.                    )   |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 09/09/91         |
|---------------------------------------------------------------------|
| $Log: sk_alt_dsp.c,v $
| Revision 5.3  2001/08/09 09:18:03  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:44:39  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:50  scott
| Update - LS10.5
|
| Revision 5.0  2001/06/19 08:15:03  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/03/22 06:46:44  scott
| Updated to add sleep delay - did not work with LS10-GUI
|
| Revision 4.1  2001/03/22 03:55:59  scott
| Updated to ensure sleep was included in error message.
| Updated to add app.schema - removes code related to tables from program and
| allows for better quality contol.
| Updated to perform routine maintenance to ensure standards are maintained.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_alt_dsp.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_alt_dsp/sk_alt_dsp.c,v 5.3 2001/08/09 09:18:03 scott Exp $";

#define		X_OFF		0
#define		Y_OFF		2
#include 	<pslscr.h>
#include 	<get_lpno.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>

#define		MAX_HHBR	512
#define		MAX_HHCC	128

#define		COMPANY		'C'
#define		BRANCH		'B'
#define		WAREHOUSE	'W'

#define		BY_CO		 (local_rec.dsp_type [0] == COMPANY)
#define		BY_BR		 (local_rec.dsp_type [0] == BRANCH)
#define		BY_WH		 (local_rec.dsp_type [0] == WAREHOUSE)

#define		VALID_BR	 (!strcmp (ccmr_rec.est_no, comm_rec.est_no))
#define		VALID_WH	 (!strcmp (ccmr_rec.cc_no, comm_rec.cc_no))

char	*UNDERLINE  = "^^GGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^";
char	*UNDERLINE1 = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^";

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct ccmrRecord	ccmr_rec;
struct inccRecord	incc_rec;

	long	storeHhbr [MAX_HHBR],
			storeHhcc [MAX_HHCC];

	int		noHhcc = 0,
			noHhbr = 0;

	extern	int		TruePosition;

	float	totalOnHand;

	char	headItem [17];
	char	headDesc [41];

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	itemNumber [17];
	char	desc [41];
	float	stockOnHand;
	char	dsp_type [10];
	char	dsp_type_desc  [10];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "dsp_type",	 3, 2, CHARTYPE,
		"U", "          ",
		" ", "C", "Display By    ", "C(ompany) / B(ranch) / W(arehouse) ",
		 NO, NO,  JUSTLEFT, "CBW", "", local_rec.dsp_type},
	{1, LIN, "dsp_type_desc", 3, 20, CHARTYPE,
		"AAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dsp_type_desc},
	{1, LIN, "itemNumber",	 4, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number   ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.itemNumber},
	{1, LIN, "desc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Item Desc     ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

#include <RealCommit.h>

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ProcessDisplay 		(void);
void 	ProcessAlternates 	(void);
void 	LoadHhcc 			(void);
void 	CalcOnHand 			(long);
int  	spec_valid 			(int);
int  	StoreHash 			(long);
int  	heading 			(int);


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
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		/*-----------------
		| Update records. |
		-----------------*/
		ProcessDisplay ();
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
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	open_rec (soic, soic_list, soic_no_fields, "soic_id_no2");
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
	abc_fclose (soic);
	SearchFindClose ();
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	if (LCHECK ("itemNumber"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.itemNumber, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.itemNumber);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();

		DSP_FLD ("itemNumber");

		strcpy (local_rec.desc,inmr_rec.description);
		DSP_FLD ("desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("dsp_type"))
	{
		switch (local_rec.dsp_type [0])
		{
		case BRANCH:
			strcpy (local_rec.dsp_type_desc, ML ("Branch   "));
			break;

		case WAREHOUSE:
			strcpy (local_rec.dsp_type_desc, ML ("Warehouse"));
			break;

		case COMPANY:
		default:
			strcpy (local_rec.dsp_type_desc, ML ("Company  "));
			break;
		}

		DSP_FLD ("dsp_type_desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*=======================
| Process whole report. |
=======================*/
void
ProcessDisplay (
 void)
{
	LoadHhcc ();

	Dsp_prn_open (0, 2, 12, " Display Alternate Items ", 
		     (char *)0,  (char *)0,  (char *)0,  
		     (char *)0,  (char *)0,  (char *)0);

	sprintf (err_str, "     STOCK NO      |     I T E M    D E S C R I P T I O N     |AVAILABLE STOCK");
	Dsp_saverec (err_str);
	Dsp_saverec ("");
	Dsp_saverec ("  [REDRAW]  [PRINT]  [NEXT SCN]  [PREV SCN]  [EDIT/END] ");

	totalOnHand = 0.00;

	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.item_no,"%-16.16s",local_rec.itemNumber);
	cc = find_rec (inmr,&inmr_rec,GTEQ,"r");

	if (strlen (clip (inmr_rec.alternate)) != 0 && 
	    inmr_rec.hhsi_hash == 0L)
	{
		noHhbr = 0;

		StoreHash (inmr_rec.hhbr_hash);
		sprintf (headItem, "%-16.16s", inmr_rec.item_no);
		sprintf (headDesc, "%-40.40s", inmr_rec.description);
		CalcOnHand (inmr_rec.hhbr_hash);
		ProcessAlternates ();

		Dsp_saverec (UNDERLINE);

		sprintf (err_str, 
			" TOTAL ON HAND       %-40.40s ^E %12.2f ",
			" ",
			totalOnHand);
		Dsp_saverec (err_str);
		Dsp_saverec (UNDERLINE1);
	
		Dsp_srch ();
		Dsp_close ();
	}
	else
	{
		sprintf (err_str, 
			" *** No alternate stock items for item : %-16.16s ***",
			inmr_rec.item_no);

		Dsp_saverec (err_str);
		Dsp_srch ();
		Dsp_close ();
	}
}

/*---------------------------------
| Process all alternates for item |
---------------------------------*/
void
ProcessAlternates (
 void)
{
	int		firstInmr = TRUE;

	strcpy (inmr2_rec.co_no, comm_rec.co_no);
	sprintf (inmr2_rec.item_no, "%-16.16s", inmr_rec.alternate);

	cc = find_rec (inmr,&inmr2_rec,COMPARISON,"r");
	while (!cc && 
	         strlen (clip (inmr_rec.alternate)) != 0 &&
	         inmr_rec.hhsi_hash == 0L)
	{
		strcpy (inmr_rec.item_no, 	  inmr2_rec.item_no);
		strcpy (inmr_rec.description, inmr2_rec.description);

		if (firstInmr)
		{
			sprintf (err_str, "^1 %-16.16s    %-40.40s   %12.2f ^6",
				headItem, headDesc, local_rec.stockOnHand);

			Dsp_saverec (err_str);

			firstInmr = FALSE;
		}

		if (!StoreHash (inmr2_rec.hhbr_hash))
			return;

		CalcOnHand (inmr2_rec.hhbr_hash);

		sprintf (err_str, " %-16.16s  ^E %-40.40s ^E %12.2f ",
			inmr2_rec.item_no, inmr2_rec.description, local_rec.stockOnHand);
		Dsp_saverec (err_str);

		strcpy (inmr2_rec.co_no, 	comm_rec.co_no);
		strcpy (inmr2_rec.item_no,	inmr2_rec.alternate);
		cc = find_rec (inmr, &inmr2_rec, COMPARISON, "r");
	}
}

/*------------------------
| Load valid hhcc hashes |
------------------------*/
void
LoadHhcc (
 void)
{
	noHhcc = 0;

	strcpy (ccmr_rec.co_no, comm_rec.co_no);

	if (BY_CO)
		strcpy (ccmr_rec.est_no, "  ");
	else
		strcpy (ccmr_rec.est_no, comm_rec.est_no);

	if (BY_CO || BY_BR)
		strcpy (ccmr_rec.cc_no, "  ");
	else
		strcpy (ccmr_rec.cc_no, comm_rec.cc_no);

	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no) &&
	       (!BY_BR || (BY_BR && VALID_BR)) &&
	       (!BY_WH || (BY_WH && VALID_WH)))
	{
		if (noHhcc + 1 >= MAX_HHCC)
		{
			sprintf (err_str, ML (mlSkMess100), MAX_HHCC);
	
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return;
		}
		storeHhcc [noHhcc++] = ccmr_rec.hhcc_hash;
	       
		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
	return;
}

/*=====================================================
| Calculate available stock for item at C/B/W level   |
=====================================================*/
void
CalcOnHand (
	long	hhbrHash)
{
	int	hhcc_ok;
	int	i;
	float	realCommitted;

	local_rec.stockOnHand = 0.00;

	incc_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (incc, &incc_rec, GTEQ, "r");
	while (!cc && incc_rec.hhbr_hash == hhbrHash)
	{
		hhcc_ok = FALSE;
		for (i = 0; i < noHhcc; i++)
		{
			if (incc_rec.hhcc_hash == storeHhcc [i])
			{
				hhcc_ok = TRUE;
				break;
			}
		}
		if (!hhcc_ok)
		{
			cc = find_rec (incc, &incc_rec, NEXT, "r");
			continue;
		}

		/*---------------------------------
		| Calculate Actual Qty Committed. |
		---------------------------------*/
		realCommitted = RealTimeCommitted 
						(
							incc_rec.hhbr_hash, 
							incc_rec.hhcc_hash
						);
		/*--------------
		| calc on_hand |
		--------------*/
		local_rec.stockOnHand += (incc_rec.closing_stock -
				      		  	  incc_rec.committed -
							  	  realCommitted -
				      		  	  incc_rec.backorder -
				      		  	  incc_rec.forward);

		cc = find_rec (incc, &incc_rec, NEXT, "r");
	}
	totalOnHand += local_rec.stockOnHand;
}

/*-------------------------------------------
| Check if hash has already been processed. |
| Return - FALSE if already processed.      |
|        - TRUE if OK to process.           |
-------------------------------------------*/
int
StoreHash (
	long	hhbrHash)
{
	int	i;

	for (i = 0; i < noHhbr; i++)
	{
		if (storeHhbr [i] == hhbrHash)
			return (FALSE);
	}

	if (noHhbr + 1 >= MAX_HHBR)
	{
		sprintf (err_str, ML (mlSkMess101), MAX_HHBR);

		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	storeHhbr [noHhbr++] = hhbrHash;

	return (TRUE);
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
	
	switch (local_rec.dsp_type [0])
	{
		case COMPANY:
		strcpy (err_str, ML (mlSkMess014));
		break;
			
		case BRANCH:
		strcpy (err_str, ML (mlSkMess015));
		break;
		
		case WAREHOUSE:
		strcpy (err_str, ML (mlSkMess016));
		break;

		default:
		strcpy (err_str, ML (mlSkMess017));
		break;
	}
		
	rv_pr (err_str ,20, 0, 1);
	line_at (1,0,80);

	box (0,2,80,3);

	line_at (20,0,80);
	print_at (21,0,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22,0,ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

