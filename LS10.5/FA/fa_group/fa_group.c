/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|---------------------------------------------------------------------|
|  Program Name  : ( fa_group.c     )                                 |
|  Program Desc  : ( Fixed Assets group maintenance.              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Updates files : See /usr/ver(x)/DOCS/Programs                      |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 30/12/87         |
|---------------------------------------------------------------------|
|  Date Modified : (30/12/87)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (23/11/88)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (29/03/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (11/09/97)      | Modified  by  : Rowena S Maandig |
|  Date Modified : (19/08/99)      | Modified  by  : Mars dela Cruz.  | 
|                :                                                    |
|  Comments      : Include new scrgen.                                |
|                : fatr_group is 2 char (formerly 3 char)             |
|                :                                                    |
|  (29/03/94)    : INF 10647. Changes for ver9 compile on SCO.        |
|                :                                                    |
|  Date Modified : (15/05/1997)    | Modified  by : Scott B Darrow    |
|   Comments     :  New fixed assets system                           |
|                :                                                    |
|  (11/09/97)    : Incorporate multilingual conversion.               |
|  (19/08/99)    : Ported to ANSI standards.                          |
|                :                                                    |
| $Log: fa_group.c,v $
| Revision 5.3  2001/09/10 10:32:35  cha
| SE-129.
|
| Revision 5.2  2001/08/09 09:13:11  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:25:54  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:05:52  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:26:25  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:14:33  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:55:18  gerry
| forced Revsion No start 2.0 Rel-15072000
|
| Revision 1.11  1999/11/17 06:40:02  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.10  1999/11/08 04:54:45  scott
| Updated due to warnings using -Wall flag on compiler.
|
| Revision 1.9  1999/10/01 07:48:36  scott
| Updated for standard function calls.
|
| Revision 1.8  1999/09/29 10:10:38  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/16 02:49:16  scott
| Updated from Ansi Project.
|
| Revision 1.6  1999/09/14 01:13:52  marlyn
| Ported to ANSI standards.
|
| Revision 1.5  1999/06/14 23:57:36  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: fa_group.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/FA/fa_group/fa_group.c,v 5.3 2001/09/10 10:32:35 cha Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_fa_mess.h>

int	new_item = 0;

	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_cc_name"},
		{"comm_cc_short"},
	};

	int comm_no_fields = 10;
	
	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tco_short[16];
		char	tes_no[3];
		char	tes_name[41];
		char	tes_short[16];
		char	tcc_no[3];
		char	tcc_name[41];
		char	tcc_short[10];
	} comm_rec;

	/*================================+
	 | Fixed Asset Transactions file. |
	 +================================*/
#define	FATR_NO_FIELDS	12

	struct dbview	fatr_list [FATR_NO_FIELDS] =
	{
		{"fatr_co_no"},
		{"fatr_group"},
		{"fatr_group_desc"},
		{"fatr_dep_rule"},
		{"fatr_nxt_asset"},
		{"fatr_ass_life"},
		{"fatr_max_depr"},
		{"fatr_tax_dtype"},
		{"fatr_tax_pa_flag"},
		{"fatr_int_dtype"},
		{"fatr_int_pa_flag"},
		{"fatr_stat_flag"}
	};

	struct tag_fatrRecord
	{
		char	co_no [3];
		char	group [6];
		char	group_desc [41];
		char	dep_rule [2];
		long	nxt_asset;
		char	ass_life [8];
		Money	max_depr;
		char	tax_dtype [2];
		char	tax_pa_flag [2];
		char	int_dtype [2];
		char	int_pa_flag [2];
		char	stat_flag [2];
	}	fatr_rec;

