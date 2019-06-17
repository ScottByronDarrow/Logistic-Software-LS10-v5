/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: pc_chk_iss.c,v 5.5 2001/12/11 08:39:23 scott Exp $
|  Program Name  : (pc_chk_iss.c)
|  Program Desc  : (Production Control Issue Checking)
|---------------------------------------------------------------------|
|  Date Written  : 11/02/92        | Author       : Campbell Mander.  |
|---------------------------------------------------------------------|
| $Log: pc_chk_iss.c,v $
| Revision 5.5  2001/12/11 08:39:23  scott
| Updated to fix core dump of no pcms
|
| Revision 5.4  2001/11/08 05:18:52  scott
| Added <arralloc.h>
|
| Revision 5.3  2001/11/07 04:52:11  scott
| Updated to convert to app.schema
| Updated to replace fgets with scanf
| Updated to clean code and remove disk write with memory sort.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_chk_iss.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_chk_iss/pc_chk_iss.c,v 5.5 2001/12/11 08:39:23 scott Exp $";

#include	<pslscr.h>
#include	<proc_sobg.h>
#include 	<twodec.h>
#include 	<ml_std_mess.h>
#include 	<ml_pc_mess.h>
#include 	<arralloc.h>

#ifdef GVISION
#include <RemoteFile.h>
#include <RemotePipe.h>
#define	popen	Remote_popen
#define	pclose	Remote_pclose
#define	fprintf	Remote_fprintf
#endif	/* GVISION */

FILE	*pout;
FILE	*fout;

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct ineiRecord	inei_rec;
struct inumRecord	inum_rec;
struct pchcRecord	pchc_rec;
struct pcmsRecord	pcms_rec;
struct pcwoRecord	pcwo_rec;
struct rghrRecord	rghr_rec;
struct rglnRecord	rgln_rec;
struct inccRecord	incc_rec;
struct inloRecord	inlo_rec;

	char	*data	= "data";

	int		printerNo		= 0,
			checkSequence	= 0,
			qCreate			= FALSE,	
			firstTime		= TRUE,
			firstRound		= TRUE,
			pformatOpen		= FALSE,
			printOk			= FALSE,
			printLot		= FALSE,
			printBin		= FALSE,
			printReq		= FALSE;

/*
 *	Structure for dynamic array,  for the lotRec lines for qsort
 */
struct LotStruct
{
	char	lotNo [8];
	long	expiryDate;
	char	location [11];
	float	qty;
}	*lotRec;
	DArray lot_details;
	int	lotCnt = 0;

/*
 * function prototypes
 */

void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ProcessPcms 	(long, int);
void 	HeadingOutput 	(void);
int		LotSort			(const	void *,	const void *);
int 	Process 		(void);

/*
 * Main Processing Routine.
 */
int
main (
	int		argc, 
	char	*argv[])
{
	if (argc < 7)
	{
		print_at (0,0, mlPcMess702, argv [0]);
		print_at (1,0, mlPcMess703);
		print_at (2,0, mlPcMess704);
		print_at (3,0, mlPcMess705);
		print_at (4,0, mlPcMess706);
		print_at (5,0, mlPcMess707);

		return (EXIT_FAILURE);

	}

	printerNo 		= atoi (argv [1]);
	checkSequence 	= atoi (argv [2]);	
	if (argv [3] [0] == 'Y')  /* print picking list or not */
		printOk = TRUE;
	else
		printOk = FALSE;

	if (argv [4] [0] == 'Y')  /* print lot details or not */
		printLot = TRUE;
	else
		printLot = FALSE;

	if (argv [5] [0] == 'Y')  /* print bin location details or not */
		printBin = TRUE;
	else
		printBin = FALSE;

	if (argv [6] [0] == 'Y')  /* print the required amount only or not */
		printReq = TRUE;
	else
		printReq = FALSE;

	/*
	 * Setup required parameters
	 */
	init_scr ();

	OpenDB ();

	Process ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (pchc,  pchc_list, PCHC_NO_FIELDS, "pchc_id_no");
	open_rec (pcms,  pcms_list, PCMS_NO_FIELDS, "pcms_id_no");
	open_rec (pcwo,  pcwo_list, PCWO_NO_FIELDS, "pcwo_hhwo_hash");
	open_rec (rghr,  rghr_list, RGHR_NO_FIELDS, "rghr_id_no");
	open_rec (rgln,  rgln_list, RGLN_NO_FIELDS, "rgln_id_no");
	open_rec (incc,	 incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inlo,  inlo_list, INLO_NO_FIELDS, "inlo_id_exp");
}

