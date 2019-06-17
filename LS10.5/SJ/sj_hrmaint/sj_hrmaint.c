/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sj_hrmaint.c,v 5.3 2001/08/09 09:17:26 scott Exp $
|  Program Name  : (sj_hrmaint.c)
|  Program Desc  : (Create Service Order)
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 05/08/87         |
|---------------------------------------------------------------------|
| $Log: sj_hrmaint.c,v $
| Revision 5.3  2001/08/09 09:17:26  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/09 01:45:38  scott
| RELEASE 5.0
|
| Revision 5.1  2001/08/06 23:41:26  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_hrmaint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_hrmaint/sj_hrmaint.c,v 5.3 2001/08/09 09:17:26 scott Exp $";

#define	TXT_REQD

/*===============================
|   Include file dependencies   |
===============================*/
#include <pslscr.h>
#include <GlUtils.h>
#include <getnum.h>
#include <ml_std_mess.h>
#include <ml_sj_mess.h>

/*===================================
|   Constants, defines and stuff    |
===================================*/
extern	int	X_EALL;
extern	int	Y_EALL;

	char	*cumr2 	= "cumr2",
            *data 	= "data";

    /*=====================
    |   Local variables   |
    =====================*/
	char	branchNo [3];

	int		envDbCo = 0,
			new_item = 0,
			envDbFind  = 0;

	int		held_ser_job = FALSE;

#include	"schema"

struct commRecord	comm_rec;
struct sjhrRecord	sjhr_rec;
struct sjjdRecord	sjjd_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct esmrRecord	esmr_rec;

	Money	*cumr_balance	=	&cumr_rec.bo_current;

	int		db_stopcrd = 1,
			db_crdterm = 1,
			db_crdover = 1,
			held_job = 0;

	double	c_left 		= 0.00,
			total_owing = 0.00;

	char	*scn_desc [] = {	"JOB HEADER.", "JOB DETAILS." };

struct 
{
	long	t_chg_client;
	long	t_end_client;
	char	t_cust_ord_no [11];
	double	t_cost_estim;
	char	t_estim_type [2];
	char	t_contact [21];
	char	t_invoice_no [9];
	long	t_issue_date;
	long	t_reqd_date;
	double	t_inv_amt;
	char	t_gl_exp_code [MAXLEVEL + 1];
	char	t_status [2];
	double	t_fixed_labour;
	long	t_comp_date;
} temp_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct 
{
	char	dummy [11];
	char	systemDate [11];
	char	c_client [7];
  	char	e_client [7];
	char	chg_client [41];
	char	end_client [41];
	char	estim_type [11];
	char	order_no [7];
	char    acc_desc [26];
	char    job_detail [71];
	long	c_hash;
	long	e_hash;
} local_rec;

