/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_nst_frz.c,v 5.2 2001/08/09 09:19:23 scott Exp $
|  Program Name  : (sk_nst_frz.c)                                    |
|  Program Desc  : (Stock Take Freeze Update.               )      |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 22/03/89         |
|---------------------------------------------------------------------|
| $Log: sk_nst_frz.c,v $
| Revision 5.2  2001/08/09 09:19:23  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:27  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:50  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:38:11  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2000/12/21 11:11:59  ramon
| Updated to correct the errors when compiled in LS10-GUI.
|
| Revision 3.1  2000/11/20 07:40:19  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:20:46  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:28  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  1999/11/03 07:32:16  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.10  1999/10/13 02:42:07  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.9  1999/10/08 05:32:42  scott
| First Pass checkin by Scott.
|
| Revision 1.8  1999/06/20 05:20:24  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_nst_frz.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_nst_frz/sk_nst_frz.c,v 5.2 2001/08/09 09:19:23 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>
#include	<twodec.h>

#define	SERIAL		 (inmr_rec.serial_item [0] == 'Y')

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	char	byWhat [2];
	char	stockTakeSelection [2];
	char	lower [17];
	char	upper [17];

	int		serialUpdate = 0;

	char	*inval_cls;
 	char 	*result;

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inwuRecord	inwu_rec;
struct sttsRecord	stts_rec;
struct insfRecord	insf_rec;
struct inloRecord	inlo_rec;
struct inloRecord	inlo2_rec;

char	*data	=	"data",
		*inlo2	=	"inlo2";

int		envVarMultLoc = 0;
/*
#include	<LocHeader.h>
*/
/*=======================
| Function Declarations |
=======================*/ 
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ReadMisc 			(void);
void 	ProcessFile 		(void);
void 	ProcessByGroup 		(void);
void 	ProcessByItem 		(void);
void 	ProcessAbcLocaton 	(void);
int  	ValidInmr 			(void);
void 	ProcessIncc 		(void);
void 	ProcessInwu 		(long);
void 	ProcessInlo 		(long);
int  	CheckLocn 			(void);
void 	AddStts 			(long);

/*==========================
| Main Processing Routine. |
==========================*/ 
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	if (argc != 6)
	{
		print_at (0,0,mlSkMess108,argv [0]);
		return (EXIT_FAILURE);
	}

	if (strncmp (argv [0],"sk_nst_sfrz",11) == 0)
		serialUpdate = 1;

	sptr = chk_env ("SK_IVAL_CLASS");
	if (sptr)
		inval_cls = strdup (sptr);
	else
		inval_cls = "ZKPN";

	sptr = chk_env ("MULT_LOC");
	envVarMultLoc = (sptr == (char *)0) ? 0 : atoi (sptr);

	upshift (inval_cls); 

	sprintf (byWhat,"%-1.1s",argv [4]);
	sprintf (stockTakeSelection,"%-1.1s",argv [5]);
	switch (byWhat [0])
	{
	case	'G':
		sprintf (lower,"%-12.12s",argv [2]);
		sprintf (upper,"%-12.12s",argv [3]);
		break;

	case	'I':
		sprintf (lower,"%-16.16s",argv [2]);
		sprintf (upper,"%-16.16s",argv [3]);
		break;

	case	'A':
		sprintf (lower,"%-1.1s",argv [2]);
		sprintf (upper,"%-1.1s",argv [3]);
		break;

	case	'L':
		sprintf (lower,"%-10.10s",argv [2]);
		sprintf (upper,"%-10.10s",argv [3]);
		break;

	}
	OpenDB ();


	dsp_screen ("Updating Stock Take Freeze.",
					comm_rec.co_no,comm_rec.co_name);

	ProcessFile ();
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

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	ReadMisc ();
	abc_alias (inlo2, inlo);
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	/*
	OpenLocation (ccmr_rec.hhcc_hash);
	*/

	if (serialUpdate)
	{
		open_rec (insf, insf_list, INSF_NO_FIELDS, "insf_id_no2");
		open_rec (stts, stts_list, STTS_NO_FIELDS, "stts_id_no");
	}
	open_rec (inlo2, inlo_list, INLO_NO_FIELDS, "inlo_hhwh_hash");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (incc);
	abc_fclose (inwu);
	abc_fclose (inmr);
	abc_fclose (inlo2);
	/*
	CloseLocation ();
	*/
	if (serialUpdate)
	{
		abc_fclose (insf);
		abc_fclose (stts);
	}
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
	switch (byWhat [0])
	{
	case	'G':
		ProcessByGroup ();
		break;

	case	'I':
		ProcessByItem ();
		break;

	case	'A':
	case	'L':
		ProcessAbcLocaton ();
		break;
	}
}