/*
 * Close data base files
 */
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (inei);
	abc_fclose (inum);
	abc_fclose (pchc);
	abc_fclose (pcms);
	abc_fclose (pcwo);
	abc_fclose (rghr);
	abc_fclose (rgln);
	abc_fclose (incc);
	abc_fclose (inlo);
	abc_dbclose (data);
}

int
Process (void)
{
	long	hhwoHash	= 0L;

	pformatOpen = FALSE;

	while (scanf ("%ld", &hhwoHash) != EOF)
	{
		pcwo_rec.hhwo_hash	=	hhwoHash;
		cc = find_rec (pcwo, &pcwo_rec, EQUAL, "r");
		if (cc || (	pcwo_rec.order_status [0] != 'I' &&
					pcwo_rec.order_status [0] != 'R'))
		{
			abc_unlock (pcwo);
			continue;
		}
		strcpy (rghr_rec.co_no, pcwo_rec.co_no);
		strcpy (rghr_rec.br_no, pcwo_rec.br_no);
		rghr_rec.hhbr_hash 	= pcwo_rec.hhbr_hash;
		rghr_rec.alt_no 	= pcwo_rec.rtg_alt;
		cc = find_rec (rghr, &rghr_rec, EQUAL, "r");
		if (cc)
			strcpy (rghr_rec.print_all, "A");

		rgln_rec.hhgr_hash 	= rghr_rec.hhgr_hash;
		rgln_rec.seq_no 	= pcwo_rec.rtg_seq;
		cc = find_rec (rgln, &rgln_rec, GTEQ, "r");
		if (cc)
			cc = find_rec (rgln, &rgln_rec, NEXT, "r");

		if (cc || rgln_rec.hhgr_hash != rghr_rec.hhgr_hash)
			rgln_rec.seq_no = 32768;

		qCreate = FALSE;
		if (checkSequence == 0 && pcwo_rec.order_status [0] == 'I')
			qCreate = TRUE;

		/*
		 * Process hhwo hash
		 */
		ProcessPcms (hhwoHash, rgln_rec.seq_no);

		/*
		 * W/O is currently I(ssuing) and no materials
		 * need picking so pass hhwo hash to pc_qcreat
		 */
		if (qCreate)
		{
			if ((pout = popen ("pc_qcreat", "w")) == NULL)
				file_err (errno, "pc_qcreat", "POPEN");

			/*
			 * pass hhwo hash to pc_qcreat
			 */
			fprintf (pout, "%010ld\n", hhwoHash);

#ifdef GVISION
			Remote_fflush (pout);
#else
			fflush (pout);
#endif
			pclose (pout);
		}
	}

	if (pformatOpen == TRUE)
	{
		fprintf (fout, ".EOF\n");
#ifdef GVISION
		Remote_fflush (pout);
#else
		fflush (pout);
#endif
		pclose (fout);
	}

	return (EXIT_SUCCESS);
}

/*
 * Process All pcms records for current hhwo hash.
 */
