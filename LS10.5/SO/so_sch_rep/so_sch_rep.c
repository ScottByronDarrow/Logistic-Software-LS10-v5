/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_sch_rep.c,v 5.3 2002/07/17 09:58:10 scott Exp $
|  Program Name  : (so_sch_rep.c)                                    |
|  Program Desc  : (Print Report of Scheduled Non-released      )   |	
|                  (Sales Orders By Cust / Item / Due Date      )   |
|---------------------------------------------------------------------|
|  Author        : Irfan Gohir     | Date Written  : 01/10/93         |
|---------------------------------------------------------------------|
| $Log: so_sch_rep.c,v $
| Revision 5.3  2002/07/17 09:58:10  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:22:04  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:52:02  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:20:38  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/03/23 01:20:13  scott
| Updated to ensure "0xff" is used instead of "~" for end of range.
|
| Revision 4.1  2001/03/15 05:58:28  scott
| Updated to fix prompts not compatable with LS10-GUI.
|
| Revision 4.0  2001/03/09 02:41:55  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/03/08 02:12:16  scott
| Updated to increase the delivery address number from 0-999 to 0-32000
| This change did not require a change to the schema
| As a general practice all programs have had app.schema added and been cleaned
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_sch_rep.c,v $",
		*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_sch_rep/so_sch_rep.c,v 5.3 2002/07/17 09:58:10 scott Exp $";

#include <pslscr.h>	
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_so_mess.h>
#include <ml_std_mess.h>

#define	LOWER	 (label ("startItem"))
#define	UPPER	 (label ("endItem"))
#define	SLEEP_TIME	2

#define	ITEM	0
#define	BRANCH	1

#include	"schema"

struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr_rec2;
struct cudiRecord	cudi_rec;
struct sohrRecord	sohr_rec;
struct sohrRecord	sohr_rec2;
struct solnRecord	soln_rec;
struct solnRecord	soln_rec2;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr_rec2;

	FILE	*fout;
	FILE	*fsort;

	char	noPrompt  [11],
			yesPrompt [11];
	char	*data  = "data",
			*cumr2 = "cumr2",
			*inmr2 = "inmr2",
			*sohr2 = "sohr2",
            *soln2 = "soln2";


/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	startCustNo [7];
	char	startCustName [41];
	char	endCustNo [7];
	char	endCustName [41];
	char	systemDate [11];
	char	firstDate [11];
	char	lastDate [11];
	char	startItem [17];
	char	startItemDesc [41];
	char	endItem [17];
	char	endItemDesc [41];
	long	startDate;
	long	endDate;
	int		printerNumber;
	char	printerString [3];
	char 	back [2];
	char 	backDesc [11];
	char	onite [2];
	char	oniteDesc [11];
} local_rec;

struct {
	char	custNo [7];
	char	custName [41];
	char	order_no [9];
	char	itemNo [17];
	char	itemDesc [41];
	float	qtyOrder;
	long	longDueDate;
	char	dueDate [11];
	char	deliveryAddress [82];
} data_rec;