void
ProcessByGroup (
 void)
{
	char	curr_gp [13];

	abc_selfield (inmr,"inmr_id_no_3");

	strcpy (inmr_rec.co_no		,comm_rec.co_no);
	sprintf (inmr_rec.inmr_class		,"%-1.1s",lower);
	sprintf (inmr_rec.category	,"%-11.11s",lower + 1);
	sprintf (inmr_rec.item_no	,"%-16.16s"," ");
	cc = find_rec (inmr,&inmr_rec,GTEQ,"r");
	while (!cc && !strcmp (inmr_rec.co_no,comm_rec.co_no))
	{
		sprintf (curr_gp,"%-1.1s%-11.11s",
					inmr_rec.inmr_class,inmr_rec.category);
		if (strncmp (curr_gp,upper,12) > 0)
			break;

		dsp_process ("Processing : ",inmr_rec.item_no);
		ProcessIncc ();

		cc = find_rec (inmr,&inmr_rec,NEXT,"r");
	}
}

void
ProcessByItem (
 void)
{
	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.item_no,"%-16.16s",lower);
	cc = find_rec (inmr,&inmr_rec,GTEQ,"r");
	while (!cc && !strcmp (inmr_rec.co_no,comm_rec.co_no) && 
				strncmp (inmr_rec.item_no,upper,16) <= 0)
	{
		dsp_process ("Processing : ",inmr_rec.item_no);
		ProcessIncc ();

		cc = find_rec (inmr,&inmr_rec,NEXT,"r");
	}
}

void
ProcessAbcLocaton (
 void)
{
	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.item_no,"%-16.16s"," ");
	cc = find_rec (inmr,&inmr_rec,GTEQ,"r");
	while (!cc && !strcmp (inmr_rec.co_no,comm_rec.co_no))
	{
		if (byWhat [0] == 'L' || ValidInmr ())
		{
			dsp_process ("Processing : ",inmr_rec.item_no);
			ProcessIncc ();
		}

		cc = find_rec (inmr,&inmr_rec,NEXT,"r");
	}
}

int
ValidInmr (
 void)
{
	if (byWhat [0] == 'A' && strncmp (inmr_rec.abc_code,lower,1) >= 0 && 
			         		 strncmp (inmr_rec.abc_code,upper,1) <= 0)
		return (EXIT_FAILURE);
	return (EXIT_SUCCESS);
}

void
ProcessIncc (
 void)
{
	char	systemDate [11];
	long	lsystemDate;

	if ((result = strstr (inval_cls, inmr_rec.inmr_class)))
		return;

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (incc,&incc_rec,COMPARISON,"u");
	if (!cc && incc_rec.stat_flag [0] == '0')
	{

		strcpy (systemDate, DateToString (TodaysDate ()));
		lsystemDate = TodaysDate ();

		if (byWhat [0] == 'L' && !CheckLocn ())
		{
			abc_unlock (incc);
			return;
		}

		/*-----------------------------------------------
		| Serial item Stock take and Not a Serial Item. |
		-----------------------------------------------*/
		if (serialUpdate && !SERIAL)
		{
			abc_unlock (incc);
			return;
		}
		/*--------------------------------------
		| Normal Stock Take and a Serial Item. |
		--------------------------------------*/
		if (!serialUpdate && SERIAL)
		{
			abc_unlock (incc);
			return;
		}

		incc_rec.stake 			= incc_rec.closing_stock;
		incc_rec.freeze_date 	= lsystemDate;
		strcpy (incc_rec.stat_flag,stockTakeSelection);
		cc = abc_update (incc,&incc_rec);
		if (cc) 
			file_err (cc, (char *)incc, "DBUPDATE");

		if (SERIAL && serialUpdate)
			AddStts (incc_rec.hhwh_hash);

		ProcessInwu (incc_rec.hhwh_hash);

		if (envVarMultLoc)
			ProcessInlo (incc_rec.hhwh_hash);
	}
	abc_unlock (incc);
}

