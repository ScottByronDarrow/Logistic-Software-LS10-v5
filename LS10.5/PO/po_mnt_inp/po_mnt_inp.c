/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: po_mnt_inp.c,v 5.3 2002/07/25 11:17:31 scott Exp $
|  Program Name  : (po_mnt_inp.c) 
|  Program Desc  : (Maintain P/O files - pocr,pocf,polh,podt,pocl) 
|---------------------------------------------------------------------|
|  Author        : Choo.           | Date Written  : 04/10/90         |
|---------------------------------------------------------------------|
| $Log: po_mnt_inp.c,v $
| Revision 5.3  2002/07/25 11:17:31  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.2  2001/08/09 09:15:46  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:01  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:41  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/03/22 07:14:53  scott
| Dito
|
| Revision 4.1  2001/03/22 07:14:27  scott
| Updated to allow default on sub prime field.
|
| Revision 4.0  2001/03/09 02:32:56  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/02/28 08:24:58  scott
| Updated to clean program and add app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_mnt_inp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_mnt_inp/po_mnt_inp.c,v 5.3 2002/07/25 11:17:31 scott Exp $";

#include <pslscr.h>
#include <GlUtils.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>

#define	BY_CURRENCY	 (inputType [0] == 'C')
#define	BY_FREIGHT	 (inputType [0] == 'F')
#define	BY_LICENCE	 (inputType [0] == 'L')
#define	BY_DUTY		 (inputType [0] == 'D')
#define	BY_CLASS	 (inputType [0] == 'T')
#define	BY_QA   	 (inputType [0] == 'Q')

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
   	int  	newCode = 0;

	char	inputType [2];

	FILE	*pout;

#include	"schema"

