/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: tm_convert.c,v 5.3 2001/11/12 05:37:35 scott Exp $
|  Program Name  : (tm_convert.c)
|  Program Desc  : (Create a Customer from a Lead and vice versa.) 
|                 (NB. This is not part of stand alone TM)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 07/08/91         |
|---------------------------------------------------------------------|
| $Log: tm_convert.c,v $
| Revision 5.3  2001/11/12 05:37:35  scott
| Updated to convert to app.schema
| Updated to clean up code
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: tm_convert.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_convert/tm_convert.c,v 5.3 2001/11/12 05:37:35 scott Exp $";

#define	TXT_REQD
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_tm_mess.h>

#define	D_to_L	 (local_rec.con_type [0] == 'D')
#define	L_to_D	 (local_rec.con_type [0] == 'L')

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int  	newProspect 	= 0,
   	     	newCustomer 	= 0,
			envDbCo 		= 0,
			envDbFind 		= 0;

	char	branchNo [3];

	extern	int	TruePosition;

#include	"schema"

struct commRecord	comm_rec;
struct tmpmRecord	tmpm_rec;
struct tmpmRecord	tmpm2_rec;
struct cumrRecord	cumr_rec;
struct cumdRecord	cumd_rec;

	char	*data  = "data",
			*tmpm2 = "tmpm2";

long	systemDate	=	0L;
int		defaultCumd;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy 		[11];
	char	con_type 	[2];
	char 	from 		[7];
	char 	fr_name 	[61];
	char 	fr_prmpt 	[61];
	char 	fr_comm 	[61];
	char 	to 			[7];
	char 	to_name 	[61];
	char 	to_prmpt 	[61];
	char 	to_comm 	[61];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "from",	 4, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", local_rec.fr_prmpt, local_rec.fr_comm,
		 NE, NO,  JUSTLEFT, "", "", local_rec.from},
	{1, LIN, "fr_name",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Name                   ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.fr_name},
	{1, LIN, "to",	 7, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", local_rec.from, local_rec.to_prmpt, local_rec.to_comm,
		 NE, NO,  JUSTLEFT, "", "", local_rec.to},
	{1, LIN, "to_name",	 8, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", local_rec.fr_name, "Name                   ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.to_name},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	Convert 		(void);
void	SrchTmpm		(char *);
int 	spec_valid 		(int);
int 	GetCumd 		(void);
int 	heading 		(int);

#include <FindCumr.h>

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int argc,
	char *argv [])
{
	if (argc != 2)
	{
		print_at (0,0,mlTmMess700, argv [0]);
		return (EXIT_FAILURE);
	}

	systemDate = TodaysDate ();

	TruePosition	=	TRUE;

	sprintf (local_rec.con_type, "%-1.1s", argv [1]);

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind = atoi (get_env ("DB_FIND"));

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	
	OpenDB (); 	

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	if (D_to_L)
	{
		strcpy (local_rec.fr_prmpt, "From Customer Number    ");
		strcpy (local_rec.fr_comm, 	"Enter Customer number to Convert From");
		strcpy (local_rec.to_prmpt, "To Lead Number          ");
		strcpy (local_rec.to_comm, 	"Enter Lead number to Convert To");
	}
	else
	{
		strcpy (local_rec.fr_prmpt, "From Lead number       ");
		strcpy (local_rec.fr_comm, 	"Enter Lead No number Convert Form");
		strcpy (local_rec.to_prmpt, "To Customer number     ");
		strcpy (local_rec.to_comm, 	"Enter Customer number To Convert To");
	}

	init_vars (1);

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	/*--------------------
	| Main control loop. |
	--------------------*/
	while (prog_exit == 0)	
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		abc_unlock (tmpm);
		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		Convert ();
	}
	CloseDB (); 
	FinishProgram ();
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

	abc_alias (tmpm2, tmpm);

	open_rec (tmpm, tmpm_list, TMPM_NO_FIELDS, (envDbFind) ? "tmpm_id_no3" 
							      						   : "tmpm_id_no");
	open_rec (tmpm2, tmpm_list, TMPM_NO_FIELDS, "tmpm_hhcu_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (envDbFind) ? "cumr_id_no3" 
							                               : "cumr_id_no");

	open_rec (cumd, cumd_list, CUMD_NO_FIELDS, "cumd_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (tmpm);
	abc_fclose (tmpm2);
	abc_fclose (cumr);
	abc_fclose (cumd);
	abc_dbclose (data);
}

