/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: ArchiveFiles.c,v 5.6 2002/10/09 06:04:58 robert Exp $
|  Program Name  : (ArchiveFiles.c) 
|  Program Desc  : (Archive files to disk from archive files)
|---------------------------------------------------------------------|
|  Date Written  : (6th May 2002)  | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
| $Log: ArchiveFiles.c,v $
| Revision 5.6  2002/10/09 06:04:58  robert
| Modified to save archive data into text file format to make
| it OS independent
|
| Revision 5.5  2002/07/31 01:47:07  scott
| Updated to fix wrong paramater on restore of Invoice.
|
| Revision 5.4  2002/05/20 06:41:29  cha
| Ditto
|
| Revision 5.3  2002/05/20 06:34:28  cha
| Updated to make sure that filename has the right length.
|
| Revision 5.2  2002/05/10 08:57:09  scott
| Updated to prevent Null being displayed.
|
| Revision 5.1  2002/05/08 02:05:00  scott
| .
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ArchiveFiles.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/ArchiveFiles/ArchiveFiles.c,v 5.6 2002/10/09 06:04:58 robert Exp $";

#define	S_DUMM		0
#define	S_HEAD		1

/*
 *   Include file dependencies  
 */
#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_gl_mess.h>
#include 	<Archive.h>
#include 	<tabdisp.h>
#include 	<hot_keys.h>

#ifdef GVISION
	#include <RemoteFile.h>
	#define fprintf Remote_fprintf
	#define fopen Remote_fopen
	#define fgets Remote_fgets
	#define fclose Remote_fclose
#endif

	char	*archiveTab	= "archiveTag",
			*data 		= "data",
			*currentUserName;

	char	bufferData 			[256],
			filePath			[256];

	int		noInTab				= 0,
			noRecordsFound		= TRUE,
			printerOutputOpen	= FALSE,
			printerNumber		= 1;

	struct {
		char	success	[31];
		char	restore	[31];
		char	open	[31];
		char	add		[31];
		char	find	[31];
		char	error	[31];
		char	none	[31];
		char	ready	[31];
		char	steady	[31];
		} ArchiveMsg;

	FILE	*fout;

	static	int	RestartFunc	 	(int, KEY_TAB *);
	static	int	ExitFunc	 	(int, KEY_TAB *);
	static 	int TagAllFunc 		(int, KEY_TAB *);
	static 	int TagFunc 		(int, KEY_TAB *);

#ifdef	GVISION
	static KEY_TAB listKeys [] =
	{
	   { " ABORT ", 				FN1, 	RestartFunc, 
		"Exit without update.", 						"A" }, 
	   { NULL, 				FN16, 	ExitFunc, 
		"", 						"A" }, 
    	{ " TAG / UNTAG ALL ", 	  'A', 	 TagAllFunc, 
	  	"Examine Order Lines For Release.", 						"A" }, 
    	{ " TAG / UNTAG LINE ", 	  'T', 	 TagFunc, 
	  	"Examine Order Lines For Release.", 						"A" }, 
		END_KEYS
	};
#else
	static KEY_TAB listKeys [] =
	{
	   { "[RESTART]", 				FN1, 	RestartFunc, 
		"Exit without update.", 						"A" }, 
	   { NULL, 				FN16, 	ExitFunc, 
		"", 						"A" }, 
    	{ " TAG / UNTAG [A]LL ", 	  'A', 	 TagAllFunc, 
	  	"Examine Order Lines For Release.", 						"A" }, 
    	{ " [T]AG / UNTAG LINE ", 	  'T', 	 TagFunc, 
	  	"Examine Order Lines For Release.", 						"A" }, 
		END_KEYS
	};
#endif

#include	"schema"

struct commRecord	comm_rec;
struct arctRecord	arct_rec;
struct arhrRecord	arhr_rec;
struct arlnRecord	arln_rec;
struct cumrRecord	cumr_rec;
struct sumrRecord	sumr_rec;
struct inmrRecord	inmr_rec;
struct arpcwoRecord	arpcwo_rec;
struct arsohrRecord	arsohr_rec;
struct arsolnRecord	arsoln_rec;
struct arpohrRecord	arpohr_rec;
struct arpolnRecord	arpoln_rec;

extern	int	TruePosition;

    struct	
    {
		char	dummy	 		[11];
		char	filename 		[21];
		char	tranTypeDesc	[31];
		char	loadType		[2];
		char	loadTypeDesc	[11];
		char	systemDate 		[11];
		int 	tranType;
		long	lsystemDate;
		long	startDate;
		long	endDate;
	} 	local_rec;