static struct var vars [] =
{
	{1, LIN, "service_no",	 4, 18, LONGTYPE,
		"NNNNNN", "          ",
		" ", "0", "Service Job No ", "Enter a valid order no or RETURN for a new one ",
		 NE, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.order_no},
	{1, LIN, "charge_to",	 5, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Charge To ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.c_client},
	{1, LIN, "charge_to_name",	 5, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.chg_client},
	{1, LIN, "end_client",	 6, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "End Client ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.e_client},
	{1, LIN, "end_client_name",	 6, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.end_client},
	{1, LIN, "cust_order_no",	 8, 18, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "Cust Order No ", " ",
		 NO, NO,  JUSTLEFT, "", "", sjhr_rec.cust_ord_no},
	{1, LIN, "reqd_date",	 9, 18, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Date Required ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.reqd_date},
	{1, LIN, "cost_est",	10, 18, DOUBLETYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0", "Cost Estimate ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.cost_estim},
	{1, LIN, "cost_fixed",	11, 18, CHARTYPE,
		"U", "          ",
		" ", "V", "Est. Type ", " F(ixed) or V(ariable). ",
		 NO, NO,  JUSTLEFT, "FV", "", local_rec.estim_type},
	{1, LIN, "contact",	12, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Contact ", " ",
		 NO, NO,  JUSTLEFT, "", "", sjhr_rec.contact},
	{1, LIN, "fixed_labr",	13, 18, DOUBLETYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0", "Fixed Labour Rate", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.fixed_labour},
	{1, LIN, "comp_date",	14, 18, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "Completion Date", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.comp_date},
	{1, LIN, "glacct",	16, 18, CHARTYPE,
		GlMask, "          ",
		"0", "0", GlDesc, " ",
		YES, NO,  JUSTLEFT, "", "", sjhr_rec.gl_expense},
	{1, LIN, "gl_desc",	17, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", "Description ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.acc_desc},
	{2, TXT, "desc",	8, 5, 0,
		"", "          ",
		" ", " ", "                        J o b    D e s c r i p t i o n.               ", " ",
		 7, 70,  7, "", "", local_rec.job_detail},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*===============================
|   Local function prototypes   |
===============================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
int  	spec_valid 			(int);
void 	LoadDetails 		(void);
void 	GetChargeClient 	(void);
int  	CheckCumr 			(double);
int  	WarnUser 			(char *, int);
void 	Busy 				(int);
int  	Update 				(void);
int  	GetOrderNumber 		(void);
void 	SaveTemp 			(void);
void 	CopyBack 			(void);
void 	SrchSjhr 			(char *);
int		CheckGlClass 		(void);
int  	heading 			(int);

#include <FindCumr.h>

/*============================
| Main Processing Routine.   |
============================*/
int
main (
 int argc,
 char *argv [])
{
	int	i;
	char	*sptr;

	sprintf (GlMask,   "%-*.*s", MAXLEVEL, MAXLEVEL, "AAAAAAAAAAAAAAAA");
	sprintf (GlDesc,   "%-*.*s", MAXLEVEL, MAXLEVEL, " Account Number ");

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);

	init_scr ();
	set_tty (); 
	set_masks ();
	GL_SetMask ("XXXXXXXXXXXXXXXX");

	X_EALL = 33;
	Y_EALL = 8;

	for (i = 0; i < 2; i++)
    {
		tab_data [ i ]._desc = scn_desc [ i ];
    }
	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind = atoi (get_env ("DB_FIND"));

	OpenDB ();

	/*-----------------------------
	| Check and Get Credit terms. |
	-----------------------------*/
	sptr = get_env ("SO_CRD_TERMS");
	db_stopcrd = (* (sptr + 0) == 'S');
	db_crdterm = (* (sptr + 1) == 'S');
	db_crdover = (* (sptr + 2) == 'S');

	strcpy (branchNo, (!envDbCo) ? " 0" : comm_rec.est_no);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		new_item = FALSE;
		lcount [ 2 ] = 0;
		init_vars (1);
		init_vars (2);

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		scn_display (1);
		entry (1);
		if (prog_exit || restart)
        {
            continue;
        }

		scn_write (1);
		scn_display (1);
		scn_display (2);

		/*-------------------------------
		| Enter screen 2 tabular input .|	
		-------------------------------*/
		if (new_item)
        {
			entry (2);
        }
		else
        {
            edit (2);
        }

		if (prog_exit || restart)
        {
            continue;
        }

		edit_all ();

		if (restart)
        {			
            continue;
        }
		Update ();
	}	/* end of input control loop	*/

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

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

    abc_alias (cumr2, cumr);

	open_rec (sjhr,  sjhr_list, SJHR_NO_FIELDS, "sjhr_id_no");
	open_rec (sjjd,  sjjd_list, SJJD_NO_FIELDS, "sjjd_id_no");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, (!envDbFind) ? "cumr_id_no" : "cumr_id_no3");

	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (glmr,  glmr_list, GLMR_NO_FIELDS, "glmr_id_no");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (sjhr);
	abc_fclose (cumr);
	abc_fclose (sjjd);
	abc_fclose (esmr);
	abc_fclose (glmr);
	abc_fclose (cumr2);
	GL_Close ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("service_no"))
	{
		held_ser_job = FALSE;
		if (SRCH_KEY)
		{
			SrchSjhr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (sjhr_rec.order_no == 0L)
		{
			new_item = TRUE;
			sjhr_rec.fixed_labour = 0.00;
			return (EXIT_SUCCESS);
		}

		strcpy (sjhr_rec.co_no,comm_rec.co_no);
		strcpy (sjhr_rec.est_no,comm_rec.est_no);
		strcpy (sjhr_rec.dp_no,comm_rec.dp_no);
		cc = find_rec (sjhr,&sjhr_rec,COMPARISON,"u");
		if (cc)
		{
			errmess (ML (mlSjMess004));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		new_item = FALSE;
		strcpy (local_rec.estim_type, 
               (sjhr_rec.estim_type [0] == 'V')  ? "V(ariable)" : "F(ixed)   ");
		
		if (find_hash (cumr2,&cumr_rec,EQUAL,"r",sjhr_rec.chg_client))
        {
		    if (cc)
		    {
			    errmess (ML (mlStdMess021));
    			sleep (sleepTime);
	    		return (EXIT_FAILURE);
		    }
        }
		held_ser_job = FALSE;

		if (CheckCumr (0.00))
        {
            held_ser_job = TRUE;
        }

		local_rec.c_hash = sjhr_rec.chg_client;
		local_rec.e_hash = sjhr_rec.end_client;
		GetChargeClient ();

		if (sjhr_rec.end_client != 0L)
		{
			if (find_hash (cumr2,&cumr_rec,EQUAL,"r",sjhr_rec.end_client))
			{
				errmess (ML (mlStdMess021));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			strcpy (local_rec.end_client,cumr_rec.dbt_name);
		}
		entry_exit = TRUE;

		strcpy (glmrRec.co_no,comm_rec.co_no);
		sprintf (glmrRec.acc_no,
                 "%-*.*s", 
                 MAXLEVEL, 
                 MAXLEVEL,
                 sjhr_rec.gl_expense);

        cc = find_rec (glmr,&glmrRec,COMPARISON,"r");
		if (cc)
        {
			sprintf (local_rec.acc_desc,"%-25.25s"," ");
        }
		else
        {
			strcpy (local_rec.acc_desc,glmrRec.desc);
        }

		LoadDetails ();
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("charge_to"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNo);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.c_client));

        cc = find_rec (cumr ,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			errmess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}
		held_ser_job = FALSE;
		
		/*--------------------------------------
		| Check if customer is on stop credit. |
		--------------------------------------*/
		if (cumr_rec.stop_credit [0] == 'Y')
		{
			if (db_stopcrd)
			{
				print_mess (ML (mlStdMess060));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			else
			{
				cc = WarnUser (ML (mlStdMess060),0);
				if (cc)
                {
					return (cc);
                }

				held_ser_job = TRUE;
			}
		}

		total_owing = 	cumr_balance [0] + 
			      		cumr_balance [1] +
		              	cumr_balance [2] + 
			      		cumr_balance [3] +
			      		cumr_balance [4] +
			      		cumr_balance [5];

		c_left = total_owing - cumr_rec.credit_limit;

		/*---------------------------------------------
		| Check if customer is over his credit limit. |
		---------------------------------------------*/
		if ((cumr_rec.credit_limit <= total_owing) && 
           (cumr_rec.credit_limit != 0.00) &&
           (cumr_rec.crd_flag [0] != 'Y'))
		{
			if (db_crdover)
			{
				print_mess (ML (mlStdMess061));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			else
			{
				cc = WarnUser (ML (mlStdMess061),0);
				if (cc)
                {
                    return (EXIT_FAILURE);
                }
				held_ser_job = TRUE;
			}
		}
		/*-----------------------
		| Check Credit Terms	|
		-----------------------*/
		if (cumr_rec.od_flag && 
           (cumr_rec.crd_flag [0] != 'Y'))
		{
			if (db_crdterm)
			{
				print_mess (ML (mlStdMess062));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			else
			{
				cc = WarnUser (ML (mlStdMess062),0);
				if (cc)
                {
					return (EXIT_FAILURE);
                }

				held_ser_job = TRUE;
			}
		}

		local_rec.c_hash = cumr_rec.hhcu_hash;
		GetChargeClient ();
		DSP_FLD ("charge_to_name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_client"))
	{
		if (FLD ("end_client") == NA)
        {
            return (EXIT_SUCCESS);
        }

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNo);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.e_client));
		cc = find_rec (cumr ,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}
		local_rec.e_hash = cumr_rec.hhcu_hash;
		strcpy (local_rec.end_client,cumr_rec.dbt_name);
		DSP_FLD ("end_client_name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cust_order_no")) 
	{
		if (!strcmp (sjhr_rec.cust_ord_no,"          "))
		{
			if (cumr_rec.po_flag [0] == 'Y')
			{
				print_mess (ML (mlStdMess048));
				return (EXIT_FAILURE);
			}
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cost_est"))
	{
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cost_fixed"))
	{
		strcpy (local_rec.estim_type, 
               (local_rec.estim_type [0] == 'V') ? "V(ariable)" : "F(ixed)   ");
		DSP_FLD ("cost_fixed");
		return (EXIT_SUCCESS);
	}

	/*=========================================
	| Validate General Ledger Account Number. |
	=========================================*/
	if (LCHECK ("glacct"))
	{
		if (FLD ("glacct") == NA)
        {
			return (EXIT_SUCCESS);
        }

		if (SRCH_KEY)
        {
			return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));
        }

		strcpy (glmrRec.co_no,comm_rec.co_no);
		sprintf (glmrRec.acc_no,"%-*.*s", MAXLEVEL,MAXLEVEL,sjhr_rec.gl_expense);

        cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess024));
			sleep (sleepTime);		
			return (EXIT_FAILURE);
		}
			
		if (CheckGlClass ())
        {
			return (EXIT_FAILURE);
        }

		strcpy (local_rec.acc_desc, glmrRec.desc);
		DSP_FLD ("gl_desc");

		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Validate Detail text lines. |
	-----------------------------*/
    if (LCHECK ("desc"))
    {
	}
	return (EXIT_SUCCESS);
}