struct commRecord	comm_rec;
struct podtRecord	podt_rec;
struct pocfRecord	pocf_rec;
struct polhRecord	polh_rec;
struct poclRecord	pocl_rec;
struct qamrRecord	qamr_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	systemDate [11];
	char 	full_desc [81];
	char 	acct_desc [81];

	char	category [3];
	char	lic_no [11];

} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "curr_code",	 4, 32, CHARTYPE,
		"UUU", "          ",
		" ", "", "Code", " ",
		YES, NO,  JUSTLEFT, "A", "Z", pocrRec.code},
	{1, LIN, "curr_prime",	 5, 32, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Prime Unit", "Enter prime unit of currency. (Singular)",
		YES, NO,  JUSTLEFT, "", "", pocrRec.prime_unit},
	{1, LIN, "sub_prime",	 6, 32, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", " ", "Sub Unit", "Enter sub-unit that makes up the prime unit of currency. (Singular)",
		NO, NO,  JUSTLEFT, "", "", pocrRec.sub_unit},
	{1, LIN, "curr_desc",	 7, 32, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description", " ",
		YES, NO,  JUSTLEFT, "", "", pocrRec.description},
	{1, LIN, "ex_factor",	 8, 32, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "", "Current Exchange Factor.", " ",
		YES, NO, JUSTRIGHT, "", "", (char*)&pocrRec.ex1_factor},
	{1, LIN, "ex_factor",	 9, 32, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "", "Forward Exchange One Month", " ",
		YES, NO, JUSTRIGHT, "", "", (char*)&pocrRec.ex2_factor},
	{1, LIN, "ex_factor",	 10, 32, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "", "Forward Exchange Two Months", " ",
		YES, NO, JUSTRIGHT, "", "", (char*)&pocrRec.ex3_factor},
	{1, LIN, "ex_factor",	 11, 32, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "", "Forward Exchange Three Months", " ",
		YES, NO, JUSTRIGHT, "", "", (char*)&pocrRec.ex4_factor},
	{1, LIN, "ex_factor",	12, 32, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "", "Forward Exchange Four Months", " ",
		YES, NO, JUSTRIGHT, "", "", (char*)&pocrRec.ex5_factor},
	{1, LIN, "ex_factor",	13, 32, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "", "Forward Exchange Five Months", " ",
		YES, NO, JUSTRIGHT, "", "", (char*)&pocrRec.ex6_factor},
	{1, LIN, "ex_factor",	14, 32, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "", "Forward Exchange Six Months", " ",
		YES, NO, JUSTRIGHT, "", "", (char*)&pocrRec.ex7_factor},
	{1, LIN, "ex_date",	15, 32, EDATETYPE,
		"NN/NN/NN", "          ",
		" ", local_rec.systemDate, "Date Last Update", " ",
		YES, NO, JUSTRIGHT, "", "", (char*)&pocrRec.ldate_up},
	{1, LIN, "gl_acct",	16, 32, CHARTYPE,
		GlMask, "          ",
		"0", "", "Control Account", " ",
		YES, NO,  JUSTLEFT, "0123456789*", "", pocrRec.gl_ctrl_acct},
	{1, LIN, "exch_var",	17, 32, CHARTYPE,
		GlMask, "          ",
		"0", "", "Exch. Var. Account", " ",
		YES, NO,  JUSTLEFT, "0123456789*", "", pocrRec.gl_exch_var},
	{1, LIN, "freight_code",	 4, 20, CHARTYPE,
		"UUU", "          ",
		" ", "", "Code.      ", " ",
		YES, NO,  JUSTLEFT, "", "", pocf_rec.code},
	{1, LIN, "freight_desc",	 5, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Description", " ",
		YES, NO,  JUSTLEFT, "", "", pocf_rec.description},
	{1, LIN, "load_type",	 6, 20, CHARTYPE,
		"U", "          ",
		" ", " ", "Loading Type.", "P (ercent of FOB),U (nit charge) or Blank for no default",
		YES, NO,  JUSTLEFT, "PU ", "", pocf_rec.load_type},
	{1, LIN, "freight_load",	 7, 20, DOUBLETYPE,
		"NNN.NN", "          ",
		" ", " ", "Freight Load.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&pocf_rec.freight_load},
	{1, LIN, "lead_time",	 8, 20, INTTYPE,
		"NNNN", "          ",
		" ", " ", "Lead Time (days).", " ",
		YES, NO, JUSTRIGHT, "0.00", "9999", (char *)&pocf_rec.lead_time},
	{1, LIN, "lic_cate",	 4, 25, CHARTYPE,
		"UU", "          ",
		" ", "NA", "Licence Category.", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.category},
	{1, LIN, "lic_no",	 5, 25, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", "Licence Number.", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.lic_no},
	{1, LIN, "lic_area",	 7, 25, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "Licence Area.", " ",
		YES, NO,  JUSTLEFT, "", "", polh_rec.lic_area},
	{1, LIN, "lic_val",	 8, 25, DOUBLETYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0", "Licence Value.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&polh_rec.lic_val},
	{1, LIN, "tender_rate",	 9, 25, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0", "Tender Rate.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&polh_rec.tender_rate},
	{1, LIN, "ap_lic_rate",	10, 25, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0", "Applied Licence Rate.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&polh_rec.ap_lic_rate},
	{1, LIN, "type",	11, 25, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Type.", " ",
		YES, NO,  JUSTLEFT, "", "", polh_rec.type},
	{1, LIN, "from_date",	12, 25, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "From Date.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&polh_rec.from_date},
	{1, LIN, "to_date",	13, 25, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "To Date.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&polh_rec.to_date},
	{1, LIN, "tot_value",	14, 25, DOUBLETYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0", "Total Allocated.", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&polh_rec.tot_alloc},
	{1, LIN, "comm1",	16, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Comment 1 ", " ",
		 NO, NO,  JUSTLEFT, "", "", polh_rec.comment_1},
	{1, LIN, "comm2",	17, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "        2 ", " ",
		 NO, NO,  JUSTLEFT, "", "", polh_rec.comment_2},
	{1, LIN, "comm3",	18, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "        3 ", " ",
		 NO, NO,  JUSTLEFT, "", "", polh_rec.comment_3},
	{1, LIN, "duty_code",	 4, 18, CHARTYPE,
		"UU", "          ",
		" ", "", "Duty Code.", " ",
		YES, NO,  JUSTLEFT, "", "", podt_rec.code},
	{1, LIN, "description",	 5, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Description.", " ",
		YES, NO,  JUSTLEFT, "", "", podt_rec.description},
	{1, LIN, "im_tariff",	 7, 18, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "Import Tariff.", " ",
		YES, NO,  JUSTLEFT, "", "", podt_rec.im_tariff},
	{1, LIN, "im_band",	 8, 18, CHARTYPE,
		"U", "          ",
		" ", " ", "Import Band.", " ",
		YES, NO,  JUSTLEFT, "", "", podt_rec.im_band},
	{1, LIN, "ex_tariff",	10, 18, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "Export Tariff.", " ",
		YES, NO,  JUSTLEFT, "", "", podt_rec.ex_tariff},
	{1, LIN, "ex_band",	11, 18, CHARTYPE,
		"U", "          ",
		" ", " ", "Export Band.", " ",
		YES, NO,  JUSTLEFT, "", "", podt_rec.ex_band},
	{1, LIN, "duty_type",	13, 18, CHARTYPE,
		"U", "          ",
		" ", "", "Duty Type.", "P (ercent) D (ollar)",
		YES, NO,  JUSTLEFT, "PD", "", podt_rec.duty_type},
	{1, LIN, "im_duty",	14, 18, DOUBLETYPE,
		"NNNN.NN", "          ",
		" ", " ", "Import Duty.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&podt_rec.im_duty},
	{1, LIN, "ex_draw",	15, 18, DOUBLETYPE,
		"NNNN.NN", "          ",
		" ", " ", "Export Drawback.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&podt_rec.ex_draw},
	{1, LIN, "cl_type",	 4, 22, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Classification Type", " ",
		NE, NO,  JUSTLEFT, "", "", pocl_rec.type},
	{1, LIN, "cl_desc",	 5, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Classification Desc", " ",
		YES, NO,  JUSTLEFT, "", "", pocl_rec.desc},
	{1, LIN, "qa_status",	 4, 15, CHARTYPE,
		"N", "          ",
		" ", "", " QA Status Code.", " ",
		NE, NO,  JUSTLEFT, "1", "9", qamr_rec.qa_status},
	{1, LIN, "qa_desc",	 5, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " QA Description ", " ",
		 YES, NO,  JUSTLEFT, "", "", qamr_rec.qa_desc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


/*========================
| Function Declarations. |
========================*/
int 	CheckClass 			 (void);
int 	heading 			 (int);
int 	spec_valid 			 (int);
void 	CloseDB 			 (void);
void 	OpenDB 				 (void);
void 	SrchPocf 			 (char *);
void 	SrchPocl 			 (char *);
void 	SrchPocr 			 (char *);
void 	SrchPodt 			 (char *);
void 	SrchPolh 			 (char *);
void 	SrchQamr 			 (char *);
void 	Update 				 (void);
void 	UpdatePocf 			 (void);
void 	UpdatePocl 			 (void);
void 	UpdatePocr 			 (void);
void 	UpdatePodt 			 (void);
void 	UpdatePolh 			 (void);
void 	UpdateQamr 			 (void);
void 	shutdown_prog 		 (void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	int		i;

	if (argc != 2)
	{
		print_at (0,0,mlPoMess730,argv [0]);
        return (EXIT_FAILURE);
	}
	sprintf (GlMask,   "%-*.*s", MAXLEVEL, MAXLEVEL, "AAAAAAAAAAAAAAAA");
	sprintf (GlFormat, "%-*.*s", MAXLEVEL, MAXLEVEL, "XXXXXXXXXXXXXXXX");

	/*-------------------
	| Printer Number	|
	-------------------*/
	sprintf (inputType,"%-1.1s", argv [1]);

	if (!BY_CURRENCY && !BY_FREIGHT && 
		 !BY_LICENCE && !BY_DUTY && 
		 !BY_CLASS && !BY_QA)
	{
		print_at (0,0,mlPoMess730,argv [0]);
        return (EXIT_FAILURE);
	}
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);

	for (i = label ("curr_code"); i <= label ("qa_desc"); i++)
		vars [i].scn = 2;

	if (BY_CURRENCY)
	{
		for (i = label ("curr_code"); i <= label ("exch_var"); i++)
			vars [i].scn = 1;
	}
	if (BY_FREIGHT)
	{
		for (i = label ("freight_code"); i <= label ("lead_time"); i++)
			vars [i].scn = 1;
	}

	if (BY_LICENCE)
	{
		for (i = label ("lic_cate"); i <= label ("comm3"); i++)
			vars [i].scn = 1;
	}

	if (BY_DUTY)
	{
		for (i = label ("duty_code"); i <= label ("ex_draw"); i++)
			vars [i].scn = 1;
	}

	if (BY_CLASS)
	{
		for (i = label ("cl_type"); i <= label ("cl_desc"); i++)
			vars [i].scn = 1;
	}
	if (BY_QA)
	{
		for (i = label ("qa_status"); i <= label ("qa_desc"); i++)
			vars [i].scn = 1;
	}

	init_scr 	 ();
	set_tty 	 ();
	set_masks 	 ();
	init_vars 	 (1);

	OpenDB ();

	if (BY_CURRENCY)
		GL_SetMask (GlFormat);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	/*===================================
	| Beginning of input control loop . |
	===================================*/
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
		edit (1);

		if (!restart)
			Update ();

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
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	if (BY_CLASS)
		open_rec (pocl,pocl_list,POCL_NO_FIELDS,"pocl_id_no");

	if (BY_DUTY)
		open_rec (podt,podt_list,PODT_NO_FIELDS,"podt_id_no");

	if (BY_FREIGHT)
		open_rec (pocf,pocf_list,POCF_NO_FIELDS,"pocf_id_no");

	if (BY_CURRENCY)
	{
		OpenPocr ();
		OpenGlmr ();
	}
	if (BY_LICENCE)
		open_rec (polh,polh_list,POLH_NO_FIELDS,"polh_id_no");

	if (BY_QA)
		open_rec (qamr,qamr_list,QAMR_NO_FIELDS,"qamr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	if (BY_CLASS)
		abc_fclose (pocl);

	if (BY_DUTY)
		abc_fclose (podt);

	if (BY_FREIGHT)
		abc_fclose (pocf);

	if (BY_CURRENCY)
	{
		GL_Close ();
	}

	if (BY_LICENCE)
		abc_fclose (polh);

	if (BY_QA)
		abc_fclose (polh);

	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	int		i;
        
	if (LCHECK ("cl_type"))
	{
		strcpy (pocl_rec.co_no, comm_rec.co_no);
		
		if (SRCH_KEY)
		{
		   SrchPocl (temp_str);
		   return (EXIT_SUCCESS);
		}
		
		cc = find_rec (pocl,&pocl_rec,COMPARISON,"u");
		if (cc)
			newCode = TRUE;
		else
		{
			newCode = FALSE;
			entry_exit = TRUE;
			DSP_FLD ("cl_type");
			DSP_FLD ("cl_desc");
		}
		return (EXIT_SUCCESS);
	}
	/*---------------
	| Validate code |
	---------------*/
	if (LCHECK ("curr_code"))
	{
		if (SRCH_KEY)
		{
		   SrchPocr (temp_str);
		   return (EXIT_SUCCESS);
		}
		strcpy (pocrRec.co_no,comm_rec.co_no);
		cc = find_rec (pocr,&pocrRec,COMPARISON,"u");
		if (cc)
			newCode = TRUE;
		else
		{
			newCode = FALSE;
			entry_exit = TRUE;
			for (i = 0; i < 10; i++)
				display_field (field + i);
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Control Account. |
	---------------------------*/
	if (LCHECK ("gl_acct"))
	{
		if (SRCH_KEY)
			return SearchGlmr (comm_rec.co_no, temp_str, "F*P");

		if (!dflt_used)
		{
			strcpy (glmrRec.co_no,comm_rec.co_no);
			GL_FormAccNo (pocrRec.gl_ctrl_acct, glmrRec.acc_no, 0);
			cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
			if (cc) 
			{
			    /*G/L Account is not on file.*/
			    errmess (ML (mlStdMess186));
			    sleep (sleepTime);		
			    return (EXIT_FAILURE);
			}
				
			if (CheckClass ())
				return (EXIT_FAILURE);

			strcpy (local_rec.acct_desc,glmrRec.desc);
		}

		move (1,2); cl_line ();
		/*Control Account :%s",local_rec.acct_desc*/
		print_at (2,1,ML (mlPoMess147),local_rec.acct_desc);
		return (EXIT_SUCCESS);
	}

	/*-------------------------------------
	| Validate Exchange Variation Account |
	--------------------------------------*/
	if (LCHECK ("exch_var"))
	{
		if (SRCH_KEY)
			return SearchGlmr (comm_rec.co_no, temp_str, "F*P");

		if (!dflt_used)
		{
			strcpy (glmrRec.co_no,comm_rec.co_no);
			GL_FormAccNo (pocrRec.gl_exch_var, glmrRec.acc_no, 0);
			cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
			if (cc) 
			{
			    errmess (ML (mlStdMess186));
			    sleep (sleepTime);		
			    return (EXIT_FAILURE);
			}
				
			if (CheckClass ())
				return (EXIT_FAILURE);

			strcpy (local_rec.acct_desc,glmrRec.desc);
		}

		move (1,2); cl_line ();
		print_at (2,1,ML (mlPoMess148),local_rec.acct_desc);
		return (EXIT_SUCCESS);
	}

	/*---------------
	| Validate code |
	---------------*/
	if (LCHECK ("freight_code"))
	{
		if (SRCH_KEY)
		{
		   SrchPocf (temp_str);
		   return (EXIT_SUCCESS);
		}
		strcpy (pocf_rec.co_no,comm_rec.co_no);
		cc = find_rec (pocf,&pocf_rec,COMPARISON,"u");
		if (cc)
			newCode = TRUE;
		else
		{
			newCode = FALSE;
			entry_exit = TRUE;
		}
		return (EXIT_SUCCESS);
	}
        
	/*----------------------------------------------
	| Reset freight loading dependant on load type |
	----------------------------------------------*/
	if (LCHECK ("load_type"))
	{
		if (strcmp (pocf_rec.load_type," "))
			vars [field + 1].required = YES;
		else
		{
			vars [field + 1].required = NA;
			pocf_rec.freight_load = 0L;
		}
		return (EXIT_SUCCESS);             
	}

	/*----------------------------------------
	| Check if license number already exists |
	----------------------------------------*/
	if (LCHECK ("lic_no"))
	{
		if (SRCH_KEY)
		{
		   SrchPolh (temp_str);
		   return (EXIT_SUCCESS);
		}
		strcpy (polh_rec.co_no,comm_rec.co_no);
		strcpy (polh_rec.est_no,comm_rec.est_no);
		strcpy (polh_rec.lic_cate,local_rec.category);
		strcpy (polh_rec.lic_no,local_rec.lic_no);
		cc = find_rec (polh,&polh_rec,COMPARISON,"u");
		if (cc)
			newCode = TRUE;
		else
		{
			newCode = FALSE;
			entry_exit = TRUE;
		}
		return (EXIT_SUCCESS);
	}

	/*-------------------
	| Validate periods  |
	-------------------*/
	if (LCHECK ("to_date"))
	{
		if (prog_status != ENTRY && polh_rec.from_date > polh_rec.to_date)
		{
			errmess (ML (mlStdMess019));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);             
	}

	if (LCHECK ("from_date"))
	{
		if (prog_status != ENTRY)
		{
			if (polh_rec.from_date > polh_rec.to_date)
			{
				/*From Date  > To Date.*/
				errmess (ML (mlStdMess019));
				return (EXIT_FAILURE);
			}
		}
		return (EXIT_SUCCESS);             
	}

	if (LCHECK ("duty_code"))
	{
		if (SRCH_KEY)
		{
		   SrchPodt (temp_str);
		   return (EXIT_SUCCESS);
		}
		strcpy (podt_rec.co_no,comm_rec.co_no);
		cc = find_rec (podt,&podt_rec,COMPARISON,"u");
		if (cc)
			newCode = TRUE;
		else
		{
			newCode = FALSE;
			entry_exit = TRUE;
			for (i = 0; i < 9; i++)
			    display_field (30 + i);
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Instruction Code. |
	----------------------------*/
	if (LCHECK ("qa_status"))
	{
		if (SRCH_KEY)
		{
			SrchQamr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (qamr_rec.co_no, comm_rec.co_no);
		strcpy (qamr_rec.br_no, comm_rec.est_no);
		cc = find_rec (qamr, &qamr_rec, COMPARISON, "w");
		if (cc) 
			newCode = 1;
		else    
		{
			newCode = 0;
			entry_exit = 1;
			DSP_FLD ("qa_desc");
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);             
}

void
Update (
 void)
{
	if (BY_CURRENCY)
		UpdatePocr ();

	if (BY_FREIGHT)
		UpdatePocf ();

	if (BY_LICENCE)
		UpdatePolh ();

	if (BY_DUTY)
		UpdatePodt ();

	if (BY_CLASS)
		UpdatePocl ();

	if (BY_QA)
		UpdateQamr ();
}

/*=======================
| Update Currency file. |
=======================*/
void
UpdatePocr (
 void)
{
	strcpy (pocrRec.stat_flag, "0");
	strcpy (pocrRec.pocr_operator, "/");
	strcpy (pocrRec.co_no,comm_rec.co_no);
	if (newCode)
	{
		cc = abc_add (pocr,&pocrRec);
		if (cc)
			file_err (cc, pocr, "DBADD");
	}
	else
	{
		cc = abc_update (pocr,&pocrRec);
		if (cc)
			file_err (cc, pocr, "DBUPDATE");
	}
	abc_unlock (pocr);
}

/*==============================
| Update Country freight file. |
==============================*/
void
UpdatePocf (
 void)
{
	if (newCode)
	{
		pocf_rec.last_update = StringToDate (local_rec.systemDate);
		cc = abc_add (pocf,&pocf_rec);
		if (cc)
			file_err (cc, pocf, "DBADD");
	}
	else
	{
		cc = abc_update (pocf,&pocf_rec);
		if (cc)
			file_err (cc, pocf, "DBUPDATE");
	}
	abc_unlock (pocf);
}

/*======================
| Update Licence file. |
======================*/
void
UpdatePolh (
 void)
{
	/*----------------------
	| Update the polh file |
	----------------------*/
	if (newCode)
	{
		strcpy (polh_rec.co_no,comm_rec.co_no);
		strcpy (polh_rec.est_no,comm_rec.est_no);
		strcpy (polh_rec.lic_cate,local_rec.category);
		strcpy (polh_rec.lic_no,local_rec.lic_no);
		cc = abc_add (polh,&polh_rec);
		if (cc)
			file_err (cc, polh, "DBADD");
	}
	else
	{
		cc = abc_update (polh,&polh_rec);
		if (cc)
			file_err (cc, polh, "DBUPDATE");
	}
	abc_unlock (polh);
}

/*===================
| Update Duty file. |
===================*/
void
UpdatePodt (
 void)
{
	if (newCode)
	{
		cc = abc_add (podt,&podt_rec);
		if (cc)
			file_err (cc, podt, "DBADD");
	}
	else
	{
		cc = abc_update (podt,&podt_rec);
		if (cc)
			file_err (cc, podt, "DBUPDATE");
	}
	abc_unlock (podt);
}


/*====================
| Search for rr_code |
====================*/
void
SrchPocr (
 char *key_val)
{
	_work_open (3,0,40);
	save_rec ("#No.","#Currency code description");                       
	strcpy (pocrRec.co_no,comm_rec.co_no);
	sprintf (pocrRec.code ,"%-3.3s",key_val);
	cc = find_rec (pocr, &pocrRec, GTEQ, "r");
	while (!cc && !strcmp (pocrRec.co_no,comm_rec.co_no) && 
		      	  !strncmp (pocrRec.code,key_val,strlen (key_val)))
	{                        
		cc = save_rec (pocrRec.code, pocrRec.description);                       
		if (cc)
			break;
		cc = find_rec (pocr,&pocrRec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pocrRec.co_no,comm_rec.co_no);
	sprintf (pocrRec.code,"%-3.3s",temp_str);
	cc = find_rec (pocr, &pocrRec, GTEQ, "r");
	if (cc)
		file_err (cc, pocr, "DBFIND");
}

/*====================================
| Search for Quality Assurance code. |
====================================*/
void
SrchQamr (
 char *key_val)
{
	_work_open (2,0,40);
	save_rec ("#QA", "#QA Description");
	strcpy (qamr_rec.co_no, comm_rec.co_no);
	strcpy (qamr_rec.br_no, comm_rec.est_no);
	sprintf (qamr_rec.qa_status, "%-1.1s", key_val);

	cc = find_rec (qamr, &qamr_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (qamr_rec.co_no, comm_rec.co_no) &&
		   !strcmp (qamr_rec.br_no, comm_rec.est_no) &&
		   !strncmp (qamr_rec.qa_status, key_val, strlen (key_val)))
	{
		cc = save_rec (qamr_rec.qa_status, qamr_rec.qa_desc);
		if (cc)
			break;

		cc = find_rec (qamr, &qamr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (qamr_rec.co_no, comm_rec.co_no);
	strcpy (qamr_rec.br_no, comm_rec.est_no);
	sprintf (qamr_rec.qa_status, "%-1.1s", temp_str);
	cc = find_rec (qamr, &qamr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, qamr, "DBFIND");
}

int
CheckClass (
 void)
{
	if (glmrRec.glmr_class [2][0] != 'P')
		/*Account is not a posting account.", */
		return print_err (ML (mlStdMess025));

	return (EXIT_SUCCESS);
}

/*=================
| Search for code |
=================*/
void
SrchPocf (
 char *key_val)
{
	_work_open (3,0,40);
	save_rec ("#No.","#Country / Freight");                       
	strcpy (pocf_rec.co_no,comm_rec.co_no);
	sprintf (pocf_rec.code,"%-3.3s",key_val);
	cc = find_rec (pocf,&pocf_rec,GTEQ,"r");
	while (!cc && !strcmp (pocf_rec.co_no,comm_rec.co_no) && 
		      	  !strncmp (pocf_rec.code,key_val,strlen (key_val)))
	{                        
		cc = save_rec (pocf_rec.code,pocf_rec.description);                       
		if (cc)
		        break;
		cc = find_rec (pocf,&pocf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;
	strcpy (pocf_rec.co_no,comm_rec.co_no);
	sprintf (pocf_rec.code,"%-3.3s",temp_str);
	cc = find_rec (pocf,&pocf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, pocf, "DBFIND");
}

/*============================
| Search for licence number. |
============================*/
void
SrchPolh (
 char *key_val)
{
	_work_open (10,0,40);
	save_rec ("#Licence No","#Licence Area");                       
	strcpy (polh_rec.co_no,comm_rec.co_no);
	strcpy (polh_rec.est_no,comm_rec.est_no);
	strcpy (polh_rec.lic_cate,local_rec.category);
	sprintf (polh_rec.lic_no,"%-10.10s",key_val);
	cc = find_rec (polh,&polh_rec,GTEQ,"r");
        while (!cc && !strcmp (polh_rec.co_no,comm_rec.co_no) && 
              !strcmp (polh_rec.est_no,comm_rec.est_no) && 
              !strcmp (polh_rec.lic_cate,local_rec.category) && 
              !strncmp (polh_rec.lic_no,key_val,strlen (key_val)))
    	{                        
	        cc = save_rec (polh_rec.lic_no,polh_rec.lic_area);                       
		if (cc)
		        break;
		cc = find_rec (polh,&polh_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;
	strcpy (polh_rec.co_no,comm_rec.co_no);
	strcpy (polh_rec.est_no,comm_rec.est_no);
	strcpy (polh_rec.lic_cate,local_rec.category);
	sprintf (polh_rec.lic_no,"%-10.10s",temp_str);
	cc = find_rec (polh,&polh_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, polh, "DBFIND");
}

/*=================
| Search for code |
=================*/
void
SrchPodt (
 char *key_val)
{
	_work_open (2,0,40);
	save_rec ("#No","#Duty Description");                       
	strcpy (podt_rec.co_no,comm_rec.co_no);
	sprintf (podt_rec.code,"%-2.2s",key_val);
	cc = find_rec (podt,&podt_rec,GTEQ,"r");
        while (!cc && !strcmp (podt_rec.co_no,comm_rec.co_no) && 
		      !strncmp (podt_rec.code,key_val,strlen (key_val)))
    	{                        
	        cc = save_rec (podt_rec.code,podt_rec.description);                       
		if (cc)
		        break;
		cc = find_rec (podt,&podt_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;
	strcpy (podt_rec.co_no,comm_rec.co_no);
	sprintf (podt_rec.code,"%-2.2s",temp_str);
	cc = find_rec (podt,&podt_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, podt, "DBFIND");
}

void
UpdatePocl (
 void)
{
	if (newCode)
		cc = abc_add (pocl, &pocl_rec);
	else
		cc = abc_update (pocl, &pocl_rec);
	if (cc)
		file_err (cc, pocl, (newCode) ? "DBADD" : "DBUPDATE");
}

void
UpdateQamr (
 void)
{
	if (newCode)
		cc = abc_add (qamr, &qamr_rec);
	else
		cc = abc_update (qamr, &qamr_rec);
	if (cc)
		file_err (cc, qamr, (newCode) ? "DBADD" : "DBUPDATE");
}

void
SrchPocl (
 char *key_val)
{
	_work_open (6,0,40);
	save_rec ("#Type","#Description ");                       
	sprintf (pocl_rec.type ,"%-6.6s",key_val);
	cc = find_rec (pocl, &pocl_rec, GTEQ, "r");
	while (!cc && !strcmp (pocl_rec.co_no,comm_rec.co_no) && 
		      	  !strncmp (pocl_rec.type,key_val,strlen (key_val)))
	{                        
		cc = save_rec (pocl_rec.type, pocl_rec.desc);                       
		if (cc)
		        break;
		cc = find_rec (pocl,&pocl_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;
	strcpy (pocl_rec.co_no,comm_rec.co_no);
	sprintf (pocl_rec.type,"%-6.6s",temp_str);
	cc = find_rec (pocl, &pocl_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, pocl, "DBFIND");
}

/*====================================================
| Display Screen Heading.                            |
====================================================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		move (0,1);
		line (80);

		if (BY_CLASS)
		{
			/* Classification Type Maintenance.*/
			rv_pr (ML (mlPoMess141),24,0,1);
			box (0,3,80,2);
		}

		if (BY_CURRENCY)
		{
			/* Currency Exchange Maintenance.*/
			rv_pr (ML (mlPoMess101),26,0,1);
			box (0, 3, 80, 14);
		}

		if (BY_FREIGHT)
		{
			/* Country/Freight File Maintenance. */
			rv_pr (ML (mlPoMess142),25,0,1);
			box (0,3,80,5);
		}

		if (BY_LICENCE)
		{
			/*Purchase Order Licence Control Maintenance*/
			rv_pr (ML (mlPoMess143),25,0,1);
			box (0,3,80,15);
			move (1,6);
			line (79);
			move (1,15);
			line (79);
			/* C o m m e n t s.*/
			us_pr (ML (mlPoMess146),5,15,1);
		}

		if (BY_DUTY)
		{
			/* Duty Control Maintenance.*/
			rv_pr (ML (mlPoMess145),26,0,1);
			box (0,3,80,12);
			move (1,6);
			line (79);
			move (1,9);
			line (79);
			move (1,12);
			line (79);
		}
		if (BY_QA)
		{
			/* Quality Assurance Maintenance.*/
			rv_pr (ML (mlPoMess144),24,0,1);
			box (0,3,80,2);
		}

		move (0,20);
		line (80);
		
		strcpy (err_str,ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.co_no,comm_rec.co_name);
		if (BY_QA)
		{
			strcpy (err_str,ML (mlStdMess039));
			print_at (22,0,err_str,comm_rec.est_no,comm_rec.est_name);
		}
		else
		{
			move (0,22);
			line (80);
		}
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
