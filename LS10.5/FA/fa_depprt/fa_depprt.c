/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( fa_depprt.c  )                                   |
|  Program Desc  : ( Fixed Asset Depreciation Schedule report for   ) |
|                  ( tax and internal.                              ) |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Updates files : See /usr/ver(x)/DOCS/Programs                      |
|---------------------------------------------------------------------|
|  Date Written  : (15/05/1997)    | Author       : Scott B Darrow    |
|---------------------------------------------------------------------|
|  Date Modified : (12/09/97)    | Modified  by : Ana Marie Tario     |
|  Date Modified : (17/08/99)    | Modified  by : Mars dela Cruz      |
|   Comments     : (12/09/97) - Incorporated multilingual conversion  |
|                :              and DMY4 date.					      |
|                : (15/10/97) - Inserted additional ML().             |
|                : (17/08/99) - Ported to ANSI standards.             |
|                :                                                    |
|                :                                                    |
| $Log: fa_depprt.c,v $
| Revision 5.3  2002/07/17 09:57:11  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:13:10  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:25:53  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:05:50  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:26:23  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:14:32  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:55:17  gerry
| forced Revsion No start 2.0 Rel-15072000
|
| Revision 1.18  1999/12/23 20:35:11  cam
| Changes for GVision compatibility.  Move description fields 3 to the right
| and use separate variables for their values.
|
| Revision 1.17  1999/12/06 01:46:57  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.16  1999/11/17 06:40:01  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.15  1999/11/08 04:54:45  scott
| Updated due to warnings using -Wall flag on compiler.
|
| Revision 1.14  1999/10/12 05:18:02  scott
| Updated for get_mend and get_mbeg
|
| Revision 1.13  1999/10/01 07:48:36  scott
| Updated for standard function calls.
|
| Revision 1.12  1999/09/29 10:10:38  scott
| Updated to be consistant on function names.
|
| Revision 1.11  1999/09/17 07:26:25  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.10  1999/09/16 02:49:16  scott
| Updated from Ansi Project.
|
| Revision 1.9  1999/09/13 09:57:17  marlyn
| Ported to ANSI standards.
|
| Revision 1.8  1999/06/14 23:57:35  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: fa_depprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/FA/fa_depprt/fa_depprt.c,v 5.3 2002/07/17 09:57:11 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_fa_mess.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <twodec.h>
#include <std_decs.h>

#define	INTERNAL	(ReportType[0]	==	'I')
#define	TAX			(ReportType[0]	==	'T')
#define	CF			comma_fmt

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_crd_date"},
	};

	int comm_no_fields = 6;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tes_name[41];
		long	tcrd_date;
	} comm_rec;

	/*=============================+
	 | Fixed Assets Master Record. |
	 +=============================*/
#define	FAMR_NO_FIELDS	33

	struct dbview	famr_list [FAMR_NO_FIELDS] =
	{
		{"famr_co_no"},
		{"famr_ass_group"},
		{"famr_ass_no"},
		{"famr_famr_hash"},
		{"famr_ass_desc1"},
		{"famr_ass_desc2"},
		{"famr_ass_desc3"},
		{"famr_ass_desc4"},
		{"famr_pur_date"},
		{"famr_ass_life"},
		{"famr_cost_price"},
		{"famr_disp_date"},
		{"famr_disp_price"},
		{"famr_gl_crd_acc"},
		{"famr_gl_dbt_acc"},
		{"famr_gl_ass_acc"},
		{"famr_f_y_rule"},
		{"famr_f_y_amt"},
		{"famr_dep_rule"},
		{"famr_max_deprec"},
		{"famr_priv_use_tax"},
		{"famr_tax_open_val"},
		{"famr_tax_dtype"},
		{"famr_tax_pa_flag"},
		{"famr_tax_d_pc"},
		{"famr_tax_d_amt"},
		{"famr_int_open_val"},
		{"famr_int_dtype"},
		{"famr_int_pa_flag"},
		{"famr_int_d_pc"},
		{"famr_int_d_amt"},
		{"famr_gl_updated"},
		{"famr_stat_flag"}
	};

	struct tag_famrRecord
	{
		char	co_no [3];
		char	ass_group [6];
		char	ass_no [6];
		long	famr_hash;
		char	ass_desc[4] [41];
		Date	pur_date;
		char	ass_life [8];
		Money	cost_price;
		Date	disp_date;
		Money	disp_price;
		char	gl_crd_acc [17];
		char	gl_dbt_acc [17];
		char	gl_ass_acc [17];
		char	f_y_rule [2];
		Money	f_y_amt;
		char	dep_rule [2];
		Money	max_deprec;
		float	priv_use_tax;
		Money	tax_open_val;
		char	tax_dtype [2];
		char	tax_pa_flag [2];
		float	tax_d_pc;
		Money	tax_d_amt;
		Money	int_open_val;
		char	int_dtype [2];
		char	int_pa_flag [2];
		float	int_d_pc;
		Money	int_d_amt;
		Date	gl_updated;
		char	stat_flag [2];
	}	famr_rec;

	/*================================+
	 | Fixed Asset Transactions file. |
	 +================================*/
