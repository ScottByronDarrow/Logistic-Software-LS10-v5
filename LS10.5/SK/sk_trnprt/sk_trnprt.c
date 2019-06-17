/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_trnprt.c,v 5.3 2001/11/07 07:34:43 scott Exp $
|  Program Name  : (sk_trnprt.c)                                      |
|  Program Desc  : (Print-out flagged delivery dockets.       )       |
|---------------------------------------------------------------------|
|  Author        : Trevor van Bremen     | Date Written : 06/03/91    |
|---------------------------------------------------------------------|
| $Log: sk_trnprt.c,v $
| Revision 5.3  2001/11/07 07:34:43  scott
| Updated to remove gets and replace with scanf
|
| Revision 5.2  2001/08/09 09:20:24  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:46:09  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:18:15  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:39:33  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.3  2000/12/22 06:56:47  ramon
| Updated to correct the errors when compiled in LS10-GUI.
|
| Revision 3.2  2000/11/22 07:34:19  scott
| Updated to fix select field error on comr. Due to automatic update.
|
| Revision 3.1  2000/11/20 07:40:38  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:21:34  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:12:19  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  2000/01/24 21:22:50  cam
| Changes for GVision compatibility.  Added conditional compile code for use of
| Remote Files.
|
| Revision 1.12  1999/11/18 05:25:28  scott
| Updated from gvision testing
|
| Revision 1.11  1999/10/13 02:42:20  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.10  1999/10/08 05:33:02  scott
| First Pass checkin by Scott.
|
| Revision 1.9  1999/06/20 05:20:57  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
| Revision 1.9  1999/06/20 05:20:57  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_trnprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_trnprt/sk_trnprt.c,v 5.3 2001/11/07 07:34:43 scott Exp $";

#include	<ml_sk_mess.h>
#include	<ml_std_mess.h>
#include	<pslscr.h>
#include	<twodec.h>
#include	<LocHeader.h>

#ifdef GVISION
#include <RemoteFile.h>
#define	pr_open	Remote_pr_open
#define	fgets	Remote_fgets
#define	fclose	Remote_fclose
#endif	/* GVISION */

#define		SERIAL_ITEM	 (inmr_rec.serial_item [0] == 'Y')
#define		DIFF_REF	 (strcmp (ithr_rec.tran_ref, itln_rec.tran_ref))

#define		ZERO_LINE	 (itln_rec.qty_order == 0.00 && \
				  		  itln_rec.qty_border == 0.00)

#define		NON_STOCK	 (inmr_rec.inmr_class [0] == 'Z')

#define		PHANTOM		 (inmr_rec.inmr_class [0] == 'P')

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct esmrRecord	esmr_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inccRecord	incc_rec;
struct ithrRecord	ithr_rec;
struct itlnRecord	itln_rec;
struct inumRecord	inum_rec;
struct soktRecord	sokt_rec;
struct sokdRecord	sokd_rec;
struct itloRecord	itlo_rec;

char	iss_co [3],
		iss_coname [41],
		iss_coshort [16],
		iss_co_adr [3] [41],
		iss_br [3],
		iss_brname [41],
		iss_brshort [16],
		iss_br_adr [3] [41],
		iss_wh [3],
		iss_whname [41],
		iss_whshort [10],
		iss_whadd1 [41],
		iss_whadd2 [41],
		iss_whadd3 [41];

char	rec_co [3],
		rec_coname [41],
		rec_coshort [16],
		rec_co_adr [3] [41],
		rec_br [3],
		rec_br_adr [3] [41],
		rec_brname [41],
		rec_brshort [16],
		rec_wh [3],
		rec_whname [41],
		rec_whshort [10],
		rec_whadd1 [41],
		rec_whadd2 [41],
		rec_whadd3 [41];

FILE	*fin,
		*fout;

int		part_printed = FALSE;
int		line_spacing = 1;
int		line_no;

char	format_line [3] [151];

struct
{
	char	item_desc [41];
	float	qty_ord;
	float	qty_bord;
	char	serial_no [26];
	char	location [11];
	char	uom [5];
	long	j_date;
	char	systemDate [11];
	char	sys_time [6];
	char	message [41];
	char	full_supply [31];
	int	printerNumber;
} local_rec;

#include <pr_format3.h>
#include <SkIrCodes.c>

