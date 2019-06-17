/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_jtmaint.c,v 5.6 2002/07/24 08:38:42 scott Exp $
|  Program Name  : (cm_jtmaint.c)
|  Program Desc  : (Contract Management Job Type Maintenance)
|---------------------------------------------------------------------|
|  Date Written  : (23/02/93)      | Author       : Campbell Mander.  |
|---------------------------------------------------------------------|
| $Log: cm_jtmaint.c,v $
| Revision 5.6  2002/07/24 08:38:42  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.5  2002/07/09 02:53:31  scott
| .
|
| Revision 5.4  2002/07/03 04:21:40  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.3  2002/01/10 07:09:22  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
| Revision 5.2  2001/08/09 08:57:33  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 22:56:21  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_jtmaint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_jtmaint/cm_jtmaint.c,v 5.6 2002/07/24 08:38:42 scott Exp $";

#define	WIP		0
#define	LAB		1
#define	O_H		2
#define	SAL		3
#define	COG		4
#define	VAR		5
#define	_INT	6

#define MAXLINES	100
#define TABLINES	3
#include	<pslscr.h>	
#include	<GlUtils.h>	
#include	<ml_std_mess.h>	
#include	<ml_cm_mess.h>	

#include	"schema"

struct commRecord	comm_rec;
struct cmjtRecord	cmjt_rec;
struct cmjdRecord	cmjd_rec;
struct cmcmRecord	cmcm_rec;

	char	*data	= 	"data",
			*cmcm2	=	"cmcm2";

   	int  	newCode = 0;

struct storeRec {
	long	valHhcmHash;
} store [MAXLINES];

