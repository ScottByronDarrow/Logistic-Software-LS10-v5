/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : (so_mimaint.c  )                                   |
|  Program Desc  : (Maintain Missing Invoice Control File.      )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, somc, somi,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  somc,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 01/11/89         |
|---------------------------------------------------------------------|
|  Date Modified : (12/06/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (13/04/94)      | Modified  by  : Roel Michels     |
|  Date Modified : (11/09/97)      | Modified  by  : Marnie Organo    |
|  Date Modified : (27/10/1997)    | Modified by : Campbell Mander.   |
|  Date Modified : (20/10/1999)    | Modified by: Virgilio Blones, Jr.|
|                                                                     |
|  Comments      : (12/06/91) Changed prompt for label complete from  |
|   (13/04/94)   : PSL 10673 - Online conversion                      |
|   (11/09/97)   : Updated for Multilingual Conversion.               |
|  (27/10/1997)  : SEL. 9.9.3 Update for 8 character invoice numbers. |
|  (20/10/1999)  : SEL 9.10 Fixed sequence number extraction          |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_mimaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_mimaint/so_mimaint.c,v 5.2 2001/08/09 09:21:36 scott Exp $";

#define MAXWIDTH	135
#include <pslscr.h>
#include <minimenu.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>

#define	SEL_UPDATE	0
#define	SEL_IGNORE	1
#define	SEL_DELETE	2
#define	START_SEQ	1
#define	END_SEQ		2

   	int new_item = 0;
	int	sup_flag = FALSE;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"},
		{"comm_inv_date"}
	};

	int comm_no_fields = 7;

	struct {
		int  termno;
		char tco_no[3];
		char tco_name[41];
		char test_no[3];
		char test_name[41];
		long t_dbt_date;
		long t_inv_date;
	} comm_rec;

	/*=================================
	| Company Master File Base Record |
	=================================*/
	struct dbview comr_list[] ={
		{"comr_co_no"},
		{"comr_co_name"},
	};

	int	comr_no_fields = 2;

	struct	{
		char	co_co_no[3];
		char	co_co_name[41];
	} comr_rec;

	/*=========================================
	| Establishment/Branch Master File Record |
	=========================================*/
	struct dbview esmr_list[] ={
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
		{"esmr_short_name"},
	};

	int	esmr_no_fields = 4;

	struct	{
		char	em_co_no[3];
		char	em_est_no[3];
		char	em_name[41];
		char	em_short_name[16];
	} esmr_rec;

	/*================================================
	| Sales Order Processing Missing Invoice Control |
	================================================*/
	struct dbview somc_list[] ={
		{"somc_co_no"},
		{"somc_br_no"},
		{"somc_active"},
		{"somc_start_seq"},
		{"somc_end_seq"},
		{"somc_stat_flag"},
	};

	int	somc_no_fields = 6;

	struct	{
		char	mc_co_no[3];
		char	mc_br_no[3];
		char	mc_active[2];
		char	mc_st_seq[9];
		char	mc_en_seq[9];
		char	mc_stat_flag[2];
	} asomc_rec, somc_rec;

	/*================================================
	| Sales Order Processing Missing Invoice Control |
	================================================*/
	struct dbview somi_list[] ={
		{"somi_co_no"},
		{"somi_br_no"},
		{"somi_inv_no"},
	};

	int	somi_no_fields = 3;

	struct	{
		char	mi_co_no[3];
		char	mi_br_no[3];
		char	mi_inv_no[9];
	} somi_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	br_no[3];
	char	complete[5];
	char	complete_desc[5];
	char	active[2];
	char	st_seq[9];
	char	en_seq[9];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "co_no", 4, 20, CHARTYPE, 
		"UU", "          ", 
		" ", "", "Company Number.", " ", 
		NE, NO, JUSTRIGHT, "", "", somc_rec.mc_co_no}, 
	{1, LIN, "co_name", 5, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Company Name.", " ", 
		NA, NO, JUSTLEFT, "", "", comr_rec.co_co_name}, 
	{1, LIN, "br_no", 6, 20, CHARTYPE, 
		"NN", "          ", 
		" ", " ", "Branch Number.", " ", 
		NE, NO, JUSTRIGHT, "0", "99", local_rec.br_no}, 
	{1, LIN, "br_name", 7, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Branch Name.", " ", 
		NA, NO, JUSTLEFT, "", "", esmr_rec.em_name}, 
	{1, LIN, "st_seq", 9, 20, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "0", "Start Sequence.", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.st_seq}, 
	{1, LIN, "en_seq", 10, 20, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "0", "End Sequence.", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.en_seq}, 
	{1, LIN, "complete", 11, 20, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Active   <Y/N> ? ", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.complete}, 
	{1, LIN, "complete_desc", 11, 23, CHARTYPE, 
		"AAA", "          ",
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.complete_desc},

	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};


