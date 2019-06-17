/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: MenuDeleteControl.c,v 5.0 2002/05/07 10:03:04 scott Exp $
|  Program Name  : (menuDeleteControl.c)
|  Program Desc  : (Menu system control program for deletes.
|---------------------------------------------------------------------|
| $Log: MenuDeleteControl.c,v $
| Revision 5.0  2002/05/07 10:03:04  scott
| Updated to ensure version numbers on new programs are correct.
|
| Revision 1.7  2002/01/23 09:11:23  scott
| S/C 00713 - When prompting for the User-defined number of days paid transactions are held in customer ledger, the blank where user will enter the number is not positioned in the right location.  The error was observed in boy character-based and in GUI.
|
| Revision 1.6  2001/11/21 08:49:27  scott
| Updated lineup
|
| Revision 1.5  2001/09/04 08:34:03  scott
| Updated to add ability to abort, only works if fields being changed.
|
| Revision 1.4  2001/08/28 10:12:01  robert
| additional update for LS10.5-GUI
|
| Revision 1.3  2001/08/28 00:23:17  scott
| Updated from scotts machine - ongoing WIP release 10.5
|
| Revision 1.2  2001/08/20 23:40:39  scott
| Updated for development related to bullet proofing
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: MenuDeleteControl.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/MenuDeleteControl/MenuDeleteControl.c,v 5.0 2002/05/07 10:03:04 scott Exp $";

#define	MAX_DELETE_PROGRAMS		20

#include <pslscr.h>	
#include <hot_keys.h>
#include <tabdisp.h>
#include <ml_std_mess.h>
#include <ml_gl_mess.h>
#include <DeleteControl.h>

#include	"schema"

#define		NO_TRAN_TYPES	12

#define		RULE_SYSTEM		1
#define		RULE_DATE		2
#define		RULE_AUDIT		3
#define		RULE_CUIN 		4
#define		RULE_MIN_STRAN 	5
#define		RULE_MIN_UTRAN 	6
#define		RULE_TRANTYPE 	7

struct commRecord	comm_rec;

	char	*data			= "data";

	static	int		dataChanged	=	FALSE,
					currLine	=	0,
					inputColumn	=	57,
					inputRow	=	5,
					restartAll	=	FALSE;

	static	char	yesPrompt		[11],
					noPrompt		[11],
					canPrompt		[11],
					cannotPrompt	[11];

	static char *transactionDesc [] =
	{
		"Stock balances",
		"Stock receipts",
		"Stock issues",
		"Stock adjustments",
		"Stock purchases",
		"Invoices",
		"Credits ",
		"Production issues.",
		"Stock Transfers",
		"Production receipts",
		"Stock write offs",
		"Direct delivery purchase",
		"Direct delivery invoice."
	};

/*===========================================
| The structure envVar groups the values of |
| environment settings together.            |
===========================================*/
struct tagEnvVar
{
	int		crPurge;
	int		dbGrpPurge;
	int		poPurge;
	int		dbPurge;
	int		skLocDaysDel;
	char	skTranDelEx [21];
	char	soArchive [2];
	int		soRtDelDays;
} envVar;

/*===========================================
| The structure delVar groups the values of |
| environment settings together.            |
===========================================*/
struct tagDelete
{
	char	code	[21];
	char	desc1	[71];
	char	desc2	[71];
	int		days;
	char	ref [21];
	int		spare_1;
	int		spare_2;
	int		spare_3;
} delRec [MAX_DELETE_PROGRAMS];

char	dispLine [256];

static	void	ProcessStandard 	(int);
static	void	ProcessInvoice 		(int);
static	void	ProcessInventory 	(int);
static	void	ProcessCustomer 	(int);
static	void	CheckEnvironment 	(void);
static	void	LoadDelh 			(void);
static	void	UpdateDelh 			(void);
static	int 	Process				(void);
static	int		StandardFunc		(char *);
void			OpenDB				(void);
void			CloseDB				(void);
void			shutdown_prog		(void);
void 			ProcessHeading 		(void);
/*
 * Main Processing Routine.
 */
int
main (
	int		argc,
	char	*argv [])
{
	/*
	 * Check environment variables and set values in the envVar structure.
	 */
	CheckEnvironment ();
	
	OpenDB ();

    init_scr ();
    set_tty ();

	/*
	 * Setup common prompts to avoid ML being called multiple times.
	 */
	strcpy (yesPrompt, 		ML ("Yes"));
	strcpy (noPrompt,  		ML ("No "));
	strcpy (canPrompt, 		ML ("can"));
	strcpy (cannotPrompt,	ML ("cannot"));	

	/*
	 * Draw what is required for header.
	 */
   	ProcessHeading ();

	/*
	 * Main processing routine.
	 */
	Process ();

	/*
	 * Program exit sequence.
	 */
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
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec); 
	open_rec (delh, delh_list, DELH_NO_FIELDS, "delh_id_no");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (delh);
	abc_dbclose (data);
}