/*======================= 
| Function Declarations |
=======================*/
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ProcessLines 	(long);
void 	CheckSpacing 	(int);
void 	ProcessPhantom 	(void);
void 	OpenAudit 		(void);
void 	CloseAudit 		(void);
void 	ReadCcmr 		(void);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	long	hhitHash;

	if (argc < 2)
	{
		print_at (0,0,"Usage : %s <printerNumber>",argv [0]);
		return (EXIT_FAILURE);
	}

	local_rec.printerNumber = atoi (argv [1]);

	OpenDB ();


	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	
	while (scanf ("%ld", &hhitHash) != EOF)
	{
		sprintf (local_rec.sys_time, "%-5.5s", TimeHHMM ());
		ProcessLines (hhitHash);
	}
	
	CloseDB (); 
	FinishProgram ();
    return (EXIT_SUCCESS);
}

/*========================	
| Open ALL open tables.  |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (ithr, ithr_list, ITHR_NO_FIELDS, "ithr_hhit_hash");
	open_rec (itln, itln_list, ITLN_NO_FIELDS, "itln_id_no");
	open_rec (sokt, sokt_list, SOKT_NO_FIELDS, "sokt_id_no");
	open_rec (sokd, sokd_list, SOKD_NO_FIELDS, "sokd_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	
	ReadCcmr ();

	OpenLocation (ccmr_rec.hhcc_hash);

	open_rec (itlo, itlo_list, ITLO_NO_FIELDS, "itlo_id_no");
	
	abc_selfield (ccmr, "ccmr_hhcc_hash");
}

/*========================	
| Close ALL open tables. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (comr);
	abc_fclose (esmr);
	abc_fclose (ccmr);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (ithr);
	abc_fclose (itln);
	abc_fclose (inum);
	abc_fclose (sokt);
	abc_fclose (sokd);
	abc_fclose (itlo);
	CloseLocation ();
	abc_dbclose ("data");
}

/*===============================
| Hair's da gutz ov da program	|
===============================*/
void
ProcessLines (
	long	hhitHash)
{
	int		first_time = TRUE;
	int		i;
	int		NoAllocation	= TRUE;
	int		loopCount		= 0;
	float	cnv_fct			= 0.00;
	float	std_cnv_fct		= 0.00;

	ithr_rec.hhit_hash	=	hhitHash;
	cc = find_rec (ithr, &ithr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ithr, "DBFIND");

	if (ithr_rec.type [0] == 'T')
	{
		if (ithr_rec.printed [0] == 'Y')
			strcpy (local_rec.message, "TRANSFER DISPATCH    ** REPRINT **");
		else
			strcpy (local_rec.message, "TRANSFER DISPATCH                 ");
	}
	if (ithr_rec.type [0] == 'R')
	{
		if (ithr_rec.printed [0] == 'Y')
			strcpy (local_rec.message, "REQUEST FOR TRANSFER ** REPRINT **");
		else
			strcpy (local_rec.message, "REQUEST FOR TRANSFER              ");
	}
	if (ithr_rec.type [0] == 'U')
	{
		if (ithr_rec.printed [0] == 'Y')
			strcpy (local_rec.message, "TWO STEP ISSUE           *REPRINT*");
		else
			strcpy (local_rec.message, "TWO STEP ISSUE                    ");
	}
	if (ithr_rec.type [0] == 'M')
	{
		if (ithr_rec.printed [0] == 'Y')
			strcpy (local_rec.message, "STOCK TRANSFER           *REPRINT*");
		else
			strcpy (local_rec.message, "STOCK TRANSFER                    ");
	}
	if (ithr_rec.full_supply [0] == 'Y')
		strcpy (local_rec.full_supply, "**** FULL SUPPLY TRANSFER ****");
	else
		sprintf (local_rec.full_supply, "%30.30s"," ");

	itln_rec.hhit_hash 	= hhitHash;
	itln_rec.line_no 	= 0;
	cc = find_rec (itln, &itln_rec, GTEQ, "r");
	while (!cc && itln_rec.hhit_hash == hhitHash)
	{
		inmr_rec.hhbr_hash	=	itln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inmr, "DBFIND");

		sprintf (local_rec.item_desc, "%-40.40s", itln_rec.item_desc);
		strcpy (local_rec.serial_no, itln_rec.serial_no);
		local_rec.qty_ord 		= itln_rec.qty_order;
		local_rec.qty_bord 		= itln_rec.qty_border;
		incc_rec.hhcc_hash 	= itln_rec.i_hhcc_hash;
		incc_rec.hhbr_hash 	= itln_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, EQUAL, "r");
		if (cc)
			file_err (cc, incc, "DBFIND");

		if (MULT_LOC)
			strcpy (local_rec.location, "SEE BELOW ");
		else
			strcpy (local_rec.location, incc_rec.location);

		local_rec.j_date 	= ithr_rec.iss_date;

		if (first_time)
			OpenAudit ();

		first_time = FALSE;

		if (ZERO_LINE && !NON_STOCK)
		{
			cc = find_rec (itln, &itln_rec, NEXT, "r");
			continue;
		}
		/*-------------------------------
		| Validate the unit of measure. |
		-------------------------------*/
		inum_rec.hhum_hash = inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inum, "DBFIND");

		std_cnv_fct	=	inum_rec.cnv_fct;
		strcpy (local_rec.uom, inum_rec.uom);

		inum_rec.hhum_hash	=	itln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inum, "DBFIND");

		if (std_cnv_fct == 0.00)
			std_cnv_fct = 1;

		cnv_fct 		= inum_rec.cnv_fct/std_cnv_fct;

		parse (clip (format_line [0]));

		NoAllocation	=	TRUE;
		/*-----------------------------------
		| We have the potential of having   |
		| more then one location so we have |
		| to ProcessLines each one.         |
		-----------------------------------*/
		itlo_rec.itff_hash	=	itln_rec.itff_hash;
		itlo_rec.pr_line	=	itln_rec.line_no;
		itlo_rec.line_no	=	0;
		loopCount = 0;
		for (cc = find_rec (itlo, &itlo_rec, GTEQ, "r");
			 !cc &&
			  itlo_rec.itff_hash ==	itln_rec.itff_hash &&
			  itlo_rec.pr_line	==	itln_rec.line_no;
			 cc = find_rec (itlo, &itlo_rec, NEXT, "r"), loopCount++)
		{
			NoAllocation	=	FALSE;

			local_rec.qty_ord = itlo_rec.qty * itlo_rec.l_cnv_fct;

			/*--------------------------------
			| Store location specific values |
			--------------------------------*/
			strcpy (local_rec.location, itlo_rec.location);
			strcpy (local_rec.uom, itlo_rec.i_uom);

			if (loopCount == 1)
			{
				if (SERIAL_ITEM || DIFF_REF)
				{
					if (!SERIAL_ITEM)
						sprintf (local_rec.serial_no, "%-25.25s", " ");
		
					if (!DIFF_REF)
						sprintf (itln_rec.tran_ref, "%-16.16s", " ");
		
					parse (clip (format_line [1]));
					CheckSpacing (TRUE);
				}
				else
				{
					parse (clip (format_line [2]));
					CheckSpacing (FALSE);
				}
			}
			else
			{
				parse (clip (format_line [2]));
				CheckSpacing (FALSE);
			}
		}
		
		if (NoAllocation)
		{
			cc = DisplayLL
			 (									/*----------------------*/
				itln_rec.line_no,				/*	Line number.		*/
				1,								/*  Row for window		*/
				10,								/*  Col for window		*/
				4,								/*  length for window	*/
				incc_rec.hhwh_hash,				/*	Warehouse hash.		*/
				itln_rec.hhum_hash,				/*	UOM hash			*/
				itln_rec.i_hhcc_hash,			/*	CC hash.			*/
				inum_rec.uom,					/* UOM					*/
				itln_rec.qty_order,				/* Quantity.			*/
				cnv_fct,						/* Conversion factor.	*/
				StringToDate (ttod ()),			/* Expiry Date.			*/
				TRUE,							/* Silent mode			*/
				FALSE,           				/* Input Mode.			*/
				inmr_rec.lot_ctrl				/* Lot controled item. 	*/
												/*----------------------*/
			);

			for (i = 0; i < MAX_LOTS; i++)
			{
				if (GetHHWH (itln_rec.line_no, i) == 0L)
					break;

				strcpy (local_rec.location, GetLoc (itln_rec.line_no, i));
				local_rec.qty_ord = GetBaseQty (itln_rec.line_no, i);
				strcpy (local_rec.location, GetLoc (itln_rec.line_no, i));
				strcpy (local_rec.uom, GetUOM (itln_rec.line_no, i));

				parse (clip (format_line [2]));
				CheckSpacing (FALSE);

				if (SERIAL_ITEM || DIFF_REF)
				{
					if (!SERIAL_ITEM)
						sprintf (local_rec.serial_no, "%-25.25s", " ");
		
					if (!DIFF_REF)
						sprintf (itln_rec.tran_ref, "%-16.16s", " ");
		
					parse (clip (format_line [1]));
					CheckSpacing (TRUE);
				}
			}
		}
		if (PHANTOM)
			ProcessPhantom ();

		cc = find_rec (itln, &itln_rec, NEXT, "r");
	}

	ithr_rec.hhit_hash	=	hhitHash;
	cc = find_rec (ithr, &ithr_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, ithr, "DBFIND");

	strcpy (ithr_rec.printed, "Y");
	cc = abc_update (ithr, &ithr_rec);
	if (cc)
		file_err (cc, ithr, "DBUPDATE");

	if (!first_time)
		CloseAudit ();
}

