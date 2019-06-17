/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cmmt_del.c,v 5.3 2001/10/05 02:52:08 cha Exp $
|  Program Name  : (so_cmmt_del.c)                     
|  Program Desc  : (Real-time Committal record deletion)
|---------------------------------------------------------------------|
|  Date Written  : (12/05/94)      | Author        : Campbell Mander. |
|---------------------------------------------------------------------|
| $Log: cmmt_del.c,v $
| Revision 5.3  2001/10/05 02:52:08  cha
| This program has to be changed to cater for new
| allocation type defined for goods returns.
|
| Revision 5.2  2001/08/20 23:45:29  scott
| Updated for development related to bullet proofing
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cmmt_del.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_cmmt_del/cmmt_del.c,v 5.3 2001/10/05 02:52:08 cha Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <proc_sobg.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct soicRecord	soic_rec;
struct inlaRecord	inla_rec;
struct solnRecord	soln_rec;
struct colnRecord	coln_rec;
struct cohrRecord	cohr_rec;
struct poglRecord	pogl_rec;

	char	*data = "data";

	int		printerNo;
	int		auditOpen = FALSE;

	FILE	*fout;


/*======================= 
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	Process 			(void);
void 	OpenAudit 			(void);
void 	CloseAudit 			(void);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 2)	
	{
		print_at (0,0,mlStdMess036,argv [0]);
		return (EXIT_FAILURE);
	}

	printerNo = atoi (argv [1]);

	init_scr ();
	set_tty (); 

	OpenDB ();

	Process ();

	recalc_sobg ();

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
	if (auditOpen)
		CloseAudit ();

	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (soic, soic_list, SOIC_NO_FIELDS, "soic_id_no");
	open_rec (inla, inla_list, INLA_NO_FIELDS, "inla_pid");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_hhcl_hash");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_hhco_hash");
	open_rec (pogl, pogl_list, POGL_NO_FIELDS, "pogl_hhgl_hash");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhsl_hash");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (ccmr);
	abc_fclose (inmr);
	abc_fclose (soic);
	abc_fclose (inla);
	abc_fclose (coln);
	abc_fclose (cohr);
	abc_fclose (pogl);
	abc_fclose (soln);
	abc_dbclose (data);
}

/*------------------------
| Process relevent Data. |
------------------------*/
void
Process (
 void)
{
	int		cnt;
	char	cntStr [10];

	dsp_screen (" Deletion Of Real-time Committal Records ",
				comm_rec.co_no,
				comm_rec.co_name);

	/*----------------------
	| Delete soic records. |
	----------------------*/
	cnt = 0;
	strcpy (soic_rec.status, " ");
	soic_rec.pid  = 0L;
	soic_rec.line = 0;
	cc = find_rec (soic, &soic_rec, GTEQ, "u");
	while (!cc)
	{
		/*--------------------------------------------
		| Print to audit if deleting an active soic. |
		--------------------------------------------*/
		if (soic_rec.status [0] == 'A')
		{
			/*-----------------
			| Print to audit. |
			-----------------*/
			if (!auditOpen)
			{
				OpenAudit ();
				auditOpen = TRUE;
			}

			/*-------------------
			| Find inmr record. |
			-------------------*/
			inmr_rec.hhbr_hash	=	soic_rec.hhbr_hash;
			cc = find_rec (inmr,&inmr_rec, COMPARISON,"r");
			if (cc)
			{
				sprintf (inmr_rec.item_no,     "%-16.16s", "UNKNOWN");
				sprintf (inmr_rec.description, "%-40.40s", "UNKNOWN");
			}

			/*-------------------
			| Find ccmr record. |
			-------------------*/
			ccmr_rec.hhcc_hash	=	soic_rec.hhcc_hash;
			cc = find_rec (ccmr,&ccmr_rec, COMPARISON,"r");
			if (cc)
			{
				strcpy (ccmr_rec.co_no,  "??");
				strcpy (ccmr_rec.est_no, "??");
				strcpy (ccmr_rec.cc_no,  "??");
			}
		
			fprintf (fout, "| %-16.16s ", inmr_rec.item_no);
			fprintf (fout, "| %-40.40s ", inmr_rec.description);
			fprintf (fout, "| %2.2s / %2.2s / %2.2s ", 
						 ccmr_rec.co_no, ccmr_rec.est_no, ccmr_rec.cc_no);
			fprintf (fout, "| %8.2f ",    soic_rec.qty);
			fprintf (fout, "|%-10.10s",   DateToString (soic_rec.date_create));
			fprintf (fout, "| %8.8s ",    ttoa (soic_rec.time_create, "HH:MM"));
			fprintf (fout, "| %-14.14s ", soic_rec.op_id);
			fprintf (fout, "| %-14.14s ", soic_rec.program);
			fprintf (fout, "| %10ld |\n", soic_rec.pid);

			fflush (fout);

			/*------------------------
			| Add an RC sobg record. |
			------------------------*/
			add_hash 
			(
				ccmr_rec.co_no,
				ccmr_rec.est_no,
				"RC",
				0,
				soic_rec.hhbr_hash,
				soic_rec.hhcc_hash,
				soic_rec.pid,
				0.00
			);
		}

		cnt++;
		sprintf (cntStr, "%d", cnt);
		dsp_process ("Record : ", cntStr);

		/*---------------------
		| Delete soic record. |
		---------------------*/
		cc = abc_delete (soic);
		if (cc)
			file_err (cc, soic, "DBDELETE");

		strcpy (soic_rec.status, " ");
		soic_rec.pid  = 0L;
		soic_rec.line = 0;
		cc = find_rec (soic, &soic_rec, GTEQ, "u");
	}
	abc_unlock (soic);

	/*--------------------------------------------------------
	| Delete out all allocated records from runtime screens. |
	--------------------------------------------------------*/
	inla_rec.pid	=	0L;
	cc = find_rec (inla, &inla_rec, GTEQ, "u");
	while (!cc)
	{
		if 
		(
			inla_rec.hhcl_hash == 0L &&
			inla_rec.hhsl_hash == 0L &&
			inla_rec.cmrd_hash == 0L &&
			inla_rec.itff_hash == 0L &&
			inla_rec.pcms_hash == 0L &&
			inla_rec.hhgl_hash == 0L
		)
		{
			abc_delete (inla);
			cc = find_rec (inla, &inla_rec, GTEQ, "u");
		}
		else
		{
			abc_unlock (inla);
			cc = find_rec (inla, &inla_rec, NEXT, "u");
		}
	}
	abc_unlock (inla);
	/*-------------------------------------------------------------
	| Delete out all allocated records from invoices not on file. |
	-------------------------------------------------------------*/
	abc_selfield (inla, "inla_hhcl_id");
	inla_rec.hhcl_hash	=	1L;
	inla_rec.inlo_hash	=	0L;
	cc = find_rec (inla, &inla_rec, GTEQ, "u");
	while (!cc && inla_rec.hhcl_hash != 0L)
	{
		coln_rec.hhcl_hash	=	inla_rec.hhcl_hash;
		cc = find_rec (coln, &coln_rec, COMPARISON, "r");
		if (!cc)
		{
			cohr_rec.hhco_hash	=	coln_rec.hhco_hash;
			cc = find_rec (cohr, &cohr_rec, COMPARISON, "r");
			if (!cc)
			{
				if (cohr_rec.stat_flag [0] == 'D' ||
					cohr_rec.stat_flag [0] == '9' ||
					cohr_rec.stat_flag [0] == '1')
				{
					abc_delete (inla);
					cc = find_rec (inla, &inla_rec, GTEQ, "u");
					continue;
				}
			}
		}
		else
		{
			abc_delete (inla);
			cc = find_rec (inla, &inla_rec, GTEQ, "u");
			continue;
		}
		cc = find_rec (inla, &inla_rec, NEXT, "u");
	}
	abc_unlock (inla);

	/*-----------------------------------------------------------
	| Delete out all allocated records from orders not on file. |
	-----------------------------------------------------------*/
	abc_selfield (inla, "inla_hhsl_id");
	inla_rec.hhsl_hash	=	1L;
	inla_rec.inlo_hash	=	0L;
	cc = find_rec (inla, &inla_rec, GTEQ, "u");
	while (!cc && inla_rec.hhsl_hash != 0L)
	{
		soln_rec.hhsl_hash	=	inla_rec.hhsl_hash;
		cc = find_rec (soln, &soln_rec, COMPARISON, "r");
		if (cc)
		{
			abc_delete (inla);
			cc = find_rec (inla, &inla_rec, GTEQ, "u");
		}
		else
		{
			abc_unlock (inla);
			cc = find_rec (inla, &inla_rec, NEXT, "u");
		}
	}
	abc_unlock (inla);
}