void
ProcessPcms (
	long 	hhwoHash, 
	int		seqNo)
{
	float	qty	= 0.00;
	int		i;

	firstTime = TRUE;

	pcms_rec.hhwo_hash 	= hhwoHash;
	pcms_rec.uniq_id 	= 0;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "r");
	while (!cc && pcms_rec.hhwo_hash == hhwoHash)
	{
		if (pcms_rec.act_qty_in [0] == 'Y')
		{
			cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			continue;
		}
		if (pcms_rec.iss_seq != seqNo && rghr_rec.print_all [0] == 'N')
		{
			cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			continue;
		}
		if (firstTime && printOk)
			HeadingOutput ();
		/*
		 * Lookup inmr for component
		 */
		inmr_rec.hhbr_hash	=	pcms_rec.mabr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			continue;
		}
		/*
		 * Lookup UOM
		 */
		inum_rec.hhum_hash	= pcms_rec.uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
			strcpy (inum_rec.uom, "????");
		/*
		 * Lookup inei for component
		 */
		inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (inei_rec.est_no, comm_rec.est_no);
		cc = find_rec (inei, &inei_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			continue;
		}
		/*
		 * Lookup Hazard Class
		 */
		strcpy (pchc_rec.co_no, comm_rec.co_no);
		strcpy (pchc_rec.type, "Z");
		sprintf (pchc_rec.pchc_class, "%-4.4s", inei_rec.hzrd_class);
		cc = find_rec (pchc, &pchc_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (pchc_rec.pchc_class, " ");
			strcpy (pchc_rec.desc,   " ");
		}
		if (pcms_rec.iss_seq == 0)
			qCreate = FALSE;

		if (!printOk)
		{
			cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			continue;
		}
		/*
		 * find all lot and bin location details.
		 */
		pcms_rec.matl_wst_pc += 100.00;
		pcms_rec.matl_wst_pc /= 100.00;
		pcms_rec.matl_qty *= pcms_rec.matl_wst_pc;

		fprintf (fout, "|%2d",		pcms_rec.iss_seq);
		fprintf (fout, "|%-16.16s",	inmr_rec.item_no);
		fprintf (fout, "|%-24.24s",	inmr_rec.description);
		fprintf (fout, "|%-5.5s",	inmr_rec.description + 35);
		fprintf (fout, "|%-4.4s",	pchc_rec.pchc_class);
		fprintf (fout, "|%14.6f%-4.4s", n_dec 
										(
											pcms_rec.matl_qty - 
											pcms_rec.qty_issued, 
											inmr_rec.dec_pt
										), inum_rec.uom);

		/* 
		 * Print lot and bin location details 
		 */
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		incc_rec.hhcc_hash = pcwo_rec.hhcc_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, incc, "DBFIND");

		/*
		 * Allocate the initial array.
		 */
		ArrAlloc (&lot_details, &lotRec, sizeof (struct LotStruct), 10);
		lotCnt = 0;

		qty = 0.00;
		inlo_rec.hhwh_hash = incc_rec.hhwh_hash;
		strcpy (inlo_rec.loc_type, " ");
		inlo_rec.expiry_date = 0L;
		cc = find_rec (inlo, &inlo_rec, GTEQ, "r");
		while (!cc && inlo_rec.hhwh_hash == incc_rec.hhwh_hash)
		{
			/*
			 * Check the array size before adding new element.
			 */
			if (!ArrChkLimit (&lot_details, lotRec, lotCnt))
				sys_err ("ArrChkLimit (lotRec)", ENOMEM, PNAME);

			if (n_dec (inlo_rec.qty, inmr_rec.dec_pt) > 0.00)
			{
				/*
				 * Load values into array element lotCnt.
				 */
				strcpy (lotRec [lotCnt].lotNo, inlo_rec.lot_no);
				strcpy (lotRec [lotCnt].location, inlo_rec.location);
				lotRec [lotCnt].expiryDate 	= inlo_rec.expiry_date;
				lotRec [lotCnt].qty = (float) n_dec (inlo_rec.qty, inmr_rec.dec_pt);
				/*
				 * Increment array counter.
				 */
				lotCnt++;

				/* 
				 * Only print the required amount 
				 */
				if (printReq) 
				{
					qty += (float) n_dec (inlo_rec.qty, inmr_rec.dec_pt);
					if (qty >= (pcms_rec.matl_qty - pcms_rec.qty_issued))
						break;
				}
			}
			cc = find_rec (inlo, &inlo_rec, NEXT, "r");
		}

		/*
		 * Sort the array in item description order.
		 */
		qsort (lotRec, lotCnt, sizeof (struct LotStruct), LotSort);

		firstRound = TRUE;
		/*
		 * Step through the sorted array getting the appropriate records.
		 */
		for (i = 0; i < lotCnt; i++)
		{
			if (firstRound == FALSE)
			{
				fprintf (fout, "|%2s",			" ");
				fprintf (fout, "|%-16.16s",		" ");
				fprintf (fout, "|%-24.24s",		" ");
				fprintf (fout, "|%-5.5s",		" ");
				fprintf (fout, "|%-4.4s",		" ");
				fprintf (fout, "|%14.6s%-4.4s",	" ", " ");
			}
			firstRound = FALSE;

			fprintf (fout, "|%7.7s", lotRec [i].lotNo);
			fprintf (fout, "|%10.10s", DateToString (lotRec [i].expiryDate));
			fprintf (fout, "|%10.10s", lotRec [i].location);
			fprintf (fout, "|%14.6f",  lotRec [i].qty);
			fprintf (fout, "|%14.6f",  lotRec [i].qty);
			fprintf (fout, "|%10.10s|\n", " ");
		}
		/*
	 	 *	Free up the array memory
	 	 */
		ArrDelete (&lot_details);

		if (firstRound == TRUE)
		{
			fprintf (fout, "|%-7.7s",		" ");
			fprintf (fout, "|%-10.10s",		" ");
			fprintf (fout, "|%-10.10s",		" ");
			fprintf (fout, "|%-14.14s",		" ");
			fprintf (fout, "|%-10.10s|\n",	" ");
		}
		cc = find_rec (pcms, &pcms_rec, NEXT, "r");
	}

	if (pformatOpen == TRUE)
	{
		pformatOpen = FALSE;
		fprintf (fout, ".EOF\n");
#ifdef GVISION
		Remote_fflush (fout);
#else
		fflush (fout);
#endif
		pclose (fout);
	}
}

