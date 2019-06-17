/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( mh_sdmaint.c   )                                 |
|  Program Desc  : ( Machine History Spec Type Control File.      )   |
|                  ( MH01                                         )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, mhsd,     ,     ,     ,     ,     ,         |
|  Database      : (mach)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  mhsd,     ,     ,     ,     ,     ,     ,         |
|  Database      : (mach)                                             |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 17/09/87      
|---------------------------------------------------------------------|
|  Date Modified : (29/08/88)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (30/06/89)      | Modified  by  : Rog Gibbison.    |
|  Date Modified : (29/03/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (05/09/97)      | Modified  by  : Ana Marie Tario. |
|  Date Modified : (16/08/99)      | Modified  by  : Mars dela Cruz.  |
|                                                                     |
|  Comments      : Removed return from read_comm() & update().        |
|                : (30/06/89) - General Tidy Up & Debug               |
|                :                                                    |
|  (29/03/94)    : INF 10647. Changes for ver9 compile on SCO.        |
|  (05/09/97)    : Incorporated multilingual conversion and DMY4 date.|
|  (16/08/99)    : Modified for ANSI compliance.                      |
|                :                                                    |
|                                                                     |
| $Log: mh_sdmaint.c,v $
| Revision 5.2  2001/08/09 09:14:13  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:29:57  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:09:28  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:38  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:38  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:01:22  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.7  1999/11/08 08:09:29  scott
| Updated for fix warnings due to usage of -Wall flag.
|
| Revision 1.6  1999/09/29 10:11:23  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/17 09:23:25  scott
| Updated from Ansi
|
| Revision 1.4  1999/06/15 03:03:06  scott
| Update for log and database name.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: mh_sdmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MH/mh_sdmaint/mh_sdmaint.c,v 5.2 2001/08/09 09:14:13 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_mh_mess.h>

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"}
	};

	int comm_no_fields = 3;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
	} comm_rec;

	/*==================================
	| Spec_type and Code Control File. |
	==================================*/
	struct dbview mhsd_list[] ={
		{"mhsd_co_no"},
		{"mhsd_spec_type"},
		{"mhsd_code"},
		{"mhsd_desc"}
	};

	int mhsd_no_fields = 4;

	struct {
		char	sd_co_no[3];
		char	sd_spec_type[2];
		char	sd_code[5];
		char	sd_desc[41];
	} mhsd_rec;

	/*===============================
	| Machine History Control File. |
	===============================*/
	struct dbview mccf_list[] ={
		{"mccf_co_no"},
		{"mccf_spec_type"},
		{"mccf_spec_desc"}
	};

	int mccf_no_fields = 3;

	struct {
		char	cf_co_no[3];
		char	cf_spec_type[2];
		char	cf_spec_desc[16];
	} mccf_rec;

   	int  	new_item = 0;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	spec_type[2];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "spec_type", 4, 20, CHARTYPE, 
		"U", "          ", 
		" ", "", "Specification Type.", "", 
		YES, NO, JUSTLEFT, "1234567", "", local_rec.spec_type}, 
	{1, LIN, "spec_desc", 4, 40, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", " ", 
		NA, NO, JUSTLEFT, "", "", mccf_rec.cf_spec_desc}, 
	{1, LIN, "code", 5, 20, CHARTYPE, 
		"UUUU", "          ", 
		" ", "", "Code.", " ", 
		YES, NO, JUSTLEFT, "", "", mhsd_rec.sd_code}, 
	{1, LIN, "code_desc", 6, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Code Description.", " ", 
		YES, NO, JUSTLEFT, "", "", mhsd_rec.sd_desc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*=====================
| Function Prototypes |
======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void update (void);
void show_mccf (char *key_val);
void show_mhsd (char *key_val);
int heading (int scn);
int spec_valid (int field);


/*===========================
| Main Processing Routine . |
===========================*/
int 
main (
 int	argc, 
 char	*argv[])
{
	SETUP_SCR (vars);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty	();
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
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading(1);
		entry(1);
		if (prog_exit)
		{
			shutdown_prog ();
			return (EXIT_SUCCESS);
        }
		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		update ();
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
	abc_dbopen("data");

	open_rec ("mhsd",mhsd_list,mhsd_no_fields,"mhsd_id_no");
	open_rec ("mccf",mccf_list,mccf_no_fields,"mccf_id_no");
	read_comm ( comm_list, comm_no_fields, (char *) &comm_rec );
}

/*=========================
| Close data base files . |
=========================*/
void 
CloseDB (
 void)
{
	abc_fclose ("mhsd");
	abc_fclose ("mccf");
	abc_dbclose ("data");
}

int 
spec_valid (
 int field)
{
	/*---------------------
	| Validate spec  type |
	---------------------*/
	if (LCHECK ("spec_type"))
	{
		if (SRCH_KEY)
		{
		   show_mccf (temp_str);
		   return (EXIT_SUCCESS);
		}
		strcpy (mccf_rec.cf_co_no,comm_rec.tco_no);
		strcpy (mccf_rec.cf_spec_type,local_rec.spec_type);
		cc = find_rec ("mccf",&mccf_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML(mlMhMess021));
			return (EXIT_FAILURE);
		}
		DSP_FLD ("spec_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("code"))
	{
		if (SRCH_KEY)
		{
		   show_mhsd (temp_str);
		   return (EXIT_SUCCESS);
		}
		strcpy (mhsd_rec.sd_co_no,comm_rec.tco_no);
		strcpy (mhsd_rec.sd_spec_type,local_rec.spec_type);
		cc = find_rec ("mhsd",&mhsd_rec,COMPARISON,"u");
		if (cc)
			new_item = TRUE;
		else
		{
			new_item = FALSE;
			entry_exit = TRUE;
			DSP_FLD ("code_desc");
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);             
}

void 
update (
 void)
{
	strcpy (mhsd_rec.sd_co_no,comm_rec.tco_no);
	if (new_item)
	{
		cc = abc_add ("mhsd",&mhsd_rec);
		if (cc)
		   sys_err ("Error in mhsd During (DBADD)",cc,PNAME);
	}
	else
	{
		cc = abc_update ("mhsd",&mhsd_rec);
		if (cc)
		   sys_err ("Error in mhsd During (DBUPDATE)",cc,PNAME);
		abc_unlock ("mhsd");
	}
}

/*============================
| Search for mccf_spec_type |
============================*/
void 
show_mccf (
 char *key_val)
{
	work_open ();
	save_rec ("# ","#Description");
	strcpy (mccf_rec.cf_co_no,comm_rec.tco_no);
	sprintf (mccf_rec.cf_spec_type,"%-1.1s",key_val);
	cc = find_rec ("mccf",&mccf_rec,GTEQ,"r");
	while (!cc && !strcmp (mccf_rec.cf_co_no,comm_rec.tco_no) && !strncmp(mccf_rec.cf_spec_type,key_val,strlen(key_val)))
	{                        
		cc = save_rec (mccf_rec.cf_spec_type,mccf_rec.cf_spec_desc); 
		if (cc)
		        break;
		cc = find_rec ("mccf",&mccf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;
	strcpy (mccf_rec.cf_co_no,comm_rec.tco_no);
	sprintf (mccf_rec.cf_spec_type,"%-1.1s",temp_str);
	cc = find_rec ("mccf",&mccf_rec,COMPARISON,"r");
	if (cc)
 	        sys_err ("Error in mccf During (DBFIND)",cc,PNAME);
}

/*========================
| Search for mhsd_code   |
========================*/
void 
show_mhsd (
 char *key_val)
{
	work_open ();
	save_rec ("#Code","#Description");
	strcpy (mhsd_rec.sd_co_no,comm_rec.tco_no);
	strcpy (mhsd_rec.sd_spec_type,local_rec.spec_type);
	sprintf (mhsd_rec.sd_code,"%-4.4s",key_val);
	cc = find_rec ("mhsd",&mhsd_rec,GTEQ,"r");
	while (!cc && !strcmp (mhsd_rec.sd_co_no,comm_rec.tco_no) && mhsd_rec.sd_spec_type[0] == local_rec.spec_type[0] && !strncmp(mhsd_rec.sd_code,key_val,strlen(key_val)))
	{                        
		cc = save_rec (mhsd_rec.sd_code,mhsd_rec.sd_desc);                       
		if (cc)
		        break;
		cc = find_rec ("mhsd",&mhsd_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;
	strcpy (mhsd_rec.sd_co_no,comm_rec.tco_no);
	strcpy (mhsd_rec.sd_spec_type,local_rec.spec_type);
	sprintf (mhsd_rec.sd_code,"%-4.4s",temp_str);
	cc = find_rec ("mhsd",&mhsd_rec,COMPARISON,"r");
	if (cc)
 	        sys_err ("Error in mhsd During (DBFIND)",cc,PNAME);
}

int 
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear ();
		rv_pr (ML (mlMhMess020),20,0,1);
		move (0,1);
		line (80);

		box (0,3,80,3);

		move (0,20);
		line (80);
		print_at (21,0,ML(mlStdMess038),comm_rec.tco_no,comm_rec.tco_name);
		move (0,22);
		line (80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
