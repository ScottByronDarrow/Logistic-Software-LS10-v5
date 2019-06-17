/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: po_poprint.i.c,v 5.1 2002/07/16 02:42:25 scott Exp $
|  Program Name  : (po_poprint.i.c)                 
|  Program Desc  : (Print Purchase Order by Supplier)
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 31/03/89         |
|---------------------------------------------------------------------|
| $Log: po_poprint.i.c,v $
| Revision 5.1  2002/07/16 02:42:25  scott
| Updated from service calls and general maintenance.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_poprint.i.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_poprint.i/po_poprint.i.c,v 5.1 2002/07/16 02:42:25 scott Exp $";

#include <pslscr.h>
#include <ml_po_mess.h>
#include <ml_std_mess.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <twodec.h>

#ifdef GVISION
#include <RemoteFile.h>
#include <RemotePipe.h>
#define	popen	Remote_popen
#define	pclose	Remote_pclose
#define	fprintf	Remote_fprintf
#define	fflush	Remote_fflush
#endif	/* GVISION */

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct pohrRecord	pohr_rec;
struct sumrRecord	sumr_rec;

	int		printerNumber 	= 1,
			envVarCrCo 		= 0,
			envVarCrFind 	= 0,
			automaticPrint	= 0,
			poPrint			= 0,
			running			= 0;

	char	branchNumber 	[3],
			poPrintProgram 	[15];

	long	hhpoHash		= 0L;

	FILE	*pout;


/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	char	supplierNumber [7];
	char	sup_name [41];
	char	pOrderNumber [16];
	char	po_prmpt [17];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "supplierNumber",	 4, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Supplier No   ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.supplierNumber},
	{1, LIN, "name",	 5, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Supplier Name ", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name},
	{1, LIN, "pOrderNumber",	 6, 18, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", local_rec.po_prmpt, " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.pOrderNumber},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


#include <FindSumr.h>
/*
 * Function Declarations 
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void	ReadMisc 		(void);
int  	spec_valid 		(int);
void 	ProcessFile 	(void);
void 	SrchPohr 		(char *);
int 	heading 		(int);

/*
 * Main Processing Routine . 
 */
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];


	if (argc < 2)
	{
		print_at (0,0,mlPoMess732,argv [0]);
		return (EXIT_FAILURE);
	}

	printerNumber = atoi (argv [1]);

	automaticPrint = FALSE;
	if (argc == 3)
	{
		hhpoHash = atol (argv [2]);
		automaticPrint = TRUE;
	}
	envVarCrCo 		= atoi (get_env ("CR_CO"));
	envVarCrFind 	= atoi (get_env ("CR_FIND"));

	OpenDB ();

	/*
	 * Get Purchase order print program. 
	 */
	sptr = chk_env ("PO_PRINT");
	if (sptr == (char *)0)
		poPrint = FALSE;
	else
	{
		poPrint = TRUE;
		sprintf (poPrintProgram, "%-14.14s", sptr);
	}

	strcpy (branchNumber, (envVarCrCo) ? comm_rec.est_no : " 0");

	if (!automaticPrint)
	{
		SETUP_SCR (vars);

		strcpy (local_rec.po_prmpt, "Purchase Ord No");
		/*
		 * Setup required parameters 
		 */
		init_scr ();
		set_tty ();
		set_masks ();
		init_vars (1);

		while (prog_exit == 0)
		{
			/*
			 * Reset control flags 
			 */
			entry_exit 	= FALSE;
			edit_exit 	= FALSE;
			prog_exit 	= FALSE;
			restart 	= FALSE;
			search_ok 	= TRUE;
			init_vars (1);
	
			/*
			 * Entry screen 1 linear input 
			 */
			heading (1);
			entry (1);
			if (restart || prog_exit)
				continue;
	
			/*
			 * Edit screen 1 linear input 
			 */
			heading (1);
			scn_display (1);
			edit (1);
			if (restart)
				continue;
	
			ProcessFile ();
		}
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*
	 * Validate hhpo hash 
	 */
	pohr_rec.hhpo_hash	=	hhpoHash;
	cc = find_rec (pohr, &pohr_rec, COMPARISON, "r");
	if (!cc)
	{
		sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (!cc)
			ProcessFile ();
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence. 
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files 
 */
void
OpenDB (void)
{
	abc_dbopen ("data");

	ReadMisc ();

	open_rec (pohr, pohr_list, POHR_NO_FIELDS, (automaticPrint) 
									? "pohr_hhpo_hash" : "pohr_id_no");
	
	if (automaticPrint)
		open_rec (sumr,sumr_list,SUMR_NO_FIELDS,"sumr_hhsu_hash");
	else
		open_rec (sumr,sumr_list,SUMR_NO_FIELDS, (!envVarCrFind) 
									? "sumr_id_no" : "sumr_id_no3");
}
/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (pohr);
	abc_fclose (sumr);
	abc_dbclose ("data");
}

/*
 * Get common info from commom database file. 
 */
void
ReadMisc (void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr,comr_list,COMR_NO_FIELDS,"comr_co_no");
	strcpy (comr_rec.co_no,comm_rec.co_no);
	cc = find_rec (comr,&comr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "comr", "DBFIND");

	abc_fclose (comr);
}

