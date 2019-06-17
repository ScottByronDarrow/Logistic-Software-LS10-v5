/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: lsale_inp.c,v 5.2 2001/08/09 09:21:31 scott Exp $
|  Program Desc  : (Order Entry to lost sales file.             )     |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
| $Log: lsale_inp.c,v $
| Revision 5.2  2001/08/09 09:21:31  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:51:29  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:20:01  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:41:19  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:22:38  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:13:26  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  2000/07/13 22:43:25  scott
| Updated as general maintenance and addition of app.schema
| As maintenance is "item lost sales" the default on a blank item was removed.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lsale_inp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_lsale_inp/lsale_inp.c,v 5.2 2001/08/09 09:21:31 scott Exp $";

#include <ml_std_mess.h>
#include <ml_so_mess.h>
#include <pslscr.h>

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct pocrRecord	pocr_rec;
struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;
struct exafRecord	exaf_rec;
struct exsfRecord	exsf_rec;
struct exlsRecord	exls_rec;
struct ffdmRecord	ffdm_rec;
struct inlsRecord	inls_rec;

	char	branchNumber [3];

	int		envDbMcurr,
			envDbCo,
			envDbFind;

	long	lSystemDate;

	double	lcl_ex_rate;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	item [17];
	char	desc [41];
	char	cust [7];
	char	name [41];
	char	area [3];
	char	area_desc [41];
	char	sale [3];
	char	salesman [41];
	char	res_code [3];
	char	res_desc [61];
	char	curr [41];
	float	qty;	
	double	value;	
	double	cost;	
} local_rec;
	