void
LoadDetails (
 void)
{
	/*===================
	|  read job details |
	===================*/
	scn_set (2);
	lcount [2] = 0;
	init_vars (2);

	strcpy (sjjd_rec.co_no,comm_rec.co_no);
	strcpy (sjjd_rec.est_no,comm_rec.est_no);
	strcpy (sjjd_rec.dp_no,comm_rec.dp_no);
	sjjd_rec.order_no = sjhr_rec.order_no;
	sjjd_rec.line_no = 0;
	cc = find_rec (sjjd ,&sjjd_rec,GTEQ,"u");

	while (!cc && 
           !strcmp (sjjd_rec.co_no,comm_rec.co_no) && 
           !strcmp (sjjd_rec.est_no,comm_rec.est_no) && 
           !strcmp (sjjd_rec.dp_no,comm_rec.dp_no) && 
          (sjjd_rec.order_no == sjhr_rec.order_no))
	{
		/*==============================================
		| add next description line to tabular memory  |
		==============================================*/
		strcpy (local_rec.job_detail,sjjd_rec.detail);

		putval (lcount [2]++);

		cc = find_rec (sjjd ,&sjjd_rec,NEXT,"u");
	}
	scn_set (1);
}

void
GetChargeClient (
 void)
{
	strcpy (local_rec.c_client,cumr_rec.dbt_no);
	strcpy (local_rec.chg_client,cumr_rec.dbt_name);
	if (strcmp (cumr_rec.class_type,"INT"))
	{
		strcpy (sjhr_rec.cust_type,"E");
		FLD ("end_client") = NA;
		FLD ("glacct")     = NA;
	}
	else
	{
		strcpy (sjhr_rec.cust_type ,"I");
		FLD ("end_client") = YES;
		FLD ("glacct")      = YES;
	}
}

