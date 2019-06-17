/*=====================================================================
|  Copyright (C) 2000 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: Costing.h,v 5.0 2002/05/08 01:41:04 scott Exp $
|=====================================================================|
|  Program Name  : (Costings.h) 
|  Program Desc  : (Standard Stock Costing header)
|=====================================================================|
| $Log: Costing.h,v $
| Revision 5.0  2002/05/08 01:41:04  scott
| CVS administration
|
| Revision 1.3  2002/01/09 00:53:39  scott
| Updated to change define for FindInsfCost () to include hhbrHash
|
| Revision 1.2  2001/08/06 22:49:49  scott
| RELEASE 5.0
|
| Revision 1.1  2001/06/20 05:00:58  scott
| New include for costing routines.
|
=====================================================================*/
#ifndef	_Costings_h
#define	_Costings_h

#include	<CostingStruct.h>

#ifdef	CCMAIN
#	define	GLOBAL_COST
#else
#	define	GLOBAL_COST	extern
#endif

#ifdef	CCMAIN
	/*
	 * Inventory cost file. (FIFO, LIFO)
	 */
	struct dbview	incf_list [] =
	{
		{"incf_hhwh_hash"},
		{"incf_hhcf_hash"},
		{"incf_fifo_date"},
		{"incf_seq_no"},
		{"incf_fifo_cost"},
		{"incf_act_cost"},
		{"incf_fifo_qty"},
		{"incf_gr_number"},
		{"incf_fob_nor_cst"},
		{"incf_frt_ins_cst"},
		{"incf_duty"},
		{"incf_licence"},
		{"incf_lcost_load"},
		{"incf_land_cst"},
		{"incf_stat_flag"}
	};

	/*
	 * Inventory Establishment/Branch Stock File.
	 */
	struct dbview	inei_list [] =
	{
		{"inei_hhbr_hash"},
		{"inei_est_no"},
		{"inei_hhis_hash"},
		{"inei_avge_cost"},
		{"inei_last_cost"},
		{"inei_prev_cost"},
		{"inei_date_lcost"},
		{"inei_lpur_qty"},
		{"inei_min_stock"},
		{"inei_max_stock"},
		{"inei_safety_stock"},
		{"inei_abc_code"},
		{"inei_abc_update"},
		{"inei_std_cost"},
		{"inei_std_batch"},
		{"inei_min_batch"},
		{"inei_max_batch"},
		{"inei_prd_multiple"},
		{"inei_hndl_class"},
		{"inei_hzrd_class"},
		{"inei_prod_class"},
		{"inei_expiry_prd1"},
		{"inei_expiry_prd2"},
		{"inei_expiry_prd3"},
		{"inei_dflt_bom"},
		{"inei_dflt_rtg"},
		{"inei_eoq"},
		{"inei_last_bom"},
		{"inei_last_rtg"},
		{"inei_qa_status"},
		{"inei_stat_flag"}
	};

	/*
	 * Inventory Transfer Fifo/Lifo holding records.
	 */
	struct dbview	itff_list [] =
	{
		{"itff_itff_hash"},
		{"itff_fifo_date"},
		{"itff_fifo_cost"},
		{"itff_act_cost"},
		{"itff_fifo_qty"},
		{"itff_gr_number"},
		{"itff_fob_nor_cst"},
		{"itff_frt_ins_cst"},
		{"itff_duty"},
		{"itff_licence"},
		{"itff_lcost_load"},
		{"itff_land_cst"},
		{"itff_stat_flag"}
	};

	/*
	 * Inventory Stock Take Control File. |
	 */
	struct dbview	insc_list [] =
	{
		{"insc_hhcc_hash"},
		{"insc_stake_code"},
		{"insc_start_date"},
		{"insc_start_time"},
		{"insc_frz_date"},
		{"insc_frz_time"},
		{"insc_page_no"},
		{"insc_description"},
		{"insc_serial_take"}
	};

	/*
	 * Inventory Serial Number File. 
	 */
	struct dbview	insf_list [] =
	{
		{"insf_hhsf_hash"},
		{"insf_hhbr_hash"},
		{"insf_hhwh_hash"},
		{"insf_status"},
		{"insf_receipted"},
		{"insf_serial_no"},
		{"insf_chasis_no"},
		{"insf_date_in"},
		{"insf_date_out"},
		{"insf_hhcu_hash"},
		{"insf_location"},
		{"insf_exch_rate"},
		{"insf_fob_fgn_cst"},
		{"insf_fob_nor_cst"},
		{"insf_frt_ins_cst"},
		{"insf_duty"},
		{"insf_licence"},
		{"insf_lcost_load"},
		{"insf_land_cst"},
		{"insf_other_cst"},
		{"insf_istore_cost"},
		{"insf_prep_cost"},
		{"insf_exch_var"},
		{"insf_est_cost"},
		{"insf_act_cost"},
		{"insf_po_number"},
		{"insf_gr_number"},
		{"insf_invoice_no"},
		{"insf_hhsu_hash"},
		{"insf_crd_invoice"},
		{"insf_final_costing"},
		{"insf_stock_take"},
		{"insf_pd_rate"},
		{"insf_paid_cost"},
		{"insf_des_flag"},
		{"insf_stat_flag"}
	};

