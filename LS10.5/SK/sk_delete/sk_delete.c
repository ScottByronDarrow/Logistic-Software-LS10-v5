/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_delete.c,v 5.4 2001/11/08 02:24:18 scott Exp $
|  Program Name  : (sk_delete.c   )                                   |
|  Program Desc  : (Delete Master, Branch, Warehouse Stock Items)     |
|---------------------------------------------------------------------|
| $Log: sk_delete.c,v $
| Revision 5.4  2001/11/08 02:24:18  scott
| Updated as inei not opened.
|
| Revision 5.3  2001/08/09 09:18:26  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:44:52  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:03  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_delete.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_delete/sk_delete.c,v 5.4 2001/11/08 02:24:18 scott Exp $";

#include 	<pslscr.h>	
#include 	<ml_std_mess.h>	
#include 	<ml_sk_mess.h>	
#include 	<Costing.h>	

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct srskRecord	srsk_rec;
struct inexRecord	inex_rec;
struct inccRecord	incc_rec;
struct intrRecord	intr_rec;
struct inisRecord	inis_rec;
struct inadRecord	inad_rec;
struct inloRecord	inlo_rec;
struct inmeRecord	inme_rec;
struct inwuRecord	inwu_rec;
struct sttfRecord	sttf_rec;
struct sttsRecord	stts_rec;
struct inspRecord	insp_rec;
struct pcmsRecord	pcms_rec;
struct bmmsRecord	bmms_rec;

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
	int	pid,
		wk_no;
	
	int		del_mr = 0,
			del_br = 0,
			del_wh = 0,
			del_tr = 0;

	/*===================
	| Work file record. |
	===================*/
	struct {
		char	wk_type [2];
		long	wk_hhbr_hash;
		long	wk_hhcc_hash;
	} wk_rec;

	FILE	*fout;

	int		lpno;


/*=======================
| Function Declarations |
=======================*/
void	shutdown_prog 	(void);
void	OpenDB 			(void);
void 	CloseDB 		(void);
void 	del_files 		(long, long);
void 	pro_status 		(int, int, int, int);
void 	dscreen 		(void);
void 	DeleteInex 		(long);
void 	head_output 	(void);
void 	PrintItemDetails(void);
void 	Delete_inad 	(long);
void 	Delete_inlo 	(long);
void 	Delete_inme	 	(long);
void 	Delete_insf 	(long);
void 	Delete_inwu 	(long);
void 	Delete_sttf 	(long);
void 	Delete_stts 	(long);
void 	Delete_insp 	(long);
void	DeleteBmms 		(long);
void	DeletePcms 		(long);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc < 3)
	{
		print_at (0,0, mlStdMess072,argv [0]); 
		return (EXIT_FAILURE);
	}
	else 
		pid = atoi (argv [1]);

	lpno = atoi (argv [2]);

	OpenDB ();

	init_scr ();
	set_tty ();
	dscreen ();

	head_output ();
	cc = RF_READ (wk_no, (char *) &wk_rec);
	while (!cc)
	{
		del_files (wk_rec.wk_hhbr_hash, wk_rec.wk_hhcc_hash);
		cc = RF_READ (wk_no, (char *) &wk_rec);
	}
	fprintf (fout,".EOF\n");
	pclose (fout);
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

void
OpenDB (
 void)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");

	sprintf (filename,"%s/WORK/sk_del%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pid);

	if ((cc = RF_OPEN (filename,sizeof (wk_rec),"r",&wk_no)) != 0) 
		sys_err ("Error in work_file During (WKOPEN)", cc, PNAME);

	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (srsk, srsk_list, SRSK_NO_FIELDS, "srsk_hhbr_hash");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_hhbr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	open_rec (intr, intr_list, INTR_NO_FIELDS, "intr_hhbr_hash");

	OpenInsf ();
	OpenInei ();
	OpenIncf ();
	abc_selfield (insf, "insf_hhwh_hash");
}