/*==========================================
| Validate credit period and credit limit. |
==========================================*/
int
CheckCumr (
 double inc_amt)
{
	if (cumr_rec.crd_flag [0] == 'Y')
    {
		return (EXIT_SUCCESS);
    }

	total_owing = (cumr_balance [0] + 
	           	   cumr_balance [1] + 
                   cumr_balance [2] + 
                   cumr_balance [3] + 
                   cumr_balance [4] + 
                   cumr_balance [5] + 
                   cumr_rec.ord_value +
                   inc_amt);

	if (cumr_rec.stop_credit [0] == 'Y')
    {
		return (EXIT_FAILURE);
    }

	/*---------------------------------------------
	| Check if customer is over his credit limit. |
	---------------------------------------------*/
	if ((cumr_rec.credit_limit <= total_owing) && 
       (cumr_rec.credit_limit != 0.00))
    {
        return (EXIT_FAILURE);
    }

	/*-----------------------
	| Check Credit Terms	|
	-----------------------*/
	if (cumr_rec.od_flag)
    {
		return (EXIT_FAILURE);
    }
	
	return (EXIT_SUCCESS);
}

/*============================
| Warn user about something. |
============================*/
int
WarnUser (
 char *wn_mess, 
 int wn_flip)
{
	int	i;
	
	clear_mess ();
	print_mess (wn_mess);

	if (!wn_flip)
	{
		i = prmptmsg (ML (mlSjMess047), "YyNnMm", 1, 2);
		move (1,2);
		print_at (2,1,"%-78.78s"," ");
		if (i == 'Y' || i == 'y') 
        {
			return (EXIT_SUCCESS);
        }

		if (i == 'M' || i == 'm') 
		{
			DbBalWin (cumr_rec.hhcu_hash, comm_rec.fiscal,comm_rec.dbt_date);
			i = prmptmsg (ML (mlSjMess027), "YyNn",1,2);
			heading (1);
			scn_display (1);
			move (1,2);
			print_at (2,1,"%-78.78s"," ");
			if (i == 'Y' || i == 'y') 
            {
				return (EXIT_SUCCESS);
            }
		}

		return (EXIT_FAILURE);
	}

	if (wn_flip == 9)
    {
		return (EXIT_FAILURE);
    }
	else
    {
		return (EXIT_SUCCESS);
    }
}

