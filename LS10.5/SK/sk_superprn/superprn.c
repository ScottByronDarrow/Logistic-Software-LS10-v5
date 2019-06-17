/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: superprn.c,v 5.1 2001/08/09 09:20:14 scott Exp $
|  Program Name  : (sk_superprn.c)                     
|  Program Desc  : (Print Supercession/Alternate Number)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 15th May 2001    |
|---------------------------------------------------------------------|
| $Log: superprn.c,v $
| Revision 5.1  2001/08/09 09:20:14  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:18:02  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/05/15 06:43:45  scott
| Updated to add app.schema - removes code related to tables from program as it allows for better quality contol.
| Updated to perform routine maintenance to ensure standards are maintained.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: superprn.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_superprn/superprn.c,v 5.1 2001/08/09 09:20:14 scott Exp $";

#include 	<ml_sk_mess.h>
#include 	<ml_std_mess.h>
#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
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

	dsp_screen ("Printing Stock Supercessions.",
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
		if (strcmp (inmr_rec.supercession,"                ") ||
		     strcmp (inmr_rec.alternate,"                "))
			PrintLine ();

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
	fprintf (pp,"| %16.16s ",inmr_rec.alternate);

	if (strcmp (inmr_rec.supercession ,"                "))
	{
		strcpy (inmr2_rec.co_no, comm_rec.co_no);
		strcpy (inmr2_rec.item_no, inmr_rec.supercession);
		cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
		if (cc)
		{
			fprintf (pp,"| %16.16s "," ");
			fprintf (pp,"|%40.40s|\n"," ");
		}
		else
		{
			fprintf (pp,"| %16.16s ",inmr2_rec.item_no);
			fprintf (pp,"|%40.40s|\n",inmr2_rec.description);
		}
	}
	else
	{
		fprintf (pp,"| %16.16s "," ");
		fprintf (pp,"|%40.40s|\n"," ");
	}

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
	fprintf (pp,".L158\n");
	fprintf (pp,".B1\n");
	fprintf (pp,".E%s SUPERCESSION / ALTERNATE REPORT.\n",
					clip (comm_rec.cc_name));
	fprintf (pp,".B1\n");
	fprintf (pp,".E%s \n",clip (comm_rec.co_name));

	fprintf (pp, ".R================");
	fprintf (pp, "===================");
	fprintf (pp, "=========================================");
	fprintf (pp, "===================");
	fprintf (pp, "===================");
	fprintf (pp, "=========================================\n");

	fprintf (pp, "================");
	fprintf (pp, "===================");
	fprintf (pp, "=========================================");
	fprintf (pp, "===================");
	fprintf (pp, "===================");
	fprintf (pp, "=========================================\n");

	fprintf (pp, "|    GROUP     |");
	fprintf (pp, "   ITEM  NUMBER   |");
	fprintf (pp, "   I T E M      D E S C R I P T I O N   |");
	fprintf (pp, "     ALTERNATE    |");
	fprintf (pp, "   SUPERCESSION   |");
	fprintf (pp, "       SUPERCESSION   DESCRIPTION       |\n");

	fprintf (pp, "|              |");
	fprintf (pp, "                  |");
	fprintf (pp, "                                        |");
	fprintf (pp, "       NUMBER     |");
	fprintf (pp, "       NUMBER     |");
	fprintf (pp, "                                        |\n");

	fprintf (pp, "|--------------|");
	fprintf (pp, "------------------|");
	fprintf (pp, "----------------------------------------|");
	fprintf (pp, "------------------|");
	fprintf (pp, "------------------|");
	fprintf (pp, "----------------------------------------|\n");

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