static	struct	var	vars [] =
{
	{1, LIN, "item",	 5, 15, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Item No.", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.item},
	{1, LIN, "desc",	 5, 60, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", "Desc.", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc},
	{1, LIN, "cust",	 6, 15, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Cust No.", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.cust},
	{1, LIN, "name",	 6, 60, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", "Name.", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.name},
	{1, LIN, "area",	 8, 15, CHARTYPE,
		"UU", "          ",
		" ", " ", "Area Code.", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.area},
	{1, LIN, "adesc",	 8, 60, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", "Desc.", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.area_desc},
	{1, LIN, "sale",	 9, 15, CHARTYPE,
		"UU", "          ",
		" ", " ", "Sale Code.", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.sale},
	{1, LIN, "salesman",	 9, 60, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", "Salesman.", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.salesman},
	{1, LIN, "curr",	 10, 15, CHARTYPE,
		"UUU", "          ",
		" ", "", "Currency.", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.curr},
	{1, LIN, "currdesc",	 10, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Currency.", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.curr},
	{1, LIN, "qty",	12, 15, FLOATTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0.0", "Qty.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.qty},
	{1, LIN, "value",	13, 15, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0.0", "Value.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.value},
	{1, LIN, "val_desc",	 13, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.curr},
	{1, LIN, "cost",	14, 15, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0.0", "Lcl Cost.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.cost},
	{1, LIN, "resc",	15, 15, CHARTYPE,
		"UU", "          ",
		" ", " ", "Reason.", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.res_code},
	{1, LIN, "resd",	15, 60, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Desc.", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.res_desc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindCumr.h>
/*=======================
| Function Declarations |
=======================*/
void 	SetupDefaults	(void);
void	shutdown_prog 	(void);
void	OpenDB 			(void);
void	CloseDB 		(void);
void	ReadMisc 		(void);
void	AddInls 		(void);
int  	spec_valid 		(int);
int  	FindExaf 		(char *);
int  	FindExsf 		(char *);
void 	SrchExaf 		(char *);
void 	SrchExsf 		(char *);
void 	SrchExls 		(char *);
int  	heading 		(int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char * argv [])
{
	char	*sptr = chk_env ("DB_MCURR");

	if (sptr)
		envDbMcurr = atoi (sptr);

	SETUP_SCR (vars);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr	();
	set_tty		();
	set_masks	();
	init_vars	(1);

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind = atoi (get_env ("DB_FIND"));

	OpenDB ();

	strcpy (branchNumber, (envDbCo) ? comm_rec.est_no : " 0");

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

		SetupDefaults ();

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit)
			break;

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		AddInls ();

	}	/* end of input control loop	*/
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
SetupDefaults (
 void)
{
	strcpy (exls_rec.demand_ok, "N");
	FLD ("area") = dflt_used ? NA : YES;
	FLD ("sale") = dflt_used ? NA : YES;
}
	
/*=======================
| program exit sequence	|
=======================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*======================
| Open Database files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	ReadMisc ();

	open_rec ("inmr", inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec ("exls", exls_list, EXLS_NO_FIELDS, "exls_id_no");
	open_rec ("ffdm", ffdm_list, FFDM_NO_FIELDS, "ffdm_id_no2");
	open_rec ("exaf", exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec ("exsf", exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec ("cumr", cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec ("inls", inls_list, INLS_NO_FIELDS, "inls_id_no");
	if (envDbMcurr)
		open_rec ("pocr", pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

/*=======================
| Close Database files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose ("ccmr");
	abc_fclose ("inmr");
	abc_fclose ("exls");
	abc_fclose ("exaf");
	abc_fclose ("exsf");
	abc_fclose ("cumr");
	abc_fclose ("inls");
	abc_fclose ("ffdm");
	if (envDbMcurr)
		abc_fclose ("pocr");

	SearchFindClose ();
	abc_dbclose ("data");
}


/*=============================================
| Get common info from commom database file . |
=============================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec ("ccmr", ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec ("ccmr", &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");
}

void
AddInls (
 void)
{
	inls_rec.date = TodaysDate();
	inls_rec.qty = local_rec.qty;
	inls_rec.value = local_rec.value / lcl_ex_rate;
	inls_rec.cost = local_rec.cost;
	inls_rec.status [0] = '0';
	if (!strlen (clip (local_rec.res_desc)))
		sprintf (inls_rec.res_desc,"%-60.60s"," ");
	else	
		sprintf (inls_rec.res_desc,"%-60.60s",local_rec.res_desc);

	cc = abc_add ("inls",&inls_rec);
	if (cc)
	       file_err (cc, "inls", "DBADD");

	if (exls_rec.demand_ok [0] == 'Y')
	{
		ffdm_rec.hhbr_hash	=	inls_rec.hhbr_hash;
		ffdm_rec.hhcc_hash	=	inls_rec.hhcc_hash;
		ffdm_rec.date		=	inls_rec.date;
		strcpy (ffdm_rec.type, "5");
		cc = find_rec ("ffdm", &ffdm_rec, COMPARISON, "r");
		if (cc)
		{
			ffdm_rec.qty	=	inls_rec.qty;
			cc = abc_add ("ffdm", &ffdm_rec);
			if (cc)
				file_err (cc, "ffdm", "DBADD");
		}
		else
		{
			ffdm_rec.qty	+=	inls_rec.qty;
			cc = abc_update ("ffdm", &ffdm_rec);
			if (cc)
				file_err (cc, "ffdm", "DBUPDATE");
		}
	}
}

int
spec_valid (
 int field)
{
	strcpy (inls_rec.co_no,comm_rec.co_no);
	strcpy (inls_rec.est_no,comm_rec.est_no);
	inls_rec.hhcc_hash = ccmr_rec.hhcc_hash;

	/*------------------
	| Validate to item |
	------------------*/
	if (LCHECK ("item"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		/*
		if (strlen (clip (local_rec.item)) == 0)
		{
			inls_rec.hhbr_hash = 0L;
			return (EXIT_SUCCESS);
		}
		*/

		cc = FindInmr (comm_rec.co_no, local_rec.item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (local_rec.item, inmr_rec.item_no);

		inls_rec.hhbr_hash = inmr_rec.hhbr_hash;
		sprintf (local_rec.desc,"%-40.40s",inmr_rec.description);
		DSP_FLD ("item");
		DSP_FLD ("desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cust"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (strlen (clip (local_rec.cust)) == 0)
		{
			inls_rec.hhcu_hash = 0L;
			strcpy (inls_rec.area_code,"  ");
			strcpy (inls_rec.sale_code,"  ");
			sprintf (local_rec.curr, "%-40.40s", " ");
			lcl_ex_rate = 1.00;
			FLD ("area") = YES;
			FLD ("sale") = YES;
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		sprintf (cumr_rec.dbt_no,"%-6.6s",pad_num (local_rec.cust));
		cc = find_rec ("cumr", &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}

		if (envDbMcurr)
		{
			strcpy (pocr_rec.co_no, cumr_rec.co_no);
			strcpy (pocr_rec.code,  cumr_rec.curr_code);
			cc = find_rec ("pocr", &pocr_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, "pocr", "DBFIND");
			lcl_ex_rate = pocr_rec.ex1_factor;
			strcpy (local_rec.curr, pocr_rec.description);
			DSP_FLD ("currdesc");
			DSP_FLD ("val_desc");
		}
		else
		{
			sprintf (local_rec.curr, "%-40.40s", " ");
			lcl_ex_rate = 1.00;
			DSP_FLD ("currdesc");
		}

		inls_rec.hhcu_hash = cumr_rec.hhcu_hash;
		strcpy (inls_rec.area_code,cumr_rec.area_code);
		strcpy (inls_rec.sale_code,cumr_rec.sman_code);
		if (FindExaf (cumr_rec.area_code))
			return (EXIT_FAILURE);

		if (FindExsf (cumr_rec.sman_code))
			return (EXIT_FAILURE);

		sprintf (local_rec.name,"%-40.40s",cumr_rec.dbt_name);
		sprintf (local_rec.area,"%-2.2s",cumr_rec.area_code);
		sprintf (local_rec.sale,"%-2.2s",cumr_rec.sman_code);
		sprintf (local_rec.area_desc,"%-40.40s",exaf_rec.area);
		sprintf (local_rec.salesman,"%-40.40s",exsf_rec.salesman);
		FLD ("area") = NA;
		FLD ("sale") = NA;
		DSP_FLD ("name");
		DSP_FLD ("area");
		DSP_FLD ("sale");
		DSP_FLD ("adesc");
		DSP_FLD ("salesman");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("area"))
	{
		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}

		if (strlen (clip (local_rec.area)) == 0)
		{
			strcpy (inls_rec.area_code,"  ");
			return (EXIT_SUCCESS);
		}

		cc = FindExaf (local_rec.area);
		if (cc)
			return (EXIT_FAILURE);

		strcpy (inls_rec.area_code,exaf_rec.area_code);
		sprintf (local_rec.area_desc,"%-40.40s",exaf_rec.area);
		DSP_FLD ("adesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sale"))
	{
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}

		if (strlen (clip (local_rec.sale)) == 0)
		{
			strcpy (inls_rec.sale_code,"  ");
			return (EXIT_SUCCESS);
		}

		cc = FindExsf (local_rec.sale);
		if (cc)
			return (EXIT_FAILURE);

		strcpy (inls_rec.sale_code,exsf_rec.salesman_no);
		sprintf (local_rec.salesman,"%-40.40s",exsf_rec.salesman);
		DSP_FLD ("salesman");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("resc"))
	{
		if (SRCH_KEY)
		{
			SrchExls (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exls_rec.co_no,comm_rec.co_no);
		sprintf (exls_rec.code,"%-2.2s",local_rec.res_code);
		cc = find_rec ("exls", &exls_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess163));
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.res_desc,"%-60.60s",exls_rec.description);
		strcpy (inls_rec.res_code,exls_rec.code);
		strcpy (inls_rec.res_desc,local_rec.res_desc);

		if (strlen (clip (local_rec.res_desc)) > 0)
			skip_entry = 1;

		DSP_FLD ("resd");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

int
FindExaf (
 char *area_code)
{
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%2.2s",area_code);
	cc = find_rec ("exaf", &exaf_rec, COMPARISON, "r");
	if (cc)
	{
		print_mess (ML (mlStdMess108));
		return (EXIT_FAILURE);
	}
    return (EXIT_SUCCESS);
}

int
FindExsf (
 char *sman_code)                                                                       
{
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%2.2s",sman_code);
	cc = find_rec ("exsf", &exsf_rec, COMPARISON, "r");
	if (cc)
	{
		print_mess (ML (mlStdMess135));
		return (EXIT_FAILURE);
	}
    return (EXIT_SUCCESS);
}

void
SrchExaf (
 char *key_val)
{
	work_open ();
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",key_val);
	cc = find_rec ("exaf", &exaf_rec, GTEQ, "r");
	save_rec ("#Area Code.","#Area Description");
	while (!cc && !strcmp (exaf_rec.co_no,comm_rec.co_no) && 
		      !strncmp (exaf_rec.area_code,key_val,strlen (key_val)))
	{
		cc = save_rec (exaf_rec.area_code, exaf_rec.area);
		if (cc)
			break;
		cc = find_rec ("exaf", &exaf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",temp_str);
	cc = find_rec ("exaf", &exaf_rec, COMPARISON, "r");
	if (cc)
	       file_err (cc, "exaf", "DBFIND");
}

void
SrchExsf (
 char *key_val)
{
	work_open ();
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",key_val);
	cc = find_rec ("exsf", &exsf_rec, GTEQ, "r");
	save_rec ("#Sman Code.","#Salesman Name   ");
	while (!cc && !strcmp (exsf_rec.co_no,comm_rec.co_no) && 
		      !strncmp (exsf_rec.salesman_no,key_val,strlen (key_val)))
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
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",temp_str);
	cc = find_rec ("exsf", &exsf_rec, COMPARISON, "r");
	if (cc)
	       file_err (cc, "exsf", "DBFIND");
}

void
SrchExls (
 char *key_val)
{
	work_open ();
	save_rec ("#Ls","#Lost Sale Description");
	strcpy (exls_rec.co_no,comm_rec.co_no);
	sprintf (exls_rec.code,"%-2.2s",key_val);
	cc = find_rec ("exls",&exls_rec,GTEQ,"r");
	while (!cc && !strncmp (exls_rec.code,key_val,strlen (key_val)) && 
			!strcmp (exls_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (exls_rec.code,exls_rec.description);
		if (cc)
			break;
		cc = find_rec ("exls",&exls_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exls_rec.co_no,comm_rec.co_no);
	sprintf (exls_rec.code,"%-2.2s",temp_str);
	cc = find_rec ("exls",&exls_rec,COMPARISON,"r");
	if (cc)
	       file_err (cc, "exsf", "DBFIND");
}

int
heading (
 int scn)
{
	int	s_size = 132;

	swide ();

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlSoMess017),50,0,1);
		move (0,1);
		line (s_size);

		move (1,input_row);
		if (scn == 1)
			box (0,4,132,11);

		move (1,7);
		line (s_size - 1);
		move (1,11);
		line (s_size - 1);
		move (0,19);
		line (s_size - 1);
		move (0,20);
		print_at (20,0,ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
		print_at (21,0,ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);
		move (0,22);
		line (s_size);
		move (1,input_row);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);

		if (exls_rec.demand_ok [0] == 'Y')
			rv_pr ("Note : This code updates demand history",6,16,1);
		else
		{
			move (6,16);line (40);
		}
	}
    return (EXIT_SUCCESS);
}