#define	FATR_NO_FIELDS	4

	struct dbview	fatr_list [FATR_NO_FIELDS] =
	{
		{"fatr_co_no"},
		{"fatr_group"},
		{"fatr_group_desc"},
		{"fatr_stat_flag"}
	};

	struct tag_fatrRecord
	{
		char	co_no [3];
		char	group [6];
		char	group_desc [41];
	}	fatr_rec;

	/*=======================================+
	 | Fixed asset General Ledger work file. |
	 +=======================================*/
#define	FAGL_NO_FIELDS	11

	struct dbview	fagl_list [FAGL_NO_FIELDS] =
	{
		{"fagl_famr_hash"},
		{"fagl_posted"},
		{"fagl_tran_date"},
		{"fagl_dep_rule"},
		{"fagl_crd_acc"},
		{"fagl_dbt_acc"},
		{"fagl_int_amt"},
		{"fagl_tax_amt"},
		{"fagl_int_pc"},
		{"fagl_tax_pc"},
		{"fagl_stat_flag"}
	};

	struct tag_faglRecord
	{
		long	famr_hash;
		char	posted [2];
		Date	tran_date;
		char	dep_rule [2];
		char	crd_acc [17];
		char	dbt_acc [17];
		Money	int_amt;
		Money	tax_amt;
		float	int_pc;
		float	tax_pc;
		char	stat_flag [2];
	}	fagl_rec;

	char	*data = "data",
			*comm = "comm",
			*famr = "famr",
			*fatr = "fatr",
			*fagl = "fagl";

	int		lpno;

	FILE	*fout;

	char	Lower[6],
			Upper[6];

extern	int	TruePosition;
extern	int	EnvScreenOK;
char	ReportType[2];

double	GroupTotal [6],
		GrandTotal [6];
double	AccDep			= 0.00;
double	DepThisPeriod	= 0.00;
double	BookValue		= 0.00;

char	PV[6][21];
char	disp_str[300];
int 	CheckAsset (char *);
char	*DMSK	=	"NNNN,NNN,NNN.NN";
char	*BlankDesc	=	"                                        ";

/*============================ 
| local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	back[2];
	char	backDesc[5];
	char	onight[2];
	char	onightDesc[5];
	int		lpno;
	char	lp_str[3];
	char	StartAssetGroup[6];
	char	StartAssetDesc[41];
	char	EndAssetGroup[6];
	char	EndAssetDesc[41];
			
} local_rec;

static	struct	var	vars[]	=	
{
	{1, LIN, "StartAssetGroup", 4, 2, CHARTYPE, 
		"UUUUU", "          ", 
		" ", " ", "Start Asset Group code   : ", "Enter Start Asset group code. [SEARCH] available ", 
		NE, NO, JUSTLEFT, "", "", local_rec.StartAssetGroup}, 
	{1, LIN, "StartAssetDesc", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Start Asset Group Desc.  : ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.StartAssetDesc}, 
	{1, LIN, "EndAssetGroup", 6, 2, CHARTYPE, 
		"UUUUU", "          ", 
		" ", " ", "Start Asset Group code   : ", "Enter Start Asset group code. [SEARCH] available ", 
		NE, NO, JUSTLEFT, "", "", local_rec.EndAssetGroup}, 
	{1, LIN, "EndAssetDesc", 7, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "End Asset Group Desc.    : ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.EndAssetDesc}, 
	{1, LIN, "lpno", 9, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer number           : ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno}, 
	{1, LIN, "back", 10, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background               : ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back}, 
	{1, LIN, "back_desc", 10, 32, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTRIGHT, "", "", local_rec.backDesc}, 
	{1, LIN, "onight", 11, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight                : ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight}, 
	{1, LIN, "onight_desc", 11, 32, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTRIGHT, "", "", local_rec.onightDesc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 

};

/*======================
| Function Prototypes  |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void PrintHeading (void);
void ProcessFile (void);
void ProcAsset (char *AssGroup);
void PrintGroupTotals (void);
void PrintGrandTotals (void);
void CalcClosing (void);
void run_prog (char *prog_name);
int spec_valid (int field);
int CheckAsset (char *AssGroup);
int SrchFatr (char *key_val);
int heading (int scn);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char *argv[])
{
   	TruePosition	=	TRUE;
	EnvScreenOK		=	FALSE;

	if (argc != 2 && argc != 5)
	{
		print_at (0,0,mlFaMess700,argv[0]);
		print_at (1,0,mlFaMess701,argv[0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	sprintf (ReportType,"%-1.1s", argv[1]);

	/*===========================
	| Open main database files. |  
	===========================*/
	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec );

	if (argc == 5)
	{
		lpno = atoi(argv[2]);
		sprintf (Lower,"%-5.5s",argv[3]);
		sprintf (Upper,"%-5.5s",argv[4]);

		sprintf (err_str,"Processing %s Asset Register.", 
									(TAX) ? "Tax" : "Internal");

		dsp_screen (err_str,comm_rec.tco_no,comm_rec.tco_name);
		PrintHeading ();
		ProcessFile ();
		fprintf (fout,".EOF\n");
		pclose (fout);
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

	while (prog_exit == 0)
	{
		/*=====================
		| Reset control flags |
		=====================*/
		entry_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;

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

		run_prog (argv[0]);
		prog_exit = 1;
	}
	shutdown_prog();
	return (EXIT_SUCCESS);
}