void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (srsk);
	abc_fclose (inex);
	abc_fclose (inis);
	abc_fclose (incc);
	abc_fclose (intr);
	SearchFindClose ();
	CloseCosting ();
	abc_dbclose ("data");

	RF_DELETE (wk_no);
}

void
del_files (
	long	hhbrHash, 
	long	hhccHash)
{
	cc = find_hash (inmr, &inmr_rec, EQUAL, "u", hhbrHash);
	move (31,6);
	print_at (6,31,"%s",inmr_rec.item_no);	
	fflush (stdout);
	if (cc)
		return;

	if (wk_rec.wk_type [0] == 'M' || wk_rec.wk_type [0] == 'A')
	{
		rv_pr (ML (mlSkMess609), 7,9,1);
		rv_pr (ML (mlSkMess614), 30,9,0);
		rv_pr (ML (mlSkMess615), 53,9,0);
		/*--------------------------------
		| Detete inventory Transactions. |
		--------------------------------*/

		srsk_rec.hhbr_hash = hhbrHash;
		cc = find_rec (srsk,&srsk_rec, EQUAL, "u");
		if (!cc)
		{
			cc = abc_delete (srsk);
			if (cc)
				file_err (cc, srsk, "DBDELETE");
		}

		PrintItemDetails ();
		abc_delete (inmr);
		pro_status (del_mr++, del_br, del_wh, del_tr);
		
		/*--------------------------------
		| Detete inventory Transactions. |
		--------------------------------*/
		cc = find_hash (intr, &intr_rec, EQUAL, "u", hhbrHash);
		while (!cc)
		{
			abc_delete (intr);
			pro_status (del_mr, del_br, del_wh, del_tr++);
			cc = find_hash (intr, &intr_rec, EQUAL, "u", hhbrHash);
		}
		Delete_insp (hhbrHash);

		/*------------------------------------
		| Detete inventory supplier records. |
		------------------------------------*/
		cc = find_hash (inis, &inis_rec, EQUAL, "u", hhbrHash);
		while (!cc)
		{
			abc_delete (inis);
			cc = find_hash (inis, &inis_rec, EQUAL, "r", hhbrHash);
		}
	}
	if (wk_rec.wk_type [0] == 'B' || wk_rec.wk_type [0] == 'A')
	{
		rv_pr (ML (mlSkMess609), 7,9,0);
		rv_pr (ML (mlSkMess614), 30,9,1);
		rv_pr (ML (mlSkMess615), 53,9,0);
		/*----------------------------------
		| Detete inventory Branch Records. |
		----------------------------------*/
		ineiRec.hhbr_hash	=	hhbrHash;
		cc = find_rec (inei, &ineiRec, EQUAL, "r");
		while (!cc)
		{
			abc_delete (inei);
			pro_status (del_mr, del_br++, del_wh, del_tr);
			cc = find_rec (inei, &ineiRec, EQUAL, "r");
		}
	}
	if (wk_rec.wk_type [0] == 'W' || wk_rec.wk_type [0] == 'A')
	{
		rv_pr (ML (mlSkMess609), 7,9,0);
		rv_pr (ML (mlSkMess614), 30,9,0);
		rv_pr (ML (mlSkMess615), 53,9,1);

	    if (wk_rec.wk_type [0] == 'A')
	    {
			/*-------------------------------------
			| Detete inventory Warehouse Records. |
			-------------------------------------*/
			cc = find_hash (incc,&incc_rec,EQUAL,"u", hhbrHash);
			while (!cc && incc_rec.hhbr_hash	== hhbrHash)
			{
				/*----------------------
				| Detete fifo Records. |
				----------------------*/
				cc = FindIncf (incc_rec.hhwh_hash, TRUE, "u");
				while (!cc && incfRec.hhwh_hash == incc_rec.hhwh_hash)
				{
					abc_delete (incf);
					pro_status (del_mr, del_br, del_wh++, del_tr);
					cc = FindIncf (incc_rec.hhwh_hash, TRUE, "u");
				}
				Delete_inad (incc_rec.hhwh_hash);
				Delete_inlo (incc_rec.hhwh_hash);
				Delete_inme (incc_rec.hhwh_hash);
				Delete_insf (incc_rec.hhwh_hash);
				Delete_inwu (incc_rec.hhwh_hash);
				Delete_sttf (incc_rec.hhwh_hash);

				pro_status (del_mr, del_br, del_wh++, del_tr);

				abc_delete (incc);
				cc = find_hash (incc,&incc_rec,EQUAL,"r",hhbrHash);
			}
	    }
	    else
	    {
			abc_selfield (incc, "incc_id_no");
			incc_rec.hhcc_hash = hhccHash;
			incc_rec.hhbr_hash = hhbrHash;
			cc = find_rec (incc,&incc_rec,COMPARISON,"u");
			if (!cc)
			{
				/*----------------------
				| Detete fifo Records. |
				----------------------*/
				cc = FindIncf (incc_rec.hhwh_hash, TRUE, "u");
				while (!cc && incfRec.hhwh_hash == incc_rec.hhwh_hash)
				{
					abc_delete (incf);
					pro_status (del_mr, del_br, del_wh++, del_tr);
					cc = FindIncf (incc_rec.hhwh_hash, TRUE, "u");
				}
				Delete_inad (incc_rec.hhwh_hash);
				Delete_inlo (incc_rec.hhwh_hash);
				Delete_inme (incc_rec.hhwh_hash);
				Delete_insf (incc_rec.hhwh_hash);
				Delete_inwu (incc_rec.hhwh_hash);
				Delete_sttf (incc_rec.hhwh_hash);

				abc_delete (incc);
				pro_status (del_mr, del_br, del_wh++, del_tr);
			}
			abc_selfield (incc, "incc_hhbr_hash");
	    }
	}
	DeleteInex (hhbrHash);
	DeleteBmms (hhbrHash);
	DeletePcms (hhbrHash);
}