void
CheckSpacing (
 int status)
{
	int	no_lines;

	if (status)
		no_lines = 2;
	else
		no_lines = 1;

	if (line_spacing > no_lines)
		fprintf (fout, ".B%d\n", line_spacing - no_lines);
}

/*-------------------------------
| Process BOM for phantom items |
-------------------------------*/
void
ProcessPhantom (
 void)
{
	int	tmp_cnt;

	line_no = 0;

	/*-----------
	| Print BOM |
	-----------*/
	strcpy (sokt_rec.co_no, ithr_rec.co_no);
	sokt_rec.hhbr_hash = inmr_rec.hhbr_hash;
	sokt_rec.line_no = 0;
	cc = find_rec ("sokt", &sokt_rec, GTEQ, "r");
	while (!cc && !strcmp (sokt_rec.co_no, ithr_rec.co_no) &&
	       sokt_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		line_no++;
		inmr2_rec.hhbr_hash	=	sokt_rec.mabr_hash;
		cc = find_rec (inmr, &inmr2_rec, COMPARISON, "r");
		if (cc)
		{
			fprintf (fout, "%-34.34sITEM NOT FOUND ON FILE\n", " ");
			cc = find_rec ("sokt", &sokt_rec, NEXT, "r");
			continue;
		}

		fprintf (fout, "              %-16.16s",inmr2_rec.item_no);
		fprintf (fout, "    %-35.35s",inmr2_rec.description);
		fprintf (fout, "          %6.2f\n",
			itln_rec.qty_order * sokt_rec.matl_qty);

		cc = find_rec ("sokt", &sokt_rec, NEXT, "r");
	}

	/*---------------------------
	| Print Instruction Details |
	---------------------------*/
	strcpy (sokd_rec.co_no, ithr_rec.co_no);
	strcpy (sokd_rec.type,  "P");
	sokd_rec.hhbr_hash = inmr_rec.hhbr_hash;
	sokd_rec.line_no = 0;
	cc = find_rec ("sokd", &sokd_rec, GTEQ, "r");
	while (!cc && !strcmp (sokd_rec.co_no, ithr_rec.co_no) &&
	        sokd_rec.hhbr_hash == inmr_rec.hhbr_hash &&
	        sokd_rec.type [0] == 'P')
	{
		line_no++;
		fprintf (fout, "               %-40.40s\n",sokd_rec.text);
		cc = find_rec ("sokd", &sokd_rec, NEXT, "r");
	}

	if (line_spacing > 1)
	{
		tmp_cnt = line_no % line_spacing;
		if (tmp_cnt != 0)
			fprintf (fout, ".B%d\n", line_spacing - tmp_cnt);
	}
}

