/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_st_del.c,v 5.2 2001/08/09 09:20:02 scott Exp $
|  Program Name  : (sk_st_del.c  )                                    |
|  Program Desc  : (Stock take selection delete                 )     |
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow. | Date Written  : 30/08/1996       |
|---------------------------------------------------------------------|
| $Log: sk_st_del.c,v $
| Revision 5.2  2001/08/09 09:20:02  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:55  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:17:47  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:39:03  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2000/12/22 03:46:08  ramon
| Updated to correct the errors when compiled in LS10-GUI.
|
| Revision 3.1  2000/12/12 05:44:47  scott
| Updated to add app.schema and to clean up code.
| Updated to fix problem when two stock takes are operating the programs deletes
| data input for all codes not just one being deleted.
|
| Revision 3.0  2000/10/10 12:21:18  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:59  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  2000/04/25 08:39:18  ambhet
| SC#2828 - Modified to update the inlo_stake qty to zero if its being deleted.
|
| Revision 1.9  1999/11/11 06:00:08  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.8  1999/11/03 07:32:34  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.7  1999/10/13 02:42:16  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.6  1999/10/08 05:32:55  scott
| First Pass checkin by Scott.
|
| Revision 1.5  1999/06/20 05:20:45  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_st_del.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_st_del/sk_st_del.c,v 5.2 2001/08/09 09:20:02 scott Exp $";

#include	<pslscr.h>
#include	<ml_sk_mess.h>
#include	<ml_std_mess.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inscRecord	insc_rec;
struct sttsRecord	stts_rec;
struct sttfRecord	sttf_rec;
struct inloRecord	inlo_rec;
struct inccRecord	incc_rec;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	mode_desc[41];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "stk_mode",	4, 25, CHARTYPE,
		"U", "          ",
		" ", "", "Stock Take Selection", "Input Stock Take Selection Code A-Z.",
		YES, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", insc_rec.stake_code},
	{1, LIN, "stk_desc",	5, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description  ", " ",
		 NA, NO,  JUSTLEFT, "", "", insc_rec.description},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*======================= 
