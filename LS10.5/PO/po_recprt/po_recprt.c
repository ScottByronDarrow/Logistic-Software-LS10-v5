/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_recprt.c,v 5.4 2002/07/17 09:57:39 scott Exp $
|  Program Name  : (po_recprt.c )                                     |
|  Program Desc  : (Print Pro Forma Receipting Report.   	 )        |	
|                  (                                        )         |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 28/02/89         |
|---------------------------------------------------------------------|
|  Date Modified : (28/02/89)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (17/04/89)      | Modified  by  : Huon Butterworth |
|  Date Modified : (14/06/89)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (03/08/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (02/05/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (07/07/92)      | Modified  by  : Simon Dubey.     |
|  Date Modified : (11/04/94)      | Modified  by  : Roel Michels     |
|                                                                     |
|  Comments      : (17/04/89) Changed MONEYTYPEs to DOUBLETYPEs.      |
|                : (14/06/89) - Removed sort so report prints in order|
|                :              the screen shows.                     |
|                : (03/08/90) - General Update for New Scrgen. S.B.D. |
|                : (02/05/91) - Updated to make order same as         |
|                :              po_receipt.                           |
|                : (07/07/92) - To include inex desc lines DFH 7287.  |
|                : (11/04/94) - PSL 10673 - Online conversion         |
|                : (04/11/97) - Ikea Shipment Mods.                   |
|                : (10/11/97) - Incorporated ML (), full year and      | 
|                               changed invoice length from 6 to 8.   |
| $Log: po_recprt.c,v $
| Revision 5.4  2002/07/17 09:57:39  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2002/02/01 03:37:15  robert
| SC 00743 - added delay on error message
|
| Revision 5.2  2001/08/09 09:16:05  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:14  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:00  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:16  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:41  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:18:02  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:33  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.15  1999/12/06 01:32:50  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.14  1999/11/11 06:43:18  scott
| Updated to remove PNAME from heading as not available with ^P
|
| Revision 1.13  1999/11/05 05:17:16  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.12  1999/09/29 10:12:12  scott
| Updated to be consistant on function names.
|
| Revision 1.11  1999/09/29 09:09:13  scott
| Updated from ansi project and testing
|
| Revision 1.10  1999/09/21 04:38:10  scott
| Updated from Ansi project
|
| Revision 1.9  1999/06/17 10:06:36  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_recprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_recprt/po_recprt.c,v 5.4 2002/07/17 09:57:39 scott Exp $";

#define	MOD	5

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_po_mess.h>
#include <ml_std_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct poslRecord	posl_rec;
struct poshRecord	posh_rec;
struct posdRecord	posd_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct inexRecord	inex_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inumRecord	inum_rec;

	char	*data = "data";

	FILE	*pout;

struct	{
	char	dummy [11];
	int		printerNumber;
	char	printerString [3];
} local_rec;

static	struct	var	vars []	=	
{
	{1, LIN, "printerNumber", 4, 10, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber}, 
	{1, LIN, "shipmentNo", 5, 10, CHARTYPE, 
		"UUUUUUUUUUUU", "          ", 
		" ", "1", "Ship No.", " ", 
		YES, NO, JUSTRIGHT, "", "", posh_rec.csm_no}, 
	{1, LIN, "vessel", 5, 45, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Vessel", " ", 
		NA, NO, JUSTLEFT, "", "", posh_rec.vessel}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

#include	<LocHeader.h>
/*=======================
| Function Declarations |
=======================*/
int 	heading 		 (int);
int 	spec_valid 		 (int);
void 	CloseDB 		 (void);
void 	GetLocation 	 (long, long, char *);
void 	OpenDB 			 (void);
void 	PrintInex 		 (void);
void 	PrintLine 		 (void);
void 	Process 		 (long);
void 	ProcessPoln 	 (long);
void 	ProcessPosl 	 (long, long);
void 	ReadMisc 		 (void);
void 	ReportHeading 	 (long, char *);
void 	RunProgram 		 (char *);
void 	SrchPosh 		 (char *);
void 	shutdown_prog 	 (void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	long	hhshHash;

	if (argc != 1 && argc != 3)
	{
		print_at (0,0,"Usage : %s <printerNumber> <shipmentNo>\007\n\r",argv [0]);
		print_at (1,0," or   : %s\007\n\r",argv [0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	OpenDB ();

	if (argc == 3)
	{
		abc_selfield (posh, "posh_id_no");

		local_rec.printerNumber = atoi (argv [1]);

		hhshHash = atol (argv [2]);

		Process (hhshHash);

		shutdown_prog ();

        return (EXIT_SUCCESS);
	}

	init_scr ();
	set_tty ();
	set_masks ();

	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		init_vars (1);	

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		RunProgram (argv [0]);
	}	/* end of input control loop	*/
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_csm_id");
	open_rec (posd, posd_list, POSD_NO_FIELDS, "posd_id_no");
	open_rec (posl, posl_list, POSL_NO_FIELDS, "posl_id_no");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no,	comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec ("ccmr", &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	OpenLocation (ccmr_rec.hhcc_hash);
}

void
CloseDB (
 void)
{
	abc_fclose (posh);
	abc_fclose (posd);
	abc_fclose (posl);
	abc_fclose (poln);
	abc_fclose (inmr);
	abc_fclose (inex);
	abc_fclose (incc);
	abc_fclose (pohr);
	abc_fclose (inum);
	CloseLocation ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			errmess (ML (mlStdMess020));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
	
		sprintf (local_rec.printerString,"%d",local_rec.printerNumber);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("shipmentNo"))
	{
		if (SRCH_KEY)
		{
			SrchPosh (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (posh_rec.co_no,comm_rec.co_no);
		strcpy (posh_rec.csm_no,zero_pad (posh_rec.csm_no, 12));
		cc = find_rec (posh,&posh_rec,COMPARISON,"r");
		if (cc)
		{
			errmess (ML (mlStdMess050));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		display_field (field + 1);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*=============================
| Search for shipment number. |
=============================*/
void
SrchPosh (
 char *key_val)
{
	work_open ();
	save_rec ("#Shipment No.","#Ship Name");
	strcpy (posh_rec.co_no,comm_rec.co_no);
	sprintf (posh_rec.csm_no, "%-12.12s", key_val);
	cc = find_rec (posh,&posh_rec,GTEQ,"r");
	while (!cc && !strcmp (posh_rec.co_no,comm_rec.co_no))
	{
		if (!strncmp (posh_rec.csm_no,key_val,strlen (key_val)))
		{
			cc = save_rec (posh_rec.csm_no,posh_rec.vessel);
			if (cc)
				break;
		}
		cc = find_rec (posh,&posh_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (posh_rec.co_no,comm_rec.co_no);
	sprintf (posh_rec.csm_no, "%-12.12s", temp_str);
	cc = find_rec (posh,&posh_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in posh During (DBFIND)",cc,PNAME);
}

void
RunProgram (
 char *prog_name)
{
	shutdown_prog ();

	sprintf (err_str, "%s %0ld", prog_name, posh_rec.hhsh_hash);
	sys_exec (err_str);
}

void
ReportHeading (
	long	hhshHash, 
	char	*shipmentNo)
{
	if ((pout = popen ("pformat","w")) == 0)
		sys_err ("Error in pformat during (POPEN)",errno,PNAME);

	fprintf (pout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (pout,".LP%d\n",local_rec.printerNumber);

	fprintf (pout,".11\n");
	fprintf (pout,".PI12\n");
	fprintf (pout,".L156\n");
	fprintf (pout,".B1\n");
	fprintf (pout,".E%s\n",clip (comm_rec.co_name));
	fprintf (pout,".EPRO-FORMA RECEIPTING REPORT\n");
	fprintf (pout,".EFOR SHIPMENT : %s\n",shipmentNo);
	fprintf (pout,".EAs At %-24.24s\n",SystemTime ());

	fprintf (pout,".R================");
	fprintf (pout,"=================");
	fprintf (pout,"=========================================");
	fprintf (pout,"=======");
	fprintf (pout,"===========");
	fprintf (pout,"===========");
	fprintf (pout,"===========");
	fprintf (pout,"=====================");
	fprintf (pout,"============\n");

	fprintf (pout,"================");
	fprintf (pout,"=================");
	fprintf (pout,"=========================================");
	fprintf (pout,"=======");
	fprintf (pout,"===========");
	fprintf (pout,"===========");
	fprintf (pout,"===========");
	fprintf (pout,"=====================");
	fprintf (pout,"============\n");

	fprintf (pout,"| I.D. NUMBER   ");
	fprintf (pout,"| ITEM NUMBER    ");
	fprintf (pout,"|         D E S C R I P T I O N          ");
	fprintf (pout,"| UOM. ");
	fprintf (pout,"| SHIP QTY.");
	fprintf (pout,"| OLD LOC. ");
	fprintf (pout,"| QTY. REC.");
	fprintf (pout,"|    NEW LOCATION    ");
	fprintf (pout,"| B/ORDER  |\n");

	fprintf (pout,"|---------------");
	fprintf (pout,"|----------------");
	fprintf (pout,"|----------------------------------------");
	fprintf (pout,"|------");
	fprintf (pout,"|----------");
	fprintf (pout,"|----------");
	fprintf (pout,"|----------");
	fprintf (pout,"|--------------------");
	fprintf (pout,"|----------|\n");
	fflush (pout);
}

void
Process (
 long hhshHash)
{
	strcpy (posh_rec.co_no,comm_rec.co_no);
	posh_rec.hhsh_hash = hhshHash;
	cc = find_rec (posh,&posh_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in posh during (DBFIND)",cc,PNAME);

	dsp_screen (" Printing Pro-Forma Receipting Report ",
					comm_rec.co_no,comm_rec.co_name);

	ReportHeading (hhshHash, posh_rec.csm_no);

	strcpy (posd_rec.co_no,comm_rec.co_no);
	posd_rec.hhsh_hash = hhshHash;
	posd_rec.hhpo_hash = 0L;

	cc = find_rec (posd,&posd_rec,GTEQ,"r");
	/*-------------------------------
	| purchase order ship details	|
	-------------------------------*/
	while (!cc && !strcmp (posd_rec.co_no,comm_rec.co_no) && 
			posd_rec.hhsh_hash == hhshHash)
	{
		ProcessPosl (hhshHash,posd_rec.hhpo_hash);
		cc = find_rec (posd,&posd_rec,NEXT,"r");
	}
	fprintf (pout,".EOF\n");
	pclose (pout);
}

void
ProcessPosl (
	long	hhshHash, 
	long	hhpoHash)
{
	strcpy (posl_rec.co_no,comm_rec.co_no);
	posl_rec.hhsh_hash = hhshHash;
	posl_rec.hhpl_hash = 0L;

	cc = find_rec (posl,&posl_rec,GTEQ,"r");
	/*---------------------------------------
	| purchase order ship detail lines	|
	---------------------------------------*/
	while (!cc && !strcmp (posl_rec.co_no,comm_rec.co_no) && 
		      posl_rec.hhsh_hash == hhshHash)
	{
		if (posl_rec.hhpo_hash == hhpoHash)
			ProcessPoln (posl_rec.hhpl_hash);

		cc = find_rec (posl,&posl_rec,NEXT,"r");
	}
}

void
ProcessPoln (
	long	hhplHash)
{
	/*-------------------------------
	| purchase order detail lines	|
	-------------------------------*/
	poln_rec.hhpl_hash	=	hhplHash;
	cc = find_rec (poln,&poln_rec,COMPARISON,"r");
	if (!cc)
	{
		inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
		cc = find_rec (inmr,&inmr_rec,EQUAL,"r");
		if (!cc)
			PrintLine ();
	}
}

void
PrintLine (
 void)
{
	char	location [11];
	float	StdCnvFct 	= 1.00,
			PurCnvFct 	= 1.00,
			CnvFct		= 1.00;

	GetLocation (poln_rec.hhbr_hash,poln_rec.hhcc_hash,location);

	if (pohr_rec.hhpo_hash != poln_rec.hhpo_hash)
	{
		pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
		cc = find_rec (pohr,&pohr_rec,EQUAL,"r");
		if (cc)
			strcpy (pohr_rec.pur_ord_no," ");
	}
	
	dsp_process (" Processing : ",inmr_rec.item_no);
	if (posl_rec.ship_qty <= 0.00)
		return;

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
	StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	poln_rec.hhum_hash;
	cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
	PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

	CnvFct	=	StdCnvFct / PurCnvFct;
	
	fprintf (pout,"|%-15.15s",posd_rec.inv_no);
	fprintf (pout,"|%-16.16s",inmr_rec.item_no);
	fprintf (pout,"|%-40.40s",poln_rec.item_desc);
	fprintf (pout,"| %-4.4s ",inum_rec.uom);
	fprintf (pout,"|%10.2f",posl_rec.ship_qty * CnvFct);
	fprintf (pout,"|%-10.10s",location);
	fprintf (pout,"|%-10.10s"," ");
	fprintf (pout,"|%-20.10s"," ");
	fprintf (pout,"|%10.2f|\n",inmr_rec.backorder * CnvFct);
	PrintInex ();
}

void
GetLocation (
	long 	hhbrHash, 
	long 	hhccHash, 
	char 	*location)
{
	long	workInloHash	=	0L;
	char	*workLocation	=	(char *) 0;
	char	old_locn [11];

	sprintf (old_locn,"%-10.10s"," ");

	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = hhbrHash;
	cc = find_rec (incc,&incc_rec,COMPARISON,"r");
	if (cc)
	{
		sprintf (location,"%-10.10s"," ");
		return;
	}
	cc = FindLocation 
		 (
			incc_rec.hhwh_hash,
			incc_rec.hhcc_hash, 
			workLocation,
			ReceiptLocation,
			&workInloHash
		);
	if (!cc)
		strcpy (old_locn,workLocation);

	strcpy (location,old_locn);
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
		rv_pr (ML (" Pro-Forma Receipting Report "),25,0,1);
		move (0,1);
		line (80);

		box (0,3,80,2);

		move (0,20);
		line (80);
		move (0,21);
		print_at (21,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
		move (0,22);
		line (80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

void
PrintInex (
 void)
{

	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no   = 0;

	cc = find_rec (inex, &inex_rec, GTEQ, "r");

	if (cc)
		return;

	while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		fprintf (pout,"|%-15.15s"," ");
		fprintf (pout,"|%-16.16s"," ");
		fprintf (pout,"|%-40.40s",inex_rec.desc);
		fprintf (pout,"| %4.4s "," ");
		fprintf (pout,"|%10.2s"," ");
		fprintf (pout,"|%-10.10s"," ");
		fprintf (pout,"|%-10.10s"," ");
		fprintf (pout,"|%-20.10s"," ");
		fprintf (pout,"|%10.2s|\n"," ");
		cc = find_rec (inex, &inex_rec, NEXT, "r");
	}
}
