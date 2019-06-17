/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_abc_val.c,v 5.4 2002/07/17 09:57:51 scott Exp $
|  Program Name  : (sk_abc_val.c)
|  Program Desc  : (Maintain ABC Inventory Codes)
|                  (Print Inventory Management Report)
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 03/01/90         |
|---------------------------------------------------------------------|
| $Log: sk_abc_val.c,v $
| Revision 5.4  2002/07/17 09:57:51  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/08/09 09:17:53  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:44:26  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:39  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_abc_val.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_abc_val/sk_abc_val.c,v 5.4 2002/07/17 09:57:51 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#define		MAXWH	99		
#define		BY_CO		 (reportLevel [0] == 'C')
#define		BY_BR		 (reportLevel [0] == 'B')
#define		BY_WH		 (reportLevel [0] == 'W')
#define		SALES 		 (sortBy [0] == 'S')
#define		MARGIN		 (sortBy [0] == 'M')
#define		UPD		 	 (action == 2 || action == 3)
#define		PRINT		 (action == 1 || action == 3)
#define		CAL(amt,pc)  (amt * DOLLARS (pc))

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inimRecord	inim_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inccRecord	incc2_rec;

	char	*incc2		=	"incc2";
	float	*incc_con	=	&incc_rec.c_1;
	Money	*incc_val	=	&incc_rec.c_val_1;
	Money	*incc_prf	=	&incc_rec.c_prf_1;

	int		found 			= FALSE,
			printerNumber	= 0,
			period			= 0,
			action			= 0,
			first_time 		= TRUE,
			curr_month		= 0;

	char	reportLevel 	[2],
			sortBy 			[2],
			useBreak 		[5],
			dataString 		[131],
			updateCode 		[2],
			previousCode 	[2],
			yesPrompt 		[11],
			noPrompt 		[11];


	float	codePercent [3]	= {0.00,0.00,0.00},
			qtySold [3]		= {0.00,0.00,0.00},
			onHand [3]		= {0.00,0.00,0.00},
			workQtySold		= 0.00,
			xOnHand			= 0.00,
			cumSalesPc 		= 0.00,
			cumMarginPc 	= 0.00,
			cumOnHandPc 	= 0.00,
			weeksDemand 	= 0.00;

	double	movement			= 0.00,
			margin [3]			= {0.00,0.00,0.00},
			extendSales [3]		= {0.00,0.00,0.00},
			extendMargin [3]	= {0.00,0.00,0.00},
			extendOnHand [3]	= {0.00,0.00,0.00},
			workExtendSales 	= 0.00,
			workExtendMargin 	= 0.00,
			eOnHand 			= 0.00,
			cumSales 			= 0.00,
			cumMargin 			= 0.00,
			cumOnHand 			= 0.00;

	long	historyDate			= 0L,
			hhccHash [MAXWH];


	FILE	*fsort;
	FILE	*fout;
	char 	*_SortRead 		 (FILE *);
	char	*srt_offset [131];

