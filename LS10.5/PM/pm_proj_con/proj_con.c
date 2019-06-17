/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( pm_proj_con.c )                                  |
|  Program Desc  : ( Project-Contractor Program.                  )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, pmpm,pmpc ,pmpr ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  pmpc,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Ross Baquillos  | Date Written  : 16/12/95         |
|---------------------------------------------------------------------|
|  Date Modified : (27/12/95)      | Modified by : Jojo M. Gatchalian |
|  Date Modified : (02/01/96)      | Modified by : Joy G. Medel       |
|  Date Modified : (03/01/96)      | Modified by : Rommel S. Maldia   |
|  Date Modified : (10/01/96)      | Modified by : Joy G. Medel       |
|  Date Modified : (23/01/96)      | Modified by : Joy G. Medel       |
|  Date Modified : (04/09/97)      | Modified by : Jiggs A Veloz      |
|  Date Modified : (16/10/97)      | Modified by : Elizabeth D. Paid  |
|  Date Modified : (17/09/1999)    | Modified by : Ramon A. Pacheco   |
|                                                                     |
|  Comments      :                                                	  |
|  (27/12/95)    : 				   - Schema Changes.                  |
|  (02/01/96)    : 				   - Fixed bugs.                      |
|  (03/01/96)    : 				   - Fixed bugs.                      |
|  (10/01/96)    : 				   - Added Contractor Lost status.    |
|  (23/01/96)    : 				   - Fixed Bugs.                      |
|  (04/09/97)    : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
|  (16/10/97)    : SEL change the length of pmpm_proj_no, pmpc_proj_no|
|  (17/09/1999)  : Ported to ANSI standards.                          |
|                :                                                    |
|                                                                     |
| $Log: proj_con.c,v $
| Revision 5.3  2002/07/25 11:17:30  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.2  2001/08/09 09:15:10  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:34:05  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:55  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:06  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:22  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:49:26  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:03:57  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.15  1999/11/16 02:56:14  scott
| Updated to correct warning errors found using -Wall
|
| Revision 1.14  1999/10/01 07:49:02  scott
| Updated for standard function calls.
|
| Revision 1.13  1999/09/29 10:11:50  scott
| Updated to be consistant on function names.
|
| Revision 1.12  1999/09/24 04:23:21  scott
| Updated from Ansi Project.
|
| Revision 1.11  1999/06/17 07:54:54  scott
| Updated for Log required for cvs and database name.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: proj_con.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PM/pm_proj_con/proj_con.c,v 5.3 2002/07/25 11:17:30 scott Exp $";

#include <pslscr.h>
#include <minimenu.h>
#include <ml_pm_mess.h>
#include <ml_std_mess.h>

typedef int BOOL;

#define	SEL_UPDATE	0
#define	SEL_IGNORE	1
#if 0
#define	SEL_DELETE	2
#endif
#define	SEL_DEFAULT	99

#define SLEEP_TIME	3

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"}
	};

	int	comm_no_fields = 4;

	struct tag_commRecord
	{
		int		termno;
		char	tco_no [3];
		char	tco_name [41];
		char	test_no [3];
	} comm_rec;

	/*=========================+
	 | External Salesman File. |
	 +=========================*/
#define	EXSF_NO_FIELDS	4

	struct dbview	exsf_list [EXSF_NO_FIELDS] =
	{
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_salesman"},
		{"exsf_area_code"}
	};

	struct tag_exsfRecord
	{
		char	co_no [3];
		char	salesman_no [3];
		char	salesman [41];
		char	area_code [3];
	}	exsf_rec;


MENUTAB upd_menu [] =
{
	{ " 1. UPDATE RECORD WITH CHANGES MADE.   ",
	  "" },
	{ " 2. IGNORE CHANGES JUST MADE TO RECORD.",
	  "" },
#ifdef SEL_DELETE
	{ " 3. DELETE RECORD.                     ",
	  "" },
#endif
	{ ENDMENU }
};

	/*====================
	| External Area file |
	====================*/
	struct dbview exaf_list [] =
	{
		{"exaf_co_no"},
		{"exaf_area_code"},
		{"exaf_area"},
	};

	int	exaf_no_fields = 3;

	struct tag_exafRecord
	{
		char	co_no [3];
		char	area_code [3];
		char	area_desc [41];
	} exaf_rec;


	char	branchNo[3];

	/*===================================+
	 | Project Monitoring Prospects File |
	 +===================================*/
