/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_auddel.c,v 5.4 2001/08/28 06:13:36 scott Exp $
|  Program Name  : (so_auddel.c)
|  Program Desc  : (Inventory Audit Delete Program)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: so_auddel.c,v $
| Revision 5.4  2001/08/28 06:13:36  scott
| Updated to change " ( to "(
|
| Revision 5.3  2001/08/14 02:44:45  scott
| Updated for new delete wizard
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_auddel.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_auddel/so_auddel.c,v 5.4 2001/08/28 06:13:36 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<ml_std_mess.h>
#include 	<ml_so_mess.h>
#include 	<DeleteControl.h>

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct cuinRecord	cuin_rec;
struct esmrRecord	esmr_rec;
struct trlnRecord	trln_rec;
struct trhrRecord	trhr_rec;
struct trshRecord	trsh_rec;
struct sosfRecord	sosf_rec;
struct cohrRecord	cohr_rec;
struct cohrRecord	cohr2_rec;
struct colnRecord	coln_rec;
struct arhrRecord	arhr_rec;
struct arlnRecord	arln_rec;
struct sonsRecord	sons_rec;

	char	*cohr2 = "cohr2";
	char	findStatus [2];

	int		archiveRecord	=	0,
			checkCuin		=	TRUE;
	long	daysOffset 		= 	0L;

/*======================= 
| Function Declarations |
=======================*/
void 	DeleteInvoice		(char *);
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ProcessColn 		(long);
void	DeleteSONS 			(long);
int  	CreateArhr 			(void);
int  	CreateArln 			(void);
int  	CheckCuin 			(char *);
int  	IntFindCumr 		(long);

/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr = get_env ("SO_ARCHIVE");

	if (sptr && !strncmp (sptr, "Y", 1))
		archiveRecord = TRUE;
	else
		archiveRecord = FALSE;
	
	/*--------------------------- 
	| Open main database files. |
	---------------------------*/
	OpenDB ();

	sprintf (err_str, 
		"Invoice Audit Delete %s", 
		 (archiveRecord) ? "(Archive)" : "");

	/*
	 * Check if delete control file defined for purge.
	 */
	if (FindDeleteControl (comm_rec.co_no, "PS-INVOICE-CREDIT"))
	{
		print_mess (ML ("Delete control file not maintained, cannot delete"));
		sleep (sleepTime);
	}
	else
	{
		sprintf (findStatus, "%-1.1s", delhRec.reference);
		daysOffset	=	(long) delhRec.purge_days;
		checkCuin	=	delhRec.spare_fg1;
	}
	dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, "  ");
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && !strcmp (esmr_rec.co_no, comm_rec.co_no))
	{
		DeleteInvoice ("I");
		DeleteInvoice ("C");

		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
DeleteInvoice (
 char *type)
{
	strcpy (cohr_rec.co_no,esmr_rec.co_no);
	strcpy (cohr_rec.br_no,esmr_rec.est_no);
	sprintf (cohr_rec.type,"%-1.1s", type);
	strcpy (cohr_rec.stat_flag,findStatus);

	cc = find_rec (cohr,&cohr_rec,GTEQ,"r");

	while (!cc && !strcmp (cohr_rec.co_no,esmr_rec.co_no) &&
	              !strcmp (cohr_rec.br_no,esmr_rec.est_no) &&
		      cohr_rec.type [0] == type [0] &&
		      cohr_rec.stat_flag [0] == findStatus [0])
	{
		if (cohr_rec.date_raised + daysOffset < comm_rec.dbt_date)
		{
			if (checkCuin && !CheckCuin (cohr_rec.inv_no))
			{
				cc = find_rec (cohr, &cohr_rec, NEXT, "r");
				continue;
			}

			dsp_process ("Trans : ",cohr_rec.inv_no);

			ProcessColn (cohr_rec.hhco_hash);

			if (archiveRecord)
			{
				if (CreateArhr ()) 
					file_err (cc, "arhr", "DBADD");
			}
		
			cohr2_rec.hhco_hash	=	cohr_rec.hhco_hash;
			cc = find_rec (cohr2, &cohr2_rec, COMPARISON, "u");
			if (cc)
				file_err (cc, "cohr2", "DBFIND");

			trhr_rec.hhtr_hash	=	cohr_rec.hhtr_hash;
			cc = find_rec (trhr, &trhr_rec, EQUAL, "r");
			if (!cc)
			{
				cc = abc_delete (trhr);
				if (cc)
					file_err (cc, "trhr", "DBDELETE");
			}
			
			trln_rec.hhtr_hash	=	cohr_rec.hhtr_hash;
			cc = find_rec (trln, &trln_rec, GTEQ, "r");
			while (!cc && trln_rec.hhtr_hash == cohr_rec.hhtr_hash)
			{
				cc = abc_delete (trln);
				if (cc)
					file_err (cc, "trln", "DBDELETE");

				cc = find_rec (trln, &trln_rec, NEXT, "r");
			}
			trsh_rec.hhco_hash	=	cohr_rec.hhco_hash;
			cc = find_rec (trsh, &trsh_rec, EQUAL, "r");
			if (!cc)
			{
				cc = abc_delete (trsh);
				if (cc)
					file_err (cc, "trsh", "DBDELETE");
			}
			sosf_rec.hhco_hash	=	cohr_rec.hhco_hash;
			cc = find_rec (sosf, &sosf_rec, EQUAL, "r");
			if (!cc)
			{
				cc = abc_delete (sosf);
				if (cc)
					file_err (cc, "sosf", "DBDELETE");
			}
				
			cc = abc_delete (cohr2);
			if (cc)
				file_err (cc, "cohr2", "DBDELETE");
		}
		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}
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

void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cohr2, cohr);
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_up_id");
	open_rec (cohr2,cohr_list, COHR_NO_FIELDS, "cohr_hhco_hash");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_id_no2");
	open_rec (trhr, trhr_list, TRHR_NO_FIELDS, "trhr_hhtr_hash");
	open_rec (trln, trln_list, TRLN_NO_FIELDS, "trln_hhtr_hash");
	open_rec (trsh, trsh_list, TRSH_NO_FIELDS, "trsh_hhco_hash");
	open_rec (sosf, sosf_list, SOSF_NO_FIELDS, "sosf_hhco_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (sons, sons_list, SONS_NO_FIELDS, "sons_id_no2");

	if (archiveRecord)
	{
		open_rec (arhr, arhr_list, ARHR_NO_FIELDS, "arhr_hhco_hash");
		open_rec (arln, arln_list, ARLN_NO_FIELDS, "arln_id_no");
	}
}

void
CloseDB (
 void)
{
	abc_fclose (sosf);
	abc_fclose (trsh);
	abc_fclose (esmr);
	abc_fclose (cohr);
	abc_fclose (cohr2);
	abc_fclose (coln);
	abc_fclose (cuin);
	abc_fclose (cumr);
	abc_fclose (trhr);
	abc_fclose (trln);
	abc_fclose (sons);

	if (archiveRecord)
	{
		abc_fclose (arhr);
		abc_fclose (arln);
	}

	abc_dbclose ("data");
}

/*===========================
| Process all order lines . |
===========================*/
void
ProcessColn (
	long	hhcoHash)
{
	coln_rec.hhco_hash 	= hhcoHash;
	coln_rec.line_no 	= 0;

	cc = find_rec (coln, &coln_rec, GTEQ,"r");
	while (!cc && coln_rec.hhco_hash == hhcoHash) 
	{
		if (archiveRecord)
		{
			/*---------------------
			| Create arln records |
			---------------------*/
			if (CreateArln ())
				file_err (cc, "arln", "DBADD");
		}
		DeleteSONS (coln_rec.hhcl_hash);

		cc = abc_delete (coln);
		if (cc)
			file_err (cc, "coln", "DBDELETE");

		coln_rec.hhco_hash 	= hhcoHash;
		coln_rec.line_no 	= 0;
		cc = find_rec (coln,&coln_rec,GTEQ,"r");
	}
}

/*=============================================
| Delete purchase order non stock lines file. |
=============================================*/
void	
DeleteSONS (
 long	hhclHash)
{
	sons_rec.hhcl_hash 	= hhclHash;
	sons_rec.line_no 	= 0;
	cc = find_rec (sons, &sons_rec, GTEQ, "r");
	while (!cc && sons_rec.hhcl_hash == hhclHash)
	{
		cc = abc_delete (sons);
		if (cc)
			file_err (cc, "sons", "DBDELETE");

		sons_rec.hhcl_hash 	= hhclHash;
		sons_rec.line_no 	= 0;
		cc = find_rec (sons, &sons_rec, GTEQ, "r");
	}
}
/*===============================
| Create arcive header details. |
===============================*/
int
CreateArhr (
 void)
{
	memcpy ((char *)&arhr_rec, (char *)&cohr_rec, sizeof (struct arhrRecord));

	/*----------------------------
	| Create arcive header file. |
	----------------------------*/
	return (abc_add (arhr, &arhr_rec));
}

/*===========================
| Create arcive line items. |
===========================*/
int
CreateArln (
 void)
{
	memcpy ((char *)&arln_rec, (char *)&coln_rec, sizeof (struct arlnRecord));

	/*-------------------------------------
	| Add an archiveRecord coln record to arln. |
	-------------------------------------*/
	return (abc_add (arln, &arln_rec));
}

/*===============================================
| Returns 1 if the cohr/coln is deleteable,  	|
| Returns 0 otherwise				|
===============================================*/
int
CheckCuin (
 char *inv_no)
{
	cc = IntFindCumr (cohr_rec.hhcu_hash);
	if (cc)
		return (EXIT_FAILURE);

	cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sprintf (cuin_rec.inv_no,"%-8.8s",inv_no);

	return (find_rec (cuin, &cuin_rec, COMPARISON, "r"));
}

/*=================================
| Find customer and check if H/O. |
=================================*/
int
IntFindCumr (
 long _hhcu_hash)
{
	cc = find_hash (cumr,&cumr_rec,EQUAL,"r",_hhcu_hash);
	if (cc)
		return (EXIT_FAILURE);

	if (cumr_rec.ho_dbt_hash == 0L)
		return (EXIT_SUCCESS);

	return (find_hash (cumr,&cumr_rec,EQUAL,"r",cumr_rec.ho_dbt_hash));
}
