/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (lrp_noinis.c )                                    |
|  Program Desc  : (Print Missing Inventory Supplier Report.    )     |
|                  (                                              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Date Written  : (07/07/1998)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: lrp_noinis.c,v $
| Revision 5.2  2001/08/09 09:29:51  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:37  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:27  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:36  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:30  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:58:41  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.10  2000/07/10 01:52:29  scott
| Updated to replace "@ (" with "@(" to ensure psl_what works correctly
|
| Revision 1.9  2000/07/05 00:10:49  scott
| General Maintenance - Added app.schema
|
| Revision 1.8  2000/07/03 21:44:03  johno
| Service Call Number: 16408 - Selecon
| Skip items with inmr source of MP and MC
|
| Revision 1.7  2000/02/08 19:29:31  cam
| Fixes for GVision, Oracle and future DBIF compatibility.  Removed call to
| find_rec CURRENT and updated surrounding DBIF calls.
|
| Revision 1.6  1999/12/06 01:34:18  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.5  1999/09/29 10:10:49  scott
| Updated to be consistant on function names.
|
| Revision 1.4  1999/09/17 07:26:40  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.3  1999/09/16 09:20:43  scott
| Updated from Ansi Project
|
| Revision 1.2  1999/06/15 07:27:04  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lrp_noinis.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_noinis/lrp_noinis.c,v 5.2 2001/08/09 09:29:51 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_lrp_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct inisRecord	inis_rec;
struct inisRecord	inis2_rec;
struct sumrRecord	sumr_rec;

	char 	*inis2	= "inis2";

	int		printerNumber;
	int		no_inis = TRUE;
	int		any_inis = FALSE;
	
	char	lower [13];
	char	upper [13];

	FILE	*fout;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	OpenDB 					(void);
void 	CloseDB 				(void);
void 	OutputMissingInis 		(void);
void 	OutputSupplierSupers 	(void);
void 	Process 				(void);
void 	PrintNoInis 			(void);
void 	PrintAnyInis 			(void);
void 	PrintSupplierInis 		(void);
int	 	ValidateGroup 			(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv [])
{
	char	*sptr;

	if (argc < 4)
	{
		print_at (0,0, "Usage %s <LPNO> <LOWER> <UPPER>\n\r",argv [0]);
        return (EXIT_FAILURE);
	}

	printerNumber = atoi (argv [1]);
	sprintf (lower,"%-13.13s",argv [2]);
	sprintf (upper,"%-13.13s",argv [3]);

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr++;

	if (!strcmp (sptr, "lrp_ssinis"))
		no_inis = FALSE;
	else
		if (!strcmp (sptr, "lrp_any_inis"))
			any_inis = TRUE;

	OpenDB ();

	Process ();

	fprintf (fout, ".EOF\n");
	pclose (fout);

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

	abc_alias (inis2, inis);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no_3");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no2");
	open_rec (inis2,inis_list, INIS_NO_FIELDS, "inis_id_no3");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (inis);
	abc_fclose (inis2);
	abc_fclose (sumr);

    abc_dbclose ("data");
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
OutputMissingInis (void)
{
	dsp_screen ("Printing LRP - Missing Inventory Suppliers",
                comm_rec.co_no,comm_rec.co_name);

	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in opening pformat During (POPEN)",errno,PNAME);
	
	fprintf (fout, ".START%s\n",DateToString (comm_rec.inv_date));
	fprintf (fout, ".LP%d\n",printerNumber);

	fprintf (fout, ".12\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L80\n");
	fprintf (fout, ".ELRP - MISSING INVENTORY SUPPLIER\n");
	if (any_inis)
		fprintf (fout, ".EINCLUDING GLOBAL BRANCH SUPPLIERS\n");
	else
		fprintf (fout, ".EEXCLUDING GLOBAL BRANCH SUPPLIERS\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".E%s\n", clip (comm_rec.est_name));
	fprintf (fout, ".EAS AT %-24.24s\n",SystemTime ());

	fprintf (fout, ".R        ===================");
	fprintf (fout, "============================================\n");

	fprintf (fout, "        ===================");
	fprintf (fout, "============================================\n");

	fprintf (fout, "        |    ITEM NUMBER   ");
	fprintf (fout, "|    DESCRIPTION                           |\n");

	fprintf (fout, "        |------------------");
	fprintf (fout, "|------------------------------------------|\n");

	fflush (fout);
}

/*=====================================================================
| Start Out Put To Standard Print.
=====================================================================*/
void
OutputSupplierSupers (void)
{
	dsp_screen ("Printing Inventory Supplier Supercessions ",comm_rec.co_no,comm_rec.co_name);

	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in opening pformat During (POPEN)",errno,PNAME);
	
	fprintf (fout, ".START%s\n",DateToString (comm_rec.inv_date));
	fprintf (fout, ".LP%d\n",printerNumber);

	fprintf (fout, ".11\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L136\n");
	fprintf (fout, ".ELRP - INVENTORY SUPPLIER SUPERCESSIONS\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".E%s\n", clip (comm_rec.est_name));
	fprintf (fout, ".EAS AT %-24.24s\n",SystemTime ());

	fprintf (fout, ".R=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=================");
	fprintf (fout, "=======");
	fprintf (fout, "==========================================");
	fprintf (fout, "========\n");

	fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=================");
	fprintf (fout, "=======");
	fprintf (fout, "==========================================");
	fprintf (fout, "========\n");

	fprintf (fout, "|   ITEM NUMBER  ");
	fprintf (fout, "|             ITEM DESCRIPTION           ");
	fprintf (fout, "| SUPPLIERS PART ");
	fprintf (fout, "|SUP NO");
	fprintf (fout, "|           SUPPLIER NAME                 ");
	fprintf (fout, "| PRI. |\n");

	fprintf (fout, "|----------------");
	fprintf (fout, "|----------------------------------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|------");
	fprintf (fout, "|-----------------------------------------");
	fprintf (fout, "|------|\n");

	fflush (fout);
}

/*==========================================================================
| processing routine for missing supplier reports and supercessed records. |
==========================================================================*/
void
Process (void)
{
	if (no_inis)
		OutputMissingInis ();
	else
		OutputSupplierSupers ();

	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.inmr_class,"%-1.1s",lower);
	sprintf (inmr_rec.category,"%-11.11s",lower + 1);
	sprintf (inmr_rec.item_no,"%-16.16s"," ");
	cc = find_rec (inmr,&inmr_rec,GTEQ,"r");

	while (!cc && !strcmp (inmr_rec.co_no,comm_rec.co_no) && ValidateGroup ())
	{
		if (!strcmp (inmr_rec.source, "MP") 
			|| !strcmp (inmr_rec.source, "MC"))
		{
			/* skip manufactured stuff */
			cc = find_rec (inmr,&inmr_rec,NEXT,"r");
			continue;
		}
		if (strcmp (inmr_rec.supercession, "                "))
		{
			cc = find_rec (inmr,&inmr_rec,NEXT,"r");
			continue;
		}
		if (no_inis)
			if (any_inis)
				PrintAnyInis ();
			else
				PrintNoInis ();
		else
			PrintSupplierInis ();

		cc = find_rec (inmr,&inmr_rec,NEXT,"r");
	}
}

/*===============================================
| Process Missing inventory supplier reports.	|
| Don't check for global inis records.			|
===============================================*/
void
PrintNoInis (void)
{
	inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (inis_rec.sup_priority, "  ");
	strcpy (inis_rec.co_no,	"  ");
	strcpy (inis_rec.br_no,	"  ");
	strcpy (inis_rec.wh_no,	"  ");

	cc = find_rec (inis, &inis_rec, GTEQ, "r");
	if (cc || inis_rec.hhbr_hash != inmr_rec.hhbr_hash)
	{
		dsp_process (" Item No. : ",inmr_rec.item_no);
		fprintf (fout,"        | %-16.16s ",inmr_rec.item_no);
		fprintf (fout,"| %-40.40s |\n",inmr_rec.description);
	}
}

/*===============================================
| Process Missing inventory supplier reports.	|
| If a global inis exists, don't print out.	|
===============================================*/
void
PrintAnyInis (void)
{
	inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (inis_rec.sup_priority, "  ");
	strcpy (inis_rec.co_no,	"  ");
	strcpy (inis_rec.br_no,	"  ");
	strcpy (inis_rec.wh_no,	"  ");

	cc = find_rec (inis, &inis_rec, GTEQ, "r");
	if (cc || inis_rec.hhbr_hash != inmr_rec.hhbr_hash)
	{
		inis2_rec.hhsu_hash	=	0L;
		inis2_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		strcpy (inis2_rec.co_no,	"  ");
		strcpy (inis2_rec.br_no,	"  ");
		strcpy (inis2_rec.wh_no,	"  ");
		cc = find_rec (inis2, &inis2_rec, GTEQ,"r");
		if (cc || inis2_rec.hhbr_hash != inmr_rec.hhbr_hash)
		{
			dsp_process (" Item No. : ",inmr_rec.item_no);
			fprintf (fout,"        | %-16.16s ",inmr_rec.item_no);
			fprintf (fout,"| %-40.40s |\n",inmr_rec.description);
		}
	}
}

/*================================================================
| Process inventory supplier reports that have been supercessed. |
================================================================*/
void
PrintSupplierInis (void)
{
	dsp_process (" Item No. : ",inmr_rec.item_no);

	inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (inis_rec.sup_priority, "  ");
	strcpy (inis_rec.co_no,	"  ");
	strcpy (inis_rec.br_no,	"  ");
	strcpy (inis_rec.wh_no,	"  ");
	cc = find_rec (inis, &inis_rec, GTEQ, "u");
	while (!cc && inis_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		/*-----------------------------
		| Status only 'C' is changed. |
		-----------------------------*/
		if (inis_rec.stat_flag [0] != 'C')
		{
			abc_unlock (inis);
			cc = find_rec (inis, &inis_rec, NEXT, "u");
			continue;
		}
		/*---------------------------
		| No Supplier record found. |
		---------------------------*/
		sumr_rec.hhsu_hash	=	inis_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc)
		{
			abc_unlock (inis);
			cc = find_rec (inis, &inis_rec, NEXT, "u");
			continue;
		}
		
		fprintf (fout, "|%-16.16s",inmr_rec.item_no);
		fprintf (fout, "|%-40.40s",inmr_rec.description);
		fprintf (fout, "|%-16.16s",inis_rec.sup_part);
		fprintf (fout, "|%-6.6s",  sumr_rec.crd_no);
		fprintf (fout, "| %-40.40s",sumr_rec.crd_name);
		fprintf (fout, "|  %-2.2s  |\n", inis_rec.sup_priority);

		strcpy (inis_rec.stat_flag, "0");

		cc = abc_update (inis, &inis_rec);
		if (cc)
			file_err (cc, "inis", "DBUPDATE");

		abc_unlock (inis);
		cc = find_rec (inis, &inis_rec, NEXT, "u");
	}
	abc_unlock (inis);
}

/*=================
| Validate group. |
=================*/
int
ValidateGroup (void)
{
	/*---------------
	| Invalid Class	|
	---------------*/
	if (inmr_rec.inmr_class [0] > upper [0])
		return (EXIT_SUCCESS);

	/*---------------------------------------
	| Classes Sames but Invalid Category	|
	---------------------------------------*/
	if (inmr_rec.inmr_class [0] == upper [0] && 
		 strcmp (inmr_rec.category,upper + 1) > 0)
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}
