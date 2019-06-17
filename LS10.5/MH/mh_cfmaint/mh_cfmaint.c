/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( mh_cfmaint.c   )                                 |
|  Program Desc  : ( Machine History Spec Type Control File.      )   |
|                  ( MH01                                         )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, mccf,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  mccf,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 09/06/87      
|---------------------------------------------------------------------|
|  Date Modified : (22/01/88)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (29/08/88)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (29/06/89)      | Modified  by  : Rog Gibbison.    |
|  Date Modified : (29/03/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (05/09/97)      | Modified  by  : Ana Marie Tario. |
|                                                                     |
|  Comments      : Removed return from read_comm() & update().        |
|                : (29/06/89) General Tidy Up.                        |
|                :                                                    |
|  (29/03/94)    : INF 10647. Changes for ver9 compile on SCO.        |
|  (05/09/97)    : Incorporated multilingual conversion and DMY4 date.|
|                :                                                    |
| $Log: mh_cfmaint.c,v $
| Revision 5.2  2001/08/09 09:14:06  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:29:52  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:09:21  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:33  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:31  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:01:18  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/11/17 06:40:20  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.9  1999/11/08 08:09:25  scott
| Updated for fix warnings due to usage of -Wall flag.
|
| Revision 1.8  1999/10/11 22:17:47  cam
| Fixed prototypes for heading ()
|
| Revision 1.7  1999/09/29 10:11:22  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/17 09:23:19  scott
| Updated from Ansi
|
| Revision 1.4  1999/06/15 03:03:02  scott
| Update for log and database name.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: mh_cfmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MH/mh_cfmaint/mh_cfmaint.c,v 5.2 2001/08/09 09:14:06 scott Exp $";

#include	<pslscr.h>
#include	<ml_mh_mess.h>
#include	<ml_std_mess.h>

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
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "spec_type", 4, 20, CHARTYPE, 
		"U", "          ", 
		" ", "", "Specification No.", "", 
		YES, NO, JUSTLEFT, "1234567", "", mccf_rec.cf_spec_type}, 
	{1, LIN, "spec_desc", 5, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description", " ", 
		YES, NO, JUSTLEFT, "", "", mccf_rec.cf_spec_desc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*=====================
| function prototypes |
=====================*/
#include	<std_decs.h>
int		spec_valid (int);
void	SrchMccf (char *);
void	OpenDB (void);
void	CloseDB (void);
void	update (void);
int	heading (int);
void	shutdown_prog(void);
/*===========================
| Main Processing Routine . |
===========================*/
int
main(
 int argc,
 char *argv[])
{
	SETUP_SCR (vars);

	init_scr();
	set_tty();
	set_masks();
	init_vars(1);

	OpenDB();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading(1);
		entry(1);
		if (prog_exit)
		{
            shutdown_prog();
			return (EXIT_FAILURE);
		}

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading(1);
		scn_display(1);
		edit(1);

		if (restart)
			continue;

		update();
	}	/* end of input control loop	*/
	shutdown_prog();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog(
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB(
 void)
{
	abc_dbopen("data");

	open_rec("mccf",mccf_list,mccf_no_fields,"mccf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void 
CloseDB(
 void)
{
	abc_fclose("mccf");
	abc_dbclose("data");
}

int
spec_valid(
 int field)
{
	/*---------------------
	| Validate spec  type |
	---------------------*/
	if (LCHECK ("spec_type"))
	{
		if (SRCH_KEY)
		{
		   SrchMccf(temp_str);
		   return(0);
		}
		strcpy(mccf_rec.cf_co_no,comm_rec.tco_no);
		cc = find_rec("mccf",&mccf_rec,COMPARISON,"u");
		if (cc)
			new_item = TRUE;
		else
		{
			new_item = FALSE;
			entry_exit = TRUE;
		}
		return(0);
	}
	return(0);             
}
void
update(
 void)
{
	strcpy(mccf_rec.cf_co_no,comm_rec.tco_no);
	if (new_item)
	{
		cc = abc_add("mccf",&mccf_rec);
		if (cc)
		   file_err (cc, "mccf", "DBADD");
	}
	else
	{
		cc = abc_update("mccf",&mccf_rec);
		if (cc)
		   file_err (cc, "mccf", "DBUPDATE");
	}
}

/*============================
| Search for mccf_spec_type |
============================*/
void
SrchMccf(
 char *key_val)
{
	work_open();
	save_rec("# ","#Description");
	strcpy(mccf_rec.cf_co_no,comm_rec.tco_no);
	sprintf(mccf_rec.cf_spec_type,"%-1.1s",key_val);
	cc = find_rec("mccf",&mccf_rec,GTEQ,"r");
	while (!cc && !strcmp(mccf_rec.cf_co_no,comm_rec.tco_no) && !strncmp(mccf_rec.cf_spec_type,key_val,strlen(key_val)))
	{                        
		cc = save_rec(mccf_rec.cf_spec_type,mccf_rec.cf_spec_desc); 
		if (cc)
			break;
		cc = find_rec("mccf",&mccf_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(mccf_rec.cf_co_no,comm_rec.tco_no);
	sprintf(mccf_rec.cf_spec_type,"%-1.1s",temp_str);
	cc = find_rec("mccf",&mccf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "mccf", "DBFIND");
}

int
heading(
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set(scn);

	clear();
	rv_pr(ML(mlMhMess019),16,0,1);
	move(0,1);
	line(80);

	box(0,3,80,2);

	move(0,20);
	line(80);

	print_at(21,0,ML(mlStdMess038), comm_rec.tco_no,comm_rec.tco_name);
	move(0,22);
	line(80);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write(scn);

	return (EXIT_SUCCESS);
}