static	struct	var	vars [] =
{
	{1, LIN, "startCustNo",	 4, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "      ", "Start Customer   ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startCustNo},
	{1, LIN, "startCustName",	 4, 32, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startCustName},
	{1, LIN, "endCustNo",	 5, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "~~~~~~", "End Customer     ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endCustNo},
	{1, LIN, "endCustName",	 5, 32, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endCustName},
	{1, LIN, "printerNumber",	7, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No.      ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber},
	{1, LIN, "back",	8, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background       ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backDesc", 8, 22, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "(", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc}, 
	{1, LIN, "onite",9, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight        ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{1, LIN, "oniteDesc", 9, 22, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "(", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.oniteDesc}, 

	{2, LIN, "startItem",	 4, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Start Item       ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startItem},
	{2, LIN, "startItemDesc",	 4, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startItemDesc},
	{2, LIN, "endItem",	 5, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "End Item         ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endItem},
	{2, LIN, "endItemDesc", 5, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endItemDesc},
	{2, LIN, "printerNumber",	7, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number   ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber},
	{2, LIN, "back",	8, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background       ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{2, LIN, "backDesc", 8, 22, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "(", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc}, 
	{2, LIN, "onite",9, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight        ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{2, LIN, "oniteDesc", 9, 22, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "(", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.oniteDesc}, 
	{3, LIN, "startDate",	 4, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.firstDate, "Start Date       ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.startDate},
	{3, LIN, "endDate",	 5, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.lastDate, "End   Date       ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.endDate},
	{3, LIN, "printerNumber",	7, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number   ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber},
	{3, LIN, "back",	8, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background       ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{3, LIN, "backDesc", 8, 22, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "(", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc}, 
	{3, LIN, "onite",9, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight        ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{3, LIN, "oniteDesc", 9, 22, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "(", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.oniteDesc}, 

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/* Global variables declared here */

extern	int	TruePosition;
extern	int	EnvScreenOK;
int		scn_no;
char	rep_type [2];
char	rep_title [61];
char	sdate [11];
char	edate [11];
char	branchNo [3];
int		envDbFind = FALSE;
int		end_dte_flag = FALSE;
int	    next_page = TRUE;
int	    first_time = TRUE;
float	total;
long	todays_date;
char	data_str [159];
char 	*sptr;


#include <FindCumr.h>
/*=======================
| Function Declarations |
=======================*/
int  	FindRange 			(int, char *, char *);
int  	GetCustomerDetails 	(void);
int  	GetDateOrderDetails (void);
int  	GetItemDetails 		(void);
int  	ReportType 			(void);
int  	ValidateInput 		(char *, long);
int  	heading 			(int);
int  	spec_valid 			(int);
void 	BuildSortFile 		(void);
void 	CloseDB 			(void);
void 	CopyArguments 		(char *, char *, char *);
void 	GetCustomerOrders 	(void);
void 	GetDeliveryAddr 	(void);
void 	GetOrderLines 		(void);
void 	GetProdOrderLines 	(void);
void 	GetProdOrders 		(void);
void 	HeaderOutput 		(void);
void 	OpenDB 				(void);
void 	PrintCustomerHeader (void);
void 	PrintCustomerLines 	(void);
void 	PrintDateLines 		(void);
void 	PrintProductHeader 	(void);
void 	PrintProductLines 	(void);
void 	ReportPrint 		(void);
void 	PrintTotal 			(void);
void 	ProcessByCustomer 	(void);
void 	ProcessByDate 		(void);
void 	ProcessByItem 		(void);
void 	ProcessReport 		(void);
void 	ReadMisc 			(void);
void 	RunProgram 			(char *, char *);
void 	SetDefaults 		(void);
void 	shutdown_prog 		(void);

	int		envVarDbCo		=	0;
	char	branchNumber [3];

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 3 && argc != 5)
	{
		print_at (0,0,mlSoMess726, argv [0]);
		print_at (1,0,mlSoMess727, argv [0]);
		return (EXIT_FAILURE);
	}
	else
		strncpy (rep_type, argv [1], 1);

	if (ReportType ())
        return (EXIT_FAILURE);    

	TruePosition	=	TRUE;
	EnvScreenOK		=	FALSE;

	SETUP_SCR (vars);

	envDbFind = atoi (get_env ("DB_FIND"));

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();		/*  sets terminal from termcap	*/
	set_tty ();		/*  get into raw mode		*/
	set_masks ();		/*  setup print using masks	*/
	swide ();

	OpenDB ();

	sptr = chk_env ("DB_CO");
	envVarDbCo = (sptr == (char *) 0) ? 0 : atoi (sptr);
	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");
	if (argc == 5)
	{
		CopyArguments (argv [2], argv [3], argv [4]);
		ProcessReport ();
		HeaderOutput ();
		ReportPrint ();
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------
	| Reset control flags |
	---------------------*/
	entry_exit 	= FALSE;
	edit_exit 	= FALSE;
	prog_exit 	= FALSE;
	search_ok 	= TRUE;
	restart 	= FALSE;
	init_vars (scn_no);		/*  set default values		*/

	SetDefaults ();

	/*----------------------------
	| Edit screen 1 linear input |
	----------------------------*/
	heading (scn_no);
		
	scn_display (scn_no);
	edit (scn_no);

    if (!restart)
        RunProgram (argv [0], argv [2]);

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	ReadMisc ();

	abc_alias (cumr2, cumr);
	abc_alias (inmr2, inmr);
	abc_alias (sohr2, sohr);
	abc_alias (soln2, soln);

	open_rec (cudi,  cudi_list, CUDI_NO_FIELDS, "cudi_id_no");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (sohr,  sohr_list, SOHR_NO_FIELDS, "sohr_id_no4");
	open_rec (sohr2, sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (soln,  soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (soln2, soln_list, SOLN_NO_FIELDS, "soln_hhbr_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (cudi);
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (sohr);
	abc_fclose (sohr2);
	abc_fclose (soln);
	abc_fclose (soln2);
	abc_dbclose (data);
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	strcpy (branchNo, (envVarDbCo) ? comm_rec.est_no : " 0" );
	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, comm_rec.est_no);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
	abc_fclose (ccmr);
}

int
ReportType (
 void)
{
	rep_title [0] = '\0';
	switch	 (toupper (rep_type [0]))
	{
	case	'C':
		strcpy (rep_type,"C");
		strcpy (rep_title, ML (mlSoMess035));
		scn_no = 1;
		break;

	case	'P':
		strcpy (rep_type,"P");
		strcpy (rep_title, ML (mlSoMess036));
		scn_no = 2;
		break;
	
	case	'D':
		strcpy (rep_type,"D");
		strcpy (rep_title, ML (mlSoMess037));
		scn_no = 3;
		break;

	default :
		print_at (0,0,"C(ustomer P(roduct D(ate only\n");
		return (EXIT_FAILURE);
	}
    return (EXIT_SUCCESS);
}

void
SetDefaults (
 void)
{
	switch	 (toupper (rep_type [0]))
	{
	case	'C':
		strcpy (local_rec.startCustNo, "      ");
		sprintf (local_rec.startCustName, "%-40.40s",  ML ("Start Customer"));
		strcpy (local_rec.endCustNo, "~~~~~~");
		sprintf (local_rec.endCustName, "%-40.40s", ML ("End Customer"));
		
		break;
		
	case	'P':
		strcpy (local_rec.startItem, "                ");
		sprintf (local_rec.startItemDesc, "%-40.40s", ML ("First Item"));
		strcpy (local_rec.endItem, "~~~~~~~~~~~~~~~~");
		sprintf (local_rec.endItemDesc, "%-40.40s",ML ("Last  Item"));
		
		break;

	case	'D':

	
		strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
		local_rec.startDate	= MonthStart (TodaysDate ());
		local_rec.endDate  	= TodaysDate ();
		todays_date 		= TodaysDate ();
		strcpy (local_rec.firstDate, DateToString (local_rec.startDate));
		strcpy (local_rec.lastDate,  DateToString (local_rec.endDate));

		break;
	}

	local_rec.printerNumber = 1;
	strcpy (local_rec.back, "N");
	strcpy (local_rec.onite, "N");
	sprintf (noPrompt,  "%-10.10s", ML ("NO"));
	sprintf (yesPrompt, "%-10.10s", ML ("YES"));
	strcpy (local_rec.backDesc, noPrompt);
	strcpy (local_rec.oniteDesc,noPrompt);
}

void
RunProgram (
 char *prog_name, 
 char *prog_desc)
{
	
	shutdown_prog ();

	strcpy (local_rec.firstDate, DateToString (local_rec.startDate));
	strcpy (local_rec.lastDate,  DateToString (local_rec.endDate));


	switch	 (toupper (rep_type [0]))
	{
	case	'C':
		if (!strcmp (local_rec.endCustNo, "~~~~~~"))
			memset ((char *)local_rec.endCustNo,0xff,sizeof (local_rec.endCustNo));
		if (local_rec.onite [0] == 'Y')
		{
			if (fork () == 0)
			{
				execlp ("ONIGHT",
					"ONIGHT",
					prog_name,
					rep_type,
					local_rec.startCustNo,
					local_rec.endCustNo,
					local_rec.printerString,
					prog_desc, (char *)0);
			}
		}
		else if (local_rec.back [0] == 'Y')
		{
			if (fork () == 0)
				execlp (prog_name,
					prog_name,
					rep_type,
					local_rec.startCustNo,
					local_rec.endCustNo,
					local_rec.printerString, (char *)0);
		}
		else 
		{
			execlp (prog_name,
				prog_name,
				rep_type,
				local_rec.startCustNo,
				local_rec.endCustNo,
				local_rec.printerString, (char *)0);
		}
		break;

	case	'P':
		if (!strcmp (local_rec.endItem, "~~~~~~~~~~~~~~~~"))
			memset ((char *)local_rec.endItem,0xff,sizeof (local_rec.endItem));

		if (local_rec.onite [0] == 'Y')
		{
			if (fork () == 0)
			{
				execlp ("ONIGHT",
					"ONIGHT",
					prog_name,
					rep_type,
					local_rec.startItem,
					local_rec.endItem,
					local_rec.printerString,
					prog_desc, (char *)0);
			}
		}
		else if (local_rec.back [0] == 'Y')
		{
			if (fork () == 0)
				execlp (prog_name,
					prog_name,
					rep_type,
					local_rec.startItem,
					local_rec.endItem,
					local_rec.printerString, (char *)0);
		}
		else 
		{
			execlp (prog_name,
				prog_name,
				rep_type,
				local_rec.startItem,
				local_rec.endItem,
				local_rec.printerString, (char *)0);
		}
		break;

	case	'D':
		if (local_rec.onite [0] == 'Y')
		{
			if (fork () == 0)
			{
				execlp ("ONIGHT",
					"ONIGHT",
					prog_name,
					rep_type,
					local_rec.firstDate,
					local_rec.lastDate,
					local_rec.printerString,
					prog_desc, (char *)0);
			}
		}
		else if (local_rec.back [0] == 'Y')
		{
			if (fork () == 0)
				execlp (prog_name,
					prog_name,
					rep_type,
					local_rec.firstDate,
					local_rec.lastDate,
					local_rec.printerString, (char *)0);
		}
		else 
		{
			execlp (prog_name,
				prog_name,
				rep_type,
				local_rec.firstDate,
				local_rec.lastDate,
				local_rec.printerString, (char *)0);
		}
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
	/*------------------------------------------
	| Validate Customer Number And Allow Search. |
	------------------------------------------*/
	if (LCHECK ("startCustNo"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startCustNo, "      ");
			sprintf (local_rec.startCustName, "%-40.40s", ML("Start Customer"));
			DSP_FLD ("startCustNo");
			DSP_FLD ("startCustName");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
  			return (EXIT_SUCCESS);
		}
		
		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.startCustNo));
		if (ValidateInput (local_rec.startCustNo, 0L))
			return (EXIT_FAILURE);

		strcpy (local_rec.startCustName, cumr_rec.dbt_name);
		DSP_FLD ("startCustName");
	}

	/*------------------------------------------
	| Validate Customer Number And Allow Search. |
	------------------------------------------*/
	if (LCHECK ("endCustNo"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endCustNo, "~~~~~~");
			sprintf (local_rec.endCustName, "%-40.40s", ML ("End Customer"));
			DSP_FLD ("endCustNo");
			DSP_FLD ("endCustName");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
  			return (EXIT_SUCCESS);
		}
		
		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.endCustNo));
		if (ValidateInput (local_rec.endCustNo, 0L))
			return (EXIT_FAILURE);

		if (strncmp (local_rec.startCustNo, local_rec.endCustNo, 6) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.endCustName, cumr_rec.dbt_name);
		DSP_FLD ("endCustName");
	}

	if (LCHECK ("startItem"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.startItem,"%-16.16s"," ");
			sprintf (local_rec.startItemDesc, "%-40.40s", ML ("First Item"));
			DSP_FLD ("startItem");
			DSP_FLD ("startItemDesc");
			return (EXIT_SUCCESS);
		}

		cc = FindRange (field, local_rec.startItem, local_rec.startItemDesc);

		return (cc);
	}

	if (LCHECK ("endItem")) 
	{
		if (dflt_used)
		{
			strcpy (local_rec.endItem,"~~~~~~~~~~~~~~~~");
			sprintf (local_rec.endItemDesc, "%-40.40s", ML ("Last Item"));
			DSP_FLD ("endItem");
			DSP_FLD ("endItemDesc");
			return (EXIT_SUCCESS);
		}

		cc = FindRange (field,local_rec.endItem,local_rec.endItemDesc);

		return (cc);
	}
	if (LCHECK ("startDate"))
	{
		end_dte_flag = FALSE;
		if (ValidateInput ((char *)NULL, local_rec.startDate))
			return (EXIT_FAILURE);
	}

	if (LCHECK ("endDate"))
	{
		end_dte_flag = TRUE;
		if (ValidateInput ((char *)NULL, local_rec.endDate))
			return (EXIT_FAILURE);
	}

	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.backDesc, (local_rec.back [0] == 'Y') ? yesPrompt
																: noPrompt);
		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onite"))
	{
		strcpy (local_rec.oniteDesc,(local_rec.onite [0] == 'Y') ? yesPrompt
																 : noPrompt);
		DSP_FLD ("oniteDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int 
ValidateInput (
	char 	*customerNumber, 
	long	dateValidate)
{
	int retval = FALSE;

	if (scn_no == 1)
	{
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			retval = TRUE;
		}
	}
	else
	{
		if (dateValidate > todays_date)
		{
			print_mess (ML (mlStdMess068));
			sleep (sleepTime);
			clear_mess ();
			retval = TRUE;
		}
			

		if ((local_rec.startDate > local_rec.endDate) && (end_dte_flag))
		{
			print_mess (ML (mlStdMess019));
			sleep (sleepTime);
			clear_mess ();
			retval = TRUE;
		}
		else if ((local_rec.endDate == 0) && (end_dte_flag))
		{
			print_mess (ML (mlStdMess111));
			sleep (sleepTime);
			clear_mess ();
			retval = TRUE;
		}
	}
	return (retval);
}

void
CopyArguments (
	char *arg3, 
	char *arg4, 
	char *arg5)
{
	switch	 (toupper (rep_type [0]))
	{
	case	'C':
		sprintf (local_rec.startCustNo, "%-6.6s", arg3);
		sprintf (local_rec.endCustNo,	"%-6.6s", arg4);
		break;

	case	'P':
		sprintf (local_rec.startItem, "%-16.16s", arg3);
		sprintf (local_rec.endItem,	  "%-16.16s", arg4);
		break;

	case	'D':
		local_rec.startDate = atol (arg3);
		local_rec.endDate 	= atol (arg4); 
		break;
	}

	sprintf (local_rec.printerString,"%-2.2s", arg5);
}

void
ProcessReport (
 void)
{

	fsort = sort_open ("datfile");
	
	switch	 (toupper (rep_type [0]))
	{
	case	'C':
		ProcessByCustomer ();
		break;
	case	'P':
		ProcessByItem ();
		break;
	case	'D':
		ProcessByDate ();
		break;
	}
	fsort = sort_sort (fsort, "datfile");
	sptr = sort_read (fsort);
}

void
ProcessByCustomer (
 void)
{
	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, branchNumber);
	strcpy (cumr_rec.dbt_no, local_rec.startCustNo);

	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");

	dsp_screen ("Processing : Printing Un-Released Scheduled Sales Orders By Customer.",comm_rec.co_no,comm_rec.co_name);

	while (!cc && 
		   (!strcmp (cumr_rec.co_no, comm_rec.co_no)) &&
		   (!strcmp (cumr_rec.est_no, branchNumber)) &&
		   (strcmp (cumr_rec.dbt_no, local_rec.endCustNo) <= 0))
	{
		dsp_process ("Cust No. :",cumr_rec.dbt_no);
		strcpy (data_rec.custNo, cumr_rec.dbt_no); 	
		strcpy (data_rec.custName, cumr_rec.dbt_name); 	
	 	GetCustomerOrders ();	
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}
}

void
ProcessByItem (
 void)
{
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	strcpy (inmr_rec.item_no, local_rec.startItem);

	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");

	dsp_screen ("Processing : Printing Un-Released Scheduled Sales Orders By Item.",comm_rec.co_no,comm_rec.co_name);

	while ((!cc) && (!strcmp (inmr_rec.co_no, comm_rec.co_no)) &&
		   (strcmp (inmr_rec.item_no, local_rec.endItem) <= 0))
	{
		dsp_process ("Item No. :",inmr_rec.item_no);
	 	GetProdOrderLines ();	
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
}

void
ProcessByDate (
 void)
{
	char	dspDate [11];

	soln_rec.hhso_hash = 0;
	soln_rec.line_no = 0;

	strcpy (sdate,DateToString (local_rec.startDate));

	dsp_screen ("Processing : Printing Un-Released Scheduled Sales Orders By Due Date.",
			   comm_rec.co_no,
			   comm_rec.co_name);

	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc)
	{
		if ((soln_rec.due_date >= local_rec.startDate) &&
			 (soln_rec.due_date <= local_rec.endDate) &&
			 (soln_rec.hhcc_hash == ccmr_rec.hhcc_hash) &&
			 (soln_rec.status [0] == 'G'))
		{
			inmr_rec2.hhbr_hash = soln_rec.hhbr_hash;			
			if (GetDateOrderDetails ())
			{
				strcpy (dspDate,DateToString (soln_rec.due_date));
				dsp_process ("Due Date. :", dspDate);
				if (GetItemDetails () && GetCustomerDetails ())
				{
					strcpy (data_rec.custNo, cumr_rec2.dbt_no); 	
					strcpy (data_rec.custName, cumr_rec2.dbt_name); 	
					data_rec.qtyOrder = soln_rec.qty_order;
					data_rec.longDueDate = soln_rec.due_date;	
					BuildSortFile ();
				}
			}
		}
		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
}

int 
GetCustomerDetails (
 void)
{
	int retval = FALSE;

	cumr_rec2.hhcu_hash = sohr_rec2.hhcu_hash;
	cc = find_rec (cumr2, &cumr_rec2, EQUAL, "r");
	if (cc)
		file_err (cc, cumr2, "DBFIND");
	else
		retval = TRUE;

	return (retval);
}

int 
GetDateOrderDetails (
 void)
{
	int retval = FALSE;

	sohr_rec2.hhso_hash = soln_rec.hhso_hash;			
	cc = find_rec (sohr2, &sohr_rec2, EQUAL, "r");
	if (!cc)
	{
		if (!strcmp (sohr_rec2.co_no, comm_rec.co_no) &&
			!strcmp (sohr_rec2.br_no, comm_rec.est_no) &&
			sohr_rec2.sch_ord [0] == 'Y')
		{
				strcpy (data_rec.order_no, sohr_rec2.order_no);
				retval = TRUE;
		}
	}
	else
		file_err (cc, sohr2, "DBFIND");
		
	return (retval);
}

void
GetCustomerOrders (
 void)
{
	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, comm_rec.est_no);
	sohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sprintf (sohr_rec.sch_ord,"Y");

	cc = find_rec (sohr, &sohr_rec, GTEQ, "r");

	while (!cc && 
	 	   (!strcmp (sohr_rec.co_no, comm_rec.co_no)) &&
		   (!strcmp (sohr_rec.br_no, comm_rec.est_no)) &&
		   (sohr_rec.hhcu_hash == cumr_rec.hhcu_hash))
	{
		strcpy (data_rec.order_no, sohr_rec.order_no); 	
		if (sohr_rec.sch_ord [0] == 'Y')
	 		GetOrderLines ();	
		cc = find_rec (sohr, &sohr_rec, NEXT, "r");
	}
}

void
GetProdOrderLines (
 void)
{
	soln_rec2.hhbr_hash = inmr_rec.hhbr_hash;

	cc = find_rec (soln2, &soln_rec2, GTEQ, "r");

	while (!cc && (soln_rec2.hhbr_hash == inmr_rec.hhbr_hash))
	{
		if ((soln_rec2.hhcc_hash == ccmr_rec.hhcc_hash) &&
			 (soln_rec2.status [0] == 'G'))
		{
			sohr_rec2.hhso_hash = soln_rec2.hhso_hash;			
			GetProdOrders ();
		}
		cc = find_rec (soln2, &soln_rec2, NEXT, "r");
	}
}

void
GetProdOrders (
 void)
{
	cc = find_rec (sohr2, &sohr_rec2, EQUAL, "r");

	if (!cc)
	{
		if ((sohr_rec2.sch_ord [0] == 'Y') &&
			 (!strcmp (sohr_rec2.co_no, comm_rec.co_no)) &&
		   	 (!strcmp (sohr_rec2.br_no, comm_rec.est_no))) 
		{
			cumr_rec2.hhcu_hash = sohr_rec2.hhcu_hash;
			cc = find_rec (cumr2, &cumr_rec2, EQUAL, "r");
			if ((!cc) && (inmr_rec.inmr_class [0] != 'Z'))
			{
				strcpy (data_rec.itemNo,   inmr_rec.item_no); 	
				strcpy (data_rec.itemDesc, inmr_rec.description); 	
				strcpy (data_rec.order_no,  sohr_rec2.order_no);
				strcpy (data_rec.dueDate,  DateToString (soln_rec2.due_date));	
				strcpy (data_rec.custNo,    cumr_rec2.dbt_no); 	
				strcpy (data_rec.custName,  cumr_rec2.dbt_name); 	
				data_rec.qtyOrder = soln_rec2.qty_order;

				GetDeliveryAddr ();
				BuildSortFile ();
			}
			else
				file_err (cc, cumr2, "DBFIND");
		}
	}
	else
		file_err (cc, sohr2, "DBFIND");
}

void
GetOrderLines (
 void)
{
	soln_rec.hhso_hash = sohr_rec.hhso_hash;
	soln_rec.line_no = 0;

	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while ((!cc) && (soln_rec.hhso_hash == sohr_rec.hhso_hash))
	{
		if ((soln_rec.hhcc_hash == ccmr_rec.hhcc_hash) &&
			 (soln_rec.status [0] == 'G'))
		{
			inmr_rec2.hhbr_hash = soln_rec.hhbr_hash;			
			if (GetItemDetails ())
			{
				data_rec.qtyOrder = soln_rec.qty_order;
				strcpy (data_rec.dueDate, DateToString (soln_rec.due_date));	

				GetDeliveryAddr ();
				BuildSortFile ();
			}
		}
		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
}

int 
GetItemDetails (
 void)
{
	int retval = FALSE;

	cc = find_rec (inmr2, &inmr_rec2, EQUAL, "r");
	if (!cc)
	{
		strcpy (data_rec.itemNo, inmr_rec2.item_no);
		strcpy (data_rec.itemDesc, inmr_rec2.description);
		if (inmr_rec2.inmr_class [0] != 'Z')
			retval = TRUE;
	}
	else
		file_err (cc, inmr2, "DBFIND");
	
	return (retval);
}

void
GetDeliveryAddr (
 void)
{
	char temp_address [210];

	if (rep_type [0] == 'P')
	{
		soln_rec.del_no = soln_rec2.del_no;
		cumr_rec.hhcu_hash = cumr_rec2.hhcu_hash;
		strcpy (sohr_rec.del_name, sohr_rec2.del_name);
		strcpy (sohr_rec.del_add1, sohr_rec2.del_add1);
		strcpy (sohr_rec.del_add2, sohr_rec2.del_add3);
		strcpy (sohr_rec.del_add2, sohr_rec2.del_add3);
	}

	if (soln_rec.del_no != 0)
	{
		cudi_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cudi_rec.del_no = soln_rec.del_no;
		cc = find_rec (cudi, &cudi_rec, EQUAL, "r");
		if (!cc)
		{
			sprintf (temp_address,
				   	"%s,%s,%s,%s,%s",
				   	clip (cudi_rec.name),
					clip (cudi_rec.adr1),
				   	clip (cudi_rec.adr2),
					clip (cudi_rec.adr3),
				   	clip (cudi_rec.adr4));
		}
		else
			file_err (cc, cudi, "DBFIND");
			
	}
	else
	{
		sprintf (temp_address,
				"%s,%s,%s,%s",
				clip (sohr_rec.del_name), 
				clip (sohr_rec.del_add1),
				clip (sohr_rec.del_add2),
				clip (sohr_rec.del_add3));
	}
	if (rep_type [0] == 'C')
		strncpy (data_rec.deliveryAddress, temp_address, 71);
	else if (rep_type [0] == 'P')
		strncpy (data_rec.deliveryAddress, temp_address, 81);
}

void
BuildSortFile (
 void)
{
	switch	 (toupper (rep_type [0]))
	{
	case	'C':
		sprintf (data_str, 
				"%-6.6s%-40.40s%-8.8s%-16.16s%-40.40s%10.2f%-10.10s%-71.71s\n",
				data_rec.custNo, 
				data_rec.custName, 
				data_rec.order_no,
		 		data_rec.itemNo, 
				data_rec.itemDesc,
				data_rec.qtyOrder,
		 		data_rec.dueDate, 
				data_rec.deliveryAddress);
		break;

	case	'P':
		sprintf (data_str, 
				"%-16.16s%-40.40s%-6.6s%-40.40s%-8.8s%10.2f%-10.10s%-81.81s\n", 
				data_rec.itemNo, 
				data_rec.itemDesc, 
				data_rec.custNo,
		 		data_rec.custName, 
				data_rec.order_no, 
				data_rec.qtyOrder,
		 		data_rec.dueDate, 
				data_rec.deliveryAddress); 
		break;

	case	'D':
		sprintf (data_str, 
				"%-8.8ld%-6.6s%-40.40s%-8.8s%-16.16s%-40.40s%10.2f\n", 
				data_rec.longDueDate, 
				data_rec.custNo, 
				data_rec.custName, 
				data_rec.order_no, 
				data_rec.itemNo, 
				data_rec.itemDesc, 
				data_rec.qtyOrder);
		break;
	}
    sort_save (fsort, data_str);
}

void
HeaderOutput (
 void)
{
	char	head [61];
	char	testEndCust [7],
			testEndItem [17];

	sprintf (head,"%-60.60s",rep_title);

	if ((fout = popen ("pformat","w")) == (FILE *)NULL)
		file_err (errno, "pformat", "popen");

	fprintf (fout,".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout,".LP%d\n",local_rec.printerNumber);
	fprintf (fout,".16\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".E%s\n",head);
	fprintf (fout,".B1\n");
	fprintf (fout,".ECOMPANY   %s : %s\n",comm_rec.co_no,clip (comm_rec.co_name));
	fprintf (fout,".EBRANCH    %s : %s\n",comm_rec.est_no,clip (comm_rec.est_name));
	fprintf (fout,".EWAREHOUSE %s : %s\n",comm_rec.cc_no,clip (comm_rec.cc_name));
	fprintf (fout,".B1\n");
	fprintf (fout,".EAS AT %s\n",SystemTime ());
	fprintf (fout,".B1\n");
	switch	 (toupper (rep_type [0]))
	{
	case	'C':
		memset ((char *)testEndCust,0xff,sizeof (testEndCust));
		if (!strncmp (testEndCust, local_rec.endCustNo,6))
			strcpy (testEndCust, "~~~~~~");

		fprintf (fout,".ESTART CUSTOMER %s    END CUSTOMER %s\n",
						local_rec.startCustNo, 
						testEndCust);
		fprintf (fout,".B1\n");
		fprintf (fout,"=======================================================");
		fprintf (fout,"=====================================================");
		fprintf (fout,"==================================================\n");
		fprintf (fout,"! ORDER  !  ITEM NUMBER    !           ITEM DESCRIPTION");
		fprintf (fout,"            !    QTY   ! DUE DATE !                    ");
		fprintf (fout,"        DELIVERY ADDRESS                       !\n");
		fprintf (fout,"!------------------------------------------------------");
		fprintf (fout,"-----------------------------------------------------");
		fprintf (fout,"-------------------------------------------------!\n");
		break;

	case	'P':
		memset ((char *)testEndItem,0xff,sizeof (testEndItem));
		if (!strncmp (testEndItem, local_rec.endItem, 16))
			strcpy (testEndItem, "~~~~~~~~~~~~~~~~");

		fprintf (fout,".ESTART ITEM %s     END ITEM %s\n",
						local_rec.startItem, 
						testEndItem);
		fprintf (fout,".B1\n");
		fprintf (fout,"=====================================================");
		fprintf (fout,"=====================================================");
		fprintf (fout,"====================================================\n");
		fprintf (fout,"!CUSTNO!             CUSTOMER NAME        ");
		fprintf (fout,"      !ORDER !    QTY   ! DUE DATE !                    ");
		fprintf (fout,"             DELIVERY ADDRESS                         ");
		fprintf (fout,"       !\n");
		fprintf (fout,"!----------------------------------------------------");
		fprintf (fout,"-----------------------------------------------------");
		fprintf (fout,"---------------------------------------------------!\n");
		break;

	case	'D':
		fprintf (fout,".ESTART DATE %s     END DATE %s\n",
						sdate, 
						edate);
		fprintf (fout,".B1\n");
		fprintf (fout,"=====================================================");
		fprintf (fout,"=====================================================");
		fprintf (fout,"====================================================\n");
		fprintf (fout,"! DUE DATE ! CUSTOMER NUMBER !             CUSTOMER  ");
		fprintf (fout,"NAME             ! ORDER NUMBER !  ITEM NUMBER   !   ");
		fprintf (fout,"           ITEM DESCRIPTION          !   QUANTITY  !\n");
		fprintf (fout,"!----------------------------------------------------");
		fprintf (fout,"-----------------------------------------------------");
		fprintf (fout,"---------------------------------------------------!\n");
	}

	fprintf (fout,".R=====================================================");
	fprintf (fout,"=====================================================");
	fprintf (fout,"====================================================\n");

	next_page = TRUE;

}

void
ReportPrint (
 void)
{

	switch	 (toupper (rep_type [0]))
	{
	case	'C':
		PrintCustomerLines ();
		break;

	case	'P':
		PrintProductLines ();
		break;

	case	'D':
		PrintDateLines ();
		break;
	}
	fprintf (fout,".EOF\n");
	pclose (fout);
	sort_delete (fout, "datfile"); 
}

void
PrintCustomerLines (
 void)
{
	char last_custNo [7], last_order_no [9];
	char order_no [9];

	last_order_no [0] = '\0';
	last_custNo [0] = '\0';
	while (sptr)
	{
		if (next_page)
			PrintCustomerHeader ();

		if (strncmp (last_custNo,sptr,6) && (!next_page))
			PrintCustomerHeader ();

		if (!strncmp (last_order_no, sptr + 46, 8) && (!next_page))
			strncpy (order_no, "        ", 8);
		else
			strncpy (order_no, sptr + 46, 8);
		
		fprintf (fout,"!%-8.8s",      order_no);       /* Get the order number */
		fprintf (fout,"!%-16.16s",    sptr+54);        /* Get the item number */
		fprintf (fout,"!%-40.40s",    sptr+70);        /* Get the item desc */
		fprintf (fout,"!%10.2f",      atof (sptr+110)); /* Get the Qty Ordered */
		fprintf (fout,"!%-10.10s",      sptr+120);     /* Get the Due Date */
		fprintf (fout,"!%-71.71s!\n", sptr+130);    /* Get the Del Address */
		strncpy (last_order_no, sptr + 46,8);
		strncpy (last_custNo, sptr, 6);
		sptr = sort_read (fsort);
		next_page = FALSE;
	}
}

void
PrintCustomerHeader (
 void)
{
	if (!next_page)
	{
		fprintf (fout,"!----------------------------------------------------");
		fprintf (fout,"-----------------------------------------------------");
		fprintf (fout,"---------------------------------------------------!\n");
	}
	fprintf (fout,"!CUSTOMER NO: %-6.6s CUSTOMER NAME:%-40.40s                    ", sptr, sptr+6);
	fprintf (fout,"                                                      ");
	fprintf (fout,"        !\n");	
	fprintf (fout,"!                                                    ");
	fprintf (fout,"                                                     ");
	fprintf (fout,"                                                   !\n");
}

void
PrintProductLines (
 void)
{
	char last_itemNo [17], last_custNo [7], custNo [7], custName [41];
	char qty [11];

	last_itemNo [0] = '\0';
	last_custNo [0] = '\0';
	custName [0] = '\0';
	total = 0;

	while (sptr)
	{
		if (strncmp (last_itemNo,sptr,16) && (!first_time))
		{
			PrintTotal ();
			total = 0;
		}

		if (next_page)
			PrintProductHeader ();

		if (strncmp (last_itemNo,sptr,16) && (!next_page))
			PrintProductHeader ();

		if (!strncmp (last_custNo, sptr + 56, 6) && (!next_page) &&
			!strncmp (last_itemNo, sptr, 16))
		{
			strncpy (custNo, "      ", 6);
			custName [0] = '\0';
		}
		else
		{
			strncpy (custNo, sptr + 56, 6);
			strncpy (custName, sptr + 62, 40);
		}
		
		fprintf (fout,"!%-6.6s",      custNo);         /* Get the cust number */
	  	fprintf (fout,"!%-40.40s",    custName);       /* Get the cust name */
		fprintf (fout,"!%-6.6s",      sptr+102);       /* Get the order number */
		fprintf (fout,"!%10.2f",      atof (sptr+110)); /* Get the Qty Ordered */
		fprintf (fout,"!%-10.10s",      sptr+120);     /* Get the Due Date */
		fprintf (fout,"!%-81.81s!\n", sptr+130);    /* Get the Del Address */
		strncpy (last_itemNo, sptr,16);
		strncpy (last_custNo, sptr + 56,6);
		strncpy (qty, sptr + 110, 10);
		total = (float) (total + atof (qty));
		sptr = sort_read (fsort);
		next_page = FALSE;
		first_time = FALSE;
	}
	PrintTotal ();
}

void
PrintProductHeader (
 void)
{
	if (!next_page)
	{
		fprintf (fout,"!----------------------------------------------------");
		fprintf (fout,"-----------------------------------------------------");
		fprintf (fout,"---------------------------------------------------!\n");
	}

	fprintf (fout,"! ITEM NO : %-16.16s ITEM DESCRIPTION : %-40.40s                  ",sptr, sptr+16);
	fprintf (fout,"                                                   !\n");
	fprintf (fout,"!                                                    ");
	fprintf (fout,"                                                     ");
	fprintf (fout,"                                                   !\n");
}

void
PrintTotal (
 void)
{
	fprintf (fout,"!----------------------------------------------------");
	fprintf (fout,"-----------------------------------------------------");
	fprintf (fout,"---------------------------------------------------!\n");
	fprintf (fout,"! Total:                                  ");
	fprintf (fout,"              %10.2f                                 ",
			total);
	fprintf (fout,"                                                      ");
	fprintf (fout,"    !\n");
}

void
PrintDateLines (
 void)
{

	char last_custNo [7], custNo [7], custName [41], last_order_no [9];
	char order_no [9], lastDate [11], dueDate [11];
	long dte;
	int blank_dbt;
	char copy_date [11];

	last_custNo [0] 	= '\0';
	custName [0] 		= '\0';
	last_order_no [0] 	= '\0';
	lastDate [0] 		= '\0';


	while (sptr)
	{
		blank_dbt = TRUE;
		sprintf (copy_date, "%-8.8s", sptr);
		dte = atol (copy_date);
		strcpy (dueDate,DateToString (dte));

		if (!strcmp (lastDate,dueDate) && (!next_page))
		{
			dueDate [0] = '\0';
		}
		else
		{
			blank_dbt = FALSE;
			if (!next_page)
			{
				fprintf (fout,"!---------------------------------------");
				fprintf (fout,"----------------------------------------");
				fprintf (fout,"----------------------------------------");
				fprintf (fout,"-------------------------------------!\n");
			}
		}

		if (!strncmp (last_custNo,sptr+8, 6) && (!next_page) && (blank_dbt))
		{
			strncpy (custNo, "      ", 6);
			custName [0] = '\0';
		}
		else
		{
			strncpy (custNo, sptr+8,6);
			strncpy (custName, sptr+14,40);
		}

		if (!strncmp (last_order_no,sptr+54, 8) && (!next_page))
			strncpy (order_no, "        ", 8);
		else
			strncpy (order_no, sptr+54,8);
		
		fprintf (fout,"! %-10.10s    ", dueDate);       /* Get the due date */
	  	fprintf (fout,"!%-6.6s      ",  custNo);	        /* Get the cust no */
		fprintf (fout,"!%-40.40s    ",  custName);       /* Get the cust name */
	  	fprintf (fout,"!%-8.8s  ",    order_no);	    /* Get the order no */
		fprintf (fout,"!%-16.16s",      sptr+62);  	    /* Get the item No */
		fprintf (fout,"!%-40.40s",      sptr+78);  	    /* Get the Item Desc */
		fprintf (fout,"! %10.2f  !\n",  atof (sptr+118)); /* Get the Qty */

		strncpy (last_order_no, sptr + 54,8);
		strncpy (last_custNo, sptr + 8,6);
		strncpy (copy_date, sptr,10);
		dte = atol (copy_date);
		sprintf (lastDate,"%-10.10s",DateToString (dte));

		sptr = sort_read (fsort);
		next_page = FALSE;
	}
}

int
FindRange (
 int field, 
 char *fld_value, 
 char *fld_desc)
{

	if (SRCH_KEY)
	{
		InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
		return (EXIT_SUCCESS);
	}
	clear_mess ();
	
	cc = FindInmr (comm_rec.co_no, fld_value, 0L, "N");
	if (!cc)
	{
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		strcpy (inmr_rec.item_no, fld_value);
		cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
	}
	if (cc)
	{
		errmess (ML (mlStdMess001));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	SuperSynonymError ();

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", fld_value);
	cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
	if (cc)
	{
		print_mess (ML (mlStdMess001));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	if (field == LOWER)
		strcpy (local_rec.startItem, inmr_rec.item_no);
	else
		strcpy (local_rec.endItem, inmr_rec.item_no);

	if (strncmp (local_rec.startItem, local_rec.endItem, 16) > 0)
	{
		print_mess (ML (mlStdMess017));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	sprintf (fld_desc, "%-40.40s", (!cc) ? inmr_rec.description : " ");

	display_field (field);
	if (field == LOWER)
		DSP_FLD ("startItemDesc");
	else
		DSP_FLD ("endItemDesc");

	return (cc);
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
		rv_pr (rep_title,42,0,1);
		line_at (1,0,132);

		move (1,input_row);

		box (0,3,132,6);

		line_at (6,1,131);
		line_at (20,0,132);

		print_at (21,0, ML (mlStdMess038),comm_rec.co_no,  comm_rec.co_name);
		print_at (21,45,ML (mlStdMess039),comm_rec.est_no, comm_rec.est_name);
		print_at (21,85,ML (mlStdMess099),comm_rec.cc_no,  comm_rec.cc_name);

		line_at (22,0,132);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