/*--------------------
| Open audit report. |
--------------------*/
void
OpenAudit (
 void)
{
	if ((fout = popen ("pformat", "w")) == NULL) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".LP%d\n", printerNo);
	fprintf (fout, ".11\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".EDELETE REAL-TIME COMMITTAL RECORDS\n");
 	fprintf (fout, ".B1\n");

	fprintf (fout, ".EAUDIT OF ACTIVE RECORDS DELETED\n");
	fprintf (fout, ".EAS AT %s\n", SystemTime ());

	fprintf (fout, 
			 ".ECOMPANY : %s - %s\n",
			 clip (comm_rec.co_no),
			 clip (comm_rec.co_name));
 	fprintf (fout, ".B1\n");
 
	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "===============");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "=================");
	fprintf (fout, "=================");
	fprintf (fout, "==============\n");

	fprintf (fout, "|   ITEM NUMBER    ");
	fprintf (fout, "|            ITEM DESCRIPTION              ");
	fprintf (fout, "| CO / BR / WH ");
	fprintf (fout, "| QUANTITY ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|   TIME   ");
	fprintf (fout, "|    OPERATOR    ");
	fprintf (fout, "|    PROGRAM     ");
	fprintf (fout, "| PROCESS ID |\n");

	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|--------------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|------------|\n");

	fprintf (fout, ".R===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "===============");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "=================");
	fprintf (fout, "=================");
	fprintf (fout, "==============\n");
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
void
CloseAudit (
 void)
{
	fprintf (fout,".EOF\n");
	pclose (fout);
}