/*====================================================
| Updated stock take figures for Inventory UOM file. |
====================================================*/
void
ProcessInwu (
 long	hhwh_hash)
{
	inwu_rec.hhwh_hash	=	hhwh_hash;
	inwu_rec.hhum_hash	=	0;
	cc = find_rec (inwu, &inwu_rec, GTEQ, "u");
	while (!cc && inwu_rec.hhwh_hash == hhwh_hash)
	{
		inwu_rec.stake = inwu_rec.closing_stock;

		cc = abc_update (inwu, &inwu_rec);
		if (cc)
			file_err (cc, (char *)inwu, "DBADD");

		cc = find_rec (inwu, &inwu_rec, NEXT, "u");
	}
	abc_unlock (inwu);
	return;
}
/*====================================================
| Updated stock take figures for Inventory UOM file. |
====================================================*/
void
ProcessInlo (
 long	hhwh_hash)
{
	inlo2_rec.hhwh_hash	=	hhwh_hash;
	cc = find_rec (inlo2, &inlo2_rec, GTEQ, "u");
	while (!cc && inlo2_rec.hhwh_hash == hhwh_hash)
	{
		inlo2_rec.stake = inlo2_rec.qty;

		cc = abc_update (inlo2, &inlo2_rec);
		if (cc)
			file_err (cc, inlo2, "DBADD");

		cc = find_rec (inlo2, &inlo2_rec, NEXT, "u");
	}
	abc_unlock (inlo2);
	return;
}

int
CheckLocn (
 void)
{
	if (envVarMultLoc)
	{
		inlo_rec.hhwh_hash 		= incc_rec.hhwh_hash;
		inlo_rec.hhum_hash 		= 0L;
		sprintf (inlo_rec.location,	"%-10.10s",	" ");
		sprintf (inlo_rec.lot_no,	"%-7.7s",	" ");
		cc = find_rec (inlo,&inlo_rec,GTEQ,"r");
		while (!cc && inlo_rec.hhwh_hash == incc_rec.hhwh_hash)
		{
			if (strncmp (inlo_rec.location,lower,10) >= 0 && 
		 	     strncmp (inlo_rec.location,upper,10) <= 0)
				return (EXIT_FAILURE);

			cc = find_rec (inlo,&inlo_rec,NEXT,"r");
		}
	}
	else
	{
		if (strncmp (incc_rec.location,lower,10) >= 0 && 
		     strncmp (incc_rec.location,upper,10) <= 0)
			return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

void
AddStts (
 long	hhwh_hash)
{
	char	ser_value [2];

	sprintf (ser_value, "%-1.1s", get_env ("SER_VALUE"));

	insf_rec.hhwh_hash = hhwh_hash;
	sprintf (insf_rec.serial_no,"%25.25s"," ");
	cc = find_rec (insf,&insf_rec,GTEQ,"r");

	while (!cc && insf_rec.hhwh_hash == hhwh_hash)
	{
		if (insf_rec.status [0] == 'F' || insf_rec.status [0] == 'C')
		{
			stts_rec.hhwh_hash = hhwh_hash;
			strcpy (stts_rec.serial_no,insf_rec.serial_no);
			cc = find_rec (stts,&stts_rec,COMPARISON,"u");
			if (cc)
			{
				strcpy (stts_rec.location,insf_rec.location);
				if (ser_value [0] == 'E')
					stts_rec.cost = insf_rec.est_cost;
				else
					stts_rec.cost = (insf_rec.act_cost != 0.00) ? insf_rec.act_cost : insf_rec.est_cost;

				strcpy (stts_rec.counted,"N");
				strcpy (stts_rec.status,insf_rec.status);
				strcpy (stts_rec.stat_flag,"0");
				cc = abc_add (stts,&stts_rec);
				if (cc) 
					file_err (cc, (char *)stts, "DBADD");
			}
			else
			{
				strcpy (stts_rec.location,insf_rec.location);
				if (ser_value [0] == 'E')
					stts_rec.cost = insf_rec.est_cost;
				else
					stts_rec.cost = (insf_rec.act_cost != 0.00) ? insf_rec.act_cost : insf_rec.est_cost;

				strcpy (stts_rec.counted,"N");
				strcpy (stts_rec.status,insf_rec.status);
				strcpy (stts_rec.stat_flag,"0");

				cc = abc_update (stts,&stts_rec);
				if (cc) 
					file_err (cc, (char *)stts, "DBUPDATE");
			}
		}

		cc = find_rec (insf,&insf_rec,NEXT,"r");
	}
	abc_unlock (stts);
}