struct {
	char	dummy[11];
	char	prev_group[6];
	char	tax_d_type[3];
	char	int_d_type[3];
	char	AssYears[5],
			AssMonth[2];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "groupno",	 4, 2, CHARTYPE,
		"UUUUU", "          ",
		" ", "", "Asset group no.    : ", " ",
		 NE, NO, JUSTLEFT, "", "", fatr_rec.group},
	{1, LIN, "assetdesc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Asset group desc.  : ", " ",
		YES, NO,  JUSTLEFT, "", "", fatr_rec.group_desc},
	{1, LIN, "DepRule",	7, 2, CHARTYPE,
		"U", "          ",
		" ", "M", "Depreciation Rule  : ", "(F) - Full Year /  (H) - Half Year) /  (M) - Per Month.",
		 NO, NO,  JUSTLEFT, "FHM", "", fatr_rec.dep_rule},
	{1, LIN, "maxdeprec",	 8, 2, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "", "Max. depreciation  : ", " ",
		 NO, NO,  JUSTLEFT, "", "", (char *) &fatr_rec.max_depr},
	{1, LIN, "AssetLife",	 9, 2, CHARTYPE,
		"NNN", "          ",
		" ", "", "Asset Life Years   : ", "Enter Asset life Years ",
		 NO, NO,  JUSTRIGHT, "", "", local_rec.AssYears},
	{1, LIN, "AssetMonth",	 9, 27, CHARTYPE,
		"NN", "          ",
		" ", "0", ":", "Enter Asset life Months ",
		 NO, NO,  JUSTLEFT, "0", "12", local_rec.AssMonth},
	{1, LIN, "TaxType",	11, 2, CHARTYPE,
		"UU", "          ",
		" ", "", "Depreciation type  : ", "Enter CP (Cost Price - straight line) DV (Diminishing value) ",
		 NO, NO,  JUSTLEFT, "CPDV", "", local_rec.tax_d_type},
	{1, LIN, "TaxFlag",	12, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Deprec. By (P/A)   : ", "Enter Depreciation calculation P(ercent) or A(mount) ",
		 NO, NO,  JUSTLEFT, "PA", "", fatr_rec.tax_pa_flag},
	{1, LIN, "IntType",	11, 40, CHARTYPE,
		"UU", "          ",
		" ", "", "Depreciation type  : ", "Enter CP (Cost Price - straight line) DV (Diminishing value) ",
		 NO, NO,  JUSTLEFT, "CPDV", "", local_rec.int_d_type},
	{1, LIN, "IntFlag",	12, 40, CHARTYPE,
		"U", "          ",
		" ", "A", "Deprec. By (P/A)   : ", "Enter Depreciation calculation P(ercent) or A(mount) ",
		 NO, NO,  JUSTLEFT, "PA", "", fatr_rec.int_pa_flag},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};
extern	int	TruePosition;
extern	int	EnvScreenOK;

