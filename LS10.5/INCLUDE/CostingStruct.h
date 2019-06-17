/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: CostingStruct.h,v 5.0 2002/05/08 01:41:04 scott Exp $
-----------------------------------------------------------------------
| $Log: CostingStruct.h,v $
| Revision 5.0  2002/05/08 01:41:04  scott
| CVS administration
|
| Revision 1.1  2001/06/20 05:00:59  scott
| New include for costing routines.
|
*/
#ifndef	COSTSTRUCTS_H
#define	COSTSTRUCTS_H

	/*
	 * Inventory cost file. (FIFO, LIFO)
	 */
	typedef	struct
	{
		long	hhwh_hash;
		long	hhcf_hash;
		Date	fifo_date;
		int		seq_no;
		double	fifo_cost;
		double	act_cost;
		float	fifo_qty;
		char	gr_number [16];
		double	fob_nor_cst;
		double	frt_ins_cst;
		double	duty;
		double	licence;
		double	lcost_load;
		double	land_cst;
		char	stat_flag [2];
	}	INCF_STRUCT;

	/*
	 * Inventory Transfer Fifo/Lifo holding records.
	 */
	typedef	struct
	{
		long	itff_hash;
		Date	fifo_date;
		double	fifo_cost;
		double	act_cost;
		float	fifo_qty;
		char	gr_number [16];
		double	fob_nor_cst;
		double	frt_ins_cst;
		double	duty;
		double	licence;
		double	lcost_load;
		double	land_cst;
		char	stat_flag [2];
	}	ITFF_STRUCT;

	/*
	 * Inventory Serial Number File.
	 */
	typedef	struct
	{
		long	hhsf_hash;
		long	hhbr_hash;
		long	hhwh_hash;
		char	status [2];
		char	receipted [2];
		char	serial_no [26];
		char	chasis_no [21];
		Date	date_in;
		Date	date_out;
		long	hhcu_hash;
		char	location [11];
		double	exch_rate;
		double	fob_fgn_cst;
		double	fob_nor_cst;
		double	frt_ins_cst;
		double	duty;
		double	licence;
		double	lcost_load;
		double	land_cst;
		double	other_cst;
		double	istore_cost;
		double	prep_cost;
		double	exch_var;
		double	est_cost;
		double	act_cost;
		char	po_number [16];
		char	gr_number [16];
		char	invoice_no [9];
		long	hhsu_hash;
		char	crd_invoice [16];
		char	final_costing [2];
		char	stock_take [2];
		double	pd_rate;
		double	paid_cost;
		char	des_flag [2];
		char	stat_flag [2];
	}	INSF_STRUCT;

	/*
	 * Inventory Establishment/Branch Stock File.
	 */
	typedef	struct
	{
		long	hhbr_hash;
		char	est_no [3];
		long	hhis_hash;
		double	avge_cost;
		double	last_cost;
		double	prev_cost;
		Date	date_lcost;
		float	lpur_qty;
		float	min_stock;
		float	max_stock;
		float	safety_stock;
		char	abc_code [2];
		char	abc_update [2];
		double	std_cost;
		float	std_batch;
		float	min_batch;
		float	max_batch;
		float	prd_multiple;
		char	hndl_class [5];
		char	hzrd_class [5];
		char	prod_class [5];
		int		expiry_prd1;
		int		expiry_prd2;
		int		expiry_prd3;
		int		dflt_bom;
		int		dflt_rtg;
		float	eoq;
		int		last_bom;
		int		last_rtg;
		char	qa_status [2];
		char	stat_flag [2];
	}	INEI_STRUCT;

	/*
	 * Inventory Stock Take Control File.
	 */
	typedef	struct
	{
		long	hhcc_hash;
		char	stake_code [2];
		Date	start_date;
		char	start_time [6];
		Date	frz_date;
		char	frz_time [6];
		long	page_no;
		char	description [41];
		char	serial_take [2];
	}	INSC_STRUCT;

#endif	/* COSTSTRUCTS_H */
