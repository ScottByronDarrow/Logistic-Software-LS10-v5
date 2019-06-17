/*=====================================================================
|  Copyright (C) 2000 - 2000 Logistic Software Limited   .            |
|=====================================================================|
| $Id: lrp_any_bmms.c,v 5.2 2001/08/09 09:29:35 scott Exp $
|  Program Name  : ( lrp_any_bmms.c  )                                |
|  Program Desc  : ( Print Missing Bill of Material Details       )   |
|---------------------------------------------------------------------|
|  Date Written  : (28/06/2000)    | Author      : John R Oliver.     |
|---------------------------------------------------------------------|
| $Log: lrp_any_bmms.c,v $
| Revision 5.2  2001/08/09 09:29:35  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:21  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:11  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:14  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 1.3  2001/01/30 05:48:46  scott
| Updated to add app.schema
|
| Revision 1.2  2001/01/22 07:10:02  scott
| Updated to use app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lrp_any_bmms.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_any_bmms/lrp_any_bmms.c,v 5.2 2001/08/09 09:29:35 scott Exp $";

#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_lrp_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct bmmsRecord	bmms_rec;
struct rghrRecord	rghr_rec;
struct sumrRecord	sumr_rec;

	int		printerNumber;
	
	char	lower[13];
	char	upper[13];

	FILE	*fout;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	AnyBmmsOut 		(void);
void 	Process 		(void);
void 	PrintAnyBmms 	(void);
int 	ValidateGroup 	(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv[])
{
	char	*sptr;

	if (argc < 4)
	{
		/*-----------------------------------
		| Usage : %s <LPNO> <LOWER> <UPPER> |
		-----------------------------------*/
		print_at (0,0, "Usage %s <LPNO> <LOWER> <UPPER>\n\r",argv[0]);
        return (EXIT_FAILURE);
	}

	printerNumber = atoi (argv[1]);
	sprintf (lower,"%-13.13s",argv[2]);
	sprintf (upper,"%-13.13s",argv[3]);

	sptr = strrchr (argv[0], '/');
	if (sptr == (char *) 0)
		sptr = argv[0];
	else
		sptr++;

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

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no_3");
	open_rec (bmms, bmms_list, BMMS_NO_FIELDS, "bmms_id_no");
	open_rec (rghr, rghr_list, RGHR_NO_FIELDS, "rghr_id_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (bmms);
	abc_fclose (rghr);
	abc_fclose (sumr);

    abc_dbclose ("data");
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
AnyBmmsOut (void)
{
	dsp_screen ("Printing LRP - Missing Bill of Material Details",
                comm_rec.co_no,comm_rec.co_name);

	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in opening pformat During (POPEN)",errno,PNAME);
	
	fprintf (fout, ".START%s\n",DateToString (comm_rec.inv_date));
	fprintf (fout, ".LP%d\n",printerNumber);

	fprintf (fout, ".11\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L120\n");
	fprintf (fout, ".ELRP - MISSING BILL OF MATERIAL DETAILS\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".E%s\n", clip (comm_rec.est_name));
	fprintf (fout, ".EAS AT %-24.24s\n",SystemTime ());

	fprintf (fout, ".R        ===================");
	fprintf (fout, "============================================");
	fprintf (fout, "=========================================\n");

	fprintf (fout, "        ===================");
	fprintf (fout, "============================================");
	fprintf (fout, "=====================================\n");

	fprintf (fout, "        |    ITEM NUMBER   ");
	fprintf (fout, "|    DESCRIPTION                           ");
	fprintf (fout, "|    MISSING BOM   |   MISSING ROUTE   |\n");

	fprintf (fout, "        |------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|------------------|-------------------|\n");

	fflush (fout);
}

/*==========================================================================
| processing routine for missing supplier reports and supercessed records. |
==========================================================================*/
void
Process (void)
{
	AnyBmmsOut ();

	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.inmr_class,"%-1.1s",lower);
	sprintf (inmr_rec.category,"%-11.11s",lower + 1);
	sprintf (inmr_rec.item_no,"%-16.16s"," ");
	cc = find_rec (inmr,&inmr_rec,GTEQ,"r");

	while (!cc && !strcmp (inmr_rec.co_no,comm_rec.co_no) && ValidateGroup ())
	{
		if (strcmp (inmr_rec.supercession, "                ") 
			|| (	strcmp (inmr_rec.source, "MP")
				&&	strcmp (inmr_rec.source, "MC")))
		{
			cc = find_rec (inmr,&inmr_rec,NEXT,"r");
			continue;
		}
		PrintAnyBmms ();

		cc = find_rec (inmr,&inmr_rec,NEXT,"r");
	}
}

/*===============================================
| Process Missing Bill of Material and Routes	|
===============================================*/
void
PrintAnyBmms (void)
{
	int		missing_BOM		= 0,
			missing_Route	= 0;

	/*---------------
	| Check for BOM |
	---------------*/
	strcpy (bmms_rec.co_no, inmr_rec.co_no);
	bmms_rec.hhbr_hash = inmr_rec.hhbr_hash;
	bmms_rec.alt_no = inmr_rec.dflt_bom;
	bmms_rec.line_no = 0;

	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
	if (cc || bmms_rec.hhbr_hash != inmr_rec.hhbr_hash
			|| bmms_rec.alt_no != inmr_rec.dflt_bom)
	{
		missing_BOM = TRUE;
	}
	else
		missing_BOM = FALSE;

	/*-----------------
	| Check for Route |
	-----------------*/
	strcpy (rghr_rec.co_no, inmr_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash = inmr_rec.hhbr_hash;
	rghr_rec.alt_no = inmr_rec.dflt_rtg;

	cc = find_rec (rghr, &rghr_rec, EQUAL, "r");
	if (cc)
	{
		missing_Route = TRUE;
	}
	else
		missing_Route = FALSE;

	if (missing_Route || missing_BOM)
	{
		dsp_process (" Item No. : ",inmr_rec.item_no);
		fprintf (fout,"        | %-16.16s ",inmr_rec.item_no);
		fprintf (fout,"| %-40.40s ",inmr_rec.description);
		fprintf (fout,"|        %c         ", missing_BOM ? 'X' : ' ');
		fprintf (fout,"|         %c         |\n", missing_Route ? 'X' : ' ');
	}
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
	if (inmr_rec.inmr_class[0] > upper[0])
		return (EXIT_SUCCESS);

	/*---------------------------------------
	| Classes Sames but Invalid Category	|
	---------------------------------------*/
	if (inmr_rec.inmr_class[0] == upper[0] && 
		 strcmp (inmr_rec.category,upper + 1) > 0)
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}
