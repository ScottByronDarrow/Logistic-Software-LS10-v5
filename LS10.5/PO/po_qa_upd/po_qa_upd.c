/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( po_qa_upd.c   )                                  |
|  Program Desc  : ( Updates QA status on inei is all supplier QA )   |
|                  ( approved suppliers.                          )   |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow. | Date Written  : 30/10/95         |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
| $Log: po_qa_upd.c,v $
| Revision 5.1  2001/08/09 09:16:01  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:11:55  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:11  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:57  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:29  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.6  2000/02/18 02:17:50  scott
| Updated to fix small compile warings errors found when compiled under Linux.
|
| Revision 1.5  1999/09/29 10:12:08  scott
| Updated to be consistant on function names.
|
| Revision 1.4  1999/09/21 04:38:08  scott
| Updated from Ansi project
|
| Revision 1.3  1999/06/17 10:06:34  scott
| Updated to remove old read_comm(), Added cvs logs, changed database names.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_qa_upd.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_qa_upd/po_qa_upd.c,v 5.1 2001/08/09 09:16:01 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>

#ifndef	LINUX
extern	int	errno;
#endif	/* LINUX */

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_inv_date"},
	};

	int comm_no_fields = 6;

	struct {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		long	tinv_date;
	} comm_rec;

	/*========================+
	 | Creditors Master File. |
	 +========================*/
#define	SUMR_NO_FIELDS	4

	struct dbview	sumr_list [SUMR_NO_FIELDS] =
	{
		{"sumr_crd_no"},
		{"sumr_hhsu_hash"},
		{"sumr_qa_status"},
		{"sumr_stat_flag"},
	};

	struct tag_sumrRecord
	{
		char	crd_no[7];
		long	hhsu_hash;
		char	qa_status [2];
		char	stat_flag [2];
	}	sumr_rec;

	/*============================================+
	 | Inventory Establishment/Branch Stock File. |
	 +============================================*/
#define	INEI_NO_FIELDS	4

	struct dbview	inei_list [INEI_NO_FIELDS] =
	{
		{"inei_hhbr_hash"},
		{"inei_est_no"},
		{"inei_qa_status"},
		{"inei_stat_flag"}
	};

	struct tag_ineiRecord
	{
		long	hhbr_hash;
		char	est_no [3];
		char	qa_status [2];
		char	stat_flag [2];
	}	inei_rec;

	/*==================================+
	 | Stock Inventory Supplier Record. |
	 +==================================*/
#define	INIS_NO_FIELDS	26

	struct dbview	inis_list [INIS_NO_FIELDS] =
	{
		{"inis_co_no"},
		{"inis_br_no"},
		{"inis_wh_no"},
		{"inis_hhbr_hash"},
		{"inis_hhsu_hash"},
		{"inis_sup_part"},
		{"inis_sup_priority"},
		{"inis_hhis_hash"},
		{"inis_fob_cost"},
		{"inis_lcost_date"},
		{"inis_duty"},
		{"inis_licence"},
		{"inis_sup_uom"},
		{"inis_pur_conv"},
		{"inis_min_order"},
		{"inis_norm_order"},
		{"inis_ord_multiple"},
		{"inis_pallet_size"},
		{"inis_lead_time"},
		{"inis_sea_time"},
		{"inis_air_time"},
		{"inis_lnd_time"},
		{"inis_dflt_lead"},
		{"inis_weight"},
		{"inis_volume"},
		{"inis_stat_flag"}
	};

	struct tag_inisRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	wh_no [3];
		long	hhbr_hash;
		long	hhsu_hash;
		char	sup_part [17];
		char	sup_priority [3];
		long	hhis_hash;
		double	fob_cost;
		Date	lcost_date;
		char	duty [3];
		char	licence [3];
		long	sup_uom;
		float	pur_conv;
		float	min_order;
		float	norm_order;
		float	ord_multiple;
		float	pallet_size;
		float	lead_time;
		float	sea_time;
		float	air_time;
		float	lnd_time;
		char	dflt_lead [2];
		float	weight;
		float	volume;
		char	stat_flag [2];
	}	inis_rec;


/*=======================
| Function Declarations |
=======================*/
void OpenDB (void);
void CloseDB (void);
void Process (void);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv[])
{
	OpenDB();

	dsp_screen ("Item QA status flag update.", comm_rec.tco_no, comm_rec.tco_name);
	Process();

	CloseDB (); 
	FinishProgram ();

    return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec("inei", inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec("inis", inis_list, INIS_NO_FIELDS, "inis_id_no2");
	open_rec("sumr", sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("inei");
	abc_fclose("inis");
	abc_fclose("sumr");
	abc_dbclose("data");
}

/*========================================================================
| Process whole inei file looking for suppliers of product in inis file. |
========================================================================*/
void
Process (
 void)
{
	int		QaSupplier = FALSE;

	inei_rec.hhbr_hash = 0L;
	strcpy (inei_rec.est_no, "  ");
	cc = find_rec ("inei", &inei_rec, GTEQ, "r");
	while (!cc)
	{
		QaSupplier = FALSE;

		inis_rec.hhbr_hash	=	inei_rec.hhbr_hash;
		strcpy (inis_rec.sup_priority, "  ");
		strcpy (inis_rec.co_no, "  ");
		strcpy (inis_rec.br_no, "  ");
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec ("inis", &inis_rec, GTEQ, "r");
		while (!cc && inis_rec.hhbr_hash	==	inei_rec.hhbr_hash)
		{
			sumr_rec.hhsu_hash = inis_rec.hhsu_hash;
			cc = find_rec ("sumr", &sumr_rec, EQUAL, "r");
			if (cc)
			{
				cc = find_rec ("inis", &inis_rec, NEXT, "r");
				continue;
			}

			dsp_process ("Supplier", sumr_rec.crd_no);

			/*------------------------------------------
			| Is a QA Supplier so set flag to 1 (Yes). |
			------------------------------------------*/
			if ( sumr_rec.qa_status[0] == '1' )
				QaSupplier = TRUE;
			else
			{
				/*-----------------------------------------------------------
				| Is not a QA Supplier so set flag to 0 (no). and break out |
 				| as all suppliers must be qa approved.                     |
				-----------------------------------------------------------*/
				QaSupplier = FALSE;
				break;
			}
			cc = find_rec ("inis", &inis_rec, NEXT, "r");
		}

		/*----------------------------------------------------------
		| All suppliers for the product are qa approved so update. |
		----------------------------------------------------------*/
		if ( QaSupplier == TRUE )
		{
			cc = find_rec ("inei", &inei_rec, EQUAL, "u");
			if (cc)
				file_err (cc, "inei", "DBFIND");

			strcpy (inei_rec.qa_status, "Y");
		
			cc = abc_update ("inei", &inei_rec);
			if (cc)
				file_err (cc, "inei", "DBUPDATE");
		}
		else
		{
			/*--------------------------------------
			| Product was approved but now is not. |
			--------------------------------------*/
			if ( inei_rec.qa_status[0] == 'Y' )
			{
				cc = find_rec ("inei", &inei_rec, EQUAL, "u");
				if (cc)
					file_err (cc, "inei", "DBFIND");

				strcpy (inei_rec.qa_status, "N");

				cc = abc_update ("inei", &inei_rec);
				if (cc)
					file_err (cc, "inei", "DBUPDATE");
			}	
		}
		cc = find_rec ("inei", &inei_rec, NEXT, "r");
	}
}