int
spec_valid (
	int field)
{
	/*------------------------------------------
	| Validate Prospect Number And Allow Search. |
	------------------------------------------*/
	if (LCHECK ("from") && D_to_L)
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);
		sprintf (cumr_rec.dbt_no, "%-6.6s", pad_num (local_rec.from));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*----------------------------
		| Check for a lead with this |
		| hhcu hash.                 |
		----------------------------*/
		tmpm2_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (tmpm2, &tmpm2_rec, COMPARISON, "r");
		if (!cc)
		{
			print_mess (ML (mlTmMess027));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.fr_name, "%-40.40s", cumr_rec.dbt_name);
		DSP_FLD ("fr_name");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("to") && D_to_L)
	{
		if (SRCH_KEY)
		{
			SrchTmpm (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (tmpm_rec.co_no, comm_rec.co_no);
		strcpy (tmpm_rec.br_no, branchNo);
		strcpy (tmpm_rec.pro_no, pad_num (local_rec.to));
		newProspect = find_rec (tmpm, &tmpm_rec, COMPARISON, "r");
		if (!newProspect)
		{
			print_mess (ML (mlTmMess054));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("from") && L_to_D)
	{
		if (SRCH_KEY)
		{
			SrchTmpm (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (tmpm_rec.co_no, comm_rec.co_no);
		strcpy (tmpm_rec.br_no, branchNo);
		strcpy (tmpm_rec.pro_no, pad_num (local_rec.from));
		cc = find_rec (tmpm, &tmpm_rec, COMPARISON, "w");
		if (cc)
		{
			print_mess (ML (mlTmMess052));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (tmpm_rec.hhcu_hash != 0L)
		{
			print_mess (ML (mlTmMess027));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.fr_name, "%-40.40s", tmpm_rec.name);
		DSP_FLD ("fr_name");
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("to") && L_to_D)
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);
		sprintf (cumr_rec.dbt_no, "%-6.6s", pad_num (local_rec.to));
		newCustomer = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (!newCustomer)
		{
			print_mess (ML (mlStdMess096));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}	

/*----------------
| Update Record. |
----------------*/
void
Convert (void)
{
	clear ();

	if (D_to_L) 
	{
		memset (&tmpm_rec, 0, sizeof (tmpm_rec));

		strcpy (tmpm_rec.co_no,         cumr_rec.co_no);
		strcpy (tmpm_rec.br_no,         cumr_rec.est_no);
		sprintf (tmpm_rec.pro_no,       "%-8.8s",   local_rec.to);
		tmpm_rec.hhcu_hash   		   = cumr_rec.hhcu_hash;
		sprintf (tmpm_rec.name,         "%-40.40s", local_rec.to_name);
		sprintf (tmpm_rec.acronym,      "%-9.9s",   cumr_rec.dbt_acronym);
		sprintf (tmpm_rec.sman_code,    "%2.2s",    cumr_rec.sman_code);
		sprintf (tmpm_rec.area_code,    "%2.2s",    cumr_rec.area_code);
		sprintf (tmpm_rec.b_sector,     "%-3.3s",   cumr_rec.class_type);
		sprintf (tmpm_rec.post_code,    "%-10.10s", cumr_rec.post_code);
		strcpy (tmpm_rec.active_flg,    "Y");
		sprintf (tmpm_rec.cont_name1, "%-20.20s", cumr_rec.contact_name);
		sprintf (tmpm_rec.cont_name2, "%-20.20s", " ");
		sprintf (tmpm_rec.cont_name3, "%-20.20s", " ");
		sprintf (tmpm_rec.cont_name4, "%-20.20s", " ");
		sprintf (tmpm_rec.cont_name5, "%-20.20s", " ");
		sprintf (tmpm_rec.cont_name6, "%-20.20s", " ");
		sprintf (tmpm_rec.cont_code1, "%-3.3s",   " ");
		sprintf (tmpm_rec.cont_code2, "%-3.3s",   " ");
		sprintf (tmpm_rec.cont_code3, "%-3.3s",   " ");
		sprintf (tmpm_rec.cont_code4, "%-3.3s",   " ");
		sprintf (tmpm_rec.cont_code5, "%-3.3s",   " ");
		sprintf (tmpm_rec.cont_code6, "%-3.3s",   " ");
		sprintf (tmpm_rec.mail1_adr, "%-40.40s", cumr_rec.ch_adr1);
		sprintf (tmpm_rec.mail2_adr, "%-40.40s", cumr_rec.ch_adr2);
		sprintf (tmpm_rec.mail3_adr, "%-40.40s", cumr_rec.ch_adr3);
		sprintf (tmpm_rec.del1_adr,  "%-40.40s", cumr_rec.dl_adr1);
		sprintf (tmpm_rec.del2_adr,  "%-40.40s", cumr_rec.dl_adr2);
		sprintf (tmpm_rec.del3_adr,  "%-40.40s", cumr_rec.dl_adr3);
		sprintf (tmpm_rec.fax_no,       "%-15.15s", cumr_rec.fax_no);
		sprintf (tmpm_rec.phone_no,     "%-15.15s", cumr_rec.phone_no);
		strcpy (tmpm_rec.n_phone_time, TimeHHMM ());
		strcpy (tmpm_rec.n_visit_time, TimeHHMM ());
		sprintf (tmpm_rec.call_bk_time, "%-5.5s",   "00:00");
		sprintf (tmpm_rec.best_ph_time, "%-5.5s",   "     ");
		strcpy (tmpm_rec.mail_flag,     "Y");
		strcpy (tmpm_rec.op_code,       "              ");
		strcpy (tmpm_rec.lst_op_code,   "              ");
		strcpy (tmpm_rec.origin,        "   ");
		strcpy (tmpm_rec.delete_flag,   "N");
		strcpy (tmpm_rec.tax_code,      cumr_rec.tax_code);
		strcpy (tmpm_rec.tax_no,        cumr_rec.tax_no);
		strcpy (tmpm_rec.stat_flag,     "0");
		tmpm_rec.phone_freq   = 1;
		tmpm_rec.n_phone_date = systemDate;
		tmpm_rec.visit_freq   = 1;
		tmpm_rec.n_visit_date = systemDate;
		tmpm_rec.call_bk_date = 0L;
		tmpm_rec.call_no      = 0L;
		tmpm_rec.lphone_date = systemDate;
		tmpm_rec.date_create  = systemDate;

		cc = abc_add (tmpm, &tmpm_rec);
		if (cc) 
		{
			if (cc == DUPADD) 
			{
			   	errmess (ML (mlTmMess054));
				sleep (sleepTime);
				return;
			}
			file_err (cc, tmpm, "DBADD");
		}
	}
	else 
	{
		memset (&cumr_rec, 0, sizeof (cumr_rec));

		defaultCumd = GetCumd ();

		strcpy (cumr_rec.co_no,      tmpm_rec.co_no);
		strcpy (cumr_rec.est_no,     tmpm_rec.br_no);
		strcpy (cumr_rec.department, comm_rec.dp_no);
		sprintf (cumr_rec.dbt_no,    "%-6.6s",   local_rec.to);
		sprintf (cumr_rec.dbt_name,  "%-40.40s", local_rec.to_name);
		sprintf (cumr_rec.dbt_acronym,"%-9.9s",   tmpm_rec.acronym);
		sprintf (cumr_rec.acc_type,	 "%-1.1s", 
									(defaultCumd) ? cumd_rec.acc_type : "O");
		sprintf (cumr_rec.stmt_type, "%-1.1s", 
									(defaultCumd) ? cumd_rec.stmt_type : "O");
		sprintf (cumr_rec.class_type, "%-3.3s", tmpm_rec.b_sector);
		sprintf (cumr_rec.price_type, "%-1.1s", 
				 					(defaultCumd) ? cumd_rec.price_type : "1");
		strcpy (cumr_rec.int_flag, "Y");
		sprintf (cumr_rec.bo_flag, "%-1.1s", 
				 					(defaultCumd) ? cumd_rec.bo_flag : "N");
		sprintf (cumr_rec.bo_cons, "%-1.1s", 
				 					(defaultCumd) ? cumd_rec.bo_cons : "N");

		if (defaultCumd)
			cumr_rec.bo_days = cumd_rec.bo_days;
		else
			cumr_rec.bo_days = 0;

		sprintf (cumr_rec.po_flag, "%-1.1s", 
									(defaultCumd) ? cumd_rec.po_flag : "Y");
		sprintf (cumr_rec.sur_flag, "%-1.1s", 
				 					(defaultCumd) ? cumd_rec.sur_flag : "N");
		sprintf (cumr_rec.ch_adr1, "%-40.40s", tmpm_rec.mail1_adr);
		sprintf (cumr_rec.ch_adr2, "%-40.40s", tmpm_rec.mail2_adr);
		sprintf (cumr_rec.ch_adr3, "%-40.40s", tmpm_rec.mail3_adr);
		sprintf (cumr_rec.dl_adr1, "%-40.40s", tmpm_rec.del1_adr);
		sprintf (cumr_rec.dl_adr2, "%-40.40s", tmpm_rec.del2_adr);
		sprintf (cumr_rec.dl_adr3, "%-40.40s", tmpm_rec.del3_adr);
		sprintf (cumr_rec.contact_name, "%-20.20s", tmpm_rec.cont_name1);
		sprintf (cumr_rec.phone_no,  "%-15.15s", tmpm_rec.phone_no);
		sprintf (cumr_rec.fax_no,    "%-15.15s", tmpm_rec.fax_no);
		sprintf (cumr_rec.post_code, "%-10.10s", tmpm_rec.post_code);
		sprintf (cumr_rec.stop_credit, "%-1.1s", 
								(defaultCumd) ? cumd_rec.stop_credit : "N");
		sprintf (cumr_rec.crd_prd, "%-3.3s", 
				 				(defaultCumd) ? cumd_rec.crd_prd : "20A");
		sprintf (cumr_rec.area_code,   "%2.2s",    tmpm_rec.area_code);
		sprintf (cumr_rec.sman_code,   "%2.2s",    tmpm_rec.sman_code);
		sprintf (cumr_rec.roy_type,    "%-1.1s", 
								(defaultCumd) ? cumd_rec.roy_type : " ");
		sprintf (cumr_rec.disc_code, "%-1.1s", 
				 				(defaultCumd) ? cumd_rec.disc_code : "A");
		sprintf (cumr_rec.tax_code, "%-1.1s", 
				 				(defaultCumd) ? cumd_rec.tax_code : " ");
		sprintf (cumr_rec.ch_to_ho_flg, "%-1.1s", 
				 				(defaultCumd) ? cumd_rec.ch_to_ho_flg : "N");
		sprintf (cumr_rec.stmnt_flg, "%-1.1s", 
				 				(defaultCumd) ? cumd_rec.stmnt_flg : " ");
		sprintf (cumr_rec.freight_chg, "%-1.1s", 
				 				(defaultCumd) ? cumd_rec.freight_chg : "Y");
		sprintf (cumr_rec.restock_fee, "%-1.1s", 
				 				(defaultCumd) ? cumd_rec.restock_fee : "Y");
		sprintf (cumr_rec.nett_pri_prt, "%-1.1s", 
				 				(defaultCumd) ? cumd_rec.nett_pri_prt : "N");
		sprintf (cumr_rec.stat_flag, "%-1.1s", 
				 				(defaultCumd) ? cumd_rec.stat_flag : "0");

		cc = abc_add (cumr, &cumr_rec);
		if (cc) 
		{
			if (cc == DUPADD) 
			{
			   	errmess (ML (mlStdMess096));
				sleep (sleepTime);
				return;
			}
			file_err (cc, cumr, "DBADD");
		}

		/*
		 * update tmpm_hhcu_hash.
		 */
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, cumr, "DBFIND");

		tmpm_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cc = abc_update (tmpm, &tmpm_rec);
		if (cc)
			file_err (cc, tmpm, "DBUPDATE");
	}

	return;
}

int
GetCumd (void)
{
	/*
	 * Check first for global defaults
	 */
	strcpy (cumd_rec.co_no,  comm_rec.co_no);
	strcpy (cumd_rec.est_no, "  ");
	cc = find_rec (cumd, &cumd_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (cumd_rec.co_no,  comm_rec.co_no);
		strcpy (cumd_rec.est_no, branchNo);
		cc = find_rec (cumd, &cumd_rec, COMPARISON, "r");
		if (cc)
			return (FALSE);
	}

	return (TRUE);
}

#include	<wild_search.h>

/*
 * Search for Prospect master file. 
 */
void
SrchTmpm (char *key_val)
{
	int		valid = 1;
	int		break_out;
	char	type_flag[2];
	char	*stype = chk_env ("DB_SER");
	char	*sptr = (*key_val == '*') ? (char *)0 : key_val;

	switch (search_key)
	{
	case	FN9:
		strcpy (type_flag,"N");
		break;

	case	FN10:
		strcpy (type_flag,"A");
		break;

	case	FN12:
		strcpy (type_flag,"D");
		break;

	default:
		sprintf (type_flag,"%-1.1s", (stype != (char *)0) ? stype : "N");
		break;
	}

	if (type_flag[0] == 'D' || type_flag[0] == 'd')
		sptr = (char *)0;

	work_open ();

	if (type_flag[0] == 'A' || type_flag[0] == 'a')
	{
		abc_selfield (tmpm, (envDbFind) ? "tmpm_id_no4" : "tmpm_id_no2");
		save_rec ("#Number","# Acronym   Prospect Name.");
	}
	else
	{
		abc_selfield (tmpm, (envDbFind) ? "tmpm_id_no3" : "tmpm_id_no");
		save_rec ("#Number","# Prospect Name.");
	}
	strcpy (tmpm_rec.co_no,comm_rec.co_no);
	strcpy (tmpm_rec.br_no,branchNo);
	sprintf (tmpm_rec.acronym,"%-9.9s", (sptr != (char *)0) ? sptr : " ");
	sprintf (tmpm_rec.pro_no,"%-6.6s", (sptr != (char *)0) ? sptr : " ");
	
	cc = find_rec (tmpm,&tmpm_rec,GTEQ,"r");
	while (!cc && !strcmp (tmpm_rec.co_no,comm_rec.co_no))
	{
		/*
		 * If Debtors Branch Owned && Correct Branch. 
		 */
		if (!envDbFind && strcmp (tmpm_rec.br_no,branchNo))
			break;

		switch (type_flag[0])
		{
		case	'A':
		case	'a':
			valid = check_search (tmpm_rec.acronym,key_val,&break_out);
			break;

		case	'D':
		case	'd':
			valid = check_search (tmpm_rec.name,key_val,&break_out);
			break_out = 0;
			break;

		default:
			valid = check_search (tmpm_rec.pro_no,key_val,&break_out);
			break;
		}

		if (valid)
		{
			sprintf (err_str," (%s) %-40.40s", tmpm_rec.acronym, tmpm_rec.name);
			cc = save_rec (tmpm_rec.pro_no,err_str);
			if (cc)
				break;
		}
		else
		{
			if (break_out)
				break;
		}

		cc = find_rec (tmpm,&tmpm_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	abc_selfield (tmpm, (envDbFind) ? "tmpm_id_no3" : "tmpm_id_no");
	if (cc)
	{
		sprintf (tmpm_rec.acronym,"%-9.9s"," ");
		return;
	}
	
	strcpy (tmpm_rec.co_no,comm_rec.co_no);
	strcpy (tmpm_rec.br_no,branchNo);
	sprintf (tmpm_rec.pro_no,"%-6.6s",temp_str);
	sprintf (tmpm_rec.acronym,"%-9.9s",temp_str);
	cc = find_rec (tmpm,&tmpm_rec,GTEQ,"r");
	if (cc)
		file_err (cc, tmpm, "DBFIND");
}
int
heading (
	int scn)
{
	if (restart)
		return (EXIT_FAILURE);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	box (0, 3, 80, 5);
	line_at (6,1,79);

	if (D_to_L)
		rv_pr (ML (mlTmMess028), 25, 0, 1);
	else
		rv_pr (ML (mlTmMess029), 25, 0, 1);

	line_at (1,0,80);

	move (1, input_row);

	if (scn == 4)
		box (0, 3, 80, 12);

	line_at (20,0,80);
	print_at (21,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	line_at (22,0,80);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}