#include <Costing.h>

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	int		period;
	double	mvment;
	float	code_brk [3];
	char	useBreak [4] [5];
	char	useBreakDesc [4] [11];
	char	act [2] [5];
	char	actDesc [2] [11];
	char	sortBy [8];
	char	sortByDesc [11];
	char	back [5];
	char	backDesc [11];
	char	onight [5];
	char	onightDesc [11];
	int		printerNumber;
	char	printerString [3];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "period",	 4, 18, INTTYPE,
		"NN", "          ",
		" ", "6", "Period. ", " Number of months history required; Default = 6 months ",
		YES, NO, JUSTRIGHT, "1", "12", (char *)&local_rec.period},
	{1, LIN, "movement",	 5, 18, DOUBLETYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Movement. ", " Minimum value movement for ABC code ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.mvment},
	{1, LIN, "prt_rep",	 4, 55, CHARTYPE,
		"U", "          ",
		" ", "Y", "Print Report.", "",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.act [0]},
	{1, LIN, "prt_rep_desc",	4, 58, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.actDesc [0]},
	{1, LIN, "updateCode",	 5, 55, CHARTYPE,
		"U", "          ",
		" ", "Y", "Update Codings.", "",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.act [1]},
	{1, LIN, "updateCodeDesc",	5, 58, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.actDesc [1]},
	{1, LIN, "sortBy",	 6, 55, CHARTYPE,
		"U", "          ",
		" ", "S", "Sort By.", " Sort report by S(ales value) or M(argin)",
		YES, NO,  JUSTLEFT, "SM", "", local_rec.sortBy},
	{1, LIN, "sortByDesc",	6, 58, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.sortByDesc},
	{1, LIN, "_code_brk",	 8, 18, FLOATTYPE,
		"NNN.NN", "          ",
		" ", " ", "A Code Break.", "",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.code_brk [0]},
	{1, LIN, "_code_brk",	 9, 18, FLOATTYPE,
		"NNN.NN", "          ",
		" ", " ", "B Code Break.", "",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.code_brk [1]},
	{1, LIN, "_code_brk",	10, 18, FLOATTYPE,
		"NNN.NN", "          ",
		" ", " ", "C Code Break.", "",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.code_brk [2]},
	{1, LIN, "use_",	 8, 55, CHARTYPE,
		"U", "          ",
		" ", "Y", "Print A Code.", "",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.useBreak [0]},
	{1, LIN, "use_desc",	8, 58, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.useBreakDesc [0]},
	{1, LIN, "use_",	 9, 55, CHARTYPE,
		"U", "          ",
		" ", "Y", "Print B Code.", "",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.useBreak [1]},
	{1, LIN, "use_desc",	9, 58, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.useBreakDesc [1]},
	{1, LIN, "use_",	10, 55, CHARTYPE,
		"U", "          ",
		" ", "Y", "Print C Code.", "",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.useBreak [2]},
	{1, LIN, "use_desc",	10, 58, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.useBreakDesc [2]},
	{1, LIN, "use_",	11, 55, CHARTYPE,
		"U", "          ",
		" ", "Y", "Print D Code.", "",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.useBreak [3]},
	{1, LIN, "use_desc",	11, 58, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.useBreakDesc [3]},
	{1, LIN, "printerNumber",	13, 18, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer number ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber},
	{1, LIN, "back",	14, 18, CHARTYPE,
		"U", "          ",
		" ", "N", "Background ", " ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",	14, 21, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onight",	14, 55, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight ", " ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight},
	{1, LIN, "onightDesc",	14, 58, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.onightDesc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


/*======================= 
| Function Declarations |
=======================*/
double 	FindAllCosts 			(long);
int  	CheckForBreak 			(void);
int  	CheckForValid 			(void);
int  	CheckWh 				(long);
int  	SetCode 				(char *);
int  	ValidateWh 				(void);
int  	heading 				(int);
int  	spec_valid 				(int);
void 	CalculateCum 			(char *);
void 	CloseDB 				(void);
void 	HeadingOutput 			(void);
void 	InitConsumption 		(void);
void 	InitValues 				(void);
void 	MainPrintRoutine 		(char *);
void 	OpenDB 					(void);
void 	PrintLine 				(void);
void 	ProcessFile 			(void);
void 	ProcessSorted 			(void);
void 	ReadCcmr 				(void);
void 	RunProgram 				(char *);
void 	SetupDefault 			(void);
void 	StoreValues				(long);
void 	SumValues 				(long);
void 	Update 					(char *);
void 	UpdateIncc 				(void);
void 	UpdateInei 				(void);
void 	UpdateInim 				(void);
void 	UpdateInmr 				(void);
void 	shutdown_prog 			(void);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 2 && argc != 11)
	{
		print_at (0,0,mlSkMess504,argv [0]);
		print_at (1,0,mlSkMess001,argv [0]);
		return (EXIT_FAILURE);
	}

	switch (argv [1] [0])
	{
	case	'C':
	case	'c':
		strcpy (reportLevel,"C");
		break;

	case	'B':
	case	'b':
		strcpy (reportLevel,"B");
		break;

	case	'W':
	case	'w':
		strcpy (reportLevel,"W");
		break;

	default	:
		print_at (3,0,mlSkMess006);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	/*===========================
	| Open main database files. |  
	===========================*/
	OpenDB ();

	if (argc == 11)
	{
		printerNumber = atoi (argv [2]);

		action = atoi (argv [3]);
		period = atoi (argv [4]);
		sprintf (sortBy,"%-1.1s",argv [5]);
		switch (argv [5] [0])
		{
		case	'S':
		case	's':
			strcpy (sortBy,"S");
			break;

		case	'M':
		case	'm':
			strcpy (sortBy,"M");
			break;

		default	:
			print_at (4,0,ML (mlSkMess007));
			return (EXIT_FAILURE);
		}

		movement 		= atof (argv [6]);
		codePercent [0] = (float) (atof (argv [7]));
		codePercent [1] = (float) (atof (argv [8]));
		codePercent [2] = (float) (atof (argv [9]));
		sprintf (useBreak,"%-4.4s",argv [10]);

		if (PRINT)
			sprintf (err_str,"%s ABC codes.","Printing");
		else
		if (UPD)
			sprintf (err_str,"%s ABC codes.","Updating");
		else
			sprintf (err_str,"%s ABC codes.","Printing/Updating");
		dsp_screen (err_str,comm_rec.co_no,comm_rec.co_name);
		ProcessFile ();
		if (found)
		{
			ProcessSorted ();
			fprintf (fout,".EOF\n");
			pclose (fout);
		}

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	/*=====================
	| Reset control flags |
	=====================*/
   	search_ok	= TRUE;
   	entry_exit	= TRUE;
   	prog_exit	= FALSE;
   	restart		= FALSE;
	init_vars (1);	
	SetupDefault ();

	/*-----------------------------
	| Edit screen 1 linear input. |
	-----------------------------*/
	do {
		heading (1);
		scn_display (1);
		edit (1);
	} while (CheckForBreak () && !restart && !prog_exit);
	prog_exit = 1;
	rset_tty ();

	if (!restart) 
		RunProgram (argv [0]);
	
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

int
CheckForBreak (
 void)
{
	float	totalPercent = 0.00;

	totalPercent	=	local_rec.code_brk [0] + 
						local_rec.code_brk [1] + 
						local_rec.code_brk [2];

	if (totalPercent > 100.00)
	{
		print_mess (ML (mlSkMess002));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

void
SetupDefault (
 void)
{
	strcpy (yesPrompt, ML ("Yes "));
	strcpy (noPrompt,  ML ("No. "));
	local_rec.period = 6;
	local_rec.mvment = 0.00;
	strcpy (local_rec.act [0],"Y");
	strcpy (local_rec.act [1],"Y");
	strcpy (local_rec.actDesc [0], yesPrompt);
	strcpy (local_rec.actDesc [1], yesPrompt);
	strcpy (local_rec.sortBy,"S");
	strcpy (local_rec.sortByDesc, ML ("Sales "));
	local_rec.code_brk [0] = 15.00;
	local_rec.code_brk [1] = 20.00;
	local_rec.code_brk [2] = 65.00;
	strcpy (local_rec.useBreak [0],"Y");
	strcpy (local_rec.useBreak [1],"Y");
	strcpy (local_rec.useBreak [2],"Y");
	strcpy (local_rec.useBreak [3],"Y");
	strcpy (local_rec.useBreakDesc [0], yesPrompt);
	strcpy (local_rec.useBreakDesc [1], yesPrompt);
	strcpy (local_rec.useBreakDesc [2], yesPrompt);
	strcpy (local_rec.useBreakDesc [3], yesPrompt);
	strcpy (local_rec.back,"N");
	strcpy (local_rec.backDesc, noPrompt);
	strcpy (local_rec.onight,"N");
	strcpy (local_rec.onightDesc, noPrompt);
	local_rec.printerNumber = 1;
}

void
RunProgram (
 char *prog_name)
{	
	char	act_str [2];
	char	pd_str [3];
	char	mv_str [10];
	char	use_str [5];
	char	brk_pc [3] [7];

	if (local_rec.act [0] [0] == 'Y' && local_rec.act [1] [0] == 'Y')
		strcpy (act_str,"3");
	else
	if (local_rec.act [1] [0] == 'Y')
		strcpy (act_str,"2");
	else
		strcpy (act_str,"1");

	sprintf (pd_str,"%2d",local_rec.period);
	local_rec.sortBy [1] = '\0';
	sprintf (mv_str,"%9.2f",local_rec.mvment);
	sprintf (brk_pc [0],"%6.2f",local_rec.code_brk [0]);
	local_rec.code_brk [1] += local_rec.code_brk [0];
	sprintf (brk_pc [1],"%6.2f",local_rec.code_brk [1]);
	local_rec.code_brk [2] += local_rec.code_brk [1];
	sprintf (brk_pc [2],"%6.2f",local_rec.code_brk [2]);

	sprintf (use_str,"%-1.1s%-1.1s%-1.1s%-1.1s",local_rec.useBreak [0],local_rec.useBreak [1],local_rec.useBreak [2],local_rec.useBreak [3]);

	shutdown_prog ();
	
	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onight [0] == 'Y')
	{ 
		if (fork () == 0)
			execlp ("ONIGHT",
				"ONIGHT",
				prog_name,
				reportLevel,
				local_rec.printerString,
				act_str,
				pd_str,
				local_rec.sortBy,
				mv_str,
				brk_pc [0],
				brk_pc [1],
				brk_pc [2],
				use_str,
				ML (mlSkMess003), (char *)0);
	}
	/*====================================
	| Test for forground or background . |
	====================================*/
	else if (local_rec.back [0] == 'Y') 
	{
		if (fork () == 0)
			execlp (prog_name,
				prog_name,
				reportLevel,
				local_rec.printerString,
				act_str,
				pd_str,
				local_rec.sortBy,
				mv_str,
				brk_pc [0],
				brk_pc [1],
				brk_pc [2],
				use_str, (char *)0);
	}
	else 
	{
		execlp (prog_name,
			prog_name,
			reportLevel,
			local_rec.printerString,
			act_str,
			pd_str,
			local_rec.sortBy,
			mv_str,
			brk_pc [0],
			brk_pc [1],
			brk_pc [2],
			use_str, (char *)0);
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

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (inim, inim_list, INIM_NO_FIELDS, "inim_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	if (BY_WH)
	{
		abc_alias (incc2,incc);
		open_rec (incc2,incc_list,INCC_NO_FIELDS,"incc_id_no");
	}
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inim);
	abc_fclose (inmr);
	abc_fclose (incc);
	if (BY_WH)
		abc_fclose (incc2);

	CloseCosting ();
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	int	i;
	char	valid_inp [2];

	/*--------------------
	| Validate end group |
	--------------------*/
	if (LCHECK ("prt_rep"))
	{
		if (local_rec.act [0] [0] == 'Y')
		{
			strcpy (local_rec.actDesc [0], yesPrompt);
			for (i = 0; i < 7; i+=2)
				vars [11 + i].required = YES;
		}
		else
		{
			strcpy (local_rec.actDesc [0],noPrompt);
			for (i = 0; i < 7; i+=2)
			{
				vars [11 + i].required = NA;
				strcpy (local_rec.useBreak [i/2], "N");
				strcpy (local_rec.useBreakDesc [i/2], noPrompt);
				display_field (11 + i);
				display_field (11 + i + 1);
			}
		}
		display_field (field+1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("updateCode"))
	{
		strcpy (local_rec.actDesc [1], 
					(local_rec.act [1] [0] == 'Y') ? yesPrompt : noPrompt);
		DSP_FLD ("updateCodeDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sortBy"))
	{
		strcpy (local_rec.sortByDesc, 
					(local_rec.sortBy [0] == 'S') ? "Sales " : "Margin");
		DSP_FLD ("sortByDesc");
		return (EXIT_SUCCESS);
	}

	/*-------------------
	| Validate Use Code |
	-------------------*/
	if (LCHECK ("use_"))
	{
		sprintf (valid_inp, "%1.1s", local_rec.useBreak [ (field - 11)/2]);

		if (valid_inp [0] == 'Y')
			strcpy (local_rec.useBreakDesc [ (field - 11)/2], yesPrompt);
		else 
			strcpy (local_rec.useBreakDesc [ (field - 11)/2], noPrompt);

		for (i = 11; i < 18; i+=2)
		{
			display_field (i);
			display_field (i + 1);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("_code_brk"))
	{
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.printerString,"%d",local_rec.printerNumber);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.backDesc, (local_rec.back [0] == 'Y') 	
					? yesPrompt : noPrompt);

		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onightDesc, 
				(local_rec.onight [0] == 'Y') ? yesPrompt : noPrompt);
		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	if (restart) 
    	return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);
	clear ();
	rv_pr (ML (mlSkMess008),30,0,1);

	line_at (1, 0,80);
	line_at (7, 1,79);
	line_at (12,1,79);
	box (0,3,80,11);

	line_at (20,0,80);
	print_at (21,0,ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name);
	print_at (22,0,ML (mlStdMess039),comm_rec.est_no,comm_rec.est_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

	return (EXIT_SUCCESS);
}

void
ReadCcmr (
 void)
{
	int	i;
	int	j;
	int	cm;			/* Current inventory mth */
	int	cd;			/* Current debtors day	*/
	static	int	days [14] = {0,31,28,31,30,31,30,31,31,30,31,30,31,31};
	int		dmy [3];


	for (i = 0; i < MAXWH; i++)
		hhccHash [i] = 0L;

	i = 0;

	open_rec (ccmr,ccmr_list,CCMR_NO_FIELDS,"ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no, (BY_CO) ? "  " : comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  (BY_WH) ? comm_rec.cc_no : "  ");
	cc = find_rec (ccmr,&ccmr_rec,GTEQ,"r");
	while (!cc && !strcmp (ccmr_rec.co_no,comm_rec.co_no) && ValidateWh ())
	{ 
		hhccHash [i++] = ccmr_rec.hhcc_hash;
		if (BY_WH)
			break;

		cc = find_rec (ccmr,&ccmr_rec,NEXT,"r");
	}
	abc_fclose (ccmr);

	/*-------------------------------
	| Find the history cutoff date	|
	-------------------------------*/
	DateToDMY (comm_rec.inv_date, &dmy [0], &dmy [1], &dmy [2]);

	/*---------------------------
	| Adjust feb for leap year. |
	---------------------------*/
	days [2] = ((dmy [2] % 4) == 0 ? 29 : 28); 

	/*-------------------------------------
	| Get current month from module date. |
	-------------------------------------*/
	cm = dmy [1];
	curr_month = cm - 2;
	if (curr_month < 0)
		curr_month += 12;

	cd = dmy [0];
	historyDate = comm_rec.inv_date - cd;

	/*-----------------------------------------------
	| Cutoff date = module date - period mths.	|
	-----------------------------------------------*/
	for (i = 1; i < period ; i++)
	{
		j = (cm - i);
		if (j < 1)
			j += 12;
		historyDate -= days [j];
	}
}

int
ValidateWh (
 void)
{
	if (BY_CO)
		return (EXIT_FAILURE);
	if (BY_BR && !strcmp (ccmr_rec.est_no,comm_rec.est_no))
		return (EXIT_FAILURE);
	if (BY_WH && !strcmp (ccmr_rec.est_no,comm_rec.est_no) &&
		!strcmp (ccmr_rec.cc_no,comm_rec.cc_no))
		return (EXIT_FAILURE);
	return (EXIT_SUCCESS);
}

void
ProcessFile (
 void)
{
	long	currHhbrHash;
	long	prevHhbrHash;
	int	store_ok = 0;
	int	print_for_wh = 0;

	InitConsumption ();
	ReadCcmr ();

	prevHhbrHash = 0L;
	incc_rec.hhbr_hash	=	0L;
	cc = find_rec (incc, &incc_rec, GTEQ, "r");
	while (!cc)
	{
		currHhbrHash = incc_rec.hhbr_hash;
		if ((prevHhbrHash != currHhbrHash && prevHhbrHash != 0L
			&& store_ok) || print_for_wh)
		{
			StoreValues (prevHhbrHash);
			store_ok = 0;
			InitConsumption ();
			print_for_wh = 0;
		}

		if (incc_rec.first_stocked > historyDate)
		{
			cc = find_rec (incc, &incc_rec, NEXT, "r");
			prevHhbrHash = currHhbrHash;
			store_ok = 0;
			continue;
		}
		cc = CheckWh (incc_rec.hhcc_hash);
		if (cc == 0)
		{
			cc = find_rec (incc, &incc_rec, NEXT, "r");
			prevHhbrHash = currHhbrHash;
			store_ok = 0;
			continue;
		}
		else
		if (cc == 1)
			SumValues (currHhbrHash);

		prevHhbrHash = currHhbrHash;
		store_ok = 1;

		cc = find_rec (incc, &incc_rec, NEXT, "r");
	}
	if (prevHhbrHash != 0L && store_ok)
		StoreValues (prevHhbrHash);
}

int
CheckWh (
 long wkhash)
{
	int	i;

	for (i = 0; i < MAXWH; i++)
	{
		if (hhccHash [i] == 0L)
		{
			if (BY_WH)
			{
				incc2_rec.hhcc_hash = hhccHash [0];
				incc2_rec.hhbr_hash = incc_rec.hhbr_hash;
				cc = find_rec (incc2,&incc2_rec, COMPARISON,"r");
				if (!cc)
					return (2);
			}
			break;
		}
			
		if (wkhash == hhccHash [i])
			return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*===============================================
| Reset consumption and profit figures for item |
===============================================*/
void
InitConsumption (
 void)
{
	qtySold [1] 		= 0.00;
	margin [1] 			= 0.00;
	onHand [1] 			= 0.00;
	extendSales [1] 	= 0.00;
	extendMargin [1] 	= 0.00;
	extendOnHand [1] 	= 0.00;
	weeksDemand 		= 0.00;
}

void
SumValues (
 long hhbrHash)
{
	register	int	i;
	int	mon;
	double	cost 	 = 0.00;
	double	sal_cost = 0.00;

	qtySold [0] 	= 0.00;
	margin 	[0] 	= 0.00;
	onHand 	[0] 	= 0.00;
	extendSales [0] = 0.00;


	for (i = curr_month, mon = 0; mon < period; i--,mon++)
	{
		if (i < 0)
			i += 12;

		sal_cost 		= incc_val [i] - incc_prf [i];
		qtySold [0] 	+= incc_con [i];
		qtySold [1] 	+= incc_con [i];
		extendSales [0]	+= DOLLARS (sal_cost);
		extendSales [1]	+= DOLLARS (sal_cost);
		margin [0] 		+= incc_prf [i];
		margin [1] 		+= incc_prf [i];
	}
	onHand [0] = incc_rec.closing_stock;
	onHand [1] += incc_rec.closing_stock;

	weeksDemand  += incc_rec.wks_demand;

	cost = FindAllCosts (hhbrHash);
	extendOnHand [1] += (double) onHand [0] * cost; 
}

void
StoreValues (
	long	hhbrHash)
{
	char	dCode [2];

	/*-------------------------------------------------------
	| Include sales figures for item if $sales > minimum	|
	-------------------------------------------------------*/
	if (extendSales [1] > movement)
	{
		qtySold [2] 	 += qtySold [1];
		margin [2]  	 += margin [1];
		onHand [2] 		 += onHand [1];
		extendSales [2]  += extendSales [1];
		extendOnHand [2] += extendOnHand [1];

		strcpy (dCode," ");
	}
	else
		strcpy (dCode,"D");

	if (first_time)
	{
		fsort = sort_open ("sk_abc_val");
		first_time = FALSE;
		found = TRUE;
	}

	if (SALES)
	{
		sprintf 
		(
			dataString,
			"%f%c%f%c%f%c%f%c%f%c%f%c%ld%c%s\n",
			extendSales [1],	1,		/* srt_offset	=	0	*/
			DOLLARS (margin [1]),1,		/* srt_offset	=	1	*/
			extendOnHand [1],	1,		/* srt_offset	=	2	*/
			qtySold [1],		1,		/* srt_offset	=	3	*/
			weeksDemand,		1,		/* srt_offset	=	4	*/
			onHand [1],			1,		/* srt_offset	=	5	*/
			hhbrHash,			1,		/* srt_offset	=	6	*/
			dCode						/* srt_offset	=	7	*/
		);

	}
	else /* if by sort by Margin	*/
	{
		sprintf 
		(
			dataString,
			"%f%c%f%c%f%c%f%c%f%c%f%c%ld%c%s\n",
			DOLLARS (margin [1]),	1,	/* srt_offset	=	0	*/
			extendSales [1],		1,	/* srt_offset	=	1	*/
			extendOnHand [1],		1,	/* srt_offset	=	2	*/
			qtySold [1],			1,	/* srt_offset	=	3	*/
			weeksDemand,			1,	/* srt_offset	=	4	*/
			onHand [1],				1,	/* srt_offset	=	5	*/
			hhbrHash,				1,	/* srt_offset	=	6	*/
			dCode						/* srt_offset	=	7	*/
		);
	}
	sort_save (fsort,dataString);
}

double	
FindAllCosts (
	long	hhbrHash)
{
	double	cost = 0.00;

	inmr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr,&inmr_rec, COMPARISON,"r");
	if (cc)
	{
		cost = 0.00;
		return (cost);
	}

	switch (inmr_rec.costing_flag [0])
	{
	case 'L':
	case 'A':
	case 'P':
	case 'T':
		cost =	FindIneiCosts 
				(
					inmr_rec.costing_flag, 
					comm_rec.est_no, 
					hhbrHash
				);
		break;

	case 'F':
		cost = 	FindIncfValue 
				(
					incc_rec.hhwh_hash, 
					incc_rec.closing_stock, 
					TRUE, 
					TRUE,
					inmr_rec.dec_pt
				);
		break;

	case 'I':
		cost = 	FindIncfValue 
				(
					incc_rec.hhwh_hash, 
					incc_rec.closing_stock, 
					TRUE, 
					FALSE,
					inmr_rec.dec_pt
				);
		break;

	case 'S':
		cost = FindInsfValue (incc_rec.hhwh_hash, TRUE);
		break;
	}
	if (cost <= 0.00)
		cost =	FindIneiCosts 
				(
					"L",
					comm_rec.est_no, 
					hhbrHash
				);

	return (cost);
}

void
ProcessSorted (
 void)
{
	char	*sptr;
	long	hhbrHash;
	char	dCode [2];
	int	valid = 0;

	first_time = TRUE;
	InitValues ();
	if (PRINT)
		HeadingOutput ();
	
	fsort = dsort_sort (fsort,"sk_abc_val");

	sptr = _SortRead (fsort);
	while (sptr != (char *) 0)
	{
		if (SALES)
		{
			workExtendSales 	= atof (srt_offset [0]);
			workExtendMargin 	= atof (srt_offset [1]);
		}
		else
		{
			workExtendMargin 	= atof (srt_offset [0]);
			workExtendSales 	= atof (srt_offset [1]);
		}
		eOnHand = atof (srt_offset [2]);

		workQtySold = (float) (atof (srt_offset [3]));
		weeksDemand = (float) (atof (srt_offset [4]));
		xOnHand 	= (float) (atof (srt_offset [5]));
		hhbrHash 	= atol (srt_offset [6]);
		sprintf (dCode,"%-1.1s", srt_offset [7]);

		CalculateCum (dCode);
		valid = SetCode (dCode);

		inmr_rec.hhbr_hash	=	hhbrHash;
		cc = find_rec (inmr,&inmr_rec, COMPARISON, (UPD) ? "u" : "r");
		if (!cc && valid)
		{
			switch (action)
			{
			case	1:
				MainPrintRoutine (dCode);
				break;
			case	2:
				Update (dCode);
				break;
			case	3:
				MainPrintRoutine (dCode);
				Update (dCode);
				break;
			}
		}

		if (!BY_CO)
			abc_unlock (inmr);

		InitValues ();

		sptr = _SortRead (fsort);
	}
	sort_delete (fsort,"sk_abc_val");
}

void
InitValues (
 void)
{
	workExtendSales 	= 0.00;
	workExtendMargin 	= 0.00;
	eOnHand 		= 0.00;
	cumSalesPc 		= 0.00;
	cumMarginPc	 	= 0.00;
	cumOnHandPc 	= 0.00;
	strcpy (previousCode, (first_time) ? " " : updateCode);
}

void
CalculateCum (
 char *add)
{
	if (add [0] != 'D')
	{
		cumSales += workExtendSales;
		cumMargin += workExtendMargin;
		cumOnHand += eOnHand;

		if (extendSales [2] != 0.00)
			cumSalesPc = (float) ((cumSales / extendSales [2]) * 100.00);
		if (margin [2] != 0.00)
			cumMarginPc = (float) ((cumMargin / margin [2]) * 100.00 * 100.00);
		if (extendOnHand [2] != 0.00)
			cumOnHandPc = (float) ((cumOnHand / extendOnHand [2]) * 100.00);
	}
}

int
SetCode (
 char *dCode)
{
	if (dCode [0] == 'D')
	{
		strcpy (updateCode,"D");
		return (EXIT_FAILURE);
	}

	if (SALES)
	{
		if (cumSalesPc <= codePercent [0])
		{
			strcpy (updateCode,"A");
			return (EXIT_FAILURE);
		}
		if (cumSalesPc > codePercent [0] && cumSalesPc <= codePercent [1])
		{
			strcpy (updateCode,"B");
			return (EXIT_FAILURE);
		}
		if (cumSalesPc > codePercent [1])
		{
			strcpy (updateCode,"C");
			return (EXIT_FAILURE);
		}
	}
	if (MARGIN)
	{
		if (cumMarginPc <= codePercent [0])
		{
			strcpy (updateCode,"A");
			return (EXIT_FAILURE);
		}
		if (cumMarginPc > codePercent [0] && cumMarginPc <= codePercent [1])
		{
			strcpy (updateCode,"B");
			return (EXIT_FAILURE);
		}
		if (cumMarginPc > codePercent [1])
		{
			strcpy (updateCode,"C");
			return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}

int
CheckForValid (
 void)
{
	if (useBreak [0] == 'Y' && updateCode [0] == 'A')
		return (EXIT_FAILURE);
	if (useBreak [1] == 'Y' && updateCode [0] == 'B')
		return (EXIT_FAILURE);
	if (useBreak [2] == 'Y' && updateCode [0] == 'C')
		return (EXIT_FAILURE);
	if (useBreak [3] == 'Y' && updateCode [0] == 'D')
		return (EXIT_FAILURE);
	return (EXIT_SUCCESS);
}

void
MainPrintRoutine (
 char *dCode)
{
	char	sub_head [13];
	char	header_buf [152];
	float	stockHolding = 0.00;

	if (!CheckForValid ())
		return;

	if (strcmp (updateCode,previousCode) != 0)
	{
		sprintf (sub_head," ABC CODE %s ",updateCode);
		expand (header_buf,sub_head);
		fprintf (fout,".PD!  %-151.151s  !\n",header_buf);
		if (!first_time)
			fprintf (fout,".PA\n");
	}


	if (dCode [0] != 'D')
	{
		if (weeksDemand != 0.00)
			stockHolding = xOnHand / weeksDemand;

		fprintf (fout,"!%-16.16s ",inmr_rec.item_no);
		fprintf (fout,"! %8.0f",workQtySold);
		fprintf (fout,"!%12.2f",workExtendSales);
		fprintf (fout,"!%12.2f",cumSales);
		fprintf (fout,"!%6.1f",cumSalesPc);

		fprintf (fout,"!%12.2f",workExtendMargin);
		fprintf (fout,"!%12.2f",cumMargin);
		fprintf (fout,"!%6.1f",cumMarginPc);

		fprintf (fout,"! %8.0f",xOnHand);
		fprintf (fout,"!%12.2f",eOnHand);
		fprintf (fout,"!%12.2f",cumOnHand);
		fprintf (fout,"!%6.1f",cumOnHandPc);
		fprintf (fout,"!%11.2f",weeksDemand);
		fprintf (fout,"!%6.1f!\n",stockHolding);
	}
	else
	{
		fprintf (fout,"!%-16.16s ",inmr_rec.item_no);
		fprintf (fout,"! %8.0f",workQtySold);
		fprintf (fout,"!%12.2f",workExtendSales);
		fprintf (fout,"!%12.12s"," ");
		fprintf (fout,"!%6.6s"," ");

		fprintf (fout,"!%12.2f",workExtendMargin);
		fprintf (fout,"!%12.12s"," ");
		fprintf (fout,"!%6.6s"," ");

		fprintf (fout,"! %8.0f",xOnHand);
		fprintf (fout,"!%12.2f",eOnHand);
		fprintf (fout,"!%12.12s"," ");
		fprintf (fout,"!%6.6s"," ");
		fprintf (fout,"!%11.2f",weeksDemand);
		fprintf (fout,"!%6.1f!\n",stockHolding);
	}
	fflush (fout);
	first_time = FALSE;
}

void
Update (
 char *dCode)
{

	if (BY_CO)
		UpdateInmr ();
	if (BY_BR)
		UpdateInei ();
	if (BY_WH)
		UpdateIncc ();

	UpdateInim ();
}

void
UpdateInmr (
 void)
{
	if (inmr_rec.abc_update [0] != 'N')
	{
		strcpy (inmr_rec.abc_code,updateCode);
		cc = abc_update (inmr,&inmr_rec);
		if (cc)
			file_err (cc, inmr, "DBUPDATE");
	}
	else
		abc_unlock (inmr);
}

void
UpdateInei (
 void)
{
	cc = FindInei (inmr_rec.hhbr_hash, comm_rec.est_no, "u");
	if (!cc && ineiRec.abc_update [0] != 'N')
	{
		strcpy (ineiRec.abc_code,updateCode);
		cc = abc_update (inei,&ineiRec);
		if (cc)
			file_err (cc, inei, "DBUPDATE");
	}
	else
		abc_unlock (inei);
}

void
UpdateIncc (
 void)
{
	abc_selfield (incc,"incc_id_no");
	incc_rec.hhcc_hash = hhccHash [0];
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (incc,&incc_rec, COMPARISON,"u");
	if (!cc && incc_rec.abc_update [0] != 'N')
	{
		strcpy (incc_rec.abc_code,updateCode);
		cc = abc_update (incc,&incc_rec);
		if (cc)
			file_err (errno, "incc", "DBUPDATE");
	}
	else
		abc_unlock (incc);
}

void
UpdateInim (
 void)
{
	strcpy (inim_rec.co_no,comm_rec.co_no);
	strcpy (inim_rec.est_no, (BY_CO) ? "  " : comm_rec.est_no);
	strcpy (inim_rec.cc_no,  (BY_WH) ? comm_rec.cc_no : "  ");
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (inim,&inim_rec, COMPARISON,"u");
	if (!cc)
	{
		inim_rec.movement = movement;
		inim_rec.a_class = codePercent [0];
		inim_rec.b_class = codePercent [1] - codePercent [0];
		inim_rec.c_class = codePercent [2] - codePercent [1];
		cc = abc_update (inim,&inim_rec);
		if (cc)
			file_err (cc, inim, "DBUPDATE");
	}
	else
	{
		abc_unlock (inim);

		inim_rec.movement = movement;
		inim_rec.a_class = codePercent [0];
		inim_rec.b_class = codePercent [1] - codePercent [0];
		inim_rec.c_class = codePercent [2] - codePercent [1];
		cc = abc_add (inim,&inim_rec);
		if (cc)
			file_err (cc, inim, "DBADD");
	}
}

void
HeadingOutput (
 void)
{
	if ((fout = popen ("pformat","w")) == NULL)
		file_err (errno, "pformat", "POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout,".LP%d\n",local_rec.printerNumber);

	fprintf (fout,".17\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".ELISTING OF ABC CODES FOR INVENTORY\n");
	fprintf (fout,".E%s\n",clip (comm_rec.co_name));
	if (BY_CO)
	{
		fprintf (fout,".EREPORT BY COMPANY\n");
		fprintf (fout,".ECO %s\n",comm_rec.co_no);
	}
	if (BY_BR)
	{
		fprintf (fout,".EREPORT BY BRANCH\n");
		fprintf (fout,".EBR : %s %s\n",
				comm_rec.est_no,clip (comm_rec.est_name));
	}
	if (BY_WH)
	{
		fprintf (fout,".EREPORT BY WAREHOUSE\n");
		fprintf (fout,".EBR :%s  %s WH :%s  %s\n",
				comm_rec.est_no, clip (comm_rec.est_name),
				comm_rec.cc_no,clip (comm_rec.cc_name));
	}
	fprintf (fout,".EAS AT %s\n",SystemTime ());

	fprintf (fout,"Sales History of %2d Months;     ",period);
	fprintf (fout,"Sort By %s\n", (SALES) ? "Sales Value" : "  Margin   ");
	fprintf (fout,"Using Percentage Breaks of :   ");
	fprintf (fout,"Code A : %4.1f%% Code B : %4.1f%% Code C : %4.1f%%\n",
		codePercent [0], codePercent [1], codePercent [2]);

	fprintf (fout,".B1\n");

	fprintf (fout,".R==================================================");
	fprintf (fout,"==================================================");
	fprintf (fout,"==================================================");
	fprintf (fout,"========\n");

	fprintf (fout,"==================================================");
	fprintf (fout,"==================================================");
	fprintf (fout,"==================================================");
	fprintf (fout,"========\n");

	fprintf (fout,"!                 ");
	fprintf (fout,"!%2d MONTHS",period);
	fprintf (fout,"! %2d MONTHS  ",period);
	fprintf (fout,"! %2d MONTHS  ",period);
	fprintf (fout,"!CUMUL.");
	fprintf (fout,"! %2d MONTHS  ",period);
	fprintf (fout,"! %2d MONTHS  ",period);
	fprintf (fout,"!CUMUL.");
	fprintf (fout,"!  STOCK  ");
	fprintf (fout,"!   STOCK    ");
	fprintf (fout,"! CUMULATIVE ");
	fprintf (fout,"!CUMUL.");
	fprintf (fout,"!  WEEK'S   ");
	fprintf (fout,"!STOCK !\n");

	fprintf (fout,"!  ITEM NUMBER    ");
	fprintf (fout,"!  SALES  ");
	fprintf (fout,"!   SALES    ");
	fprintf (fout,"!   SALES    ");
	fprintf (fout,"!SALES ");
	fprintf (fout,"!   GROSS    ");
	fprintf (fout,"!    G.M.    ");
	fprintf (fout,"! G.M. ");
	fprintf (fout,"!         ");
	fprintf (fout,"!            ");
	fprintf (fout,"!    STOCK   ");
	fprintf (fout,"!STOCK ");
	fprintf (fout,"!  DEMAND   ");
	fprintf (fout,"!HOLDG.!\n");

	fprintf (fout,"!                 ");
	fprintf (fout,"! (UNITS) ");
	fprintf (fout,"! VAL. (COST)");
	fprintf (fout,"! CUMULATIVE ");
	fprintf (fout,"!PERCNT");
	fprintf (fout,"!   MARGIN   ");
	fprintf (fout,"! CUMULATIVE ");
	fprintf (fout,"!PERCNT");
	fprintf (fout,"! (UNITS) ");
	fprintf (fout,"!   VALUE    ");
	fprintf (fout,"!   VALUE    ");
	fprintf (fout,"!PERCNT");
	fprintf (fout,"!           ");
	fprintf (fout,"!IN WKS!\n");

	PrintLine ();
	fflush (fout);
}

void
PrintLine (
 void)
{
	fprintf (fout,"!-----------------");
	fprintf (fout,"!---------");
	fprintf (fout,"!------------");
	fprintf (fout,"!------------");
	fprintf (fout,"!------");
	fprintf (fout,"!------------");
	fprintf (fout,"!------------");
	fprintf (fout,"!------");
	fprintf (fout,"!---------");
	fprintf (fout,"!------------");
	fprintf (fout,"!------------");
	fprintf (fout,"!------");
	fprintf (fout,"!-----------");
	fprintf (fout,"!------!\n");

	fflush (fout);
}

/*-----------------------
| Save offsets for each |
| numerical field.      |
-----------------------*/
char*
_SortRead (
 FILE*              srt_fil)
{
	char	*sptr;
	char	*tptr;
	int	fld_no = 1;

	sptr = sort_read (srt_fil);

	if (!sptr)
	{
		return (sptr);
	}

	srt_offset[0] = sptr;

	tptr = sptr;
	while (fld_no < 8)
	{
		tptr = strchr (tptr, 1);
		if (!tptr)
			break;
		*tptr = 0;
		tptr++;

		srt_offset[fld_no++] = sptr + (tptr - sptr);
	}

	return (sptr);
}
