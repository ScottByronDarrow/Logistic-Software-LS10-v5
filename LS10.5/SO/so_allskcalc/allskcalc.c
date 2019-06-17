/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: allskcalc.c,v 5.2 2001/08/09 09:20:32 scott Exp $
|  Program Name  : (so_allskcalc.c)
|  Program Desc  : (Recalculate Committed And On Order From)
|                  (Order Entry and Invoice From Backlog)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: allskcalc.c,v $
| Revision 5.2  2001/08/09 09:20:32  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:50:44  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:18:30  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/05/31 10:00:25  scott
| Updated to add app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: allskcalc.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_allskcalc/allskcalc.c,v 5.2 2001/08/09 09:20:32 scott Exp $";

#include 	<pslscr.h>

#define		MOD	100
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct cumrRecord	cumr_rec;
struct sobgRecord	sobg_rec;

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 	 (void);
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
int  	ProcessFile 	 (void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	set_tty ();

	OpenDB ();

	dsp_screen ("Recalculating On Order,Committed and Backorder.",
					comm_rec.co_no,comm_rec.co_name);

	ProcessFile ();

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

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (sobg, sobg_list, SOBG_NO_FIELDS, "sobg_id_no");
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
	abc_fclose (sobg);
	abc_dbclose ("data");
}

/*=======================================================================
| Process whole inmr file for company and work out on-hand and on-order |
| accordingly.                                                          |
| Returns: 0 if ok,non-zero if not ok.                                  |
=======================================================================*/
int
ProcessFile (
 void)
{
	strcpy (inmr_rec.co_no,comm_rec.co_no);
	strcpy (inmr_rec.item_no,"                ");

	/*-----------------------
	| Read whole inmr file. |
	-----------------------*/
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no,comm_rec.co_no)) 
	{
		strcpy (sobg_rec.co_no, "  ");
		strcpy (sobg_rec.br_no, "  ");
		strcpy (sobg_rec.type , "RC");
		sobg_rec.lpno = 0;
		sobg_rec.hash = inmr_rec.hhbr_hash;
		sobg_rec.hash2 = 0L;
		sobg_rec.value = 0.00;
		cc = abc_add (sobg, &sobg_rec);
		if (cc)
			file_err (cc, sobg, "DBADD");
		
		dsp_process ("Item # ",inmr_rec.item_no);
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	abc_unlock (inmr);
	cumr_rec.hhcu_hash = 0L ;
	cc = find_hash (cumr, &cumr_rec, GTEQ, "r", 0L);
	while (!cc)
	{
		strcpy (sobg_rec.co_no, "  ");
		strcpy (sobg_rec.br_no, "  ");
		strcpy (sobg_rec.type , "RO");
		sobg_rec.lpno = 0;
		sobg_rec.hash = cumr_rec.hhcu_hash;
		sobg_rec.hash2 = 0L;
		sobg_rec.value = 0.00;
		cc = abc_add (sobg, &sobg_rec);
		if (cc)
			file_err (cc, sobg, "DBADD");

		cc = find_hash (cumr, &cumr_rec, NEXT, "r", 0L);
	}
	return (EXIT_SUCCESS);
}