void
Busy (
 int flip)
{
	print_at (2,1,"%-65.65s"," ");
	if (flip)
	{
		print_at (2,1,ML (mlStdMess035));
	}
	fflush (stdout);
}

int
Update (
 void)
{
	int	i;

	if (CheckCumr (sjhr_rec.cost_estim))
	{
		print_mess (ML (mlSjMess029));
		strcpy (sjhr_rec.status,"H");
		
	}

	print_at (2,20,ML (mlStdMess035));
	fflush (stdout);

	scn_set (1);

	/*=====================
	|   update job header |
	=====================*/
	if (new_item)
	{
		if (sjhr_rec.status [0] != 'H')
        {
			strcpy (sjhr_rec.status,"O");
        }

		sjhr_rec.issue_date = StringToDate (local_rec.systemDate);
		sjhr_rec.inv_amt = 0.00;
		sprintf (sjhr_rec.invoice_no,"%8.8s"," ");
		strcpy (sjhr_rec.co_no,comm_rec.co_no);
		strcpy (sjhr_rec.est_no,comm_rec.est_no);
		strcpy (sjhr_rec.dp_no,comm_rec.dp_no);

		SaveTemp ();
		cc = GetOrderNumber ();
		if (cc)
        {
			return (EXIT_SUCCESS);
        }

        cc = abc_add (sjhr,&sjhr_rec);
        if (cc)
        {
			file_err (cc, sjhr, "DBADD");
        }

		Busy (0);
		print_at (2, 20, ML (mlSjMess031), sjhr_rec.order_no);
		PauseForKey (2, 20, "\x000", 0);
	}
	else
	{
		sjhr_rec.chg_client = local_rec.c_hash;
		sjhr_rec.end_client = local_rec.e_hash;
		sprintf (sjhr_rec.estim_type,"%1.1s",local_rec.estim_type);

		cc = abc_update (sjhr,&sjhr_rec);
		if (cc)
        {
			file_err (cc, sjhr, "DBUPDATE");
        }
	}	

	/*=====================
	|  update job details |
	=====================*/
	scn_set (2);

	for (i = 0;i < lcount [2];i++)
	{
		getval (i);
		strcpy (sjjd_rec.co_no,comm_rec.co_no);
		strcpy (sjjd_rec.est_no,comm_rec.est_no);
		strcpy (sjjd_rec.dp_no,comm_rec.dp_no);
		sjjd_rec.order_no = sjhr_rec.order_no;
		sjjd_rec.line_no = i;
		cc = find_rec (sjjd ,&sjjd_rec,COMPARISON,"u");
		if (cc == 0)
		{
			strcpy (sjjd_rec.detail,local_rec.job_detail);
			cc = abc_update (sjjd ,&sjjd_rec);
			if (cc)
            {
				file_err (cc, sjjd, "DBUPDATE");
            }
		}
		else
		{
			strcpy (sjjd_rec.co_no,comm_rec.co_no);
			strcpy (sjjd_rec.est_no,comm_rec.est_no);
			strcpy (sjjd_rec.dp_no,comm_rec.dp_no);
			sjjd_rec.order_no = sjhr_rec.order_no;
			sjjd_rec.line_no = i;
			strcpy (sjjd_rec.detail,local_rec.job_detail);

			cc = abc_add (sjjd ,&sjjd_rec);
			if (cc)
            {
                file_err (cc, sjjd, "DBADD");
            }
		}
	}

	strcpy (sjjd_rec.co_no,comm_rec.co_no);
	strcpy (sjjd_rec.est_no,comm_rec.est_no);
	strcpy (sjjd_rec.dp_no,comm_rec.dp_no);
	sjjd_rec.order_no = sjhr_rec.order_no;
	sjjd_rec.line_no = lcount [2];
	cc = find_rec (sjjd ,&sjjd_rec,GTEQ,"r");

	while (!cc && 
           !strcmp (sjjd_rec.co_no,comm_rec.co_no) && 
           !strcmp (sjjd_rec.est_no,comm_rec.est_no) && 
           !strcmp (sjjd_rec.dp_no,comm_rec.dp_no) && 
          (sjjd_rec.order_no == sjhr_rec.order_no))
	{
		cc = abc_delete (sjjd);
		if (cc)
        {
            continue;
        }

		cc = find_rec (sjjd ,&sjjd_rec,NEXT,"r");
	}
	abc_unlock (sjjd);

	scn_set (1);

    return (EXIT_SUCCESS);
}