| Function Declarations |
=======================*/
int  	spec_valid 		 (int);
void 	shutdown_prog 	 (void);
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
void 	ReadMisc 		 (void);
void 	ProcessFile 	 (void);
void 	Del_stts 		 (long);
void 	Del_sttf 		 (long);
void 	SrchInsc 		 (char *);
int  	heading 		 (int);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv[])
{
	SETUP_SCR (vars);
	
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	/*=====================
	| Reset control flags |
	=====================*/
   	entry_exit 	= FALSE;
   	prog_exit 	= FALSE;
   	restart 	= FALSE;
   	search_ok 	= TRUE;

	init_vars (1);
	heading (1);
	entry (1);
    if (restart || prog_exit) {
		shutdown_prog ();
        return (EXIT_SUCCESS);
    }

	heading (1);
	scn_display (1);
	edit (1);
	prog_exit = 1;
    if (restart) {
		shutdown_prog ();
        return (EXIT_SUCCESS);
    }

	ProcessFile ();
	
    shutdown_prog ();
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	/*---------------------------
	| Validate Stock Take code. |
	---------------------------*/
	if (LCHECK ("stk_mode"))
	{
		if (SRCH_KEY)
		{
			SrchInsc (temp_str);
			return (EXIT_SUCCESS);
		}

		insc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		cc = find_rec (insc,&insc_rec,COMPARISON,"r");
		if (cc)
		{
			sprintf (err_str,ML (mlSkMess047),insc_rec.stake_code);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("stk_desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	ReadMisc ();

	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (insc, insc_list, INSC_NO_FIELDS, "insc_id_no");
	open_rec (sttf, sttf_list, STTF_NO_FIELDS, "sttf_id_no");
	open_rec (stts, stts_list, STTS_NO_FIELDS, "stts_id_no");
	open_rec (inlo, inlo_list, INLO_NO_FIELDS, "inlo_mst_lot");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (incc);
	abc_fclose (insc);
	abc_fclose (sttf);
	abc_fclose (stts);
	abc_fclose (inlo);
	abc_fclose ("data");
}

/*===================================== 
| Get info from commom database file .|
=====================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, (char *)ccmr, "DBFIND");

	abc_fclose (ccmr);
}

void
ProcessFile (
 void)
{
	dsp_screen ("Deleting Stock Take", comm_rec.co_no, comm_rec.co_name);
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = 0L;
	cc = find_rec (incc, &incc_rec, GTEQ, "u");
	while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		dsp_process ("Item", incc_rec.sort + 12);
		if (incc_rec.stat_flag[0] != insc_rec.stake_code[0])
		{
			abc_unlock (incc);
			cc = find_rec (incc, &incc_rec, NEXT, "u");
			continue;
		}
		Del_stts (incc_rec.hhwh_hash);
		Del_sttf (incc_rec.hhwh_hash);

		inlo_rec.hhwh_hash = incc_rec.hhwh_hash;
		inlo_rec.hhum_hash = 0L;
		strcpy (inlo_rec.lot_no," ");
		cc = find_rec ("inlo",&inlo_rec,GTEQ,"r");
		while (!cc && inlo_rec.hhwh_hash == incc_rec.hhwh_hash)
		{
			inlo_rec.stake = 0.00;
			cc = abc_update ("inlo", &inlo_rec);
			if (cc)
				file_err (cc, (char *)inlo, "DBUPDATE");

			cc = find_rec ("inlo", &inlo_rec, NEXT,"r");
		}
		incc_rec.stake = 0.00;
		strcpy (incc_rec.stat_flag, "0");
		cc = abc_update (incc,&incc_rec);
		if (cc) 
			file_err (cc, (char *)incc, "DBUPDATE");

		cc = find_rec (incc, &incc_rec, NEXT, "u");
	}
	abc_unlock (incc);
	insc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	cc = find_rec (insc,&insc_rec,COMPARISON,"u");
	if (!cc)
	{
		cc = abc_delete (insc);
		if (cc)
			file_err (cc, (char *)insc, "DBDELETE");
	}
}

void
Del_stts (
	 long hhwhHash)
{

	stts_rec.hhwh_hash = hhwhHash;
	sprintf (stts_rec.serial_no,"%25.25s"," ");
	cc = find_rec (stts,&stts_rec,GTEQ,"r");

	while (!cc && stts_rec.hhwh_hash == hhwhHash)
	{
		cc = abc_delete (stts);
		if (cc)
			file_err (cc, (char *)stts, "DBDELETE");

		stts_rec.hhwh_hash = hhwhHash;
		sprintf (stts_rec.serial_no,"%25.25s"," ");
		cc = find_rec (stts, &stts_rec, GTEQ,"r");
	}
	abc_unlock (stts);
}

void
Del_sttf (
	long hhwhHash)
{
	sttf_rec.hhwh_hash = hhwhHash;
	sprintf (sttf_rec.location,"%10.10s"," ");
	cc = find_rec (sttf,&sttf_rec,GTEQ,"r");

	while (!cc && sttf_rec.hhwh_hash == hhwhHash)
	{

		cc = abc_delete (sttf);
		if (cc)
			file_err (cc, (char *)sttf, "DBDELETE");

		sttf_rec.hhwh_hash = hhwhHash;
		sprintf (sttf_rec.location,"%10.10s"," ");
		cc = find_rec (sttf, &sttf_rec, GTEQ,"r");
	}
	abc_unlock (sttf);
}

/*=============================================
| Search Routine for Stock take Control File. |
=============================================*/
void
SrchInsc (
 char *key_val)
{
	work_open ();
	save_rec ("# ","#Stock Take Selection Desc");
	insc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (insc_rec.stake_code, "%-1.1s", key_val);
	cc = find_rec (insc,&insc_rec,GTEQ,"r");
	while (!cc && insc_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		cc = save_rec (insc_rec.stake_code,insc_rec.description);
		if (cc)
			break;

		cc = find_rec (insc,&insc_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	insc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	strcpy (insc_rec.stake_code,temp_str);
	cc = find_rec (insc,&insc_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, (char *)insc, "DBFIND");
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
		
		rv_pr (ML (mlSkMess498),31,0,1);
		move (0,1);
		line (80);

		box (0,3,80,2);
		move (0,19);
		line (80);
		strcpy (err_str,ML (mlStdMess038));
		print_at (20,0,err_str,comm_rec.co_no,comm_rec.co_name);
		strcpy (err_str,ML (mlStdMess039));
		print_at (21,0,err_str,comm_rec.est_no,comm_rec.est_name);
		strcpy (err_str,ML (mlStdMess099));
		print_at (22,0,err_str,comm_rec.cc_no,comm_rec.cc_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