/*==========================================================================
| These are functions for the audit printing.                              |
|                                                                          |
| Routine to open output pipe to standard print to provide an audit trail  |
| of events. This also sends the output straight to the spooler.           |
==========================================================================*/
void
OpenAudit (
 void)
{
	char	*sptr;
	char	_line [300];

	ccmr_rec.hhcc_hash = itln_rec.i_hhcc_hash;
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	strcpy (esmr_rec.co_no, ccmr_rec.co_no);
	strcpy (esmr_rec.est_no, ccmr_rec.est_no);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");

	strcpy (comr_rec.co_no, ccmr_rec.co_no);
	cc = find_rec (comr, &comr_rec, EQUAL, "r");
	if (cc)
		sys_err ("Error in comr During (DBFIND)", cc, PNAME);

	strcpy (iss_co,			comr_rec.co_no);
	strcpy (iss_coname,		comr_rec.co_name);
	strcpy (iss_coshort,	comr_rec.co_short_name);
	strcpy (iss_co_adr [0],	comr_rec.co_adr1);
	strcpy (iss_co_adr [1],	comr_rec.co_adr2);
	strcpy (iss_co_adr [2],	comr_rec.co_adr3);
	strcpy (iss_br,			esmr_rec.est_no);
	strcpy (iss_brname,		esmr_rec.est_name);
	strcpy (iss_brshort,	esmr_rec.short_name);
	strcpy (iss_br_adr [0],	esmr_rec.adr1);
	strcpy (iss_br_adr [1],	esmr_rec.adr2);
	strcpy (iss_br_adr [2],	esmr_rec.adr3);
	strcpy (iss_wh,			ccmr_rec.cc_no);
	strcpy (iss_whname,		ccmr_rec.name);
	strcpy (iss_whshort,	ccmr_rec.acronym);
	strcpy (iss_whadd1,		ccmr_rec.whse_add1);
	strcpy (iss_whadd2,		ccmr_rec.whse_add2);
	strcpy (iss_whadd3,		ccmr_rec.whse_add3);

	ccmr_rec.hhcc_hash	=	itln_rec.r_hhcc_hash;
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	strcpy (esmr_rec.co_no, ccmr_rec.co_no);
	strcpy (esmr_rec.est_no, ccmr_rec.est_no);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");

	strcpy (comr_rec.co_no, ccmr_rec.co_no);
	cc = find_rec (comr, &comr_rec, EQUAL, "r");
	if (cc)
		sys_err ("Error in comr During (DBFIND)", cc, PNAME);

	strcpy (rec_co,		comr_rec.co_no);
	strcpy (rec_coname,	comr_rec.co_name);
	strcpy (rec_coshort,	comr_rec.co_short_name);
	strcpy (rec_co_adr [0],	comr_rec.co_adr1);
	strcpy (rec_co_adr [1],	comr_rec.co_adr2);
	strcpy (rec_co_adr [2],	comr_rec.co_adr3);
	strcpy (rec_br,		esmr_rec.est_no);
	strcpy (rec_brname,	esmr_rec.est_name);
	strcpy (rec_brshort,	esmr_rec.short_name);
	strcpy (rec_br_adr [0],	esmr_rec.adr1);
	strcpy (rec_br_adr [1],	esmr_rec.adr2);
	strcpy (rec_br_adr [2],	esmr_rec.adr3);
	strcpy (rec_wh,		ccmr_rec.cc_no);
	strcpy (rec_whname,	ccmr_rec.name);
	strcpy (rec_whshort,	ccmr_rec.acronym);
	strcpy (rec_whadd1,		ccmr_rec.whse_add1);
	strcpy (rec_whadd2,		ccmr_rec.whse_add2);
	strcpy (rec_whadd3,		ccmr_rec.whse_add3);

	if ((fout = popen ("pformat", "w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	if ((fin = pr_open ("sk_trnprt.p")) == 0) 
		sys_err ("Error in sk_trnprt.p During (FOPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.printerNumber);
	fprintf (fout, ".OP\n");

	sptr = fgets (_line, 300, fin);
	while (sptr != (char *) 0 && !feof (fin))
	{
		if (!strncmp (sptr, ".END_USER", 9))
			break;

		* (sptr + strlen (sptr) - 1) = 0;
		parse (clip (sptr));
		sptr = fgets (sptr, 300, fin);
	}
	fclose (fin);
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
 void
CloseAudit (
 void)
{
	fprintf (fout, ".EOF\n");
	pclose (fout);
}

void
ReadCcmr (
 void)
{
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, (char *)ccmr, "DBFIND"); 
}

/*-----------------------------------------
| sk_ir_code2.c changed from sk_ir_code.c |
| so as the program can print upto 6      |
| decimal places for the order, backorder |
| and supplied quantities.                |
-----------------------------------------*/