const	char 	*incf	=	"incf",
				*itff	=	"itff",
				*insf	=	"insf",
				*inei	=	"inei",
				*insc	=	"insc";
#else
	extern	struct dbview	incf_list [],
							inei_list [],
							itff_list [],
							insc_list [],
							insf_list [];

extern	const	char 	*incf, 
						*itff, 
						*insf, 
						*inei, 
						*insc;

#endif

#define	INCF_NO_FIELDS	15
#define	INEI_NO_FIELDS	31
#define	ITFF_NO_FIELDS	13
#define	INSC_NO_FIELDS	9
#define	INSF_NO_FIELDS	36

GLOBAL_COST	INCF_STRUCT	incfRec;
GLOBAL_COST	INEI_STRUCT	ineiRec;
GLOBAL_COST	ITFF_STRUCT	itffRec;
GLOBAL_COST	INSC_STRUCT	inscRec;
GLOBAL_COST	INSF_STRUCT	insfRec;

GLOBAL_COST	double	CheckIncf		(long, int, float, int);
GLOBAL_COST	double	FindIncfCost	(long, float, float, int, int);
GLOBAL_COST	double	FindIncfCostPO	(long, float, float, int, int);
GLOBAL_COST	double	FindIncfValue	(long, float, int, int, int);
GLOBAL_COST	double	FindIneiCosts	(char *, char *, long);
GLOBAL_COST	double	FindInsfCost	(long, long, char *, char *);
GLOBAL_COST	double	FindInsfValue	(long, int);
GLOBAL_COST	double	StockValue		(char *,char *,long,long,float,int,int);
GLOBAL_COST	double	SerialValue		(double, double);
GLOBAL_COST	int		AddIncf			(long, long, double, double, float, char *, double, double, double, double, double, double, char *);
GLOBAL_COST	int		CheckInsc		(long, long, char *);
GLOBAL_COST	int		CheckIncfSeq	(long);
GLOBAL_COST	int		FindIncf		(long, int, char *);
GLOBAL_COST	int		FindInei		(long, char *, char *);
GLOBAL_COST	int		FindInsf		(long, long, char *, char *, char *);
GLOBAL_COST	int		TransIncf		(long, long, float, float, int);
GLOBAL_COST	int		TransIncfToItff	(long, float, float, double, double, double,int,long);
GLOBAL_COST	int		TransInsf		(long, long, char *, char *);
GLOBAL_COST	int		UpdateInsf		(long, long, char *, char *, char *);
GLOBAL_COST	int		ValidStDate		(long);
GLOBAL_COST	void	CloseCosting	(void);
GLOBAL_COST	void	FindInsc 		(long, char *, int);
GLOBAL_COST	void	OpenInei 		(void);
GLOBAL_COST	void	OpenInsc 		(void);
GLOBAL_COST	void	OpenInsf 		(void);
GLOBAL_COST	void	OpenIncf 		(void);
GLOBAL_COST	void	OpenIncf2 		(void);
GLOBAL_COST	void	OpenItff 		(void);

GLOBAL_COST	void	PurgeIncf		(long, int, float);
GLOBAL_COST	void	ReduceIncf		(long, float, int);
GLOBAL_COST	void	SearchInsf		(long, char *, char *);
GLOBAL_COST	void	TransItffToIncf	(long, long, float);
#endif	/* _Costings_h */