int
GetOrderNumber (
 void)
{
	int	i;
	long	order_no = 0L;

	Busy (0);

	/*======================
	|   assign order number |
	=======================*/
	i = prmptmsg (ML (mlSjMess028),"YyNn",20,2);
	Busy (0);
	if (i == 'Y' || i == 'y') 
	{
		do
		{
			order_no = 0L;
			last_char = 0;

			Busy (0);
			print_at (2,20,ML (mlSjMess032));
			order_no = getlong (55,2,"NNNNNN");

			switch (last_char)
			{
            case REDRAW :
			case SEARCH :
                last_char = 0;
				break;

			case EOI :
			case RESTART :
				return (EXIT_FAILURE);

			default:
				last_char = 0;
				break;
			}

			if (order_no == 0L)
            {
                continue;
            }
			
			sjhr_rec.order_no = order_no;
			cc = find_rec (sjhr,&sjhr_rec,COMPARISON,"u");
			if (cc)
			{
				CopyBack ();
				return (EXIT_SUCCESS);
			}
			else
			{
				Busy (0);
				print_at (2,20,ML (mlSjMess033),order_no);
				sleep (sleepTime);
				fflush (stdout);
				last_char = 0;
			}
		} while (last_char != ENDINPUT) ;
	}
    else
	{
		strcpy (esmr_rec.co_no,comm_rec.co_no);
		strcpy (esmr_rec.est_no,comm_rec.est_no);

        cc = find_rec (esmr,&esmr_rec,COMPARISON,"u");
		if (cc)
		{
			errmess (ML (mlStdMess073));
			return (EXIT_FAILURE);
		}
		
		while (TRUE)
		{
			esmr_rec.nx_job_no++;
			sjhr_rec.order_no = esmr_rec.nx_job_no;
			
            cc = find_rec (sjhr,&sjhr_rec,COMPARISON,"u");
			if (cc)
			{
				CopyBack ();
				break;
			}
		}

		cc = abc_update (esmr,&esmr_rec);
		if (cc)
        {
            file_err (cc, esmr, "DBUPDATE");
        }
	}
	return (EXIT_SUCCESS);
}

void
SaveTemp (
 void)
{
	temp_rec.t_chg_client = local_rec.c_hash;
	temp_rec.t_end_client = local_rec.e_hash;
	strcpy (temp_rec.t_cust_ord_no,sjhr_rec.cust_ord_no);
	temp_rec.t_cost_estim = sjhr_rec.cost_estim;
	sprintf (temp_rec.t_estim_type,"%1.1s",local_rec.estim_type);
	strcpy (temp_rec.t_contact,sjhr_rec.contact);
	temp_rec.t_reqd_date = sjhr_rec.reqd_date;
	temp_rec.t_fixed_labour = sjhr_rec.fixed_labour;
	temp_rec.t_comp_date = sjhr_rec.comp_date;
	strcpy (temp_rec.t_status,"O");
	temp_rec.t_issue_date = StringToDate (local_rec.systemDate);
	temp_rec.t_inv_amt = 0.00;
	sprintf (temp_rec.t_gl_exp_code,"%-*.*s",MAXLEVEL,MAXLEVEL,sjhr_rec.gl_expense);
	sprintf (temp_rec.t_invoice_no,"%8.8s"," ");
}