void
run_prog (
 char *prog_name)
{	
	rset_tty ();

	clear ();
	print_at (0,0,ML(mlStdMess035));
	strcpy (err_str,ML(mlFaMess020));
	shutdown_prog ();

	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onight[0] == 'Y')
	{ 
		if (fork() == 0)
		{
			execlp
			(
				"ONIGHT",
				"ONIGHT",
				prog_name,
				ReportType,
				local_rec.lp_str,
				local_rec.StartAssetGroup,
				local_rec.EndAssetGroup,
				err_str,(char *)0
			);
        }
		else
			return;
	}

	else
	/*====================================
	| Test for forground or background . |
	====================================*/
	if (local_rec.back[0] == 'Y') 
	{
		if (fork() != 0)
			return;
		else
		{
			execlp
			(
				prog_name,
				prog_name,
				ReportType,
				local_rec.lp_str,
				local_rec.StartAssetGroup,
				local_rec.EndAssetGroup,
				(char *)0
			);
		}
	}
	else 
	{
		execlp
		( 
			prog_name,
			prog_name,
			ReportType,
			local_rec.lp_str,
			local_rec.StartAssetGroup,
			local_rec.EndAssetGroup,
			(char *)0
		);
		return;
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
	abc_dbopen(data);
	open_rec(famr, famr_list, FAMR_NO_FIELDS, "famr_id_no");
	open_rec(fatr, fatr_list, FATR_NO_FIELDS, "fatr_id_no");
	open_rec(fagl, fagl_list, FAGL_NO_FIELDS, "fagl_famr_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose(famr);
	abc_fclose(fatr);
	abc_fclose(fagl);
	abc_dbclose(data);
}


int
spec_valid (
 int field)
{

	/*----------------------------
	| Validate Asset group code. |
	----------------------------*/
	if (LCHECK ("StartAssetGroup"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.StartAssetGroup, "     ");
			strcpy (local_rec.StartAssetDesc, "Start of Range.");
			DSP_FLD ("StartAssetDesc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchFatr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (fatr_rec.co_no,comm_rec.tco_no);
		strcpy (fatr_rec.group,local_rec.StartAssetGroup);
		cc = find_rec (fatr, &fatr_rec, COMPARISON, "r");
		if (cc)
		{
			print_err(ML(mlFaMess003));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.StartAssetDesc, fatr_rec.group_desc);
		DSP_FLD ("StartAssetDesc");
		return (EXIT_SUCCESS);
	}
	/*----------------------------
	| Validate Asset group code. |
	----------------------------*/
	if (LCHECK ("EndAssetGroup"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.EndAssetGroup, "~~~~~");
			strcpy (local_rec.EndAssetDesc, "End of Range.");
			DSP_FLD ("EndAssetDesc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchFatr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (fatr_rec.co_no,comm_rec.tco_no);
		strcpy (fatr_rec.group,local_rec.EndAssetGroup);
		cc = find_rec (fatr, &fatr_rec, COMPARISON, "r");
		if (cc)
		{
			print_err(ML(mlFaMess003));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.EndAssetDesc, fatr_rec.group_desc);
		DSP_FLD ("EndAssetDesc");
		return (EXIT_SUCCESS);
	}

	if ( LCHECK("lpno") )
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.lp_str,"%d",local_rec.lpno);
		return (EXIT_SUCCESS);
	}

	if ( LCHECK("back") )
	{
		strcpy (local_rec.backDesc,(local_rec.back[0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("back_desc"); 
		return (EXIT_SUCCESS);
	}

	if ( LCHECK("onight") )
	{
		strcpy (local_rec.onightDesc,(local_rec.onight[0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("onight_desc") ;
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
PrintHeading (
 void)
{
	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in opening pformat During (DBPOPEN)",errno,PNAME);

	fprintf (fout,".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout,".LP%d\n",lpno);

	fprintf (fout,".13\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".E%s\n",clip(comm_rec.tco_name));
	fprintf (fout,".B1\n");

	fprintf (fout,".EFIXED ASSET DEPRECIATION SCHEDULE (%s)\n", (TAX) ? "TAX" : "INTERNAL");

	fprintf (fout,".B1\n");
	fprintf (fout,".EAS AT %-24.24s\n",SystemTime());

	fprintf (fout,".R========");
	fprintf (fout,"===========================================");
	fprintf (fout,"===========");
	fprintf (fout,"========");
	fprintf (fout,"=================");
	fprintf (fout,"=================");
	fprintf (fout,"=================");
	fprintf (fout,"=================");
	fprintf (fout,"==================\n");

	fprintf (fout,"========");
	fprintf (fout,"===========================================");
	fprintf (fout,"===========");
	fprintf (fout,"========");
	fprintf (fout,"=================");
	fprintf (fout,"=================");
	fprintf (fout,"=================");
	fprintf (fout,"=================");
	fprintf (fout,"==================\n");

	fprintf (fout,"| ASSET ");
	fprintf (fout,"|           ASSET DESCRIPTION              ");
	fprintf (fout,"|   DATE   ");
	fprintf (fout,"|LIFE IN");
	fprintf (fout,"|  ACQUISITION   ");
	fprintf (fout,"|   SALE PRICE   ");
	fprintf (fout,"|   ACCUMULATED  ");
	fprintf (fout,"|   DEPRECIATION ");
	fprintf (fout,"|   BOOK VALUE   |\n");

	fprintf (fout,"|NUMBER ");
	fprintf (fout,"|                                          ");
	fprintf (fout,"| ACQUIRED ");
	fprintf (fout,"| YEARS ");
	fprintf (fout,"|      COST      ");
	fprintf (fout,"|                ");
	fprintf (fout,"|  DEPRECIATION  ");
	fprintf (fout,"|   THIS PERIOD  ");
	fprintf (fout,"|                |\n");

	fprintf (fout,"|-------");
	fprintf (fout,"|------------------------------------------");
	fprintf (fout,"|----------");
	fprintf (fout,"|-------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------|\n");

	fflush (fout);
}

/*=============================
| Process Fixed Asset groups. |
=============================*/
void
ProcessFile (
 void)
{
	strcpy (fatr_rec.co_no,comm_rec.tco_no);
	strcpy (fatr_rec.group,Lower);
	cc = find_rec (fatr, &fatr_rec, GTEQ, "r");
	while (!cc && !strcmp (fatr_rec.co_no,comm_rec.tco_no) &&
				  strcmp (fatr_rec.group, Lower ) >= 0 &&
				  strcmp (fatr_rec.group, Upper ) <= 0)
	{
		if (CheckAsset ( fatr_rec.group ))
			ProcAsset( fatr_rec.group );

		cc = find_rec (fatr, &fatr_rec, NEXT, "r");
	}
	PrintGrandTotals ();
}

/*=======================
| Process Fixed Assets. |
=======================*/
void
ProcAsset (
 char	*AssGroup)
{
	char	*expand(char *, char *);
	int		i;

	expand (err_str, fatr_rec.group_desc);
	fprintf (fout, "| %-5.5s - (%80.80s)%63.63s|\n",fatr_rec.group,err_str," ");
	strcpy (famr_rec.co_no, comm_rec.tco_no);
	strcpy (famr_rec.ass_group, AssGroup);
	strcpy (famr_rec.ass_no, "     ");
	cc = find_rec ("famr", &famr_rec, GTEQ, "r");
	while (!cc && !strcmp (famr_rec.co_no, comm_rec.tco_no) &&
				  !strcmp (famr_rec.ass_group, AssGroup))
	{
		if (famr_rec.stat_flag[0] == 'D')
		{
			cc = find_rec ("famr", &famr_rec, NEXT, "r");
			continue;
		}
		CalcClosing ();
		sprintf (PV[0], CF (DOLLARS (famr_rec.cost_price), DMSK));
		sprintf (PV[1], CF (DOLLARS (famr_rec.disp_price), DMSK));
		sprintf (PV[2], CF (AccDep, DMSK));
		sprintf (PV[3], CF (DepThisPeriod, DMSK));
		sprintf (PV[4], CF (BookValue, DMSK));

		fprintf (fout,"| %5.5s ", famr_rec.ass_no);
		fprintf (fout,"| %40.40s ", famr_rec.ass_desc[0]);
		fprintf (fout,"|%-10.10s", DateToString (famr_rec.pur_date));
		fprintf (fout,"|%-7.7s", 	famr_rec.ass_life);
		fprintf (fout,"|%15.15s ", PV[0]);
		if (famr_rec.disp_date > 0L)
		{
			fprintf (fout,"|%15.15s ", PV[1]);
			fprintf (fout,"|                ");
			fprintf (fout,"|                ");
			fprintf (fout,"|                |\n");
		}
		else
		{
			fprintf (fout,"|                ");
			fprintf (fout,"|%15.15s ", PV[2]);
			fprintf (fout,"|%15.15s ", PV[3]);
			fprintf (fout,"|%15.15s |\n", PV[4]);
		}

		GroupTotal [0]	+=	DOLLARS (famr_rec.cost_price);
		GroupTotal [1]	+=  DOLLARS (famr_rec.disp_price);
		GrandTotal [0]	+=	DOLLARS (famr_rec.cost_price);
		GrandTotal [1]	+=  DOLLARS (famr_rec.disp_price);

		if (famr_rec.disp_date == 0L)
		{
			GroupTotal [2]	+=  AccDep;
			GroupTotal [3]	+=  DepThisPeriod;
			GroupTotal [4]	+=  BookValue;
			GrandTotal [2]	+=  AccDep;
			GrandTotal [3]	+=  DepThisPeriod;
			GrandTotal [4]	+=  BookValue;
		}
		dsp_process ("Asset No", famr_rec.ass_no);

		for (i = 1; i < 4; i++)
		{
			if (strcmp (famr_rec.ass_desc [i], BlankDesc))
			{
				fprintf (fout,"|       ");
				fprintf (fout,"| %40.40s ", famr_rec.ass_desc [i]);
				fprintf (fout,"|          ");
				fprintf (fout,"|       ");
				fprintf (fout,"|                ");
				fprintf (fout,"|                ");
				fprintf (fout,"|                ");
				fprintf (fout,"|                ");
				fprintf (fout,"|                |\n");
			}
		}

		cc = find_rec ("famr", &famr_rec, NEXT, "r");
	}
	PrintGroupTotals ();

	GroupTotal [0]	= 0.00;
	GroupTotal [1]	= 0.00;
	GroupTotal [2]	= 0.00;
	GroupTotal [3]	= 0.00;
	GroupTotal [4]	= 0.00;

	fprintf (fout,"|-------");
	fprintf (fout,"|------------------------------------------");
	fprintf (fout,"|----------");
	fprintf (fout,"|-------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------|\n");
}

void
PrintGroupTotals (
 void)
{
	sprintf (PV[0], CF (GroupTotal[0], DMSK));
	sprintf (PV[1], CF (GroupTotal[1], DMSK));
	sprintf (PV[2], CF (GroupTotal[2], DMSK));
	sprintf (PV[3], CF (GroupTotal[3], DMSK));
	sprintf (PV[4], CF (GroupTotal[4], DMSK));
	
	fprintf (fout,"|       ");
	fprintf (fout,"| **** ASSET GROUP TOTAL *****             ");
	fprintf (fout,"|          ");
	fprintf (fout,"|       ");
	fprintf (fout,"|%15.15s ", PV[0]);
	fprintf (fout,"|%15.15s ", PV[1]);
	fprintf (fout,"|%15.15s ", PV[2]);
	fprintf (fout,"|%15.15s ", PV[3]);
	fprintf (fout,"|%15.15s |\n", PV[4]);
}

void
PrintGrandTotals (
 void)
{
	sprintf (PV[0], CF (GrandTotal[0], DMSK));
	sprintf (PV[1], CF (GrandTotal[1], DMSK));
	sprintf (PV[2], CF (GrandTotal[2], DMSK));
	sprintf (PV[3], CF (GrandTotal[3], DMSK));
	sprintf (PV[4], CF (GrandTotal[4], DMSK));
	
	fprintf (fout,"|       ");
    fprintf (fout,"| **** ASSET GRAND TOTAL *****             ");
	fprintf (fout,"|          ");
	fprintf (fout,"|       ");
	fprintf (fout,"|%15.15s ", PV[0]);
	fprintf (fout,"|%15.15s ", PV[1]);
	fprintf (fout,"|%15.15s ", PV[2]);
	fprintf (fout,"|%15.15s ", PV[3]);
	fprintf (fout,"|%15.15s |\n", PV[4]);
}

int
CheckAsset ( 
 char	*AssGroup )
{
	strcpy (famr_rec.co_no, comm_rec.tco_no);
	strcpy (famr_rec.ass_group, AssGroup);
	strcpy (famr_rec.ass_no, "     ");
	cc = find_rec ("famr", &famr_rec, GTEQ, "r");
	if (!cc && !strcmp (famr_rec.co_no, comm_rec.tco_no) &&
			   !strcmp (famr_rec.ass_group, AssGroup))
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

void
CalcClosing (
 void)
{
	BookValue 		= DOLLARS (famr_rec.int_open_val);
	AccDep			= 0.00;
	DepThisPeriod	= 0.00;

	fagl_rec.famr_hash 	=	famr_rec.famr_hash;
	cc = find_rec ("fagl", &fagl_rec, GTEQ, "r");
	while (!cc && fagl_rec.famr_hash 	==	famr_rec.famr_hash)
	{
		if (fagl_rec.tran_date <= MonthEnd (comm_rec.tcrd_date))
		{
			if (MonthStart (fagl_rec.tran_date) == MonthStart (comm_rec.tcrd_date))
			{
				if (INTERNAL)
					DepThisPeriod = DOLLARS (fagl_rec.int_amt);
				else
					DepThisPeriod = DOLLARS (fagl_rec.tax_amt);
			}

			if (INTERNAL)
			{
				BookValue	-= DOLLARS (fagl_rec.int_amt);
				AccDep		+= DOLLARS (fagl_rec.int_amt);
			}
			else
			{
				BookValue	-= DOLLARS (fagl_rec.tax_amt);
				AccDep		+= DOLLARS (fagl_rec.tax_amt);
			}
		}
		
		cc = find_rec ("fagl", &fagl_rec, NEXT, "r");
	}
	if (BookValue < 0.00)
		BookValue = 0.00;
}

/*===================================
| Search for inventory master file. |
===================================*/
int
SrchFatr (
 char	*key_val)
{
	work_open ();
	save_rec ("#Asset group", "#Asset Group Description");
	strcpy (fatr_rec.co_no, comm_rec.tco_no);
	sprintf (fatr_rec.group, "%-5.5s", key_val);
	cc = find_rec (fatr, &fatr_rec, GTEQ, "r");
	while (!cc && !strncmp (fatr_rec.group, key_val, strlen (key_val)) && 
				  !strcmp (fatr_rec.co_no, comm_rec.tco_no))
	{
		cc = save_rec (fatr_rec.group, fatr_rec.group_desc);
		if (cc)
			break;

		cc = find_rec (fatr, &fatr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		sprintf (fatr_rec.group, "%-5.5s", " ");
		return(1);
	}
	strcpy (fatr_rec.co_no, comm_rec.tco_no);
	sprintf (fatr_rec.group, "%-5.5s", temp_str);
	cc = find_rec (fatr, &fatr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, fatr, "DBFIND");
	return(0);
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		strcpy (err_str, (TAX) ? ML(mlFaMess001) : ML(mlFaMess002));
		rv_pr (err_str,(80 - strlen(err_str)) / 2,0,1);
		move (0,1);
		line (80);

		box (0,3,80,8);
		move (1,8);
		line (79);

		move (0,19);
		line (80);
		move (0,21);
		print_at (20,0,ML(mlStdMess038),comm_rec.tco_no,clip(comm_rec.tco_name));
		print_at (21,0,ML(mlStdMess039),comm_rec.tes_no,clip(comm_rec.tes_name));
		move (0,22);
		line (80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

