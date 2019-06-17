/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: _misc_del.c,v 5.3 2001/08/14 02:50:05 scott Exp $
|  Program Name  : (psl_misc_del.c)
|  Program Desc  : (Misc Logistic file deletion Program)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 06/03/92         |
|---------------------------------------------------------------------|
| $Log: _misc_del.c,v $
| Revision 5.3  2001/08/14 02:50:05  scott
| Updated for new delete wizard
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _misc_del.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/psl_misc_del/_misc_del.c,v 5.3 2001/08/14 02:50:05 scott Exp $";

#define		MOD	1
#include 	<pslscr.h>	
#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<DeleteControl.h>

#include	"schema"

struct commRecord	comm_rec;
struct cfhsRecord	cfhs_rec;
struct bachRecord	bach_rec;
struct extrRecord	extr_rec;
struct cuccRecord	cucc_rec;
struct cuinRecord	cuin_rec;
struct cuphRecord	cuph_rec;
struct inafRecord	inaf_rec;

	char	taxCode [4];
	long	lsystemDate = 0L;

/*============================
| Local function prototypes  |
============================*/
void	OpenDB			 (void);
void	CloseDB		 	 (void);
int		ReadComm		 (void);
void	DeleteCfhs		 (long);
void	DeleteBach		 (long);
void	DeleteExtr		 (long);
void	DeleteCucc		 (long);
void	DeleteCuph		 (long);
void	DeleteInaf		 (long);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	OpenDB ();

	ReadComm ();

	init_scr ();

	dsp_screen ("Purging Misc Logistic files.",
					comm_rec.co_no,comm_rec.co_name);

	sprintf (taxCode, "%-3.3s", get_env ("GST_TAX_NAME"));

	lsystemDate = TodaysDate ();

	/*
	 * Check if delete control file defined for purge.
	 */
	cc = FindDeleteControl (comm_rec.co_no, "FREIGHT-HISTORY");
	if (!cc)
	{
		DeleteCfhs ((long) delhRec.purge_days);
	}
	cc = FindDeleteControl (comm_rec.co_no, "TAX-TRANSACTIONS");
	if (!cc)
	{
		DeleteExtr ((long) delhRec.purge_days);
	}
	cc = FindDeleteControl (comm_rec.co_no, "CREDIT-CONTROL-NOTE");
	if (!cc)
	{
		DeleteCucc ((long) delhRec.purge_days);
	}
	cc = FindDeleteControl (comm_rec.co_no, "SOP-LAST-ITEMS-SOLD");
	if (!cc)
	{
		DeleteCuph ((long) delhRec.purge_days);
	}
	cc = FindDeleteControl (comm_rec.co_no, "STOCK-DAILY-AUDIT");
	if (!cc)
	{
		DeleteInaf ((long) delhRec.purge_days);
	}
	DeleteBach ((long) 10L);

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	open_rec (cfhs, cfhs_list, CFHS_NO_FIELDS, "cfhs_id_no2");
	open_rec (bach, bach_list, BACH_NO_FIELDS, "bach_id_no");
	open_rec (extr, extr_list, EXTR_NO_FIELDS, "extr_id_no");
	open_rec (cucc, cucc_list, CUCC_NO_FIELDS, "cucc_hhcu_hash");
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_id_no2");
	open_rec (cuph, cuph_list, CUPH_NO_FIELDS, "cuph_hhcu_hash");
	open_rec (inaf, inaf_list, INAF_NO_FIELDS, "inaf_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (cfhs);
	abc_fclose (bach);
	abc_fclose (extr);
	abc_fclose (cucc);
	abc_fclose (cuin);
	abc_fclose (cuph);
	abc_fclose (inaf);
	abc_dbclose ("data");
}

/*===================================== 
| Get info from commom database file .|
=====================================*/
int
ReadComm (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	return (EXIT_SUCCESS);
}

void
DeleteCfhs (
	long	deleteDays)
{
	dsp_process ("DELETE","CARRIER HISTORY");
	strcpy (cfhs_rec.co_no, "  ");
	strcpy (cfhs_rec.br_no, "  ");
	strcpy (cfhs_rec.wh_no, "  ");
	cfhs_rec.date = 0L;

	cc = find_rec (cfhs, &cfhs_rec, GTEQ, "u");
	while (!cc)
	{
		if ((cfhs_rec.date + deleteDays) < lsystemDate)
		{
			abc_delete (cfhs);
			cc = find_rec (cfhs, &cfhs_rec, GTEQ, "u");
		}	
		else	
		{
			abc_unlock (cfhs);
			cc = find_rec (cfhs, &cfhs_rec, NEXT, "u");
		}
	}
	abc_unlock (cfhs);
}