/*
 * Heading function.
 */
void
ProcessHeading (void)
{
	int	s_size = 80;

	clear ();

	line_at (20,0,s_size);
	line_at (22,0,s_size);

	print_at (21,0, ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
}


/*
 * Check environment variables and set values in the envVar structure.
 */
void
CheckEnvironment (void)
{
	char	*sptr;

	sptr = chk_env ("CR_PURGE");
	envVar.crPurge = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("PURGE_MON");
	envVar.dbPurge = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("DB_GRP_PURGE");
	envVar.dbGrpPurge = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("PO_PURGE");
	envVar.poPurge = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr	=	chk_env ("SK_TRAN_DEL_EX");
	if (sptr == (char *)0)
		sprintf (envVar.skTranDelEx, "%-20.20s", "YYYYYYYYYYYYYYYYYYYY");
	else
		sprintf (envVar.skTranDelEx, "%-20.20s", sptr);

	sptr	=	chk_env ("SO_ARCHIVE");
	if (sptr == (char *)0)
		sprintf (envVar.soArchive, "%-1.1s", "N");
	else
		sprintf (envVar.soArchive, "%-1.1s", sptr);

	sptr = chk_env ("SO_RT_DEL_DAYS");
	envVar.soRtDelDays = (sptr == (char *)0) ? 0 : atoi (sptr);
}


/*
 * Main processing routine.
 */
static	int
Process	(void)
{
	/*
  	 * Load lines from file.  
 	 */
	LoadDelh ();

	/*
  	 * Process each Code as defined in DeleteControl.h
 	 */
	for (currLine = 0;strlen (deleteHeader [currLine].delCode);currLine++)
	{
		switch (deleteHeader [currLine].keyStructure)
		{
			/*
  	 		 * Process standard options. 
 	 		 */
			case 	1:
				inputColumn	=	57;
				inputRow	=	5;
				ProcessStandard (currLine);
				break;

			/*
  	 		 * Process specific options for invoices.
 	 		 */
			case 	2:
				inputColumn	=	66;
				inputRow	=	5;
				ProcessInvoice (currLine);
				break;

			/*
  	 		 * Process specific options for Inventory.
 	 		 */
			case 	3:
				inputColumn	=	66;
				inputRow	=	6;
				ProcessInventory (currLine);
				break;

			/*
  	 		 * Process specific options for Customer ledger.
 	 		 */
			case 	4:
				ProcessCustomer (currLine);
				break;
		}
		if (restartAll	==	TRUE)
			break;
				
		if (dataChanged == TRUE)
		{
			dataChanged = FALSE;
			currLine--;
			continue;
		}
	}
	UpdateDelh ();
	return (0);
}

static	void
LoadDelh (void)
{
	for (currLine = 0;strlen (deleteHeader [currLine].delCode);currLine++)
	{
		strcpy (delhRec.co_no, comm_rec.co_no);
		strcpy (delhRec.code, deleteHeader [currLine].delCode);
		cc = find_rec (delh, &delhRec, COMPARISON,"r");
		if (cc)
		{
			strcpy (delRec [currLine].code,  deleteHeader [currLine].delCode);
			strcpy (delRec [currLine].desc1, deleteHeader [currLine].userDesc1);
			strcpy (delRec [currLine].desc2, deleteHeader [currLine].userDesc2);
			strcpy (delRec [currLine].ref,   deleteHeader [currLine].reference);
			delRec [currLine].days		=	deleteHeader [currLine].purgeDays;
			delRec [currLine].spare_1	=	deleteHeader [currLine].spareFlag1;
			delRec [currLine].spare_2	=	deleteHeader [currLine].spareFlag2;
			delRec [currLine].spare_3	=	deleteHeader [currLine].spareFlag3;

			if (!strcmp (delRec [currLine].code, "SUPPLIER-LEDGER     "))
				delRec [currLine].days		=	envVar.crPurge;

			if (!strcmp (delRec [currLine].code, "CUSTOMER-LEDGER     "))
			{
				delRec [currLine].days		=	envVar.dbPurge;
				delRec [currLine].spare_1	=	envVar.dbGrpPurge;
			}

			if (!strcmp (delRec [currLine].code, "PO-RECEIPT-CLOSE    "))
				delRec [currLine].days		=	envVar.poPurge;

			if (!strcmp (delRec [currLine].code, "INVENTORY-TRANS     "))
				strcpy (delRec [currLine].ref, envVar.skTranDelEx);

			if (!strcmp (delRec [currLine].code, "SALES-ORDER-FILE    "))
				delRec [currLine].days		=	envVar.soRtDelDays;
		}
		else
		{
			strcpy (delRec [currLine].code,  delhRec.code);
			strcpy (delRec [currLine].desc1, delhRec.desc_1);
			strcpy (delRec [currLine].desc2, delhRec.desc_2);
			strcpy (delRec [currLine].ref, delhRec.reference);
			delRec [currLine].days		=	delhRec.purge_days;
			delRec [currLine].spare_1	=	delhRec.spare_fg1;
			delRec [currLine].spare_2	=	delhRec.spare_fg2;
			delRec [currLine].spare_3	=	delhRec.spare_fg3;
		}
	}
}
static	void
UpdateDelh (void)
{
	int		newDelh;
	for (currLine = 0;strlen (deleteHeader [currLine].delCode);currLine++)
	{
		strcpy (delhRec.co_no, comm_rec.co_no);
		strcpy (delhRec.code, delRec [currLine].code);
		newDelh = find_rec (delh, &delhRec, COMPARISON,"u");

		strcpy (delhRec.code, delRec [currLine].code);
		strcpy (delhRec.desc_1, delRec [currLine].desc1);
		strcpy (delhRec.desc_2, delRec [currLine].desc2);
		strcpy (delhRec.reference, delRec [currLine].ref);

		delhRec.purge_days	=	delRec [currLine].days;
		delhRec.spare_fg1	=	delRec [currLine].spare_1;
		delhRec.spare_fg2	=	delRec [currLine].spare_2;
		delhRec.spare_fg3	=	delRec [currLine].spare_3;

		if (newDelh)
			cc = abc_add (delh, &delhRec);
		else
			cc = abc_update (delh, &delhRec);
		if (cc)
			file_err (cc, delh, "DBADD/UPDATE");
	}
}
static	void
ProcessStandard (
	int		lineNo)
{
	int		spacePrompt	=	0,
			spaceLeft	=	0,
			spaceRight	=	0;
	

	Dsp_open (0,0,14);

	spacePrompt	=	strlen (clip (delRec [lineNo].desc1));
	spaceLeft	=	(80 - spacePrompt) / 2;
	spaceRight	=	78 - spacePrompt - spaceLeft;
	sprintf (err_str, "%*.*s%*.*s%*.*s", 
				spaceLeft, spaceLeft, " ",
				spacePrompt, spacePrompt, clip (delRec [lineNo].desc1),
				spaceRight, spaceRight, " ");
	Dsp_saverec (err_str);

	spacePrompt	=	strlen (clip (delRec [lineNo].desc2));
	spaceLeft	=	(80 - spacePrompt) / 2;
	spaceRight	=	78 - spacePrompt - spaceLeft;
	sprintf (err_str, "%*.*s%*.*s%*.*s", 
				spaceLeft, spaceLeft, " ",
				spacePrompt, spacePrompt, clip (delRec [lineNo].desc2),
				spaceRight, spaceRight, " ");
	Dsp_saverec (err_str);

	Dsp_saverec (ML ("[EDIT/END]"));

	sprintf
	(
		dispLine,
		"%-55.55s %04d",
		ML ("System defined number of days to hold transactions : "),
		deleteHeader [lineNo].purgeDays
	);
	Dsp_save_fn (dispLine, "1");

	sprintf
	(
		dispLine,
		"%-55.55s %04d",
		ML ("User   defined number of days to hold transactions : "),
		delRec [lineNo].days
	);
	Dsp_save_fn (dispLine, "2");

	Dsp_srch_fn (StandardFunc);
	Dsp_close ();
}

static	void
ProcessInvoice (
	int		lineNo)
{
	int		spacePrompt	=	0,
			spaceLeft	=	0,
			spaceRight	=	0;


	Dsp_open (0,0,14);

	spacePrompt	=	strlen (clip (delRec [lineNo].desc1));
	spaceLeft	=	(80 - spacePrompt) / 2;
	spaceRight	=	78 - spacePrompt - spaceLeft;
	sprintf (err_str, "%*.*s%*.*s%*.*s", 
				spaceLeft, spaceLeft, " ",
				spacePrompt, spacePrompt, clip (delRec [lineNo].desc1),
				spaceRight, spaceRight, " ");
	Dsp_saverec (err_str);

	spacePrompt	=	strlen (clip (delRec [lineNo].desc2));
	spaceLeft	=	(80 - spacePrompt) / 2;
	spaceRight	=	78 - spacePrompt - spaceLeft;
	sprintf (err_str, "%*.*s%*.*s%*.*s", 
				spaceLeft, spaceLeft, " ",
				spacePrompt, spacePrompt, clip (delRec [lineNo].desc2),
				spaceRight, spaceRight, " ");
	Dsp_saverec (err_str);

	Dsp_saverec (ML ("[EDIT/END]"));

	sprintf
	(
		dispLine,
		"%-64.64s %04d",
		ML ("System defined number of days to hold transactions            : "),
		deleteHeader [lineNo].purgeDays
	);
	Dsp_save_fn (dispLine, "1");

	sprintf
	(
		dispLine,
		"%-64.64s %04d",
		ML ("User   defined number of days to hold transactions            : "),
		delRec [lineNo].days
	);
	Dsp_save_fn (dispLine, "2");
	sprintf
	(
		dispLine,
		"%-64.64s %-10.10s",
		ML ("Invoices can be deleted without invoice audit being run       : "),
		(delRec [lineNo].ref [0] == 'D') ? yesPrompt : noPrompt
	);
	Dsp_save_fn (dispLine, "3");

	sprintf
	(
		dispLine,
		"%-64.64s %-10.10s",
		ML ("Invoices must have complete invoice audit before being purged : "),
		(delRec [lineNo].ref [0] == '9') ? yesPrompt : noPrompt
	);
	Dsp_save_fn (dispLine, "3");

	sprintf
	(
		dispLine,
		"%-64.64s %-10.10s",
		ML ("Invoices must have paid in full before that can be deleted.   : "),
		(delRec [lineNo].spare_1) ? yesPrompt : noPrompt
	);
	Dsp_save_fn (dispLine, "4");
	Dsp_srch_fn (StandardFunc);
	Dsp_close ();
}

static	void
ProcessInventory (
	int		lineNo)
{
	int		spacePrompt	=	0,
			spaceLeft	=	0,
			spaceRight	=	0,
			i			=	0;
	

	Dsp_open (0,0,14);

	spacePrompt	=	strlen (clip (delRec [lineNo].desc1));
	spaceLeft	=	(80 - spacePrompt) / 2;
	spaceRight	=	78 - spacePrompt - spaceLeft;
	sprintf (err_str, "%*.*s%*.*s%*.*s", 
				spaceLeft, spaceLeft, " ",
				spacePrompt, spacePrompt, clip (delRec [lineNo].desc1),
				spaceRight, spaceRight, " ");
	Dsp_saverec (err_str);

	spacePrompt	=	strlen (clip (delRec [lineNo].desc2));
	spaceLeft	=	(80 - spacePrompt) / 2;
	spaceRight	=	78 - spacePrompt - spaceLeft;
	sprintf (err_str, "%*.*s%*.*s%*.*s", 
				spaceLeft, spaceLeft, " ",
				spacePrompt, spacePrompt, clip (delRec [lineNo].desc2),
				spaceRight, spaceRight, " ");
	Dsp_saverec (err_str);

	Dsp_saverec (ML ("[EDIT/END]"));

	sprintf
	(
		dispLine,
		"%-64.64s %04d",
		ML ("System defined number of days to hold transactions            : "),
		deleteHeader [lineNo].purgeDays
	);
	Dsp_save_fn (dispLine, "1");

	sprintf
	(
		dispLine,
		"%-64.64s %04d",
		ML ("System defined minimum number of transactions to retain       : "),
		deleteHeader [lineNo].spareFlag1
	);
	Dsp_save_fn (dispLine, "5");

	sprintf
	(
		dispLine,
		"%-64.64s %04d",
		ML ("User   defined number of days to hold transactions            : "),
		delRec [lineNo].days
	);
	Dsp_save_fn (dispLine, "2");

	sprintf
	(
		dispLine,
		"%-64.64s %04d",
		ML ("User   defined minimum number of transactions to retain       : "),
		delRec [lineNo].spare_1
	);
	Dsp_save_fn (dispLine, "6");

	for (i = 0; i < NO_TRAN_TYPES; i++)
	{
		sprintf 
		(
			dispLine,
			"Transaction for (%-25.25s) %s be deleted",
			transactionDesc [i],
			(delRec [lineNo].ref [i] == 'Y') ? canPrompt : cannotPrompt
		);
		sprintf (err_str, "%02d - %02d", 7, i);
		Dsp_save_fn (dispLine, err_str);
	}
	Dsp_srch_fn (StandardFunc);
	Dsp_close ();
}

static	void
ProcessCustomer (
	int		lineNo)
{
	int		spacePrompt	=	0,
			spaceLeft	=	0,
			spaceRight	=	0;


	Dsp_open (0,0,14);

	spacePrompt	=	strlen (clip (delRec [lineNo].desc1));
	spaceLeft	=	(80 - spacePrompt) / 2;
	spaceRight	=	78 - spacePrompt - spaceLeft;
	sprintf (err_str, "%*.*s%*.*s%*.*s", 
				spaceLeft, spaceLeft, " ",
				spacePrompt, spacePrompt, clip (delRec [lineNo].desc1),
				spaceRight, spaceRight, " ");
	Dsp_saverec (err_str);

	spacePrompt	=	strlen (clip (delRec [lineNo].desc2));
	spaceLeft	=	(80 - spacePrompt) / 2;
	spaceRight	=	78 - spacePrompt - spaceLeft;
	sprintf (err_str, "%*.*s%*.*s%*.*s", 
				spaceLeft, spaceLeft, " ",
				spacePrompt, spacePrompt, clip (delRec [lineNo].desc2),
				spaceRight, spaceRight, " ");
	Dsp_saverec (err_str);

	Dsp_saverec (ML ("[EDIT/END]"));

	sprintf
	(
		dispLine,
		"%-55.55s %04d",
		ML ("System defined number of days to hold transactions : "),
		deleteHeader [lineNo].purgeDays
	);
	Dsp_save_fn (dispLine, "1");

	sprintf
	(
		dispLine,
		"%-55.55s %04d",
		ML ("User   defined number of days to hold transactions : "),
		delRec [lineNo].days
	);
	Dsp_save_fn (dispLine, "2");
	sprintf
	(
		dispLine,
		"%-55.55s %-10.10s",
		ML ("Invoices are deleted based group of transactions   : "),
		(delRec [lineNo].spare_1) ? yesPrompt : noPrompt
	);
	Dsp_save_fn (dispLine, "4");
	Dsp_srch_fn (StandardFunc);
	Dsp_close ();
}

static int
StandardFunc (
	char	*keyUsed)
{
	int		ruleCode	=	0;
	int		inputQty	=	0;
	int		pos			=	0;

	ruleCode	=	atoi (keyUsed);

	switch	(ruleCode)
	{
		case	RULE_SYSTEM:
			delRec [currLine].days	=	deleteHeader [currLine].purgeDays;
			dataChanged = TRUE;
			return (FN16);
		break;
	
		case	RULE_DATE:
			while (1)
			{
				crsr_on ();
				inputQty	= delRec [currLine].days;
				inputQty	= getint (inputColumn, inputRow, "NNNN");
				crsr_off ();
			
				if (last_char == FN2)
					inputQty	= delRec [currLine].days;

				if (last_char == FN1)
				{
					restartAll	=	TRUE;
					break;
				}
	
				if (last_char == FN16)
					continue;
	
				if (inputQty == 0.00)
				{
					print_mess (ML ("Quantity must be input."));
					sleep (sleepTime);
					continue;
				}
				delRec [currLine].days	=	inputQty;
				break;
			}
			dataChanged = TRUE;
			return (FN16);
		break;

		case	RULE_AUDIT:
			if (delRec [currLine].ref [0] == '9')
				strcpy (delRec [currLine].ref, "D");
			else
				strcpy (delRec [currLine].ref, "9");
			dataChanged = TRUE;
			return (FN16);
			break;

		case	RULE_CUIN:
			if (delRec [currLine].spare_1)
				delRec [currLine].spare_1	=	0;
			else
				delRec [currLine].spare_1	=	1;
			dataChanged = TRUE;
			return (FN16);
		break;

		case	RULE_MIN_STRAN:
			delRec [currLine].spare_1	=	deleteHeader [currLine].spareFlag1;
			dataChanged = TRUE;
			return (FN16);
		break;

		case	RULE_MIN_UTRAN:
			inputColumn	=	66;
			inputRow	=	7;
			while (1)
			{
				crsr_on ();
				inputQty	= delRec [currLine].spare_1;
				inputQty	= getint (inputColumn, inputRow, "NNNN");
				crsr_off ();
			
				if (last_char == FN2)
					inputQty	= delRec [currLine].spare_1;

				if (last_char == FN1)
				{
					restartAll	=	TRUE;
					break;
				}
	
				if (last_char == FN16)
					continue;
	
				if (inputQty == 0.00)
				{
					print_mess (ML ("Quantity must be input."));
					sleep (sleepTime);
					continue;
				}
				delRec [currLine].spare_1	=	inputQty;
				break;
			}
			dataChanged = TRUE;
			return (FN16);
		break;

		case	RULE_TRANTYPE:
			pos	=	atoi (keyUsed + 5);
			if (delRec [currLine].ref [pos] == 'N')
				delRec [currLine].ref [pos] = 'Y';
			else
				delRec [currLine].ref [pos] = 'N';
			dataChanged = TRUE;
			return (FN16);
		break;
	}
	return (0);
}
