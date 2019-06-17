/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_skcm_cp.c,v 5.2 2001/08/09 09:19:57 scott Exp $
|  Program Name  : (sk_skcm_cp.c)  
|  Program Desc  : (Stock Container Master File Copy Function
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow  | Date Written  : Mar 14th 2001    |
|---------------------------------------------------------------------|
| $Log: sk_skcm_cp.c,v $
| Revision 5.2  2001/08/09 09:19:57  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:51  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:17:40  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 1.1  2001/03/14 01:40:59  scott
| New program to copy containers.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_skcm_cp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_skcm_cp/sk_skcm_cp.c,v 5.2 2001/08/09 09:19:57 scott Exp $";

#include <pslscr.h>
#include <ml_tr_mess.h>
#include <ml_std_mess.h>

#define	SCN_HEADER	1
#define	ST_SUCCESS	0
#define	ST_ERROR	1

typedef int BOOL;  

#include	"schema"

struct commRecord	comm_rec;
struct skcmRecord	skcm_rec;
struct skcmRecord	skcm2_rec;

char 	*data 	= "data",
		*skcm2 	= "skcm2";

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy[11];
} local_rec;

static	struct	var	vars[]	=	
{
	{SCN_HEADER, LIN, "containerCode", 2, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ", "From Container No : ", "Enter Container No. [SEARCH]", 
		NE, NO, JUSTLEFT, "", "", skcm_rec.container}, 
	{SCN_HEADER, LIN, "desc", 3, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Description       : ", "Enter Container Description", 
		NA, NO, JUSTLEFT, "", "", skcm_rec.desc}, 
	{SCN_HEADER, LIN, "containerCodeTo", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ", "To Container No   : ", "<return> to generation next number based on From Container number (Uses Smart Sequence)", 
		YES, NO, JUSTLEFT, "", "", skcm2_rec.container}, 
	{SCN_HEADER, LIN, "descTo", 6, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Description       : ", "Enter Container Description", 
		NO, NO, JUSTLEFT, "", "", skcm2_rec.desc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 

};

extern	int		TruePosition;

/*=======================
| Function Declarations |
=======================*/
static BOOL IsSpaces 			(char *);
int 	spec_valid 				(int);
int 	heading 				(int);
void 	shutdown_prog 			(void);
void 	CloseDB 				(void);
void 	OpenDB 					(void);
void 	SrchSkcm 				(char *);
void 	SrchSkcm2 				(char *);
void 	Update 					(void);
void	CopyContainer 			(void);

/*===========================
| Main Processing Routine . |
===========================*/
int 
main (
 int argc, 
 char * argv[])
{
	SETUP_SCR 	 (vars);
	init_scr 	 ();
	set_tty 	 ();
	OpenDB		 ();
	set_masks 	 ();
	
	TruePosition	=	TRUE;

    while (prog_exit == 0)
	{
		search_ok 		= 	TRUE;
		entry_exit 		= 	FALSE;
		edit_exit 		= 	FALSE;
		prog_exit 		= 	FALSE;
		restart 		= 	FALSE;
		init_ok 		= 	TRUE;
		init_vars 	 (1);	
		heading 	 (1);
		entry 		 (1);   
		eoi_ok			=   TRUE;
		while (!restart && !prog_exit)
		{
			heading (1);
			scn_display (1);
			edit (1);

			if (!restart)
			{
				Update ();
				restart = TRUE;
			}
			else
				restart = TRUE;
		}
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

static BOOL
IsSpaces (
 char *str)
{ 
	/*--------------------------------------------------------
	| Return TRUE if str contains only white space or nulls. |
	--------------------------------------------------------*/
 	while (*str)
 	{
		if (!isspace (*str))
			return FALSE;
		str++;
	}
	return TRUE;
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

/*======================= 
| Open data base files. |
======================= */
void
OpenDB (
 void)
{
	abc_dbopen (data);

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	abc_alias (skcm2, skcm);

	open_rec (skcm, skcm_list, SKCM_NO_FIELDS, "skcm_id_no");
	open_rec (skcm2,skcm_list, SKCM_NO_FIELDS, "skcm_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (skcm);
	abc_fclose (skcm2);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*----------------------------
	| Validate Container Number. |
	----------------------------*/
	if (LCHECK ("containerCode"))
	{
		if (SRCH_KEY)
        {
			SrchSkcm (temp_str);
			return (ST_SUCCESS);
		}

		if (dflt_used)
		{ 
			if (IsSpaces (skcm_rec.container))
			{
				/*-----------------------------
				| Container Number not found. |
				-----------------------------*/
				print_mess (ML (mlTrMess083));
				sleep (sleepTime);
				clear_mess ();
				return (ST_ERROR);
			}
		}
		strcpy (skcm_rec.co_no, comm_rec.co_no); 
		cc = find_rec (skcm,&skcm_rec, COMPARISON, "r");
        if (cc)
		{
			print_mess (ML (mlTrMess083));
			sleep (sleepTime);
			clear_mess ();
			return (ST_ERROR);
		}
       	DSP_FLD ("containerCode");
       	DSP_FLD ("desc");
		return (ST_SUCCESS);
	}
	/*----------------------------
	| Validate Container Number. |
	----------------------------*/
	if (LCHECK ("containerCodeTo"))
	{
		int		firstTime	=	TRUE;
		if (SRCH_KEY)
        {
			SrchSkcm2 (temp_str);
			return (ST_SUCCESS);
		}

		if (dflt_used)
		{ 
			while (!GenNextSerNo (skcm_rec.container, firstTime, err_str))
			{
				firstTime	=	FALSE;
				strcpy (skcm2_rec.co_no, comm_rec.co_no); 
				sprintf (skcm2_rec.container, "%-15.15s", err_str);
				cc = find_rec (skcm2,&skcm2_rec, COMPARISON, "r");
				if (cc)
					break;
			}
		}
		strcpy (skcm2_rec.co_no, comm_rec.co_no); 
		cc = find_rec (skcm2,&skcm2_rec, COMPARISON, "r");
        if (!cc)
		{
			print_mess (ML (mlTrMess086));
			sleep (sleepTime);
			clear_mess ();
			return (ST_ERROR);
		}
		CopyContainer ();
       	DSP_FLD ("containerCodeTo");
       	DSP_FLD ("descTo");
		return (ST_SUCCESS);
	}
	/*----------------------------
	| Validate Container Number. |
	----------------------------*/
	if (LCHECK ("descTo"))
	{
		if (dflt_used)
			strcpy (skcm2_rec.desc, skcm_rec.desc);

		DSP_FLD ("descTo");

		return (ST_SUCCESS);
	}
	return (ST_SUCCESS);
}

/*==================
| Search for skcm. |
==================*/
void
SrchSkcm (
 char	*key_val)
{
	work_open ();
	save_rec ("#Container","#Description");
	strcpy (skcm_rec.co_no, comm_rec.co_no);
	sprintf (skcm_rec.container,"%-15.15s",key_val);
	cc = find_rec (skcm,&skcm_rec,GTEQ,"r");
	while (!cc && !strcmp (skcm_rec.co_no, comm_rec.co_no) &&
	      		  !strncmp (skcm_rec.container,key_val,strlen (key_val)))
	{
		cc = save_rec (skcm_rec.container,skcm_rec.desc);
		if (cc)
			break;
		cc = find_rec (skcm,&skcm_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (skcm_rec.co_no, comm_rec.co_no);
	sprintf (skcm_rec.container,"%-15.15s", temp_str);
	cc = find_rec (skcm,&skcm_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, (char *)skcm, "DBFIND");
}
/*==================
| Search for skcm. |
==================*/
void
SrchSkcm2 (
 char	*key_val)
{
	work_open ();
	save_rec ("#Container","#Description");
	strcpy (skcm2_rec.co_no, comm_rec.co_no);
	sprintf (skcm2_rec.container,"%-15.15s",key_val);
	cc = find_rec (skcm2,&skcm2_rec,GTEQ,"r");
	while (!cc && !strcmp (skcm2_rec.co_no, comm_rec.co_no) &&
	      		  !strncmp (skcm2_rec.container,key_val,strlen (key_val)))
	{
		cc = save_rec (skcm2_rec.container,skcm2_rec.desc);
		if (cc)
			break;
		cc = find_rec (skcm2,&skcm2_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (skcm2_rec.co_no, comm_rec.co_no);
	sprintf (skcm2_rec.container,"%-15.15s", temp_str);
	cc = find_rec (skcm2,&skcm2_rec,COMPARISON,"r");
	if (cc)
		file_err (cc,skcm2, "DBFIND");
}

void
Update (
 void)
{
	strcpy (skcm2_rec.co_no,comm_rec.co_no);
	cc = abc_add (skcm2,&skcm2_rec);
	if (cc)
		file_err (cc, skcm2, "DBADD");
	
	abc_unlock (skcm2);
}
void
CopyContainer (void)
{
	strcpy (skcm2_rec.stat_code, skcm_rec.stat_code);
	skcm2_rec.min_vol	= skcm_rec.min_vol;
	skcm2_rec.max_vol	= skcm_rec.max_vol;
	skcm2_rec.std_vol	= skcm_rec.std_vol;
	skcm2_rec.min_wgt	= skcm_rec.min_wgt;
	skcm2_rec.max_wgt	= skcm_rec.max_wgt;
	skcm2_rec.std_wgt	= skcm_rec.std_wgt;
	skcm2_rec.min_hei	= skcm_rec.min_hei;
	skcm2_rec.max_hei	= skcm_rec.max_hei;
	skcm2_rec.std_hei	= skcm_rec.std_hei;
	skcm2_rec.min_wid	= skcm_rec.min_wid;
	skcm2_rec.max_wid	= skcm_rec.max_wid;
	skcm2_rec.std_wid	= skcm_rec.std_wid;
	skcm2_rec.min_dth	= skcm_rec.min_dth;
	skcm2_rec.max_dth	= skcm_rec.max_dth;
	skcm2_rec.std_dth	= skcm_rec.std_dth;
	skcm2_rec.per_vol	= skcm_rec.per_vol;
	skcm2_rec.per_wgt	= skcm_rec.per_wgt;
	skcm2_rec.per_cont	= skcm_rec.per_cont;
}

int
heading (
 int	scn)
{
	char string[50] = "";

	if (!restart) 
	{
		/*---------------------------------------------
		| Transport Container Master File Maintenance |
		---------------------------------------------*/
		if (scn != cur_screen)
			scn_set (scn);

		sprintf (string, " %s ", ML (mlTrMess085));

		clear ();
		swide ();
		rv_pr (string, (132 - strlen (string))/2,0,1);

		box (0,1,132,5);
		line_at (4,1,131);
		line_at (20,0,132);
		print_at (21,0, ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
		line_at (22,0,132);
		scn_write (scn);
	}
    return (ST_SUCCESS);
}

