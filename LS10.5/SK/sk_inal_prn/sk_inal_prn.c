/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_inal_prn.c,v 5.3 2001/08/09 09:18:35 scott Exp $
|  Program Name  : (sk_inal_prn.c)                     
|  Program Desc  : (Print Stock Levy.)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 15th May 2001    |
|---------------------------------------------------------------------|
| $Log: sk_inal_prn.c,v $
| Revision 5.3  2001/08/09 09:18:35  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/07/25 02:19:06  scott
| Update - LS10.5
|
| Revision 5.0  2001/06/19 08:15:47  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.4  2001/05/22 05:08:45  cha
| Updated to process all inal records correctly.
|
| Revision 4.3  2001/05/22 03:53:30  scott
| Updated to modify report for new structure and to include currency
|
| Revision 4.1  2001/05/15 07:28:57  scott
| New Program
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_inal_prn.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_inal_prn/sk_inal_prn.c,v 5.3 2001/08/09 09:18:35 scott Exp $";

#include 	<ml_sk_mess.h>
#include 	<ml_std_mess.h>
#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct inalRecord	inal_rec;
struct inmrRecord	inmr2_rec;

/*===========
 Table Names
=============*/
static char
	*data	= "data",
	*inmr2	= "inmr2";

/*=========
| Globals |
=========*/

	int	printerNo = 1;

	FILE	*pp;

/*=======================
| Function Declarations |
=======================*/
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ProcessFile 	(void);
void 	PrintLine 		(void);
void 	HeadingOutput 	(void);
void 	EndReport 		(void);

/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{

	if (argc < 2)
	{
		print_at (0,0,mlStdMess036,argv[0]);
		return (EXIT_FAILURE);
	}

	printerNo = atoi (argv[1]);

	OpenDB ();

	HeadingOutput ();

	dsp_screen ("Printing Stock Levy Details.",
			comm_rec.co_no,comm_rec.co_name);

	ProcessFile ();

	EndReport ();	

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
	abc_dbopen (data);

	abc_alias (inmr2,inmr);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no_3");
	open_rec (inal, inal_list, INAL_NO_FIELDS, "inal_id_no2");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (inal);
	abc_fclose (inmr2);
	abc_dbclose (data);
}

/*===================================================
| Get alpha code (in order) & check company number. |
===================================================*/
void
ProcessFile (
 void)
{
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	strcpy (inmr_rec.inmr_class, " ");
	strcpy (inmr_rec.category, "           ");
	strcpy (inmr_rec.item_no, "                ");

	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no))
	{
		strcpy (inal_rec.br_no, comm_rec.est_no);
		strcpy (inal_rec.curr_code, "   ");
		inal_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		inal_rec.date_from	=	0L;
		cc = find_rec (inal, &inal_rec, GTEQ, "r");
		while (!cc && !strcmp (inal_rec.br_no, comm_rec.est_no) &&
					inal_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			PrintLine ();
			cc = find_rec (inal, &inal_rec, NEXT, "r");
		}
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
}

void
PrintLine (
 void)
{
	fprintf (pp,"| %s%s ",inmr_rec.inmr_class,inmr_rec.category);
	fprintf (pp,"| %16.16s ",inmr_rec.item_no);
	fprintf (pp,"|%40.40s",inmr_rec.description);
	fprintf (pp, "| %-10.10s  ",	DateToString (inal_rec.date_from));
	fprintf (pp, "| %-10.10s ",		DateToString (inal_rec.date_to));
	fprintf (pp, "|%-4.4s",			inal_rec.curr_code);
	fprintf (pp, "| %14.2f ",		DOLLARS (inal_rec.value));
	fprintf (pp, "|  %6.2f |\n",	inal_rec.percent);
	dsp_process ("Item : ", inmr_rec.item_no);
}

void
HeadingOutput (
 void)
{

	if ((pp = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (pp, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (pp,".LP%d\n",printerNo);
	fprintf (pp,".11\n");
	fprintf (pp,".L136\n");
	fprintf (pp,".B1\n");
	fprintf (pp,".E%s\n",clip (comm_rec.co_name));
	fprintf (pp,".B1\n");
	fprintf (pp,".E%s - STOCK LEVY REPORT.\n", clip (comm_rec.est_name));

	fprintf (pp, ".R===============");
	fprintf (pp, "===================");
	fprintf (pp, "=========================================");
	fprintf (pp, "============================================================\n");

	fprintf (pp, "===============");
	fprintf (pp, "===================");
	fprintf (pp, "=========================================");
	fprintf (pp, "============================================================\n");

	fprintf (pp, "|    GROUP     ");
	fprintf (pp, "|   ITEM  NUMBER   ");
	fprintf (pp, "|   I T E M      D E S C R I P T I O N   ");
	fprintf (pp, "|  LEVY FROM  |  LEVY TO   |CURR|  LEVY AMOUNT   |   LEVY  |\n");

	fprintf (pp, "|              ");
	fprintf (pp, "|                  ");
	fprintf (pp, "|                                        ");
	fprintf (pp, "|    DATE     |    DATE    |CODE|                | PERCENT |\n");

	fprintf (pp, "|--------------");
	fprintf (pp, "|------------------");
	fprintf (pp, "|----------------------------------------");
	fprintf (pp, "|-------------|------------|----|----------------|---------|\n");

	fprintf (pp,".PI12\n");
}

/*===========================================
| Routine to print final totals for report. |
===========================================*/
void
EndReport (
 void)
{
	fprintf (pp, ".EOF\n");
	pclose (pp);
}