/*======================
| Function Prototypes  |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void ReadMisc (void);
void update (void);
void SrchFatr (char *key_val);
int heading (int scn);
int spec_valid (int field);

int
main (
 int argc,
 char *argv[])
{
	TruePosition	=	TRUE;
	EnvScreenOK		=	FALSE;

	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();			/*  set tty mode to raw		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/
	
	OpenDB ();
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );


	strcpy(local_rec.prev_group,"00000");

	while (prog_exit == 0)
	{
		entry_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		new_item = 0;
		init_vars (1);

		heading (1);
		entry (1);
		if (prog_exit)
		{
			shutdown_prog();
			return (EXIT_SUCCESS);
        }

		if (new_item != 1)
		{
			if (strcmp (fatr_rec.tax_dtype,"C") == 0)
				strcpy (local_rec.tax_d_type,"CP");
			else
				strcpy (local_rec.tax_d_type,"DV");

			if (strcmp (fatr_rec.int_dtype,"C") == 0)
				strcpy (local_rec.int_d_type,"CP");
			else
				strcpy (local_rec.int_d_type,"DV");
		}

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		update ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB (
 void)
{
	abc_dbopen("data");
	open_rec("fatr",fatr_list,FATR_NO_FIELDS,"fatr_id_no");
}

void
CloseDB ( 
 void)
{
	abc_fclose("fatr");
	abc_dbclose("fadb");
}



int
spec_valid (
 int field)
{
	if (LCHECK("groupno"))
	{
		if (SRCH_KEY)
		{
			SrchFatr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (fatr_rec.co_no,comm_rec.tco_no);
		cc = find_rec ("fatr",&fatr_rec,COMPARISON,"u");

		if (cc)
			new_item = 1;
		else
		{
			entry_exit = 1;
			new_item = 0;
                        strncpy(local_rec.AssYears,fatr_rec.ass_life,4);
                        local_rec.AssYears[4] = '\0';
                        strncpy(local_rec.AssMonth,&fatr_rec.ass_life[5],2);
                        local_rec.AssMonth[2] = '\0';
		}
		return (EXIT_SUCCESS);
	}
			
	if (LCHECK("TaxType"))
	{
		if (strcmp (local_rec.tax_d_type,"CP") &&
            strcmp (local_rec.tax_d_type,"DV"))
		{
			print_mess (ML(mlFaMess010));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK("IntType"))
	{
		if (strcmp (local_rec.int_d_type,"CP") &&
            strcmp (local_rec.int_d_type,"DV"))
		{
			print_mess (ML(mlFaMess010));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void 
update (
 void)
{
	clear ();

	/*-------------------------------
	| add or update database record	|
	-------------------------------*/
	sprintf (fatr_rec.tax_dtype,"%-1.1s",local_rec.tax_d_type);
	sprintf (fatr_rec.int_dtype,"%-1.1s",local_rec.int_d_type);
	sprintf (fatr_rec.ass_life, "%-4.4s:%-2.2s", local_rec.AssYears,
												 local_rec.AssMonth);
	if (new_item == 1)
	{
		strcpy (fatr_rec.co_no,comm_rec.tco_no);
		strcpy (fatr_rec.stat_flag,"0");
		cc = abc_add ("fatr",&fatr_rec);
		if (cc)
			file_err (cc, "fatr", "DBADD");
	}
	else 
	{
		cc = abc_update ("fatr",&fatr_rec);
		if (cc)
			file_err (cc, "fatr", "DBUPDATE");
	}

	strcpy (local_rec.prev_group,fatr_rec.group);
}

/*===================================
| Search for inventory master file. |
===================================*/
void
SrchFatr (
 char	*key_val)
{
	work_open ();
	save_rec ("#Asset group", "#Asset Group Description");
	strcpy (fatr_rec.co_no, comm_rec.tco_no);
	sprintf (fatr_rec.group, "%-5.5s", key_val);
	cc = find_rec ("fatr", &fatr_rec, GTEQ, "r");
	while (!cc && !strncmp (fatr_rec.group, key_val, strlen (key_val)) && 
				  !strcmp (fatr_rec.co_no, comm_rec.tco_no))
	{
		cc = save_rec (fatr_rec.group, fatr_rec.group_desc);
		if (cc)
			break;

		cc = find_rec ("fatr", &fatr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		sprintf (fatr_rec.group, "%-5.5s", " ");
		return;
	}
	strcpy (fatr_rec.co_no, comm_rec.tco_no);
	sprintf (fatr_rec.group, "%-5.5s", temp_str);
	cc = find_rec ("fatr", &fatr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "fatr", "DBFIND");
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

		rv_pr (ML(mlFaMess013),24,0,1);

		print_at (0,60,ML(mlFaMess014),local_rec.prev_group);
		move (0,1);
		line (80);

		box (0,3,80,9);

		move (1,6);
		line (79);
		move (1,10);
		line (79);

		us_pr (ML(mlFaMess011), 2,10, 1);
		us_pr (ML(mlFaMess012),40,10, 1);

		move (0,20);
		line (80);

		print_at (21,0,ML(mlStdMess038), comm_rec.tco_no,comm_rec.tco_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	else
		abc_unlock ("fatr");
    return (EXIT_SUCCESS);
}