#define	PMPR_NO_FIELDS	8

	struct dbview	pmpr_list [PMPR_NO_FIELDS] =
	{
		{"pmpr_prospect_no"},
		{"pmpr_name"},
		{"pmpr_adr1"},
		{"pmpr_adr2"},
		{"pmpr_adr3"},
		{"pmpr_phone_no"},
		{"pmpr_fax_no"},
		{"pmpr_sman_code"}
	};

	struct tag_pmprRecord
	{
		char	prospect_no [7];
		char	name [41];
		char	adr1 [41];
		char	adr2 [41];
		char	adr3 [41];
		char	phone_no [16];
		char	fax_no [16];
		char	sman_code [3];
	}	pmpr_rec;

	/*==========================================+
	 | Project Monitoring Prospect Master file. |
	 +==========================================*/
#define	PMPM_NO_FIELDS	14

	struct dbview	pmpm_list [PMPM_NO_FIELDS] =
	{
		{"pmpm_proj_no"},
		{"pmpm_title"},
		{"pmpm_date_entrd"},
		{"pmpm_date_start"},
		{"pmpm_date_end"},
		{"pmpm_consultant"},
		{"pmpm_contact_name"},
		{"pmpm_dl_adr1"},
		{"pmpm_dl_adr2"},
		{"pmpm_dl_adr3"},
		{"pmpm_phone_no"},
		{"pmpm_fax_no"},
		{"pmpm_mtrl_cost"},
		{"pmpm_status"}
	};

	struct tag_pmpmRecord
	{
		char	proj_no [9];
		char	title [41];
		long	date_entrd;
		long	date_start;
		long	date_end;
		char	consultant [41];
		char	contact_name [21];
		char	dl_adr1 [41];
		char	dl_adr2 [41];
		char	dl_adr3 [41];
		char	phone_no [16];
		char	fax_no [16];
		double	mtrl_cost;
		char	status [2];
	}	pmpm_rec;

	/*=========================+
	 | Project-Contractor File |
	 +=========================*/
#define	PMPC_NO_FIELDS	10

	struct dbview	pmpc_list [PMPC_NO_FIELDS] =
	{
		{"pmpc_proj_no"},
		{"pmpc_cont_no"},
		{"pmpc_type"},
		{"pmpc_parent_no"},
		{"pmpc_area"},
		{"pmpc_mtrl_cost"},
		{"pmpc_loyal_rat"},
		{"pmpc_ord_prob"},
		{"pmpc_sman_code"},
		{"pmpc_status"},
	};

	struct tag_pmpcRecord
	{
		char	proj_no [9];
		char	cont_no [7];
		char	type [2];
		char	parent_no [7];
		char	area [3];
		double	mtrl_cost;
		float	loyal_rat;
		float	ord_prob;
		char	sman_code [3];
		char	status [2];
	}	pmpc_rec;

	/*===================================+
	 | Customer Master File Base Record. |
	 +===================================*/
#define	CUMR_NO_FIELDS	6

	struct dbview	cumr_list [CUMR_NO_FIELDS] =
	{
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_dbt_name"},
		{"cumr_area_code"},
		{"cumr_sman_code"}
	};

	struct tag_cumrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	dbt_no [7];
		char	dbt_name [41];
		char    area_code [3];
		char	sman_code [3];
	}	cumr_rec;

#include 	<FindCumr.h>

	char	*data  = "data",
			*exsf  = "exsf",
			*exaf  = "exaf",
			*pmpm  = "pmpm",
			*pmpr  = "pmpr",
			*cumr  = "cumr",
			*pmpc  = "pmpc";

	char 	prev_type [2]	=	" ";
	
	int		pmpr_not_found 	= 	FALSE,
			cumr_not_found	=	FALSE;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	proj_no [9];
	char	type [2];
	char 	cont_no [7];
	char	area [3];
	char 	parent_no [7];
	char 	custpect_name [41];
	char 	parent_contractor [41];
	double  mtrl_cost;
	float	loyal_rat;
	float	ord_prob;
	char	sman_code [3];	
	char	dummy [11];
} local_rec;


