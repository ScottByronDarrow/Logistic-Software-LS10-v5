/*=====================================================================
|  Copyright (C) 1996 - 2000 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( psl_sr_gen.c   )                                 |
|  Program Desc  : ( Generates search files for DB,SK and CR.     )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  cumr, sumr, inmr, srsk, srdb, srcr                |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  srcr, srdb, srcr,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 11/09/95         |
|---------------------------------------------------------------------|
|  Date Modified : (13/09/1997)    | Modified  by  : Leah Manibog.    |
|  Date Modified : (03/09/1999)    | Modified  by  : Ramon A. Pacheco |
|                                                                     |
|   Comment      :                                                    |
|  (13/09/1997)  : Updated for Multilingual Conversion.				  |
|  (03/09/1999)  : Ported to ANSI standards.        				  |
|                :                                                    |
|                :                                                    |
|                :                                                    |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: psl_sr_gen.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/psl_sr_gen/psl_sr_gen.c,v 5.1 2001/08/09 05:13:51 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_menu_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct srdbRecord	srdb_rec;
struct cumrRecord	cumr_rec;
struct srskRecord	srsk_rec;
struct inmrRecord	inmr_rec;
struct srcrRecord	srcr_rec;
struct sumrRecord	sumr_rec;

	char	*data	= "data";

/*============================
| Local function prototypes  |
============================*/
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB			(void);
void	Update			(void);
void	ClearData		(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	/*---------------------------
	| open main database files. |
	---------------------------*/
	OpenDB ();

	dsp_screen ("Checking for Deleted records.", comm_rec.co_no, comm_rec.co_name);
	/*----------------------
	| Main update routine. |
	----------------------*/
	ClearData ();

	dsp_screen ("Creating Search Index Files.", comm_rec.co_no, comm_rec.co_name);

	/*----------------------
	| Main update routine. |
	----------------------*/
	Update();

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
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (sumr,  sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (srsk,  srsk_list, SRSK_NO_FIELDS, "srsk_hhbr_hash");
	open_rec (srdb,  srdb_list, SRDB_NO_FIELDS, "srdb_hhcu_hash");
	open_rec (srcr,  srcr_list, SRCR_NO_FIELDS, "srcr_hhsu_hash");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (cumr);
	abc_fclose (sumr);
	abc_fclose (srsk);
	abc_fclose (srdb);
	abc_fclose (srcr);
	SearchFindClose ();
	abc_dbclose (data);
}

/*===================================
| Main update / processing routine. |
===================================*/
void
Update (
 void)
{
	int		new_srsk,
			new_srdb,
			new_srcr;

	inmr_rec.hhbr_hash = 0L;
	cumr_rec.hhcu_hash = 0L;
	sumr_rec.hhsu_hash = 0L;

	/*----------------------------
	| Process Stock master file. |
	----------------------------*/
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc)
	{
		dsp_process ("Item", inmr_rec.item_no);

		srsk_rec.hhbr_hash = inmr_rec.hhbr_hash;
		new_srsk = find_rec (srsk, &srsk_rec, EQUAL, "u");

		strcpy (srsk_rec.co_no , 		inmr_rec.co_no);
		srsk_rec.hhbr_hash 	=			inmr_rec.hhbr_hash;
		strcpy (srsk_rec.item_no,		inmr_rec.item_no);
		strcpy (srsk_rec.srsk_class,	inmr_rec.inmr_class);
		strcpy (srsk_rec.category,		inmr_rec.category);
		strcpy (srsk_rec.active_status,	inmr_rec.active_status);
		strcpy (srsk_rec.alpha_code,	inmr_rec.alpha_code);
		strcpy (srsk_rec.alternate,		inmr_rec.alternate);
		strcpy (srsk_rec.barcode,		inmr_rec.barcode);
		strcpy (srsk_rec.maker_no,		inmr_rec.maker_no);
		strcpy (srsk_rec.description, 	upshift (inmr_rec.description));
		strcpy (srsk_rec.source, 		inmr_rec.source);
		strcpy (srsk_rec.sellgrp, 		inmr_rec.sellgrp);
		strcpy (srsk_rec.buygrp, 		inmr_rec.buygrp);
		strcpy (srsk_rec.qc_reqd, 		inmr_rec.qc_reqd);
		strcpy (srsk_rec.spare, 		" ");
		
		if (new_srsk)
		{
			cc = abc_add (srsk, &srsk_rec);
			if (cc)
				file_err (cc, "srsk", "DBADD");
			
			abc_unlock (srsk);
		}
		else
		{
			cc = abc_update (srsk, &srsk_rec);
			if (cc)
				file_err (cc, "srsk", "DBUPDATE");
		}
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	abc_unlock ("srsk");

	/*-------------------------------
	| Process Customer master file. |
	-------------------------------*/
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc)
	{
		dsp_process ("Customer", cumr_rec.dbt_no);
		srdb_rec.hhcu_hash = cumr_rec.hhcu_hash;
		new_srdb = find_rec (srdb, &srdb_rec, EQUAL, "u");

		strcpy (srdb_rec.co_no, 		cumr_rec.co_no);
		strcpy (srdb_rec.br_no, 		cumr_rec.est_no);
		strcpy (srdb_rec.dbt_no, 		cumr_rec.dbt_no);
		srdb_rec.hhcu_hash		=	 	cumr_rec.hhcu_hash;
		strcpy (srdb_rec.acronym, 		cumr_rec.dbt_acronym);
		strcpy (srdb_rec.sman_code, 	cumr_rec.sman_code);
		strcpy (srdb_rec.contact_name, 	cumr_rec.contact_name);
		strcpy (srdb_rec.name, 			upshift (cumr_rec.dbt_name));

		if (new_srdb)
		{
			cc = abc_add (srdb, &srdb_rec);
			if (cc)
				file_err (cc, "srdb", "DBADD");
			
			abc_unlock (srdb);
		}
		else
		{
			cc = abc_update (srdb, &srdb_rec);
			if (cc)
				file_err (cc, "srdb", "DBUPDATE");
		}
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}
	abc_unlock ("srdb");

	/*-------------------------------
	| Process Supplier master file. |
	-------------------------------*/
	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");
	while (!cc)
	{
		dsp_process ("Supplier", sumr_rec.crd_no);
		srcr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		new_srcr = find_rec (srcr, &srcr_rec, EQUAL, "u");

		strcpy (srcr_rec.co_no, 		sumr_rec.co_no);
		strcpy (srcr_rec.br_no, 		sumr_rec.est_no);
		strcpy (srcr_rec.crd_no, 		sumr_rec.crd_no);
		srcr_rec.hhsu_hash		=	 	sumr_rec.hhsu_hash;
		strcpy (srcr_rec.acronym, 		sumr_rec.acronym);
		strcpy (srcr_rec.type_code, 	sumr_rec.type_code);
		strcpy (srcr_rec.contact_name, 	sumr_rec.cont_name);
		strcpy (srcr_rec.name, 			upshift (sumr_rec.crd_name));

		if (new_srcr)
		{
			cc = abc_add (srcr, &srcr_rec);
			if (cc)
				file_err (cc, "srcr", "DBADD");
			
			abc_unlock (srcr);
		}
		else
		{
			cc = abc_update (srcr, &srcr_rec);
			if (cc)
				file_err (cc, "srcr", "DBUPDATE");
		}
		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
	}
	abc_unlock ("srcr");
}