int
spec_valid (
	int		field)
{
	/*
	 * Validate Supplier Number. 
	 */
	if (LCHECK ("supplierNumber"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.supplierNumber));
		cc = find_rec (sumr,&sumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("name");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Purchase Order Number. 
	 */
	if (LCHECK ("pOrderNumber"))
	{
		if (SRCH_KEY)
		{
			SrchPohr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (pohr_rec.co_no,comm_rec.co_no);
		strcpy (pohr_rec.br_no,comm_rec.est_no);
		strcpy (pohr_rec.type, "O");
		pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (pohr_rec.pur_ord_no,zero_pad (local_rec.pOrderNumber, 15));
		cc = find_rec (pohr,&pohr_rec,COMPARISON,"r");
		if (cc)
		{
			errmess (ML (mlStdMess122));
			return (EXIT_FAILURE);
		}
		if (pohr_rec.status [0] == 'D')
		{
			errmess (ML (mlPoMess001));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}


/*
 * Process Purchase order line items. 
 */
void
ProcessFile (void)
{
	if (poPrint)
	{
		if ( (pout = popen (poPrintProgram,"w")) == 0)
		{
			sprintf (err_str, "Error in %s during (POPEN)",poPrintProgram);
			sys_err (err_str, errno, PNAME);
		}
		fprintf (pout,"%d\n",printerNumber);
		fprintf (pout,"S\n");
		fprintf (pout,"%ld\n",pohr_rec.hhpo_hash);
		fprintf (pout,"0\n");
		pclose (pout);

	}
	else
	{
		fprintf (stderr, 
				"\nEnvironment Variable PO_PRINT cannot be branchNolished\n");
		exit  (-1);
	}
}

/*
 * Search on Purchase Order Number. 
 */
void
SrchPohr (
	char	*keyValue)
{
	_work_open (15,0,10);
	strcpy (pohr_rec.co_no,comm_rec.co_no);
	strcpy (pohr_rec.br_no,comm_rec.est_no);
	strcpy (pohr_rec.type, "O");
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (pohr_rec.pur_ord_no,"%-15.15s",keyValue);
	save_rec ("#Purchase Order.","#Date"); 
	cc = find_rec (pohr,&pohr_rec,GTEQ,"r");
	while (!cc && !strcmp (pohr_rec.co_no,comm_rec.co_no) && 
		      !strcmp (pohr_rec.br_no,comm_rec.est_no) && 
		      !strncmp (pohr_rec.pur_ord_no,keyValue,strlen (keyValue)))
	{

		if (pohr_rec.hhsu_hash == sumr_rec.hhsu_hash && 
		     pohr_rec.type [0] != 'D')
		{
			strcpy (err_str, DateToString (pohr_rec.date_raised));
			cc = save_rec (pohr_rec.pur_ord_no, err_str);
			if (cc)
				break;
		}
		cc = find_rec (pohr,&pohr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pohr_rec.co_no,comm_rec.co_no);
	strcpy (pohr_rec.br_no,comm_rec.est_no);
	strcpy (pohr_rec.type, "O");
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (pohr_rec.pur_ord_no,"%-15.15s",temp_str);
	cc = find_rec (pohr,&pohr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, pohr, "DBFIND");
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
		line_at (1,0,80);

		rv_pr (ML ("Purchase Order Print"), 30,0,1);
		box (0,3,80,3);

		line_at (20,0,80);
		strcpy (err_str,ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.co_no,comm_rec.co_name);
		strcpy (err_str,ML (mlStdMess039));
		print_at (22,0,err_str,comm_rec.est_no,comm_rec.est_name);

		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