static	struct	var	vars[] =
{
	{ 1, LIN, "pmpc_ref",	 3, 23, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "  Project Code      : ", "Enter Project Code [SEARCH] for references",
		NE, NO,  JUSTLEFT, "", "", local_rec.proj_no },
	{ 1, LIN, "pmpc_title",	 3, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "Enter Project Title",
		NA, NO, JUSTLEFT, "", "", pmpm_rec.title },
	{ 1, LIN, "cntr_type",5, 23, CHARTYPE,
		"U", "          ",
		" ", " ", "  Contractor Type   : ", "Enter (C)ustomer or (P)rospect",
		NO, NO, JUSTLEFT, "CP", "", local_rec.type },
	{ 1, LIN, "custpect_number",	 6, 23, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "  Contractor Number : ", "Enter Contractor Number[SEARCH] for references",
		NO, NO,  JUSTLEFT, "", "", local_rec.cont_no}, 
	{ 1, LIN, "custpect_name",	 6, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.custpect_name },
	{ 1, LIN, "pmpc_area",	 7, 23, CHARTYPE,
		"UU", "          ",
		" ", "", "  Contractor Area   : ", "Enter Contractor Area[SEARCH]",
		NO, NO,  JUSTRIGHT, "", "", local_rec.area },
	{ 1, LIN, "prospect_area_desc",	 7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", exaf_rec.area_desc },
	{ 1, LIN, "parent_contr",	 8, 23, CHARTYPE,
		"UUUUUU", "          ",
		" ", "0", "  Parent Contractor : ", "Enter Parent Contractor[SEARCH] for references",
		NO, NO,  JUSTLEFT, "", "", local_rec.parent_no },
	{ 1, LIN, "parent_cntr_name", 8, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.parent_contractor },
	{ 1, LIN, "est_value",	 10, 23, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "", "  Estimated Value   : ", "Enter Estimated Value of Materials",
		YES, NO, JUSTRIGHT, "0.00", "999999999.99", (char *) &local_rec.mtrl_cost },
	{ 1, LIN, "loyalty",	 11, 23, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "", "  Loyalty Rating    : ", "Enter Contractor's loyalty rating.",
		YES, NO, JUSTRIGHT, "0.00", "100.00", (char *) &local_rec.loyal_rat },
	{ 1, LIN, "probability", 12, 23, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "", "  Order Probability : ", "Enter Contractor's probability of ordering.",
		YES, NO, JUSTRIGHT, "0.00", "100.00", (char *) &local_rec.ord_prob },
	{ 1, LIN, "pmpc_sman",	 13, 23, CHARTYPE,
		"UU", "          ",
		" ", "", "  Salesman Number   : ", "Enter Salesman Number[SEARCH]",
		NO, NO, JUSTLEFT, "", "", local_rec.sman_code },
	{ 1, LIN, "proj_sman_name",	 13, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", exsf_rec.salesman },
	{ 1, LIN, "prjcon_status",14, 23, CHARTYPE,
		"U", "          ", 
		" ", "W", "  Status            : ", "Status: (A)ctive, Order (W)on or (L)ost, Contractor wo(N) or lo(S)t",
		YES, NO, JUSTLEFT, "AWLNS", "", pmpc_rec.status },
	{ 0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy }
};


static BOOL	newCode = FALSE;

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
int		spec_valid		(int);
void	SrchPmpm		(char *);
void	update			(void);
int		heading			(int);
void	SrchExsf		(char *);
void	SrchExaf		(char *);
void	SrchPmpr		(char *);
void	SrchPmpc		(char *);

#ifdef SEL_DELETE
BOOL	FrtyDelOk		(char *);
#endif

	int		envVarDbCo		=	0;
	char	branchNumber[3];


/*==========================
| Main processing routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char	*sptr;

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	set_masks ();

	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	sptr = chk_env ("DB_CO");
	envVarDbCo = (sptr == (char *) 0) ? 0 : atoi (sptr);
	strcpy (branchNumber, (envVarDbCo) ? comm_rec.test_no : " 0");

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit	= FALSE; 
		edit_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);
		FLD ("cntr_type") = YES;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading (1);
		scn_display (1);
		FLD ("cntr_type") = NA;
		edit (1);

		if (restart)
			continue;

		update ();

	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
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
	abc_dbopen (data);

	open_rec (pmpm, pmpm_list, PMPM_NO_FIELDS, "pmpm_proj_no");
	open_rec (pmpr, pmpr_list, PMPR_NO_FIELDS, "pmpr_prospect_no");
	open_rec (exaf, exaf_list, exaf_no_fields, "exaf_id_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (pmpc, pmpc_list, PMPC_NO_FIELDS, "pmpc_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (pmpm);
	abc_fclose (exsf);
	abc_fclose (exaf);
	abc_fclose (pmpc);
	abc_fclose (cumr);
	abc_fclose (pmpr);

	abc_dbclose (data);
}

int
spec_valid (
 int field)
{

	/*----------------------------
	| Validate Reference Number. |
	----------------------------*/
	if (LCHECK ("pmpc_ref"))
	{
		if (SRCH_KEY)
		{
			SrchPmpm (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (local_rec.proj_no, "        "))
			return (EXIT_FAILURE); 

		strcpy (pmpm_rec.proj_no, local_rec.proj_no);
		cc = find_rec (pmpm, &pmpm_rec, EQUAL, "r");
		if (cc)
		{
			/*----------------------------------
			| Project Reference is not on file!|
			----------------------------------*/
			errmess (ML (mlStdMess191));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.proj_no, pmpm_rec.proj_no);
		DSP_FLD ("pmpc_ref");
		DSP_FLD ("pmpc_title");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Contractor Type.|
	--------------------------*/
	if (LCHECK ("cntr_type"))
	{
		if (dflt_used)
			local_rec.type [0] = prev_type[0];
		
		if (local_rec.type [0] == ' ')
			return (EXIT_FAILURE);
		
		prev_type[0] = local_rec.type[0];
		DSP_FLD ("cntr_type");  
		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Validate Contractor Number. |
	-----------------------------*/
	if (LCHECK ("custpect_number"))
	{
		if (SRCH_KEY)
		{
			if (local_rec.type [0] == 'C')
			{
				CumrSearch (comm_rec.tco_no, branchNumber, temp_str);
				return (EXIT_SUCCESS);
			}
			else
			{
				SrchPmpr (temp_str);
				return (EXIT_SUCCESS);
			}
		}

		if (!strcmp (local_rec.cont_no, "      "))
			return (EXIT_FAILURE);

		if (local_rec.type [0] == 'P')
		{
			strcpy (pmpr_rec.prospect_no, zero_pad (local_rec.cont_no, 6));
			cc = find_rec (pmpr, &pmpr_rec, EQUAL, "r");
			if (!cc)
			{
				strcpy (local_rec.custpect_name, pmpr_rec.name);
				DSP_FLD("custpect_name");

				strcpy (exsf_rec.co_no,comm_rec.tco_no);
				sprintf (exsf_rec.salesman_no,"%-2.2s",pmpr_rec.sman_code);
				cc = find_rec ("exsf", &exsf_rec, COMPARISON, "r");
				if (cc)
					strcpy (exsf_rec.salesman_no, "");
				else
					strcpy (local_rec.sman_code, exsf_rec.salesman_no);
			}
			else
			{
				/*---------------------------
				| Contractor is not on file!|
				---------------------------*/
				errmess (ML (mlPmMess001));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		else
		{
			strcpy (cumr_rec.co_no, comm_rec.tco_no);
			strcpy (cumr_rec.est_no, branchNumber);
			strcpy (cumr_rec.dbt_no, zero_pad (local_rec.cont_no, 6));
			cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
			if (!cc)
			{
				strcpy (local_rec.custpect_name, cumr_rec.dbt_name);
				DSP_FLD("custpect_name");

				strcpy (exsf_rec.co_no, comm_rec.tco_no);
				strcpy (exsf_rec.salesman_no, cumr_rec.sman_code);
				cc = find_rec ("exsf", &exsf_rec, EQUAL, "r");
				if (cc)
					strcpy (exsf_rec.salesman_no, "");
				else
					strcpy (local_rec.sman_code, exsf_rec.salesman_no);
			}
			else
			{
				/*--------------------------
				|Contractor is not on file!|
				--------------------------*/
				errmess (ML (mlPmMess001));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}

		strcpy (pmpc_rec.proj_no, pmpm_rec.proj_no);
		strcpy (pmpc_rec.cont_no, local_rec.cont_no);
		cc = find_rec (pmpc, &pmpc_rec, EQUAL, "u");
		if (cc)
		{
			newCode = TRUE;
			local_rec.mtrl_cost = 0L;
			local_rec.loyal_rat = 0L;
			local_rec.ord_prob =  0L;
			strcpy (pmpc_rec.status, " "); 
		}
		else
		{
			newCode = FALSE;
			strcpy (exaf_rec.co_no, comm_rec.tco_no);
			strcpy (exaf_rec.area_code, pmpc_rec.area);

			cc = find_rec ("exaf", &exaf_rec, COMPARISON, "r");
			if (cc)
			{
				/*---------------------
				| Area is not on file!|
				---------------------*/
				print_mess (ML (mlStdMess108));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			strcpy (local_rec.area, exaf_rec.area_code);
			DSP_FLD ("pmpc_area");
			DSP_FLD ("prospect_area_desc");  
			entry_exit = TRUE;
		}

		strcpy (local_rec.parent_no, pmpc_rec.parent_no);
		DSP_FLD ("parent_contr");
	 	
		if (!newCode)
		{
			strcpy (exsf_rec.co_no, comm_rec.tco_no);
			sprintf (exsf_rec.salesman_no, "%-2.2s", pmpc_rec.sman_code);
			cc = find_rec ("exsf", &exsf_rec, COMPARISON, "r");
		}
	
		strcpy (local_rec.sman_code, exsf_rec.salesman_no);
		local_rec.mtrl_cost = pmpc_rec.mtrl_cost;	
		local_rec.loyal_rat = pmpc_rec.loyal_rat;	
		local_rec.ord_prob = pmpc_rec.ord_prob;	
		DSP_FLD ("pmpc_area");
		DSP_FLD ("pmpc_sman");
		DSP_FLD ("proj_sman_name");
		DSP_FLD ("est_value");
		DSP_FLD ("loyalty");
		DSP_FLD ("probability");
		
		if (!strcmp (pmpc_rec.parent_no, "0     ")) 
			strcpy (local_rec.parent_contractor, "Main Contractor"); 
		else
		{
			strcpy (pmpr_rec.prospect_no, pmpc_rec.parent_no);
			cc = find_rec (pmpr, &pmpr_rec, EQUAL, "r");
			if (!cc)	
				strcpy (local_rec.parent_contractor, pmpr_rec.name); 
			else
			{
				strcpy (cumr_rec.co_no, comm_rec.tco_no);
				strcpy (cumr_rec.est_no, branchNumber);
				strcpy (cumr_rec.dbt_no, zero_pad (local_rec.cont_no, 6));
				cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
				if (!cc)
					strcpy (local_rec.parent_contractor, cumr_rec.dbt_name); 
				else
					strcpy (local_rec.parent_contractor, "");
			}
		}
		DSP_FLD ("parent_cntr_name");
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate Area Code.|
	--------------------*/
	if (LCHECK ("pmpc_area"))
	{
		if (dflt_used)
		{
			if (local_rec.type [0] == 'P')
			{
				strcpy (pmpr_rec.prospect_no, zero_pad (local_rec.cont_no, 6));
				cc = find_rec (pmpr, &pmpr_rec, EQUAL, "r");
				if (!cc)
				{
					strcpy (exsf_rec.co_no, comm_rec.tco_no);
					sprintf (exsf_rec.salesman_no, "%-2.2s", pmpr_rec.sman_code);
					cc = find_rec ("exsf", &exsf_rec, COMPARISON, "r");
					if (cc)
						strcpy (local_rec.area, "");
					else
						strcpy (local_rec.area, exsf_rec.area_code);
				}
			}
			else
			{
				strcpy (cumr_rec.co_no, comm_rec.tco_no);
				strcpy (cumr_rec.est_no, branchNumber);
				strcpy (cumr_rec.dbt_no, zero_pad (local_rec.cont_no, 6));
				cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
				if (cc)
					strcpy (local_rec.area, "");
				else
					strcpy (local_rec.area, cumr_rec.area_code);
			}
			DSP_FLD ("pmpc_area");
		}

		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (local_rec.area, "  "))
		{
			/*--------------------------
			| Area Code Cannot be blank|
			--------------------------*/
			errmess (ML (mlStdMess108));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
	
		strcpy (exaf_rec.co_no, comm_rec.tco_no);
		strcpy (exaf_rec.area_code, local_rec.area);

		cc = find_rec ("exaf", &exaf_rec, COMPARISON, "r");
		if (cc)
		{
			/*---------------------
			| Area is not on file!|
			---------------------*/
			print_mess (ML (mlStdMess108));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		DSP_FLD ("pmpc_area");
		DSP_FLD ("prospect_area_desc"); 

		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Parent Contractor.|
	----------------------------*/
	if (LCHECK ("parent_contr"))
	{
		if (SRCH_KEY)
		{
			SrchPmpc (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || local_rec.parent_no [0] == '0')
		{
			strcpy (local_rec.parent_contractor, "Main Contractor.");
			DSP_FLD ("parent_cntr_name");
			return (EXIT_SUCCESS);
		}

		if (!strcmp (local_rec.parent_no, local_rec.cont_no))
		{
			/*---------------------------------------------------------------
			| Entered parent number is the same with the contractor number !|
			---------------------------------------------------------------*/
			errmess (ML (mlPmMess003));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		abc_selfield (pmpc, "pmpc_id_no3");
		strcpy (pmpc_rec.proj_no, local_rec.proj_no); 
		strcpy (pmpc_rec.cont_no, local_rec.cont_no);  
		strcpy (pmpc_rec.parent_no, local_rec.parent_no); 
		cc = find_rec (pmpc, &pmpc_rec, EQUAL, "r");
		if (!cc)
			newCode = FALSE;
		else
			newCode = TRUE;

		abc_selfield (pmpc, "pmpc_id_no");
		
		strcpy (pmpr_rec.prospect_no, local_rec.parent_no); 
		cc = find_rec (pmpr, &pmpr_rec, EQUAL, "r");
		if (!cc)
		{
			strcpy (local_rec.parent_contractor, pmpr_rec.name);
			pmpr_not_found = FALSE;
		}
		else
			pmpr_not_found = TRUE;
		
		strcpy (cumr_rec.co_no,  comm_rec.tco_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no, local_rec.parent_no);
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (!cc)
		{
			strcpy (local_rec.parent_contractor, cumr_rec.dbt_name);
			cumr_not_found = FALSE;
		}
		else
			cumr_not_found = TRUE;
		
		if (cc && (pmpr_not_found && cumr_not_found))
		{
			/*----------------------------------
			| Parent Contractor is not on file!|
			----------------------------------*/
			errmess (ML (mlPmMess004));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		
		strcpy (exsf_rec.co_no, comm_rec.tco_no);
		sprintf (exsf_rec.salesman_no, "%-2.2s", pmpc_rec.sman_code);
		cc = find_rec ("exsf", &exsf_rec, COMPARISON, "r");

		strcpy (local_rec.sman_code, exsf_rec.salesman_no);
		local_rec.mtrl_cost = pmpc_rec.mtrl_cost;
		local_rec.loyal_rat = pmpc_rec.loyal_rat;
		local_rec.ord_prob = pmpc_rec.ord_prob;

		DSP_FLD ("pmpc_area");
		DSP_FLD ("pmpc_sman");
		DSP_FLD ("proj_sman_name");
		DSP_FLD ("est_value");
		DSP_FLD ("loyalty");
		DSP_FLD ("probability");

		if (!strcmp (pmpc_rec.parent_no, "0     "))
			strcpy (local_rec.parent_contractor, "Main Contractor"); 

		DSP_FLD ("parent_cntr_name");
		heading (1);
		scn_display (1);
		return (EXIT_SUCCESS);
	}
 
	/*--------------------------
	| Validate Salesman Number.|
	--------------------------*/
	if (LCHECK ("pmpc_sman"))
	{
		if (dflt_used)
		{
			if (local_rec.type [0] == 'P')
			{
				strcpy (pmpr_rec.prospect_no, local_rec.cont_no);
				cc = find_rec (pmpr, &pmpr_rec, EQUAL, "r");
				if (!cc)
				{
					strcpy (exsf_rec.co_no, comm_rec.tco_no);
					sprintf (exsf_rec.salesman_no, "%-2.2s", pmpr_rec.sman_code);
					cc = find_rec ("exsf", &exsf_rec, COMPARISON, "r");
					if (cc)
						strcpy (exsf_rec.salesman_no, "");
					else
						strcpy (local_rec.sman_code, exsf_rec.salesman_no);
				}
			}
			else
			{
				strcpy (cumr_rec.co_no, comm_rec.tco_no);
				strcpy (cumr_rec.est_no, branchNumber);
				strcpy (cumr_rec.dbt_no, zero_pad (local_rec.cont_no,6));
				cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
				if (!cc)
				{
					strcpy (exsf_rec.co_no,comm_rec.tco_no);
					strcpy (exsf_rec.salesman_no, cumr_rec.sman_code);
					cc = find_rec ("exsf", &exsf_rec, EQUAL, "r");
					if (cc)
						strcpy (exsf_rec.salesman_no, "");
					else
						strcpy (local_rec.sman_code, exsf_rec.salesman_no);
				}
			}
			DSP_FLD ("pmpc_sman");
			DSP_FLD ("proj_sman_name");
			return (EXIT_SUCCESS);
		}

		if (last_char == SEARCH)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}
		if ( !strcmp (local_rec.sman_code, "  " ) )
		{
			/*--------------------------------
			| Salesman Number Cannot be blank|
			--------------------------------*/
			errmess (ML (mlStdMess135));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
	
		strcpy (exsf_rec.co_no, comm_rec.tco_no);
		strcpy (exsf_rec.salesman_no, local_rec.sman_code);
		cc = find_rec ("exsf", &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			/*--------------------------
			| Salesman is not on file! |
			--------------------------*/
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.sman_code, exsf_rec.salesman_no);
		DSP_FLD ("pmpc_sman");
		DSP_FLD ("proj_sman_name"); 

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*====================+
| Search Project File |
=====================*/
void
SrchPmpm (
 char *	key_val)
{
	work_open ();
	save_rec ("#Code   ","#Description");

	strcpy (pmpm_rec.proj_no, key_val);

	cc = find_rec (pmpm, &pmpm_rec, GTEQ, "r");

	while (!cc && !strncmp (pmpm_rec.proj_no, key_val, strlen (key_val)))
	{
		cc = save_rec (pmpm_rec.proj_no, pmpm_rec.title);
		if (cc)
			break;

		cc = find_rec (pmpm, &pmpm_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	if (!cc)
	{
		/*----------------------+
		| Read selected record  |
		-----------------------*/
		strcpy (pmpm_rec.proj_no, temp_str);
		cc = find_rec (pmpm, &pmpm_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, pmpm, "DBFIND");
	}
}

/*==================
| Updated records. |
==================*/
void
update (
 void)
{
	abc_selfield (pmpc, "pmpc_id_no3");
	strcpy (pmpc_rec.proj_no, local_rec.proj_no);
	strcpy (pmpc_rec.cont_no, local_rec.cont_no);
	strcpy (pmpc_rec.parent_no, local_rec.parent_no);
	cc = find_rec (pmpc, &pmpc_rec, EQUAL, "u");
	if (cc || newCode)
	{
		print_mess (ML (mlStdMess035) ); sleep(2);
		strcpy (pmpc_rec.type, local_rec.type);
		strcpy (pmpc_rec.area, local_rec.area);
		pmpc_rec.mtrl_cost = local_rec.mtrl_cost;
		pmpc_rec.loyal_rat = local_rec.loyal_rat;
		pmpc_rec.ord_prob = local_rec.ord_prob;
		strcpy (pmpc_rec.sman_code, local_rec.sman_code);
		strcpy (pmpc_rec.proj_no, local_rec.proj_no);
		strcpy (pmpc_rec.cont_no, local_rec.cont_no);
		strcpy (pmpc_rec.parent_no, local_rec.parent_no);
		cc = abc_add (pmpc, &pmpc_rec);
		if (cc) 
			file_err (cc, pmpc, "DBADD");
	}
	else
	{
		BOOL exitLoop = FALSE;

		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N   ", upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case SEL_DEFAULT :
			case SEL_UPDATE :
				print_mess ( ML(mlStdMess035) ); sleep(2);
				strcpy (pmpc_rec.type, local_rec.type);
				strcpy (pmpc_rec.area, local_rec.area);
				pmpc_rec.mtrl_cost = local_rec.mtrl_cost;
				pmpc_rec.loyal_rat = local_rec.loyal_rat;
				pmpc_rec.ord_prob = local_rec.ord_prob;
				strcpy (pmpc_rec.sman_code, local_rec.sman_code);
				strcpy (pmpc_rec.proj_no, local_rec.proj_no);
				strcpy (pmpc_rec.cont_no, local_rec.cont_no);
				strcpy (pmpc_rec.parent_no, local_rec.parent_no);
				cc = abc_update (pmpc, &pmpc_rec);
				if (cc) 
					file_err (cc, pmpc, "DBUPDATE");
				abc_unlock (pmpc);
				exitLoop = TRUE;
				clear_mess ();
				break;
	
			case SEL_IGNORE :
				abc_unlock (pmpc);
				exitLoop = TRUE;
				break;

#ifdef SEL_DELETE
			case SEL_DELETE :
			{
				char badFileName [7];

				if (PjcnDelOk (badFileName))
				{
					print_mess (ML (mlStdMess035) ); sleep(2);
					clear_mess ();
					cc = abc_delete (pmpc);
					if (cc)
						file_err (cc, pmpc, "DBUPDATE");
				}
				else
				{
					/*--------------------------------------------
					| Matching Document Records Found in %-4.4s, |
					| Document Record Not Deleted				 |
					--------------------------------------------*/
					sprintf (err_str, ML(mlPmMess007), badFileName);
					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
				}

				exitLoop = TRUE;
				break;
			}
#endif

			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}
	abc_unlock (pmpc);
	abc_selfield (pmpc, "pmpc_id_no");
}

/*===========================
| Check whether it is OK to |
| delete the pmpm record.   |
| Files checked are :       |
|                           |
===========================*/
#ifdef SEL_DELETE
BOOL
FrtyDelOk (
 char *	badFileName)
{
	return (TRUE);
}
#endif

/*===========================
| edit () callback function |
===========================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		/*-----------------------------------
		| %R Project-Contractor Maintenance |
		-----------------------------------*/
		centre_at (0, 80, ML (mlPmMess005));
		move (0, 1); line (80);

		box (0, 2, 80, 12);

		move (0, 1); line (79);
		move (1, 4); line (79);
		move (1, 9); line (79);
		move (1, 22); line(79);
	
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

void
SrchExsf (
 char *	key_val)
{
	work_open ();
	save_rec ("#Salesman", "#Salesman's Name");
	strcpy (exsf_rec.co_no, comm_rec.tco_no);
	sprintf (exsf_rec.salesman_no, "%-2.2s", key_val);
	cc = find_rec ("exsf", &exsf_rec, GTEQ, "r");

	while (!cc && !strcmp (exsf_rec.co_no, comm_rec.tco_no) &&
				  !strncmp (exsf_rec.salesman_no, key_val, strlen(key_val)))
	{
		cc = save_rec (exsf_rec.salesman_no, exsf_rec.salesman);
		if (cc)
			break;

		cc = find_rec ("exsf", &exsf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exsf_rec.co_no, comm_rec.tco_no);
	sprintf (exsf_rec.salesman_no, "%-2.2s", temp_str);
	cc = find_rec ("exsf", &exsf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "exsf", "DBFIND");
}

void
SrchExaf (
 char *	key_val)
{
	work_open ();
	save_rec ("#Cd","#Area Description");
	strcpy (exaf_rec.co_no, comm_rec.tco_no);
	sprintf (exaf_rec.area_code, "%-2.2s", key_val);
	cc = find_rec ("exaf", &exaf_rec, GTEQ, "r");

	while (	!cc && 
			!strcmp (exaf_rec.co_no, comm_rec.tco_no) &&
		    !strncmp (exaf_rec.area_code,key_val,strlen(key_val)))
	{
			cc = save_rec (exaf_rec.area_code, exaf_rec.area_desc);
			if (cc)
				break;
		cc = find_rec ("exaf", &exaf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exaf_rec.co_no, comm_rec.tco_no);
	sprintf (exaf_rec.area_code, "%-2.2s", temp_str);
	cc = find_rec ("exaf", &exaf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "exaf", "DBFIND");
}

/*====================+
| Search Project File |
=====================*/
void
SrchPmpr (
 char *	key_val)
{
	work_open ();
	save_rec ("#Code   ","#Description");

	strcpy (pmpr_rec.prospect_no, key_val);

	cc = find_rec (pmpr, &pmpr_rec, GTEQ, "r");

	while (!cc && !strncmp (pmpr_rec.prospect_no, key_val, strlen(key_val)))
	{
		cc = save_rec (pmpr_rec.prospect_no, pmpr_rec.name);
		if (cc)
			break;

		cc = find_rec (pmpr, &pmpr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

	if (!cc)
	{
		/*----------------------+
		| Read selected record  |
		-----------------------*/
		strcpy (pmpr_rec.prospect_no, temp_str);
		cc = find_rec (pmpr, &pmpr_rec, COMPARISON, "r");
		if (cc)
		{
			
			file_err (cc, pmpr, "DBFIND");
		}
	}
}

/*==============================+
| Search Contractor Master File |
===============================*/
void
SrchPmpc (
 char *	key_val)
{
	work_open ();
	save_rec ("#Code   ", "#Description");
	memset (&pmpc_rec, 0, sizeof (pmpc_rec));
	strcpy (pmpc_rec.proj_no, local_rec.proj_no); 
	strcpy (pmpc_rec.cont_no, key_val);  
	cc = find_rec (pmpc, &pmpc_rec, GTEQ, "r");
	while (!cc && !strcmp (pmpc_rec.proj_no, local_rec.proj_no) &&
			!strncmp (pmpc_rec.cont_no, key_val, strlen (key_val)))
	{
		if (!strcmp (pmpc_rec.cont_no, local_rec.cont_no))
		{
			cc = find_rec (pmpc, &pmpc_rec, NEXT, "r");
			continue;
		}

		strcpy (cumr_rec.co_no,  comm_rec.tco_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no, pmpc_rec.cont_no);
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (!cc)
			cc = save_rec (cumr_rec.dbt_no, cumr_rec.dbt_name);
		
		strcpy (pmpr_rec.prospect_no, pmpc_rec.cont_no); 
		cc = find_rec (pmpr, &pmpr_rec, COMPARISON, "r");
		if (!cc)
			cc = save_rec (pmpr_rec.prospect_no, pmpr_rec.name);
		
		cc = find_rec (pmpc, &pmpc_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	/*----------------------+
	| Read selected record  |
	-----------------------*/
	strcpy (pmpc_rec.proj_no, local_rec.proj_no); 
	strcpy (pmpc_rec.cont_no, temp_str); 
	cc = find_rec (pmpc, &pmpc_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, pmpc, "DBFIND");

}