void
HeadingOutput (void)
{
	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in opening pformat During (POPEN)",errno, PNAME);

	inmr_rec.hhbr_hash	=	pcms_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inmr, "DBFIND");

	pformatOpen = TRUE;

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n", printerNo);
	fprintf (fout, ".15\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");

	fprintf (fout, ".B1\n");
	fprintf (fout, ".E PRODUCTION CONTROL ISSUE PICKING REPORT\n");
	fprintf (fout, ".E COMPANY %s : %s\n",comm_rec.co_no, comm_rec.co_name);

	fprintf (fout, ".E BRANCH %s : %s\n", comm_rec.est_no, comm_rec.est_name);
	fprintf (fout, ".B1\n");

	fprintf (fout, ".C PRODUCTION OF PRODUCT: %-16.16s  %-40.40s\n",
								inmr_rec.item_no, inmr_rec.description);

	fprintf (fout, ".C WORKS ORDER NUMBER: %-7.7s  BATCH NUMBER: %-10.10s  BATCH SIZE: %14.6f\n",
					pcwo_rec.order_no, pcwo_rec.batch_no, pcwo_rec.prod_qty);
	fprintf (fout, ".B1\n");

	fprintf (fout, ".R===");
	fprintf (fout, "=================");
	fprintf (fout, "=========================");
	fprintf (fout, "======");
	fprintf (fout, "=====");
	fprintf (fout, "===================");
	fprintf (fout, "========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===============\n");

	fprintf (fout, "===");
	fprintf (fout, "=================");
	fprintf (fout, "=========================");
	fprintf (fout, "======");
	fprintf (fout, "=====");
	fprintf (fout, "===================");
	fprintf (fout, "========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===============");
	fprintf (fout, "============\n");

	fprintf (fout, "|RQ");
	fprintf (fout, "|  ITEM  NUMBER  ");
	fprintf (fout, "|    ITEM DESCRIPTION    ");
	fprintf (fout, "|STREN");
	fprintf (fout, "|HAZD");
	fprintf (fout, "|   QTY REQUIRED   ");
	fprintf (fout, "|  LOT  ");
	fprintf (fout, "|  EXPIRY  ");
	fprintf (fout, "|   BIN    ");
	fprintf (fout, "|   QUANTITY   ");
	fprintf (fout, "| QUANTITY |\n");

	fprintf (fout, "|SQ");
	fprintf (fout, "|                ");
	fprintf (fout, "|                        ");
	fprintf (fout, "|     ");
	fprintf (fout, "|CLAS");
	fprintf (fout, "|                  ");
	fprintf (fout, "| NUMBER");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "| NUMBER   ");
	fprintf (fout, "|  AVAILABLE   ");
	fprintf (fout, "|  PICKED  |\n");

	fprintf (fout, "|--");
	fprintf (fout, "|----------------");
	fprintf (fout, "|------------------------");
	fprintf (fout, "|-----");
	fprintf (fout, "|----");
	fprintf (fout, "|------------------");
	fprintf (fout, "|-------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|--------------");
	fprintf (fout, "|----------|\n");

#ifdef GVISION
	Remote_fflush (fout);
#else
	fflush (fout);
#endif

	firstTime = FALSE;
}

int 
LotSort (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct LotStruct a = * (const struct LotStruct *) a1;
	const struct LotStruct b = * (const struct LotStruct *) b1;

	result = strcmp (a.lotNo, b.lotNo);

	return (result);
}