void
DeleteBach (
	long	deleteDays)
{
	dsp_process ("DELETE","BATCH HISTORY");
	strcpy (bach_rec.co_no, "  ");
	strcpy (bach_rec.type, "  ");
	bach_rec.run_no = 0L;
	strcpy (bach_rec.batch_no, "     ");
	bach_rec.date = 0L;

	cc = find_rec (bach, &bach_rec, GTEQ, "u");
	while (!cc)
	{
		if ((bach_rec.date + deleteDays) < lsystemDate)
		{	
			abc_delete (bach);
			cc = find_rec (bach, &bach_rec, GTEQ, "u");
		}
		else
		{
			abc_unlock (bach);
			cc = find_rec (bach, &bach_rec, NEXT, "u");
		}
	}
	abc_unlock (bach);
}

void
DeleteExtr (
	long	deleteDays)
{
	char comment [12];

	sprintf (comment, "%-3.3s HISTORY", taxCode);
	dsp_process ("DELETE", comment);
	strcpy (extr_rec.co_no, "  ");
	strcpy (extr_rec.jnl_type, "  ");
	strcpy (extr_rec.gl_per, "  ");
	strcpy (extr_rec.int_no, "        `");
	strcpy (extr_rec.ref_no, "               ");
	extr_rec.run_no = 0L;
	extr_rec.date = 0L;

	cc = find_rec (extr, &extr_rec, GTEQ, "u");
	while (!cc)
	{
		if ((extr_rec.date + deleteDays) < lsystemDate)
		{	
			abc_delete (extr);
			cc = find_rec (extr, &extr_rec, GTEQ, "u");
		}	
		else
		{
			abc_unlock (extr);
			cc = find_rec (extr, &extr_rec, NEXT, "u");
		}
	}
	abc_unlock (extr);
}

void
DeleteCucc (
	long	deleteDays)
{
	dsp_process ("DELETE","CHEQUE NOTES");

	cc = find_hash (cucc, &cucc_rec, GTEQ, "u", 0L);
	while (!cc)
	{
		if (strcmp (cucc_rec.hold_ref, "        "))
		{
			cuin_rec.hhcu_hash = cucc_rec.hhcu_hash;
			strcpy (cuin_rec.inv_no, cucc_rec.hold_ref);
			cc = find_rec (cuin, &cuin_rec, COMPARISON, "r");
			if (cc)
			{	
				abc_delete (cucc);
				cc = find_hash (cucc, &cucc_rec, GTEQ, "u", 0L);
			}
			else
			{
				abc_unlock (cucc);
				cc = find_hash (cucc, &cucc_rec, NEXT, "u", 0L);
			}
			continue;
		}
		if ((cucc_rec.cont_date + deleteDays) < lsystemDate)
		{	
			abc_delete (cucc);
			cc = find_hash (cucc, &cucc_rec, GTEQ, "u", 0L);
		}
		else
		{
			abc_unlock (cucc);
			cc = find_hash (cucc, &cucc_rec, NEXT, "u", 0L);
		}
	}
	abc_unlock (cucc);
}

void
DeleteCuph (
	long	deleteDays)
{
	dsp_process ("DELETE","CUSTOMER NOTES");

	cc = find_hash (cuph, &cuph_rec, GTEQ, "u", 0L);
	while (!cc)
	{
		if ((cuph_rec.date_cheq + deleteDays) < lsystemDate)
		{
			abc_delete (cuph);
			cc = find_hash (cuph, &cuph_rec, GTEQ, "u", 0L);
		}
		else
		{
			abc_unlock (cuph);
			cc = find_hash (cuph, &cuph_rec, NEXT, "u", 0L);
		}
	}
	abc_unlock (cuph);
}

void
DeleteInaf (
	long	deleteDays)
{
	dsp_process ("DELETE","INV AUDIT HIST");
	strcpy (inaf_rec.co_no, "  ");
	strcpy (inaf_rec.br_no, "  ");
	strcpy (inaf_rec.wh_no, "  ");
	inaf_rec.sys_date 	= 0L;
	inaf_rec.hhbr_hash 	= 0L;
	inaf_rec.hhcc_hash 	= 0L;
	inaf_rec.type 		= 0;
	inaf_rec.date 		= 0L;
	strcpy (inaf_rec.ref1, "          ");
	strcpy (inaf_rec.ref2, "          ");

	cc = find_rec (inaf, &inaf_rec, GTEQ, "u");
	while (!cc)
	{
		if ((inaf_rec.date + deleteDays) < lsystemDate)
		{
			abc_delete (inaf);
			cc = find_rec (inaf, &inaf_rec, GTEQ, "u");
		}
		else
		{
			abc_unlock (inaf);
			cc = find_rec (inaf, &inaf_rec, NEXT, "u");
		}
	}
	abc_unlock (inaf);
}