void
CopyBack (
 void)
{
	sjhr_rec.chg_client 		= temp_rec.t_chg_client;
	sjhr_rec.end_client 		= temp_rec.t_end_client;
	strcpy (sjhr_rec.cust_ord_no,temp_rec.t_cust_ord_no);
	sjhr_rec.cost_estim 		= temp_rec.t_cost_estim;
	strcpy (sjhr_rec.estim_type,temp_rec.t_estim_type);
	strcpy (sjhr_rec.contact,temp_rec.t_contact);
	sjhr_rec.reqd_date 		= temp_rec.t_reqd_date;
	sjhr_rec.fixed_labour 	= temp_rec.t_fixed_labour;
	sjhr_rec.comp_date 		= temp_rec.t_comp_date;
	strcpy (sjhr_rec.status, (held_ser_job) ? "H" : "O");
	sjhr_rec.issue_date 		= StringToDate (local_rec.systemDate);
	sjhr_rec.inv_amt = 0.00;
	sprintf (sjhr_rec.gl_expense,"%-*.*s",MAXLEVEL,MAXLEVEL,temp_rec.t_gl_exp_code);
	sprintf (sjhr_rec.invoice_no,"%8.8s"," ");
}

/*=========================================
| Search routine for Service Header File. |
=========================================*/
void
SrchSjhr (
 char *key_val)
{
	char	order_str [9];

	work_open ();
	save_rec ("#Job No","#Issued on");
	strcpy (sjhr_rec.co_no,comm_rec.co_no);
	strcpy (sjhr_rec.est_no,comm_rec.est_no);
	strcpy (sjhr_rec.dp_no,comm_rec.dp_no);
	sjhr_rec.order_no = atol (key_val);

    cc = find_rec (sjhr,&sjhr_rec,GTEQ,"r");
	while (!cc && 
           !strcmp (sjhr_rec.co_no,comm_rec.co_no) && 
           !strcmp (sjhr_rec.est_no,comm_rec.est_no) && 
           !strcmp (sjhr_rec.dp_no,comm_rec.dp_no))
	{
		sprintf (order_str,"%8ld",sjhr_rec.order_no);
		if ((strlen (key_val) == 0 || 
			!strncmp (order_str,key_val,strlen (key_val))) && 
			sjhr_rec.status [0] == 'O')
		{
			strcpy (err_str, DateToString (sjhr_rec.issue_date));
			cc = save_rec (order_str, err_str);
			if (cc)
            {
				break;
            }
		}
		cc = find_rec (sjhr,&sjhr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
    {
        return;
    }
	strcpy (sjhr_rec.co_no,comm_rec.co_no);
	strcpy (sjhr_rec.est_no,comm_rec.est_no);
	strcpy (sjhr_rec.dp_no,comm_rec.dp_no);
	sjhr_rec.order_no = atol (temp_str);
	cc = find_rec (sjhr,&sjhr_rec,COMPARISON,"r");
	if (cc)
    {
		file_err (cc, sjhr, "DBFIND");
    }
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
        {
			scn_set (scn);
        }

		if (scn == 1)
		{
			clear ();
		}

		rv_pr (ML (mlSjMess030),27,0,1);
		move (0,1);
		line (80);

		if (scn == 1)
		{
			box (0,3,80,14);
			move (1,7);
			line (79);
			move (1,15);
			line (79);
		}
		else if (scn == 2)
		{
			scn_write (1);
			scn_display (1);
		}

		move (0,20);
		line (80);
		print_at (21,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
		print_at (22,0,ML (mlStdMess039),comm_rec.est_no,comm_rec.est_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}

int
CheckGlClass (void)
{
	if (glmrRec.glmr_class [2][0] != 'P')
	{
		errmess (ML (mlStdMess025));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	
	return (EXIT_SUCCESS);
}

/* [ end of file ] */