/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int  spec_valid (int);
int  CheckSequence (int, char *, char *);
int  Update (void);
void LoadSomc (void);
void update_menu (void);
void SrchComr (char *);
void SrchEsmr (char *);
void SrchSomi (char *);
int  heading (int scn);
void disp_br (void);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char * argv [])
{
	SETUP_SCR (vars);
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		new_item	= FALSE;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		FLD ("st_seq") = YES;
		FLD ("en_seq") = YES;

		search_ok = 1;

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
		if (restart)
			continue;

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
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	abc_alias ("asomc","somc");
	open_rec ("somc",somc_list,somc_no_fields,"somc_id_no");
	open_rec ("asomc",somc_list,somc_no_fields,"somc_id_no");
	open_rec ("somi",somi_list,somi_no_fields,"somi_id_no");
	open_rec ("comr",comr_list,comr_no_fields,"comr_co_no");
	open_rec ("esmr",esmr_list,esmr_no_fields,"esmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose ("somc");
	abc_fclose ("asomc");
	abc_fclose ("somi");
	abc_fclose ("comr");
	abc_fclose ("esmr");
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	char	low_val[9];
	char	high_val[9];

	/*-------------------------------
	| Validate Company Number.	|
	-------------------------------*/
	if (LCHECK ("co_no"))
	{
		/*-------------------------
		| Search for part number. |
		-------------------------*/
		if (SRCH_KEY)
		{
			SrchComr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (comr_rec.co_co_no,somc_rec.mc_co_no);
		cc = find_rec ("comr", &comr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess130));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("co_name");
		return (EXIT_SUCCESS);
	}

	/*----------------------------------------------------------
	| Validate Establishment number (only if entered) i.e. > 0 |
	----------------------------------------------------------*/
	if (strcmp (FIELD.label,"br_no") == 0) 
	{
		if (dflt_used)
		{
			strcpy (local_rec.br_no,comm_rec.test_no);
			sprintf (esmr_rec.em_name,"%-40.40s",comm_rec.test_name);
			strcpy (somc_rec.mc_br_no,comm_rec.test_no);
			DSP_FLD ("br_no");
			DSP_FLD ("br_name");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
		   SrchEsmr (temp_str);
		   return (EXIT_SUCCESS);
		}

		strcpy (esmr_rec.em_co_no,comr_rec.co_co_no);
		strcpy (esmr_rec.em_est_no,local_rec.br_no);
		cc = find_rec ("esmr", &esmr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (somc_rec.mc_br_no,local_rec.br_no);
		disp_br ();
		DSP_FLD ("br_no");
		DSP_FLD ("br_name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("complete"))
	{
		strcpy (local_rec.complete_desc, (local_rec.complete[0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("complete_desc");
	}

	if (LCHECK ("st_seq"))
	{
		strcpy (local_rec.st_seq,zero_pad (local_rec.st_seq, 8));

		strcpy (low_val,"        ");
		strcpy (high_val, (prog_status == ENTRY) ? "~~~~~~~~" : local_rec.en_seq);

		if (SRCH_KEY)
		{
			SrchSomi (temp_str);
			return (EXIT_SUCCESS);
		}

		if (strcmp (local_rec.st_seq,high_val) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (somc_rec.mc_st_seq,local_rec.st_seq);
		strcpy (somc_rec.mc_en_seq,"        ");
		cc = find_rec ("somc", &somc_rec, GTEQ, "r");
		if (!cc && !strcmp (somc_rec.mc_st_seq,local_rec.st_seq))
		{
			new_item = FALSE;
			strcpy (local_rec.en_seq,somc_rec.mc_en_seq);
			strcpy (local_rec.complete,somc_rec.mc_active);
			strcpy (local_rec.complete_desc, (local_rec.complete[0] == 'Y') ? "Yes" : "No ");
			DSP_FLD ("st_seq");
			DSP_FLD ("en_seq");
			DSP_FLD ("complete");
			DSP_FLD ("complete_desc");
			FLD ("st_seq") = NA;
			FLD ("en_seq") = NA;
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}
		else
		{
			new_item = TRUE;
			cc = CheckSequence (START_SEQ,local_rec.st_seq,local_rec.st_seq);
		}
		return (cc);

	}

	if (strcmp (FIELD.label,"en_seq") == 0) 
	{
		strcpy (local_rec.en_seq,zero_pad (local_rec.en_seq, 8));

		strcpy (low_val,local_rec.st_seq);
		strcpy (high_val,"~~~~~~~~");

		if (SRCH_KEY)
		{
			SrchSomi (temp_str);
			return (EXIT_SUCCESS);
		}

		if (strcmp (local_rec.en_seq,low_val) < 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (new_item)
			cc = CheckSequence (END_SEQ,local_rec.st_seq,local_rec.en_seq);
		return (cc);
	}

	return (EXIT_SUCCESS);
}

int
CheckSequence (
 int seq, 
 char *s_inv_no, 
 char *e_inv_no)
{
	int	invalid = 0;

	abc_selfield ("asomc","somc_id_no");

	if (seq == START_SEQ)
	{
		strcmp (asomc_rec.mc_co_no,comr_rec.co_co_no);
		strcmp (asomc_rec.mc_br_no,local_rec.br_no);
		sprintf (asomc_rec.mc_st_seq,"%-8.8s"," ");
		sprintf (asomc_rec.mc_en_seq,"%-8.8s"," ");
		cc = find_rec ("asomc", &asomc_rec, GTEQ, "r");

		while (!cc && !strcmp (asomc_rec.mc_co_no,comr_rec.co_co_no) && !strcmp (asomc_rec.mc_br_no,local_rec.br_no))
		{
			if ((strncmp (asomc_rec.mc_st_seq,s_inv_no,8) <= 0 && strncmp (asomc_rec.mc_en_seq,e_inv_no,8) >= 0)
			|| (strncmp (asomc_rec.mc_st_seq,s_inv_no,8) <= 0 && strncmp (asomc_rec.mc_en_seq,s_inv_no,8) >= 0)
			|| (strncmp (asomc_rec.mc_st_seq,e_inv_no,8) <= 0 && strncmp (asomc_rec.mc_en_seq,e_inv_no,8) >= 0))
			{
				errmess (ML (mlSoMess018));
				sleep (sleepTime);
				invalid = 1;
				break;
			}

			cc = find_rec ("asomc", &asomc_rec, NEXT, "r");
		}
	}

	else
	{
		abc_selfield ("asomc","somc_id_no2");
		strcmp (asomc_rec.mc_co_no,comr_rec.co_co_no);
		strcmp (asomc_rec.mc_br_no,local_rec.br_no);
		sprintf (asomc_rec.mc_en_seq,"%-8.8s",e_inv_no);
		sprintf (asomc_rec.mc_st_seq,"%-8.8s"," ");
		cc = find_rec ("asomc", &asomc_rec, GTEQ, "r");

		if (!cc && (strcmp (asomc_rec.mc_st_seq,e_inv_no) > 0 || strcmp (asomc_rec.mc_en_seq,e_inv_no) < 0))
			cc = find_rec ("asomc", &asomc_rec, LT, "r");

		while (!cc && !strcmp (asomc_rec.mc_co_no,comr_rec.co_co_no) && !strcmp (asomc_rec.mc_br_no,local_rec.br_no))
		{
			if ((strncmp (asomc_rec.mc_st_seq,s_inv_no,8) >= 0 && strncmp (asomc_rec.mc_en_seq,e_inv_no,8) <= 0)
			|| (strncmp (asomc_rec.mc_st_seq,s_inv_no,8) <= 0 && strncmp (asomc_rec.mc_en_seq,e_inv_no,8) >= 0)
			|| (strncmp (asomc_rec.mc_st_seq,s_inv_no,8) <= 0 && strncmp (asomc_rec.mc_en_seq,s_inv_no,8) >= 0)
			|| (strncmp (asomc_rec.mc_st_seq,e_inv_no,8) <= 0 && strncmp (asomc_rec.mc_en_seq,e_inv_no,8) >= 0))
			{
				errmess (ML (mlSoMess018));
				sleep (sleepTime);
				invalid = 1;
				break;
			}
			cc = find_rec ("asomc", &asomc_rec, LT, "r");
		}
	}
	return (invalid);
}

/*==================================================
| Update or add a record to inventory branch file. |
==================================================*/
int
Update (
 void)
{
	/*-----------------
	| New somc record |
	-----------------*/
	if (new_item == 1)
	{
		clear ();
		/*print_at (0,0,"Adding Missing Invoice record.\n\r");*/
		print_at (0,0,ML (mlSoMess019));

		LoadSomc ();
		strcpy (somc_rec.mc_stat_flag,"0");
		cc = abc_add ("somc",&somc_rec);
		if (cc) 
			sys_err ("Error in somc During (DBADD)",cc,PNAME);
	}
	/*-----------------------
	| Existing somc record. |
	-----------------------*/
	else 
		update_menu ();

	abc_unlock ("somc"); 

	return (EXIT_SUCCESS);
}

void
LoadSomc (
 void)
{
	strcpy (somc_rec.mc_co_no,comr_rec.co_co_no);
	strcpy (somc_rec.mc_br_no,local_rec.br_no);
	strcpy (somc_rec.mc_active,local_rec.complete);
	strcpy (somc_rec.mc_st_seq,local_rec.st_seq);
	strcpy (somc_rec.mc_en_seq,local_rec.en_seq);
}

MENUTAB upd_menu [] =
	{
		{ " 1. SEL_UPDATE RECORD  ",
		  " Update Invoice Record With Changes Made. " },
		{ " 2. SEL_IGNORE CHANGES ",
		  " Ignore Changes Just Made To Invoice Records." },
		{ " 3. SEL_DELETE RECORD  ",
		  " Delete Invoice Records " },
		{ ENDMENU }
	};

/*===================
| Update mini menu. |
===================*/
void
update_menu  (
 void)
{
	for (;;)
	{
	    mmenu_print (" SEL_UPDATE SELECTION. ", upd_menu, 0);
	    switch (mmenu_select (upd_menu))
	    {
		case SEL_UPDATE :
			LoadSomc ();
			cc = abc_update ("somc",&somc_rec);
			if (cc) 
				sys_err ("Error in somc During (DBSEL_UPDATE)",cc,PNAME);
			return;

		case SEL_IGNORE :
			abc_unlock ("somc");
			return;

		case SEL_DELETE :
			abc_unlock ("somc");
			cc = abc_delete ("somc");
			if (cc)
				sys_err ("Error in somc during (DBSEL_DELETE)",cc,PNAME);
			return;
			break;
	
		default :
			break;
	    }
	}
}

void
SrchComr (
 char *key_val)
{
	work_open ();
	save_rec ("#Co No","#Company Name");
	
	sprintf (comr_rec.co_co_no,"%-2.2s",key_val);
	cc = find_rec ("comr",&comr_rec,GTEQ,"r");
	while (!cc && !strncmp (comr_rec.co_co_no,key_val,strlen (key_val)))
	{
		cc = save_rec (comr_rec.co_co_no,comr_rec.co_co_name);
		if (cc)
			break;

		cc = find_rec ("comr",&comr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	sprintf (comr_rec.co_co_no,"%-2.2s",temp_str);
	cc = find_rec ("comr",&comr_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in comr During (DBFIND)",cc,PNAME);
}

void
SrchEsmr (
 char *key_val)
{
	work_open ();
	save_rec ("#Br No.    ","#Br Name           ");
	strcpy (esmr_rec.em_co_no,comr_rec.co_co_no);
	sprintf (esmr_rec.em_est_no,"%-2.2s",key_val);
	cc = find_rec ("esmr",&esmr_rec,GTEQ,"r");
        while (!cc && !strcmp (esmr_rec.em_co_no,comr_rec.co_co_no) && !strncmp (esmr_rec.em_est_no,key_val,strlen (key_val)))
    	{
		cc = save_rec (esmr_rec.em_est_no,esmr_rec.em_name);
		if (cc)
			break;
		cc = find_rec ("esmr",&esmr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (esmr_rec.em_co_no,comr_rec.co_co_no);
	sprintf (esmr_rec.em_est_no,"%-2.2s",temp_str);
	cc = find_rec ("esmr",&esmr_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in esmr During (DBFIND)",cc,PNAME);
}

void
SrchSomi (
 char *key_val)
{
	work_open ();
	save_rec ("#Start-End Sequence","#Completed");
	strcpy (somc_rec.mc_co_no,comr_rec.co_co_no);
	strcpy (somc_rec.mc_br_no,local_rec.br_no);
	sprintf (somc_rec.mc_st_seq,"%-8.8s",key_val);
	strcpy (somc_rec.mc_en_seq,"      ");
	cc = find_rec ("somc", &somc_rec, GTEQ, "r");
        while (!cc && !strcmp (somc_rec.mc_co_no,comr_rec.co_co_no) && !strcmp (somc_rec.mc_br_no,local_rec.br_no))
    	{
		sprintf (err_str,"%-8.8s-%-8.8s",somc_rec.mc_st_seq,somc_rec.mc_en_seq);
		cc = save_rec (err_str, ((somc_rec.mc_active[0] == 'Y') ? "Yes":"No"));
		if (cc)
			break;
		cc = find_rec ("somc",&somc_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (somc_rec.mc_co_no,comr_rec.co_co_no);
	strcpy (somc_rec.mc_br_no,local_rec.br_no);
	sprintf (somc_rec.mc_st_seq,"%-8.8s",temp_str);
	sprintf (somc_rec.mc_en_seq,"%-8.8s",temp_str + 9);
	cc = find_rec ("somc", &somc_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in somc During (DBFIND)",cc,PNAME);
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
		rv_pr (ML (mlSoMess020),25,0,1);
		move (0,1);
		line (80);

		if (scn == 1)
		{
			box (0,3,80,8);
			move (1,8);
			line (79);
		}

		move (0,19);
		line (80);
		print_at (20,0,ML (mlStdMess038),comm_rec.tco_no,comm_rec.tco_name);
		disp_br ();
		move (0,22);
		line (80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

void
disp_br (
 void)
{
	print_at (21,0,ML (mlStdMess039),local_rec.br_no,clip (esmr_rec.em_name));
}