/*===================================
| Main update / processing routine. |
===================================*/
void
ClearData (
 void)
{
	inmr_rec.hhbr_hash = 0L;
	cumr_rec.hhcu_hash = 0L;
	sumr_rec.hhsu_hash = 0L;
	srsk_rec.hhbr_hash = 0L;
	srdb_rec.hhcu_hash = 0L;
	srcr_rec.hhsu_hash = 0L;

	cc = find_rec (srsk, &srsk_rec, GTEQ, "u");
	while (!cc)
	{
		inmr_rec.hhbr_hash = srsk_rec.hhbr_hash;
		/*----------------------------
		| Process Stock master file. |
		----------------------------*/
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = abc_delete (srsk);
			if (cc)
				file_err (cc, "srsk", "DBDELETE");

			cc = find_rec (srsk, &srsk_rec, GTEQ, "u");
		}
		else
		{
			abc_unlock ("srsk");
			cc = find_rec (srsk, &srsk_rec, NEXT, "u");
		}
	}

	/*-------------------------------
	| Process Customer master file. |
	-------------------------------*/
	cc = find_rec (srdb, &srdb_rec, GTEQ, "u");
	while (!cc)
	{
		dsp_process ("Customer", cumr_rec.dbt_no);
		cumr_rec.hhcu_hash = srdb_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			cc = abc_delete (srdb);
			if (cc)
				file_err (cc, "srdb", "DBDELETE");

			cc = find_rec (srdb, &srdb_rec, GTEQ, "u");
		}
		else
		{
			abc_unlock ("srdb");
			cc = find_rec (srdb, &srdb_rec, NEXT, "u");
		}
	}

	/*-------------------------------
	| Process Supplier master file. |
	-------------------------------*/
	cc = find_rec (srcr, &srcr_rec, GTEQ, "u");
	while (!cc)
	{
		dsp_process ("Supplier", sumr_rec.crd_no);
		sumr_rec.hhsu_hash = srcr_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc)
		{
			cc = abc_delete (srcr);
			if (cc)
				file_err (cc, "srcr", "DBDELETE");

			cc = find_rec (srcr, &srcr_rec, GTEQ, "u");
		}
		else
		{
			abc_unlock ("srcr");
			cc = find_rec (srcr, &srcr_rec, NEXT, "u");
		}
	}
}