void
pro_status (
 int mr_no, 
 int br_no, 
 int wh_no, 
 int tr_no)
{
	move (8,16);

	/* (16,8,"Master: %05d / Branch: %05d / W/H: %05d / Trans: %05d",mr_no, br_no, wh_no, tr_no);*/

	print_at (16,8,ML (mlSkMess607),mr_no, br_no, wh_no, tr_no);

}

void
dscreen (
 void)
{
	clear ();
	crsr_off ();
	box (0,0,78,21);
	box (24,2,32,1);
	box (30,5,18,1);
	box (4,8,22,1);
	box (28,8,22,1);
	box (52,8,22,1);
	box (5,15,66,1);

	rv_pr (ML (mlSkMess608), 26,3,1);
	rv_pr (ML (mlSkMess610), 28,14,1);
	rv_pr (ML (mlSkMess609), 7,9,0);
	rv_pr (ML (mlSkMess614), 30,9,0);
	rv_pr (ML (mlSkMess615), 53,9,0);
	move (8,16);

	/* (16,8,"Master: %05d / Branch: %05d / W/H: %05d / Trans: %05d",0, 0, 0, 0);*/

	print_at (16,8,ML (mlSkMess607),0, 0, 0, 0);
}

/*=============================
| Delete all inventory notes. |
=============================*/
void
DeleteInex (
 long hhbr_hash)
{
	inex_rec.hhbr_hash = hhbr_hash;
	inex_rec.line_no = 0;

	cc = find_rec (inex, &inex_rec, GTEQ, "u");

	while (!cc && inex_rec.hhbr_hash == hhbr_hash)
	{
		abc_delete (inex);
		inex_rec.hhbr_hash = hhbr_hash;
		inex_rec.line_no = 0;
		cc = find_rec (inex, &inex_rec, GTEQ, "u");
	}
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_output (
 void)
{
	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in opening pformat During (DBOPEN)",errno,PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n",lpno);

	fprintf (fout,".13\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".E%s\n",clip (comm_rec.co_name));
	fprintf (fout,".B1\n");

	fprintf (fout,".EITEM MASTER FILE DELETE\n");

	fprintf (fout,".B1\n");
	fprintf (fout,".EAS AT %s\n",SystemTime ());

	fprintf (fout,".R=====================================================");
	fprintf (fout,"=====================================================");
	fprintf (fout,"====================================================\n");

	fprintf (fout,"=====================================================");
	fprintf (fout,"=====================================================");
	fprintf (fout,"====================================================\n");

	fprintf (fout,"|                                                    ");
	fprintf (fout,"  I T E M   M A S T E R   F I L E   D E L E T E      ");
	fprintf (fout,"                                                    \n");

	fprintf (fout,"|----------------------------------------------------");
	fprintf (fout,"-----------------------------------------------------");
	fprintf (fout,"---------------------------------------------------|\n");

	fflush (fout);
}

void
PrintItemDetails (
 void)
{
	fprintf (fout,"| Item  Number : %-16.16s                    ",
				inmr_rec.item_no);
	fprintf (fout,"                           | Description  : %-40.40s",
				inmr_rec.description);
	fprintf (fout,"                    |\n");
	fprintf (fout,"| Alpha Number : %-16.16s                    ",
				inmr_rec.alpha_code);
	fprintf (fout,"                           | Supercession : %-16.16s",
				inmr_rec.supercession);
	fprintf (fout,"                                            |\n");
	fprintf (fout,"| Alternate No : %16.16s                    ",
				inmr_rec.alternate);
	fprintf (fout,"                           | Barcode No   : %16.16s",
				inmr_rec.barcode);
	fprintf (fout,"                                            |\n");
	fprintf (fout,"| Item Class   : %1.1s                                   ",
				inmr_rec.inmr_class);
	fprintf (fout,"                           | Item Category: %11.11s",
				inmr_rec.category);
	fprintf (fout,"                                                 |\n");
	fprintf (fout,"| Quick Code   : %8.8s                            ",
				inmr_rec.quick_code);
	fprintf (fout,"                           | ABC Code     : %1.1s        ",
				inmr_rec.abc_code);
	fprintf (fout,"                                                   |\n");
	fprintf (fout,"| Serial Item  : %3.3s                                 ",
			 (inmr_rec.serial_item [0] == 'Y') ? "YES" : "NO ");
	fprintf (fout,"                           | Lot Item     : %3.3s      ",
			 (inmr_rec.lot_ctrl [0] == 'Y') ? "YES" : "NO ");
	fprintf (fout,"                                                   |\n");
	fprintf (fout,"| Costing Flag : %s                                   ",
			inmr_rec.costing_flag);
	fprintf (fout,"                           | Source       : %2.2s       ",
			inmr_rec.source);
	fprintf (fout,"                                                   |\n");
	fprintf (fout,"| Sale Unit    : %4.4s                                ",
			inmr_rec.sale_unit);
	fprintf (fout,"                           | Pack Size    : %5.5s    ",
			inmr_rec.pack_size);
	fprintf (fout,"                                                   |\n");
	fprintf (fout,"| Buying Group : %6.6s                              ",
			inmr_rec.buygrp);
	fprintf (fout,"                           | Selling Group: %5.5s    ",
			inmr_rec.sellgrp);
	fprintf (fout,"                                                   |\n");
	fprintf (fout,"| YTD Sales    : %9.2f                           ",
			inmr_rec.ltd_sales);
	fprintf (fout,"                           | Weight       : %9.2f",
			inmr_rec.weight);
	fprintf (fout,"                                                   |\n");
	fprintf (fout,"=====================================================");
	fprintf (fout,"=====================================================");
	fprintf (fout,"====================================================\n");
	fprintf (fout, ".LRP11\n");
}

void
Delete_inad (
 long	hhwhHash)
{
	open_rec (inad, inad_list, INAD_NO_FIELDS, "inad_id_no2");
	
	inad_rec.hhwh_hash	=	hhwhHash;
	strcpy (inad_rec.lot_no, "       ");
	cc = find_rec (inad, &inad_rec, GTEQ, "u");
	while (!cc && inad_rec.hhwh_hash	==	hhwhHash)
	{
		cc = abc_delete (inad);
		if (cc)
			file_err (cc, inad, "DBDELETE");

		inad_rec.hhwh_hash	=	hhwhHash;
		strcpy (inad_rec.lot_no, "       ");
		cc = find_rec (inad, &inad_rec, GTEQ, "u");
	}
	abc_fclose (inad);
}

void
Delete_inlo (
 long	hhwhHash)
{
	open_rec (inlo, inlo_list, INLO_NO_FIELDS, "inlo_hhwh_hash");
	
	inlo_rec.hhwh_hash	=	hhwhHash;
	cc = find_rec (inlo, &inlo_rec, GTEQ, "u");
	while (!cc && inlo_rec.hhwh_hash	==	hhwhHash)
	{
		cc = abc_delete (inlo);
		if (cc)
			file_err (cc, inlo, "DBDELETE");

		inlo_rec.hhwh_hash	=	hhwhHash;
		cc = find_rec (inlo, &inlo_rec, GTEQ, "u");
	}
	abc_fclose (inlo);
}

void
Delete_inme (
 long	hhwhHash)
{
	open_rec (inme, inme_list, INME_NO_FIELDS, "inme_hhwh_hash");
	
	inme_rec.hhwh_hash	=	hhwhHash;
	cc = find_rec (inme, &inme_rec, GTEQ, "u");
	while (!cc && inme_rec.hhwh_hash	==	hhwhHash)
	{
		cc = abc_delete (inme);
		if (cc)
			file_err (cc, inme, "DBDELETE");

		inme_rec.hhwh_hash	=	hhwhHash;
		cc = find_rec (inme, &inme_rec, GTEQ, "u");
	}
	abc_fclose (inme);
}

void
Delete_insf (
 long	hhwhHash)
{

	insfRec.hhwh_hash	=	hhwhHash;
	cc = find_rec (insf, &insfRec, GTEQ, "u");
	while (!cc && insfRec.hhwh_hash	==	hhwhHash)
	{
		cc = abc_delete (insf);
		if (cc)
			file_err (cc, insf, "DBDELETE");

		insfRec.hhwh_hash	=	hhwhHash;
		cc = find_rec (insf, &insfRec, GTEQ, "u");
	}
}

void
Delete_inwu (
 long	hhwhHash)
{
	open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	
	inwu_rec.hhwh_hash	=	hhwhHash;
	inwu_rec.hhum_hash	=	0L;
	cc = find_rec (inwu, &inwu_rec, GTEQ, "u");
	while (!cc && inwu_rec.hhwh_hash	==	hhwhHash)
	{
		cc = abc_delete (inwu);
		if (cc)
			file_err (cc, inwu, "DBDELETE");

		inwu_rec.hhwh_hash	=	hhwhHash;
		inwu_rec.hhum_hash	=	0L;
		cc = find_rec (inwu, &inwu_rec, GTEQ, "u");
	}
	abc_fclose (inwu);
}

void
Delete_sttf (
 long	hhwhHash)
{
	open_rec (sttf, sttf_list, STTF_NO_FIELDS, "sttf_id_no");
	
	sttf_rec.hhwh_hash	=	hhwhHash;
	strcpy (sttf_rec.location, "          ");
	cc = find_rec (sttf, &sttf_rec, GTEQ, "u");
	while (!cc && sttf_rec.hhwh_hash	==	hhwhHash)
	{
		cc = abc_delete (sttf);
		if (cc)
			file_err (cc, sttf, "DBDELETE");

		sttf_rec.hhwh_hash	=	hhwhHash;
		strcpy (sttf_rec.location, "          ");
		cc = find_rec (sttf, &sttf_rec, GTEQ, "u");
	}
	abc_fclose (sttf);
}

void
Delete_stts (
 long	hhwhHash)
{
	open_rec (stts, stts_list, STTS_NO_FIELDS, "stts_id_no");
	
	stts_rec.hhwh_hash	=	hhwhHash;
	strcpy (stts_rec.serial_no, "                         ");
	cc = find_rec (stts, &stts_rec, GTEQ, "u");
	while (!cc && stts_rec.hhwh_hash	==	hhwhHash)
	{
		cc = abc_delete (stts);
		if (cc)
			file_err (cc, stts, "DBDELETE");

		stts_rec.hhwh_hash	=	hhwhHash;
		strcpy (stts_rec.serial_no, "                         ");
		cc = find_rec (stts, &stts_rec, GTEQ, "u");
	}
	abc_fclose (stts);
}

void
Delete_insp (
 long	hhbrHash)
{
	open_rec (insp, insp_list, INSP_NO_FIELDS, "insp_hhbr_hash");
	
	insp_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (insp, &insp_rec, GTEQ, "u");
	while (!cc && insp_rec.hhbr_hash	==	hhbrHash)
	{
		cc = abc_delete (insp);
		if (cc)
			file_err (cc, insp, "DBDELETE");

		insp_rec.hhbr_hash	=	hhbrHash;
		cc = find_rec (insp, &insp_rec, GTEQ, "u");
	}
	abc_fclose (insp);
}

void
DeleteBmms (
	long	hhbrHash)
{
	open_rec (bmms, bmms_list, BMMS_NO_FIELDS, "bmms_hhbr_hash");

	bmms_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (bmms, &bmms_rec, GTEQ, "u");
	while (!cc && bmms_rec.hhbr_hash	==	hhbrHash)
	{
		cc = abc_delete (bmms);
		if (cc)
			file_err (cc, bmms, "DBDELETE");

		bmms_rec.hhbr_hash	=	hhbrHash;
		cc = find_rec (bmms, &bmms_rec, GTEQ, "u");
	}
	abc_selfield (bmms, "bmms_mabr_hash");

	bmms_rec.mabr_hash	=	hhbrHash;
	cc = find_rec (bmms, &bmms_rec, GTEQ, "u");
	while (!cc && bmms_rec.mabr_hash	==	hhbrHash)
	{
		cc = abc_delete (bmms);
		if (cc)
			file_err (cc, bmms, "DBDELETE");

		bmms_rec.mabr_hash	=	hhbrHash;
		cc = find_rec (bmms, &bmms_rec, GTEQ, "u");
	}
	abc_fclose (bmms);
}

void
DeletePcms (
	long	hhbrHash)
{
	open_rec (pcms, pcms_list, PCMS_NO_FIELDS, "pcms_hhbr_hash");

	cc = find_rec (pcms, &pcms_rec, GTEQ, "u");
	while (!cc && pcms_rec.hhbr_hash	==	hhbrHash)
	{
		cc = abc_delete (pcms);
		if (cc)
			file_err (cc, pcms, "DBDELETE");

		pcms_rec.hhbr_hash	=	hhbrHash;
		cc = find_rec (pcms, &pcms_rec, GTEQ, "u");
	}

	abc_selfield (pcms, "pcms_mabr_hash");
	cc = find_rec (pcms, &pcms_rec, GTEQ, "u");
	while (!cc && pcms_rec.mabr_hash	==	hhbrHash)
	{
		cc = abc_delete (pcms);
		if (cc)
			file_err (cc, pcms, "DBDELETE");

		pcms_rec.mabr_hash	=	hhbrHash;
		cc = find_rec (pcms, &pcms_rec, GTEQ, "u");
	}
	abc_fclose (pcms);
}