static	struct	var vars [] =
{
	{S_HEAD, LIN, "tranType", 3, 2, INTTYPE,
		"N", "             ",
		"", "", "Transaction Type    ", " 1 - Purchase Order, 2 - Sales Order, 3 - Invoice/Credit, 4 - Works Orders",
		YES, NO,  JUSTLEFT, "1", "4", (char *) &local_rec.tranType},
	{S_HEAD, LIN, "tranTypeDesc", 3, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "             ",
		"", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.tranTypeDesc},
	{S_HEAD, LIN, "loadType", 3, 50, CHARTYPE,
		"U", "             ",
		"", "D", "Download / Upload   ", " Download / Upload <default> = Download.",
		YES, NO,  JUSTLEFT, "DU", "", local_rec.loadType},
	{S_HEAD, LIN, "loadTypeDesc", 3, 73, CHARTYPE,
		"AAAAAAAAAA", "             ",
		"", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.loadTypeDesc},
	{S_HEAD, LIN, "filename", 3, 90, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "             ",
		"", " ", "Filename   ", " Enter Filename - Search Available ",
		YES, NO,  JUSTLEFT, "", "", local_rec.filename},
	{1, LIN, "startDate",	 4, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "0", "Start Date          ", "Enter Date for Starting Transaction - Default is start ",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.startDate},
	{1, LIN, "endDate",	 4, 50, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "End   Date          ", "Enter Date for Ending Transaction - Default is Today ",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.endDate},
	{S_DUMM, TAB, "",		0, 0, INTTYPE,
		"A", "          ",
		" ", "", "", " ",
		YES, NO, JUSTRIGHT, " ", " ", NULL}
};

/*
 *   Local function prototypes  
 */
int  	heading 		 		(int);
int  	spec_valid 		 		(int);
void 	CloseDB 		 		(void);
void 	OpenDB 			 		(void);
void	ProcessPurchaseOrders	(void);
void	ProcessSalesOrders 		(void);
void	ProcessInvoices 		(void);
void	ProcessWorksOrders 		(void);
void 	ProcessAll 				(void);
void	ProcessData 			(void);
void 	InitDisplay 			(void);
void 	SrchArct 				(char *);
void	DiskPurchaseOrders 		(void);
void	DiskSalesOrders 		(void);
void	DiskInvoices 			(void);
void	DiskWorksOrders			(void);
void	HeadingPrint 			(void);
void	InitML 					(void);

/*
 *   Main Processing Function  
 */
int
main (
 int   argc,
 char *argv [])
{
	TruePosition	=	TRUE;

	currentUserName = getenv ("LOGNAME");

	if (argc < 2)
	{
		print_at (0,0, "Usage : %s [LPNO] [DIRECTORY]", argv [0]);
		return (EXIT_FAILURE);
	}
	if (argc == 2)
		printerNumber	=	atoi (argv [1]);

	if (argc == 3)
		sprintf (filePath, "%s", argv [2]);
	else
#ifndef GVISION
		sprintf (filePath, "%s/ARCHIVE/", getenv ("PROG_PATH"));
#else
		sprintf (filePath, "%s/ARCHIVE/", ServerPROG_PATH ());
#endif

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	OpenDB 	();
	InitML	();

	SETUP_SCR (vars);

	init_scr 	();
	set_tty 	();
	set_masks 	();
	init_vars 	(S_HEAD);

	while (!prog_exit)
	{
		search_ok  = TRUE;
		init_ok    = TRUE;
		entry_exit = FALSE;
		edit_exit  = FALSE;
		restart    = FALSE;
		prog_exit  = FALSE;

		init_vars (S_HEAD);

		heading (S_HEAD);
		scn_display (S_HEAD);
		entry (S_HEAD);

		if (prog_exit || restart)
			continue;

		/*
		 * processing function.
		 */
		ProcessData ();

		prog_exit = TRUE;
	}

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*
 * Open database files required.
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (arhr,   arhr_list,   ARHR_NO_FIELDS,   "arhr_id_no");
	open_rec (arln,   arln_list,   ARLN_NO_FIELDS,   "arln_id_no");
	open_rec (arsohr, arsohr_list, ARSOHR_NO_FIELDS, "arsohr_id_no");
	open_rec (arsoln, arsoln_list, ARSOLN_NO_FIELDS, "arsoln_id_no");
	open_rec (arpcwo, arpcwo_list, ARPCWO_NO_FIELDS, "arpcwo_id_no");
	open_rec (arpohr, arpohr_list, ARPOHR_NO_FIELDS, "arpohr_id_no");
	open_rec (arpoln, arpoln_list, ARPOLN_NO_FIELDS, "arpoln_id_no");
	open_rec (arct,   arct_list,   ARCT_NO_FIELDS,   "arct_id_no");
	open_rec (cumr,   cumr_list,   CUMR_NO_FIELDS,   "cumr_hhcu_hash");
	open_rec (sumr,   sumr_list,   SUMR_NO_FIELDS,   "sumr_hhsu_hash");
	open_rec (inmr,   inmr_list,   INMR_NO_FIELDS,   "inmr_hhbr_hash");
}