/*============================ 
| Local & Screen Structures. |
============================*/ 
struct
{
	char	dummy [11];
	char	accDesc [7][41];
	char	cost_head [5];
	char	cost_desc [41];
	char	dtl_lvl [2];
	long	hhcm_hash;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "code",	 	3, 19, CHARTYPE,
		"UUUU", "          ",
		" ", "", "Job Type          ", " ",
		 NE, NO,  JUSTLEFT, "", "", cmjt_rec.job_type},
	{1, LIN, "desc",	 	4, 19, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Description       ", " ",
		NO, NO,  JUSTLEFT, "", "", cmjt_rec.desc},

	{1, LIN, "gl_acc0",	6, 19, CHARTYPE,
		GlMask, "          ",
		"0", "", "Work In Progress  ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmjt_rec.wip_glacc},
	{1, LIN, "wip_desc",	6, 36, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.accDesc [WIP]},
	{1, LIN, "gl_acc1",	7, 19, CHARTYPE,
		GlMask, "          ",
		"0", "", "Labour Recovery   ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmjt_rec.lab_glacc},
	{1, LIN, "lab_desc",	7, 36, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.accDesc [LAB]},
	{1, LIN, "gl_acc2",	8, 19, CHARTYPE,
		GlMask, "          ",
		"0", "", "Overhead Recovery ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmjt_rec.o_h_glacc},
	{1, LIN, "o_h_desc",	8, 36, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.accDesc [O_H]},
	{1, LIN, "gl_acc3",	9, 19, CHARTYPE,
		GlMask, "          ",
		"0", "", "Sales             ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmjt_rec.sal_glacc},
	{1, LIN, "sal_desc",	9, 36, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.accDesc [SAL]},
	{1, LIN, "gl_acc4",	10, 19, CHARTYPE,
		GlMask, "          ",
		"0", "", "Cost Of Goods     ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmjt_rec.cog_glacc},
	{1, LIN, "cog_desc",	10, 36, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.accDesc [COG]},
	{1, LIN, "gl_acc5",	11, 19, CHARTYPE,
		GlMask, "          ",
		"0", "", "Variance          ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmjt_rec.var_glacc},
	{1, LIN, "var_desc",	11, 36, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.accDesc [VAR]},
	{1, LIN, "gl_acc6",	12, 19, CHARTYPE,
		GlMask, "          ",
		"0", "", "Internal          ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmjt_rec.int_glacc},
	{1, LIN, "int_desc",	12, 36, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.accDesc [_INT]},
	{2, TAB, "cost_head",	MAXLINES, 3, CHARTYPE,
		"UUUU", "          ",
		" ", " ", " Cost Head ", " ",
		 YES, NO, JUSTLEFT, "", "", local_rec.cost_head},
	{2, TAB, "cost_desc",	0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " Cost Head Description         ", " ",
		 NA, NO, JUSTLEFT, "", "", local_rec.cost_desc},
	{2, TAB, "dtl_lvl",	0, 2, CHARTYPE,
		"U", "          ",
		" ", cmcm_rec.dtl_lvl, "Level", " Enter A - Consolidate All, L - Consolidate Like, N - No Consolidation ",
		 YES, NO, JUSTLEFT, "ALN", "", local_rec.dtl_lvl},
	{2, TAB, "hhcm_hash",	0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.hhcm_hash},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		 	(void);
int		spec_valid		(int);
int		GetAcc			(char *, char *);
void	LoadCostHeads	(void);
int		Update			(void);
void	SrchCmjt		(char *);
void	SrchCmcm		(char *);
int		heading			(int);

/*
 * Main Processing Routine.
 */
int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR (vars);


	tab_row = 14;
	tab_col = 9;

	init_scr ();
	set_tty ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars (1);
	init_vars (2);

	OpenDB ();

	GL_SetAccWidth (comm_rec.co_no, FALSE);
	
	while (prog_exit == 0)
	{
   		entry_exit	= FALSE;
   		edit_exit 	= FALSE;
   		prog_exit 	= FALSE;
   		search_ok 	= TRUE;
   		restart 	= FALSE;
   		newCode = FALSE;
		lcount [2] = 0;
		init_vars (1);
	
		heading (1);
		scn_display (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		if (newCode)
		{
			heading (2);
			scn_display (2);
			entry (2);
			if (restart)
				continue;
		}

		edit_all ();

		if (restart)
			continue;

		Update ();
    }
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
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

	abc_alias (cmcm2, cmcm);

	open_rec (cmcm,  cmcm_list, CMCM_NO_FIELDS, "cmcm_id_no");
	open_rec (cmcm2, cmcm_list, CMCM_NO_FIELDS, "cmcm_hhcm_hash");
	open_rec (cmjt,  cmjt_list, CMJT_NO_FIELDS, "cmjt_id_no");
	open_rec (cmjd,  cmjd_list, CMJD_NO_FIELDS, "cmjd_id_no");

	OpenGlmr ();
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (cmcm);
	abc_fclose (cmcm2);
	abc_fclose (cmjt);
	abc_fclose (cmjd);
	GL_Close ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	int	i;
	int	chk_lines;

	/*-------------------
	| Valdate Tax Code. |
	-------------------*/
	if (LCHECK ("code"))
	{
		if (SRCH_KEY)
		{
			SrchCmjt (temp_str);
			return (EXIT_SUCCESS);
		}

		newCode = FALSE;
		strcpy (cmjt_rec.co_no, comm_rec.co_no);
		strcpy (cmjt_rec.br_no, comm_rec.est_no);
		cc = find_rec (cmjt, &cmjt_rec, COMPARISON, "u");
		if (cc)
			newCode = TRUE;
		else
		{
			/*---------------------------------
			| Lookup GL account descriptions. |
			---------------------------------*/
			for (i = WIP; i <= _INT; i++)
			{
				switch (i)
				{
				case	0:
					cc = GetAcc (cmjt_rec.wip_glacc, local_rec.accDesc [i]);
					break;

				case	1:
					cc = GetAcc (cmjt_rec.lab_glacc, local_rec.accDesc [i]);
					break;
			
				case	2:
					cc = GetAcc (cmjt_rec.o_h_glacc, local_rec.accDesc [i]);
					break;
			
				case	3:
					cc = GetAcc (cmjt_rec.sal_glacc, local_rec.accDesc [i]);
					break;
			
				case	4:
					cc = GetAcc (cmjt_rec.cog_glacc, local_rec.accDesc [i]);
					break;
			
				case	5:
					cc = GetAcc (cmjt_rec.var_glacc, local_rec.accDesc [i]);
					break;
			
				case	6:
					cc = GetAcc (cmjt_rec.int_glacc, local_rec.accDesc [i]);
					break;
				}
				if (cc)
					return (EXIT_FAILURE);
			}

			/*--------------------------------
			| Lookup costheads for job type. |
			--------------------------------*/
			LoadCostHeads ();
			scn_display (1);
			
			entry_exit = TRUE;
		}
		return (EXIT_SUCCESS);
	}

	/*------------------------------------------
	| Validate General Ledger Account Numbers. |
	------------------------------------------*/
	if (LNCHECK ("gl_acc", 6))
	{
		i = atoi (FIELD.label + 6);

		if (SRCH_KEY)
			SearchGlmr (comm_rec.co_no, temp_str, "F*P");
		else
		{
			switch (i)
			{
				case	0:
					cc = GetAcc (cmjt_rec.wip_glacc, local_rec.accDesc [i]);
					break;

				case	1:
					cc = GetAcc (cmjt_rec.lab_glacc, local_rec.accDesc [i]);
					break;
			
				case	2:
					cc = GetAcc (cmjt_rec.o_h_glacc, local_rec.accDesc [i]);
					break;
			
				case	3:
					cc = GetAcc (cmjt_rec.sal_glacc, local_rec.accDesc [i]);
					break;
			
				case	4:
					cc = GetAcc (cmjt_rec.cog_glacc, local_rec.accDesc [i]);
					break;
			
				case	5:
					cc = GetAcc (cmjt_rec.var_glacc, local_rec.accDesc [i]);
					break;
			
				case	6:
					cc = GetAcc (cmjt_rec.int_glacc, local_rec.accDesc [i]);
					break;
			}
			if (cc)
				return (EXIT_FAILURE);
		}
		display_field (field + 1);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cost_head"))
	{
		if (SRCH_KEY)
		{
			SrchCmcm (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmcm_rec.co_no, comm_rec.co_no);
		sprintf (cmcm_rec.ch_code, "%-4.4s", local_rec.cost_head);
		cc = find_rec (cmcm, &cmcm_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess055));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*----------------------------
		| Check whether costhead has |
		| already been entered.      |
		----------------------------*/
		chk_lines = (prog_status == ENTRY) ? line_cnt : lcount [2];
		for (i = 0; i < chk_lines; i++)
		{
			if (store [i].valHhcmHash == cmcm_rec.hhcm_hash)
			{
				if (prog_status != ENTRY && i == line_cnt)
					continue;

				print_mess (ML (mlCmMess122));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		sprintf (local_rec.cost_desc, "%-40.40s", cmcm_rec.desc);
		DSP_FLD ("cost_desc");

		strcpy (local_rec.dtl_lvl, cmcm_rec.dtl_lvl);
		local_rec.hhcm_hash = cmcm_rec.hhcm_hash;
		store [line_cnt].valHhcmHash = cmcm_rec.hhcm_hash;

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*===============================================
| Find account number, account description etc. |
===============================================*/
int
GetAcc (
 char *	acc_no,
 char *	acc_desc)
{
	GL_FormAccNo (acc_no, glmrRec.acc_no, 0);
	strcpy (glmrRec.co_no,comm_rec.co_no);
	if ((cc = find_rec (glmr, &glmrRec, COMPARISON, "r")))
	{
		print_mess (ML (mlStdMess024));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (glmrRec.glmr_class [0][0] != 'F' || 
	    glmrRec.glmr_class [2][0] != 'P')
	{
		print_mess (ML (mlStdMess025));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	sprintf (acc_desc, "%-40.40s", glmrRec.desc);
	
	return (EXIT_SUCCESS);
}

/*-------------------------------
| Load cost heads for job type. |
-------------------------------*/
void
LoadCostHeads (
 void)
{
	lcount [2] = 0;
	scn_set (2);
	cmjd_rec.hhjt_hash = cmjt_rec.hhjt_hash;
	cmjd_rec.line_no = 0;
	cc = find_rec (cmjd, &cmjd_rec, GTEQ, "r");
	while (!cc && cmjd_rec.hhjt_hash == cmjt_rec.hhjt_hash)
	{
		cc = find_hash (cmcm2, &cmcm_rec, COMPARISON, "r", cmjd_rec.hhcm_hash);
		if (!cc)
		{
			sprintf (local_rec.cost_head, "%-4.4s", cmcm_rec.ch_code);
			sprintf (local_rec.cost_desc, "%-40.40s", cmcm_rec.desc);
			strcpy (local_rec.dtl_lvl, cmjd_rec.dtl_lvl);
			local_rec.hhcm_hash = cmjd_rec.hhcm_hash;

			store [lcount [2]].valHhcmHash = cmjd_rec.hhcm_hash;
			putval (lcount [2]++);
		}

		cc = find_rec (cmjd, &cmjd_rec, NEXT, "r");
	}

	scn_display (2);
	scn_set (1);
}

/*---------------
| Update files. |
---------------*/
int
Update (
 void)
{
	int	i;

	if (newCode)
	{
		cc = abc_add (cmjt, &cmjt_rec);
		if (cc)
			file_err (cc, cmjt, "DBADD");

		cc = find_rec (cmjt, &cmjt_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, cmjt, "DBFIND1");
	}
	else
	{
		cc = abc_update (cmjt, &cmjt_rec);
		if (cc)
			file_err (cc, cmjt, "DBUPDATE");
	}

	/*--------------------------
	| Delete old cmjd records. |
	--------------------------*/
	cmjd_rec.hhjt_hash = cmjt_rec.hhjt_hash;
	cmjd_rec.line_no = 0;
	cc = find_rec (cmjd, &cmjd_rec, GTEQ, "r");
	while (!cc &&
	       cmjd_rec.hhjt_hash == cmjt_rec.hhjt_hash)
	{
		cc = abc_delete (cmjd);
		if (cc)
			file_err (cc, cmjd, "DBDELETE");

		cmjd_rec.hhjt_hash = cmjt_rec.hhjt_hash;
		cmjd_rec.line_no = 0;
		cc = find_rec (cmjd, &cmjd_rec, GTEQ, "r");
	}
	
	/*-----------------------
	| Add new cmjd records. |
	-----------------------*/
	scn_set (2);
	for (i = 0; i < lcount [2]; i++)
	{
		getval (i);

		cmjd_rec.hhjt_hash = cmjt_rec.hhjt_hash;
		cmjd_rec.line_no = i;
		cmjd_rec.hhcm_hash = local_rec.hhcm_hash;
		strcpy (cmjd_rec.dtl_lvl, local_rec.dtl_lvl);
		cc = abc_add (cmjd, &cmjd_rec);
		if (cc)
			file_err (cc, cmjd, "DBADD");
	}

	return (EXIT_SUCCESS);
}

/*----------------------------------
| Search for Category master file. |
----------------------------------*/
void
SrchCmjt (
 char *	key_val)
{
	_work_open (4,0,40);
	save_rec ("#Type", "#Job Type Description");

	strcpy (cmjt_rec.co_no, comm_rec.co_no);
	sprintf (cmjt_rec.job_type, "%-4.4s", key_val);
	cc = find_rec (cmjt, &cmjt_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmjt_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmjt_rec.job_type, key_val, strlen (key_val)))
	{
		cc = save_rec (cmjt_rec.job_type, cmjt_rec.desc);
		if (cc)
			break;

		cc = find_rec (cmjt, &cmjt_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmjt_rec.co_no, comm_rec.co_no);
	strcpy (cmjt_rec.br_no, comm_rec.est_no);
	sprintf (cmjt_rec.job_type, "%-4.4s", temp_str);
	cc = find_rec (cmjt, &cmjt_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmjt, "DBFIND2");
}

/*-----------------------------------
| Search for Cost Head master file. |
-----------------------------------*/
void
SrchCmcm (
 char *	key_val)
{
	_work_open (4,0,40);
	save_rec ("#Head", "#Cost Head Description");

	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", key_val);
	cc = find_rec (cmcm, &cmcm_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmcm_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmcm_rec.ch_code, key_val, strlen (key_val)))
	{
		cc = save_rec (cmcm_rec.ch_code, cmcm_rec.desc);
		if (cc)
			break;

		cc = find_rec (cmcm, &cmcm_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", temp_str);
	cc = find_rec (cmcm, &cmcm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmcm, "DBFIND");
}

int
heading (
 int scn)
{
	int	s_size = 80;

	if (restart) 
		return (EXIT_SUCCESS);
		
	if (scn != cur_screen)
		scn_set (scn);
	clear ();
	rv_pr (ML (mlCmMess123), (80 - strlen (ML (mlCmMess123))) / 2, 0, 1);

	line_at (1,0, s_size);

	switch (scn)
	{
	case 	1:
		/*-------------------
		| Display screen 1. |
		-------------------*/
		line_at (5,1,s_size -1);
		box (0, 2, 80, 10);
		us_pr (ML (mlCmMess124), (80 - strlen (ML (mlCmMess124))) / 2, 5, 1);
		/*-------------------
		| Display screen 2. |
		-------------------*/
		scn_set (2);
		scn_write (2);
		scn_display (2);
		//box (9, 13, 60, 5);
		us_pr (ML (mlCmMess125), (80 - strlen (ML (mlCmMess125))) / 2, 13, 1);

		scn_set (1);
		break;

	case 	2:
		/*-------------------
		| Display screen 1. |
		-------------------*/
		scn_set (1);
		scn_write (1);

		box (0, 2, 80, 10);
		line_at (5,1,s_size - 1);
		us_pr (ML (mlCmMess124), (80 - strlen (ML (mlCmMess124))) / 2, 5, 1);

		scn_display (1);

		/*-------------------
		| Display screen 2. |
		-------------------*/
		scn_set (2);
	//	box (9, 13, 60, 5);
		us_pr (ML (mlCmMess125), (80 - strlen (ML (mlCmMess125))) / 2, 13, 1);

		break;
	}

	line_at (20,0, s_size);
	strcpy (err_str, ML (mlStdMess038));
	print_at (21,0,err_str, comm_rec.co_no, comm_rec.co_name);
	strcpy (err_str, ML (mlStdMess039));
	print_at (22,0,err_str, comm_rec.est_no, comm_rec.est_name);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;

	scn_write (scn);
	return (EXIT_SUCCESS);
}