/*
 * Close database files required.
 */
void
CloseDB (void)
{
	abc_fclose (arhr);
	abc_fclose (arln);
	abc_fclose (arsohr);
	abc_fclose (arsoln);
	abc_fclose (arpcwo);
	abc_fclose (arpohr);
	abc_fclose (arpoln);
	abc_fclose (arct);
	abc_fclose (cumr);
	abc_fclose (sumr);
	abc_fclose (inmr);
	ArchiveClose ();

	if (printerOutputOpen == TRUE)
	{
		fprintf (fout,".EOF\n");
		pclose (fout);
	}
	abc_dbclose (data);
}

int
spec_valid (
 int	field)
{
	/*
	 * Validate input file name
	 */
	if (LCHECK ("filename"))
	{
		/*
		 * Search can only be used for uploads
		 */
		if (SRCH_KEY && local_rec.loadType [0] == 'U')
		{
			SrchArct (temp_str);
			return (EXIT_SUCCESS);
		}

		/*
		 * File cannot be blank.
		 */
		if (strlen (clip (local_rec.filename)) == 0)
		{
			sprintf (err_str, ML ("A Valid Filename Must Be Entered"));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Validate upload file exists.       
		 */
		if (local_rec.loadType [0] == 'U')
		{
			strcpy (arct_rec.co_no, comm_rec.co_no);
			sprintf (arct_rec.filename, "%-20.20s",local_rec.filename);
			cc = find_rec (arct, &arct_rec, COMPARISON, "r");
			if (cc)
			{
				print_mess (ML ("Archive file name not found"));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			FLD ("startDate")	=	NA;
			FLD ("endDate")		=	NA;
		}
		else
		{
			strcpy (arct_rec.co_no, comm_rec.co_no);
			sprintf (arct_rec.filename, "%-20.20s",local_rec.filename);
			cc = find_rec (arct, &arct_rec, COMPARISON, "r");
			if (!cc)
			{
				print_mess (ML ("Archive file name already used"));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			FLD ("startDate")	=	YES;
			FLD ("endDate")		=	YES;
		}
		DSP_FLD ("filename");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Start Date. 
	 */
	if (LCHECK ("startDate"))
	{
		if (dflt_used)
		{
			local_rec.startDate = 0L;
			return (EXIT_SUCCESS);
		}
		if (prog_status != ENTRY && 
			local_rec.startDate > local_rec.endDate)
		{
			print_mess (ML (mlStdMess019));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate End Date. 
	 */
	if (LCHECK ("endDate"))
	{
		if (dflt_used)
		{
			local_rec.endDate = local_rec.lsystemDate;
			return (EXIT_SUCCESS);
		}
		if (local_rec.startDate > local_rec.endDate)
		{
			print_mess (ML (mlStdMess019));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate transaction type.
	 */
	if (LCHECK ("tranType"))
	{
		switch (local_rec.tranType)
		{
			case	1:
				strcpy (local_rec.tranTypeDesc, ML ("Purchase Orders"));
				break;

			case	2:
				strcpy (local_rec.tranTypeDesc, ML ("Sales Orders"));
				break;

			case	3:
				strcpy (local_rec.tranTypeDesc, ML ("Invoices / Credits."));
				break;

			case	4:
				strcpy (local_rec.tranTypeDesc, ML ("Works Orders"));
				break;
		}
		DSP_FLD ("tranTypeDesc");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Load type.
	 */
	if (LCHECK ("loadType"))
	{
		strcpy (local_rec.loadTypeDesc, (local_rec.loadType [0] == 'D') 
						? ML ("Download") : ML ("Upload  "));

		DSP_FLD ("loadTypeDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Search Archive Control file.
 */
void
SrchArct (
	char    *keyValue)
{
    _work_open (20,0,40);
    save_rec ("#Archive File Name", "#User Name       Date        Time");

	/*
	 * Flush record buffer first 
	 */
	memset (&arct_rec, 0, sizeof (arct_rec));

	strcpy (arct_rec.co_no, comm_rec.co_no);
    sprintf (arct_rec.filename, "%20.20s", keyValue);
    for (cc = find_rec (arct, &arct_rec,  GTEQ,"r");
		 !cc && !strcmp (arct_rec.co_no, comm_rec.co_no)
		  && !strncmp (arct_rec.filename, keyValue, strlen (clip (keyValue)));
         cc = find_rec (arct, &arct_rec,  NEXT, "r"))
    {
		sprintf 
		(
			err_str, 
			"%s  %s  %s", 
			arct_rec.op_id, 
			DateToString (arct_rec.date_create),
			arct_rec.time_create
		);
		cc = save_rec (arct_rec.filename, err_str);
		if (cc)
			break;
    }
    cc = disp_srch ();
    work_close ();
    if (cc)
        return;

	strcpy (arct_rec.co_no, comm_rec.co_no);
    sprintf (arct_rec.filename, "%20.20s", temp_str);
    cc = find_rec (arct, &arct_rec,  COMPARISON, "r");
    if (cc)
       file_err (cc, arct, "DBFIND");
}

/*
 * Calls required functions to read information from archive files.
 * These files could be database files or disk based files.
 */
void
ProcessData (void)
{
	InitDisplay ();

	/* 
	 * Process transactions:
	 *	1 = Purchase order
	 *	2 = Sales order
	 *	3 = Invoices / Credits 
	 *	4 = Works Orders
	 */
	switch (local_rec.tranType)
	{
		/*
		 * Process purchase orders from archive file or disk files.
		 */
		case	1:
				if (local_rec.loadType [0] == 'D')
					ProcessPurchaseOrders ();
				else
					DiskPurchaseOrders ();
				break;

		/*
		 * Process sales orders from archive file or disk files.
		 */
		case	2:
				if (local_rec.loadType [0] == 'D')
					ProcessSalesOrders ();
				else
					DiskSalesOrders ();
				break;

		/*
		 * Process Invoices from archive file or disk files.
		 */
		case	3:
				if (local_rec.loadType [0] == 'D')
					ProcessInvoices ();
				else
					DiskInvoices ();
				break;

		/*
		 * Process works orders from archive file or disk files.
		 */
		case	4:
				if (local_rec.loadType [0] == 'D')
					ProcessWorksOrders ();
				else
					DiskWorksOrders ();
				break;

		default:
			break;
	}
	/*
	 * Perform final processing for transaction type selected.
	 */
	ProcessAll ();
}

/*
 * Read purchased orders placed onto disk.
 */
void
DiskPurchaseOrders (void)
{
	char	arpohrFileName	[255];
	char	fileName		[255];

	FILE	*fp;

	noRecordsFound	=	TRUE;

	strcpy (fileName, clip (filePath));
	strcat (fileName, clip (local_rec.filename));
	sprintf (arpohrFileName, "%s.arpohr.dat", fileName);

	/*
	 * Open archive file to reading Archive Header information.
	 */
	if ((fp = fopen (arpohrFileName, "r")) == NULL)	
	{
		tab_add (archiveTab, " No Records Found ");
		return;
	}

	while (!ArPohrRead (fp, &arpohr_rec))
	{
		sumr_rec.hhsu_hash	=	arpohr_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (sumr_rec.crd_no, " ");
			strcpy (sumr_rec.crd_name, 	 " ");
		}
		sprintf (err_str, "%s-%s", sumr_rec.crd_no, sumr_rec.crd_name);
		noRecordsFound	=	FALSE;
		/*
  	  	 * Add entry to tab screen.
 	 	 */
		tab_add 
		(
			archiveTab, 
			"%s|%-20.20s|%10.10s|%60.60s|%20.20s P %010ld",
			"*",
			arpohr_rec.pur_ord_no,
			DateToString (arpohr_rec.date_raised),
			err_str,
			ArchiveMsg.steady, 
			arpohr_rec.hhpo_hash
		);
		tab_get (archiveTab, bufferData, EQUAL, noInTab);
		redraw_line (archiveTab, TRUE);
		noInTab++;
	}
	fclose (fp);

	if (noRecordsFound)
		tab_add (archiveTab, " No Records Found ");

	return;
}

/*
 * Read purchased orders from archive file.
 */
void
ProcessPurchaseOrders (void)
{
	noRecordsFound	=	TRUE;
	strcpy (arpohr_rec.co_no, comm_rec.co_no);
	strcpy (arpohr_rec.br_no, "  ");
	strcpy (arpohr_rec.pur_ord_no, " ");
	arpohr_rec.hhsu_hash	=	0L;
	cc = find_rec (arpohr, &arpohr_rec, GTEQ, "r");
	while (!cc && !strcmp (arpohr_rec.co_no, comm_rec.co_no))
	{
		if (arpohr_rec.date_raised < local_rec.startDate ||
			arpohr_rec.date_raised > local_rec.endDate)
		{
			cc = find_rec (arpohr, &arpohr_rec, NEXT, "r");
			continue;
		}
		sumr_rec.hhsu_hash	=	arpohr_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (sumr_rec.crd_no, " ");
			strcpy (sumr_rec.crd_name, 	 " ");
		}
		sprintf (err_str, "%s-%s", sumr_rec.crd_no, sumr_rec.crd_name);
		noRecordsFound	=	FALSE;
		/*
  	  	 * Add entry to tab screen.
 	 	 */
		tab_add 
		(
			archiveTab, 
			"%s|%-20.20s|%10.10s|%60.60s|%20.20s P %010ld",
			"*",
			arpohr_rec.pur_ord_no,
			DateToString (arpohr_rec.date_raised),
			err_str,
			ArchiveMsg.ready,
			arpohr_rec.hhpo_hash
		);
		tab_get (archiveTab, bufferData, EQUAL, noInTab);
		redraw_line (archiveTab, TRUE);
		noInTab++;

		cc = find_rec (arpohr, &arpohr_rec, NEXT, "r");
	}
	if (noRecordsFound)
		tab_add (archiveTab, " No Records Found ");
}

/*
 * Read sales orders from disk file.
 */
void
DiskSalesOrders (void)
{
	char	arsohrFileName	[255];
	char	fileName		[255];
	FILE	*fp;

	noRecordsFound	=	TRUE;

	strcpy (fileName, clip (filePath));
	strcat (fileName, clip (local_rec.filename));
	sprintf (arsohrFileName, "%s.arsohr.dat", fileName);

	/*
	 * Open archive file to reading Archive Header information.
	 */
	if ((fp = fopen (arsohrFileName, "r")) == NULL)
	{
		tab_add (archiveTab, " No Records Found ");
		return;
	}

	while (!ArSohrRead (fp, &arsohr_rec))
	{
		cumr_rec.hhcu_hash	=	arsohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (cumr_rec.dbt_no, " ");
			strcpy (cumr_rec.dbt_name, 	 " ");
		}
		sprintf (err_str, "%s-%s", cumr_rec.dbt_no, cumr_rec.dbt_name);
		noRecordsFound	=	FALSE;
		/*
  	  	 * Add entry to tab screen.
 	 	 */
		tab_add 
		(
			archiveTab, 
			"%s|%-20.20s|%10.10s|%60.60s|%20.20s S %010ld",
			"*",
			arsohr_rec.order_no,
			DateToString (arsohr_rec.dt_raised),
			err_str,
			ArchiveMsg.steady, 
			arsohr_rec.hhso_hash
		);
		tab_get (archiveTab, bufferData, EQUAL, noInTab);
		redraw_line (archiveTab, TRUE);
		noInTab++;
	}

	fclose (fp);

	if (noRecordsFound)
		tab_add (archiveTab, " No Records Found ");

	return;
}
/*
 * Read sales orders from Archive file.
 */
void
ProcessSalesOrders (void)
{
	noRecordsFound	=	TRUE;

	strcpy (arsohr_rec.co_no, comm_rec.co_no);
	strcpy (arsohr_rec.br_no, "  ");
	strcpy (arsohr_rec.order_no, " ");
	arsohr_rec.hhcu_hash	=	0L;
	cc = find_rec (arsohr, &arsohr_rec, GTEQ, "r");
	while (!cc && !strcmp (arsohr_rec.co_no, comm_rec.co_no))
	{
		if (arsohr_rec.dt_raised < local_rec.startDate ||
			arsohr_rec.dt_raised > local_rec.endDate)
		{
			cc = find_rec (arsohr, &arsohr_rec, NEXT, "r");
			continue;
		}
		noRecordsFound	=	FALSE;
		cumr_rec.hhcu_hash	=	arsohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (cumr_rec.dbt_no, " ");
			strcpy (cumr_rec.dbt_name, 	 " ");
		}
		sprintf (err_str, "%s-%s", cumr_rec.dbt_no, cumr_rec.dbt_name);
		/*
  	  	 * Add entry to tab screen.
 	 	 */
		tab_add 
		(
			archiveTab, 
			"%s|%-20.20s|%10.10s|%60.60s|%20.20s S %010ld",
			"*",
			arsohr_rec.order_no,
			DateToString (arsohr_rec.dt_raised),
			err_str,
			ArchiveMsg.ready,
			arsohr_rec.hhso_hash
		);
		tab_get (archiveTab, bufferData, EQUAL, noInTab);
		redraw_line (archiveTab, TRUE);
		noInTab++;

		cc = find_rec (arsohr, &arsohr_rec, NEXT, "r");
	}
	if (noRecordsFound)
		tab_add (archiveTab, " No Records Found ");
}
/*
 * Read Invoices from Disk file.
 */
void
DiskInvoices (void)
{
	char	arhrFileName	[255];
	char	fileName		[255];

	FILE	*fp;

	noRecordsFound	=	TRUE;

	strcpy (fileName, clip (filePath));
	strcat (fileName, clip (local_rec.filename));
	sprintf (arhrFileName, "%s.arhr.dat", fileName);

	/*
	 * Open archive file to reading Archive Header information.
	 */
	if ((fp = fopen (arhrFileName, "r")) == NULL)
	{
		tab_add (archiveTab, " No Records Found ");
		return;
	}

	while (!ArhrRead (fp, &arhr_rec))
	{
		cumr_rec.hhcu_hash	=	arhr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (cumr_rec.dbt_no, " ");
			strcpy (cumr_rec.dbt_name, 	 " ");
		}
		sprintf (err_str, "%s-%s", cumr_rec.dbt_no, cumr_rec.dbt_name);
		noRecordsFound	=	FALSE;
		/*
  	  	 * Add entry to tab screen.
 	 	 */
		tab_add 
		(
			archiveTab, 
			"%s|%-20.20s|%10.10s|%60.60s|%20.20s I %010ld",
			"*",
			arhr_rec.inv_no,
			DateToString (arhr_rec.date_raised),
			err_str,
			ArchiveMsg.steady, 
			arhr_rec.hhco_hash
		);
		tab_get (archiveTab, bufferData, EQUAL, noInTab);
		redraw_line (archiveTab, TRUE);
		noInTab++;
	}
	fclose (fp);

	if (noRecordsFound)
		tab_add (archiveTab, " No Records Found ");

	return;
}
/*
 * Read Invoices from Archive file.
 */
void
ProcessInvoices (void)
{
	noRecordsFound	=	TRUE;

	strcpy (arhr_rec.co_no, comm_rec.co_no);
	strcpy (arhr_rec.br_no, "  ");
	strcpy (arhr_rec.type,  " ");
	strcpy (arhr_rec.inv_no, " ");
	cc = find_rec (arhr, &arhr_rec, GTEQ, "r");
	while (!cc && !strcmp (arhr_rec.co_no, comm_rec.co_no))
	{
		if (arhr_rec.date_raised < local_rec.startDate ||
			arhr_rec.date_raised > local_rec.endDate)
		{
			cc = find_rec (arhr, &arhr_rec, NEXT, "r");
			continue;
		}
		noRecordsFound	=	FALSE;
		cumr_rec.hhcu_hash	=	arhr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (cumr_rec.dbt_no, " ");
			strcpy (cumr_rec.dbt_name, 	 " ");
		}
		sprintf (err_str, "%s-%s", cumr_rec.dbt_no, cumr_rec.dbt_name);
		/*
  	  	 * Add entry to tab screen.
 	 	 */
		tab_add 
		(
			archiveTab, 
			"%s|%-20.20s|%10.10s|%60.60s|%20.20s I %010ld",
			"*",
			arhr_rec.inv_no,
			DateToString (arhr_rec.date_raised),
			err_str,
			ArchiveMsg.ready,
			arhr_rec.hhco_hash
		);
		tab_get (archiveTab, bufferData, EQUAL, noInTab);
		redraw_line (archiveTab, TRUE);
		noInTab++;

		cc = find_rec (arhr, &arhr_rec, NEXT, "r");
	}
	if (noRecordsFound)
		tab_add (archiveTab, " No Records Found ");
}
/*
 * Read Works Orders from Disk file.
 */
void
DiskWorksOrders (void)
{
	char	arpcwoFileName	[255];
	char	fileName		[255];

	FILE	*fp;

	noRecordsFound	=	TRUE;

	strcpy (fileName, clip (filePath));
	strcat (fileName, clip (local_rec.filename));
	sprintf (arpcwoFileName, "%s.arpcwo.dat", fileName);

	/*
	 * Open archive file to reading Archive Header information.
	 */
	if ((fp = fopen (arpcwoFileName, "r")) == NULL)
	{
		tab_add (archiveTab, " No Records Found ");
		return;
	}

	while (!ArPcwoRead (fp, &arpcwo_rec))
	{
		inmr_rec.hhbr_hash	=	arpcwo_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (inmr_rec.item_no, " ");
			strcpy (inmr_rec.description, 	 " ");
		}
		sprintf (err_str, "%s-%s", inmr_rec.item_no, inmr_rec.description);
		noRecordsFound	=	FALSE;
		/*
  	  	 * Add entry to tab screen.
 	 	 */
		tab_add 
		(
			archiveTab, 
			"%s|%-20.20s|%10.10s|%60.60s|%20.20s W %010ld",
			"*",
			arpcwo_rec.order_no,
			DateToString (arpcwo_rec.reqd_date),
			err_str,
			ArchiveMsg.steady, 
			arpcwo_rec.hhwo_hash
		);
		tab_get (archiveTab, bufferData, EQUAL, noInTab);
		redraw_line (archiveTab, TRUE);
		noInTab++;
	}
	fclose (fp);

	if (noRecordsFound)
		tab_add (archiveTab, " No Records Found ");

	return;
}
/*
 * Read Works Orders from Archive file.
 */
void
ProcessWorksOrders (void)
{
	noRecordsFound	=	TRUE;

	strcpy (arpcwo_rec.co_no, comm_rec.co_no);
	strcpy (arpcwo_rec.br_no, "  ");
	strcpy (arpcwo_rec.wh_no, "  ");
	strcpy (arpcwo_rec.order_no,  " ");
	cc = find_rec (arpcwo, &arpcwo_rec, GTEQ, "r");
	while (!cc && !strcmp (arpcwo_rec.co_no, comm_rec.co_no))
	{
		if (arpcwo_rec.create_date < local_rec.startDate ||
			arpcwo_rec.create_date > local_rec.endDate)
		{
			cc = find_rec (arpcwo, &arpcwo_rec, NEXT, "r");
			continue;
		}
		noRecordsFound	=	FALSE;
		inmr_rec.hhbr_hash	=	arpcwo_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (inmr_rec.item_no, " ");
			strcpy (inmr_rec.description, " ");
		}
		sprintf (err_str, "%s-%s", inmr_rec.item_no, inmr_rec.description);
		/*
  	  	 * Add entry to tab screen.
 	 	 */
		tab_add 
		(
			archiveTab, 
			"%s|%-20.20s|%10.10s|%60.60s|%20.20s W %010ld",
			"*",
			arpcwo_rec.order_no,
			DateToString (arpcwo_rec.reqd_date),
			err_str,
			ArchiveMsg.ready,
			arpcwo_rec.hhwo_hash
		);
		tab_get (archiveTab, bufferData, EQUAL, noInTab);
		redraw_line (archiveTab, TRUE);
		noInTab++;

		cc = find_rec (arpcwo, &arpcwo_rec, NEXT, "r");
	}
	if (noRecordsFound)
		tab_add (archiveTab, " No Records Found ");
}
/*
 * Process tagged transactions. 
 */
void
ProcessAll (void)
{
	char	fileName	[256];
	char	processType	[2];
	long	processHash;
	int		archiveError	=	0;

	int		i;
	
	strcpy (fileName, clip (filePath));
	strcat (fileName, clip (local_rec.filename));

	/*
	 * Scan what data has been addded.
	 */
	cc = tab_scan (archiveTab);
	if (cc)
		return;
	
	/*
	 * Processes each line in list.
	 */
	for (i = 0; i < noInTab ; i++)
	{
		redraw_line (archiveTab, FALSE);
		tab_get (archiveTab, bufferData, EQUAL, i);
		/*
		 * Processes tagged lines. 
		 */
		if (tagged (bufferData))
		{	
			sprintf (processType, "%-1.1s", bufferData + 116);
			processHash	=	atol (bufferData + 118);

			/*
			 * Processes purchase orders.
			 */
			if (processType [0] == 'P')
			{
				if (local_rec.loadType [0] == 'D')
					archiveError = DownloadDeletePO (fileName, processHash);
				else
					archiveError = UploadPO (fileName, processHash);
			}
			/*
			 * Processes Invoices.
			 */
			if (processType [0] == 'I')
			{
				if (local_rec.loadType [0] == 'D')
					archiveError = DownloadDeleteIN (fileName, processHash);
				else
					archiveError = UploadIN (fileName, processHash);
			}
			/*
			 * Processes Sales Orders.
			 */
			if (processType [0] == 'S')
			{
				if (local_rec.loadType [0] == 'D')
					archiveError = DownloadDeleteSO (fileName, processHash);
				else
					archiveError = UploadSO (fileName, processHash);
			}
			/*
			 * Processes Works Orders.
			 */
			if (processType [0] == 'W')
			{
				if (local_rec.loadType [0] == 'D')
					archiveError = DownloadDeleteWO (fileName, processHash);
				else
					archiveError = UploadWO (fileName, processHash);
			}

			/*
			 * Processes messages.
			 */
			if (archiveError == ArchiveErr_Ok)
			{
				if (local_rec.loadType [0] == 'D')
					strcpy (err_str, ArchiveMsg.success);
				else
					strcpy (err_str, ArchiveMsg.restore);
			}
			if (archiveError == ArchiveOpenError)
				strcpy (err_str, ArchiveMsg.open);
			if (archiveError == ArchiveAddError)
				strcpy (err_str, ArchiveMsg.add);
			if (archiveError == ArchiveFindError)
				strcpy (err_str, ArchiveMsg.find);
			if (archiveError == ArchiveErr_Failed)
				strcpy (err_str, ArchiveMsg.error);

			/*
			 * Update line with message.
			 */
			tab_update 
			(
				archiveTab, 
				"%-95.95s%-20.20s %-s",
				bufferData,
				err_str,
				bufferData + 96
			);
			HeadingPrint ();
			fprintf (fout, "%-94.94s%-20.20s|\n", bufferData + 1, err_str);
			/*
	 		* Tag line as containing an error.
	 		*/
			if (archiveError)
				tag_set (archiveTab);
			else
				tag_unset (archiveTab);
			redraw_line (archiveTab, TRUE);
		}
		else
		{
			/*
			 * Update line with message.
			 */
			tab_update 
			(
				archiveTab, 
				"%-95.95s%-20.20s %-s",
				bufferData,
				ArchiveMsg.none,
				bufferData + 96
			);
			redraw_line (archiveTab, TRUE);
		}
	}
	/*
	 * Some records found to update control files.
	 */
	if (noRecordsFound == FALSE)
	{
		tab_scan (archiveTab);
		if (local_rec.loadType [0] == 'D')
		{
			strcpy (arct_rec.co_no, comm_rec.co_no);
			strcpy (arct_rec.filename, local_rec.filename);
			sprintf (arct_rec.op_id, "%-14.14s", currentUserName);
			strcpy (arct_rec.time_create, TimeHHMM ());
			arct_rec.date_create = TodaysDate ();
			strcpy (arct_rec.stat_flag, "0");
			cc = abc_add (arct, &arct_rec);
			if (cc)
				file_err (cc, arct, "DBADD");
		}
		if (local_rec.loadType [0] == 'U')
		{
			strcpy (arct_rec.co_no, comm_rec.co_no);
			strcpy (arct_rec.filename, local_rec.filename);
			cc = find_rec (arct, &arct_rec, COMPARISON, "u");
			if (!cc)
			{
				cc = abc_delete (arct);
				if (cc)
					file_err (cc, arct, "DBDELETE");
			}
		}
	}
}
/*
 *   Initialise screen output.
 */
void
InitDisplay (void)
{
	crsr_off ();
	noInTab	=	0;
	tab_open (archiveTab, listKeys, 5, 7, 11, FALSE);
	tab_add (archiveTab, "#%-s", "S|   Document Number  | Doc Date |                Description                                 |       Status       ");
}

/*
 * Restart key processing function.
 */
static	int
RestartFunc (
	int 		key, 
	KEY_TAB 	*psUnused)
{
	restart = TRUE;
	return key;
}

/*
 * Exit key processing function.
 */
static	int
ExitFunc (
	int 		key, 
	KEY_TAB 	*psUnused)
{
	return key;
}

/*
 * Tag all function.
 */
static int 
TagAllFunc (
	int 	c, 
	KEY_TAB *psUnused)
{
	tag_all (archiveTab);
    return (EXIT_SUCCESS);
}
/*
 * UnTag all function.
 */
static int 
TagFunc (
	int 	c, 
	KEY_TAB *psUnused)
{
	tag_toggle (archiveTab);
    return (EXIT_SUCCESS);
}

/*
 * Report heading for Audit. 
 */
void
HeadingPrint (void)
{
	if (printerOutputOpen == TRUE)
		return;

	printerOutputOpen	=	TRUE;

	if ((fout = popen ("pformat","w")) == NULL)
		file_err (errno, "pformat", PNAME);

	fprintf (fout, ".START%s\n",DateToString (TodaysDate ()));
	fprintf (fout, ".LP%d\n",printerNumber);
	fprintf (fout, ".SO\n");

	fprintf (fout, ".12\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L130\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E SYSTEM ARCHIVE SELECTION AND PROCESSING AUDIT\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s - %s\n", comm_rec.co_no, comm_rec.co_name);
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EAS AT %-24.24s\n",SystemTime ());

	fprintf (fout, ".R===================================================================================================================\n");

	fprintf (fout, "===================================================================================================================\n");

	fprintf (fout, "|   DOCUMENT NUMBER  |   DATE   |                DESCRIPTION                                 |       STATUS       |\n");

	fprintf (fout, "|--------------------|----------|------------------------------------------------------------|--------------------|\n");

	fflush (fout);
}
void
InitML (void)
{
	strcpy (ArchiveMsg.success, ML (" Archive Successful "));
	strcpy (ArchiveMsg.restore, ML (" Restore Successful "));
	strcpy (ArchiveMsg.open, 	ML ("Error : Open Archive"));
	strcpy (ArchiveMsg.add, 	ML ("Error : Add Archive "));
	strcpy (ArchiveMsg.find, 	ML ("Error : Find Record "));
	strcpy (ArchiveMsg.error, 	ML ("Error : Fatal       "));
	strcpy (ArchiveMsg.none, 	ML (" Record unchanged.  "));
	strcpy (ArchiveMsg.ready, 	ML (" Ready to Archive.  "));
	strcpy (ArchiveMsg.steady, 	ML (" Ready to restore.  "));
}
int
heading (
 int    scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	swide ();

	strcpy (err_str, ML ("System Archive Selection and Processing"));
	rv_pr (err_str, (132 - strlen (err_str)) / 2,0,1);

	box (0, 2, 130, 2);		
	line_at (1,0,132);
	line_at (21,0,132);

	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);


	line_cnt = 0;

	scn_write (S_HEAD);
    return (EXIT_SUCCESS);
}
