/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( cm_req_prt.c )                                   |
|  Program Desc  : ( Print Requistion Dockets. This program is  )     |
|                  ( designed for use with lineflow, not with   )     |
|                  ( preprinted staionery.                      )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, ccmr, cmcd, cmhr, cmit, cmrd, cmrh, inmr    |
|                :  sokd, sokt,                                       |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates files :  cmrh,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.   | Date Written : (01/03/93)     |
|---------------------------------------------------------------------|
|  Date Modified : (15/11/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (10/09/97)      | Modified  by : Ana Marie Tario.  |
|  Date Modified : (09/09/1999)    | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  (15/11/95)    : PDL - Updated for version 9.                       |
|  (10/09/97)    : Incorporated multilingual conversion and DMY4 date.|
|  (09/09/1999)  : Ported to ANSI standards.                          |
|                :                                                    |
|                                                                     |
| $Log: cm_req_prt.c,v $
| Revision 5.2  2001/08/09 08:57:39  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 22:56:25  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:02:28  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:21:56  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:12:22  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:38:41  gerry
| Forced Revsions No Start to 2.0 Rel-15072000
|
| Revision 1.11  2000/02/18 01:19:55  scott
| Updated for small warning errors found when compiled with Linux.
|
| Revision 1.10  1999/12/06 01:32:36  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/11/08 04:35:41  scott
| Updated to correct warnings from usage of -Wall flag on compiler.
|
| Revision 1.8  1999/10/20 01:40:31  nz
| Updated for remainder of date routines
|
| Revision 1.7  1999/09/29 10:10:25  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/17 04:40:13  scott
| Updated for ctime -> SystemTime, datejul -> DateToString etc.
|
| Revision 1.5  1999/09/16 04:44:44  scott
| Updated from Ansi Project
|
| Revision 1.3  1999/06/14 07:35:08  scott
| Updated to add log in heading + updated for new gcc compiler.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_req_prt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_req_prt/cm_req_prt.c,v 5.2 2001/08/09 08:57:39 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_cm_mess.h>
#include	<twodec.h>

#define	ALLC_ND		0
#define	FREE_LST	1

#define	TAB_PG_LEN 	30

#define		NON_STOCK(xptr)	( xptr->_class[0] == 'Z' )
#define		PHANTOM(xptr)	( xptr->_class[0] == 'P' )
#define		SERIAL(xptr)	( xptr->serial_item[0] == 'Y' )

FILE *	fout;

char *	RULE_OFF = "==============================================================================================================================================================";
char *	MID_RULE = "|------------------------------------------------------------------------------------------------------------------------------------------------------------|";
char *	MID_BLNK = "|                                                                                                                                                            |";

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_cc_no"},
		{"comm_inv_date"}
	};

	int comm_no_fields = 6;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tcc_no[3];
		long	tinv_date;
	} comm_rec;

	/*==========================================
	| Cost Centre/Warehouse Master File Record |
	==========================================*/
	struct dbview ccmr_list[] ={
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
		{"ccmr_hhlo_hash"},
		{"ccmr_name"},
		{"ccmr_acronym"},
		{"ccmr_lpno"},
	};

	int	ccmr_no_fields = 8;

	struct	{
		char	mr_co_no[3];
		char	mr_est_no[3];
		char	mr_cc_no[3];
		long	mr_hhcc_hash;
		long	mr_hhlo_hash;
		char	mr_name[41];
		char	mr_acronym[10];
		int	mr_lpno;
	} ccmr_rec;

	/*===============================================+
	 | Contract Management Contract Description File |
	 +===============================================*/
#define	CMCD_NO_FIELDS	4

	struct dbview	cmcd_list [CMCD_NO_FIELDS] =
	{
		{"cmcd_hhhr_hash"},
		{"cmcd_line_no"},
		{"cmcd_text"},
		{"cmcd_stat_flag"}
	};

	struct tag_cmcdRecord
	{
		long	hhhr_hash;
		int		line_no;
		char	text [71];
		char	stat_flag [2];
	}	cmcd_rec;

	/*=========================================+
	 | cmhr - Contract Management Header File. |
	 +=========================================*/
#define	CMHR_NO_FIELDS	35

	struct dbview	cmhr_list [CMHR_NO_FIELDS] =
	{
		{"cmhr_co_no"},
		{"cmhr_br_no"},
		{"cmhr_cont_no"},
		{"cmhr_hhhr_hash"},
		{"cmhr_mast_hhhr"},
		{"cmhr_hhcu_hash"},
		{"cmhr_hhit_hash"},
		{"cmhr_cus_ref"},
		{"cmhr_contact"},
		{"cmhr_adr1"},
		{"cmhr_adr2"},
		{"cmhr_adr3"},
		{"cmhr_it_date"},
		{"cmhr_wip_date"},
		{"cmhr_st_date"},
		{"cmhr_due_date"},
		{"cmhr_end_date"},
		{"cmhr_hhjt_hash"},
		{"cmhr_wip_status"},
		{"cmhr_quote_type"},
		{"cmhr_progress"},
		{"cmhr_anni_day"},
		{"cmhr_quote_val"},
		{"cmhr_est_costs"},
		{"cmhr_est_prof"},
		{"cmhr_usr_ref1"},
		{"cmhr_usr_ref2"},
		{"cmhr_usr_ref3"},
		{"cmhr_usr_ref4"},
		{"cmhr_usr_ref5"},
		{"cmhr_internal"},
		{"cmhr_lab_rate"},
		{"cmhr_oh_rate"},
		{"cmhr_status"},
		{"cmhr_premise"},
	};

	struct tag_cmhrRecord
	{
		char	co_no[3];
		char	br_no[3];
		char	cont_no[7];
		long	hhhr_hash;
		long	mast_hhhr;
		long	hhcu_hash;
		long	hhit_hash;
		char	cus_ref[21];
		char	contact[41];
		char	adr[3][41];
		long	it_date;
		long	wip_date;
		long	st_date;
		long	due_date;
		long	end_date;
		long	hhjt_hash;
		char	wip_status[5];
		char	quote_type[2];
		char	progress[2];
		char	anni_day[3];
		double	quote_val;	/* money */
		double	est_costs;	/* money */
		float	est_prof;
		char	usr_ref[5][5];
		char	internal[2];
		double	lab_rate;	/* money */
		double	oh_rate;	/* money */
		char	status[2];
		char	premise[21];
	} cmhr_rec;

	/*==========================================+
	 | Contract Management Issue To Master File |
	 +==========================================*/
#define	CMIT_NO_FIELDS	6

	struct dbview	cmit_list [CMIT_NO_FIELDS] =
	{
		{"cmit_co_no"},
		{"cmit_issto"},
		{"cmit_hhit_hash"},
		{"cmit_iss_name"},
		{"cmit_hhsu_hash"},
		{"cmit_hhem_hash"}
	};

	struct tag_cmitRecord
	{
		char	co_no [3];
		char	issto [11];
		long	hhit_hash;
		char	iss_name [41];
		long	hhsu_hash;
		long	hhem_hash;
	}	cmit_rec;

	/*=============================================+
	 | Contract Management Requisition Detail File |
	 +=============================================*/
#define	CMRD_NO_FIELDS	16

	struct dbview	cmrd_list [CMRD_NO_FIELDS] =
	{
		{"cmrd_hhrq_hash"},
		{"cmrd_line_no"},
		{"cmrd_hhcm_hash"},
		{"cmrd_hhbr_hash"},
		{"cmrd_hhcc_hash"},
		{"cmrd_serial_no"},
		{"cmrd_location"},
		{"cmrd_qty_order"},
		{"cmrd_qty_border"},
		{"cmrd_qty_iss"},
		{"cmrd_cost"},
		{"cmrd_sale_price"},
		{"cmrd_disc_pc"},
		{"cmrd_due_date"},
		{"cmrd_item_desc"},
		{"cmrd_stat_flag"},
	};

	struct tag_cmrdRecord
	{
		long	hhrq_hash;
		int		line_no;
		long	hhcm_hash;
		long	hhbr_hash;
		long	hhcc_hash;
		char	serial_no[26];
		char	location[11];
		float	qty_ord;
		float	qty_bord;
		float	qty_iss;
		double	cost;		/* money */
		double	sale_price;	/* money */
		float	disc_pc;
		long	due_date;
		char	item_desc[41];
		char	stat_flag[2];
	} cmrd_rec;


	/*========================================+
	 | Contract Management Requisition Header |
	 +========================================*/
#define	CMRH_NO_FIELDS	22

	struct dbview	cmrh_list [CMRH_NO_FIELDS] =
	{
		{"cmrh_co_no"},
		{"cmrh_br_no"},
		{"cmrh_req_no"},
		{"cmrh_hhrq_hash"},
		{"cmrh_hhhr_hash"},
		{"cmrh_req_date"},
		{"cmrh_rqrd_date"},
		{"cmrh_iss_date"},
		{"cmrh_op_id"},
		{"cmrh_req_by"},
		{"cmrh_date_create"},
		{"cmrh_time_create"},
		{"cmrh_full_supply"},
		{"cmrh_printed"},
		{"cmrh_del_name"},
		{"cmrh_del_adr1"},
		{"cmrh_del_adr2"},
		{"cmrh_del_adr3"},
		{"cmrh_add_int1"},
		{"cmrh_add_int2"},
		{"cmrh_add_int3"},
		{"cmrh_stat_flag"}
	};

	struct tag_cmrhRecord
	{
		char	co_no [3];
		char	br_no [3];
		long	req_no;
		long	hhrq_hash;
		long	hhhr_hash;
		long	req_date;
		long	rqrd_date;
		long	iss_date;
		char	op_id [15];
		char	req_by [21];
		long	date_create;
		long	time_create;
		char	full_supply [2];
		char	printed [2];
		char	del_name [41];
		char	del_adr[3] [41];
		char	add_int[3] [41];
		char	stat_flag [2];
	}	cmrh_rec;

	/*==============================
	| Inventory Master File (inmr) |
	==============================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_hhsi_hash"},
		{"inmr_alpha_code"},
		{"inmr_supercession"},
		{"inmr_maker_no"},
		{"inmr_alternate"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_category"},
		{"inmr_serial_item"},
		{"inmr_pack_size"},
		{"inmr_outer_size"},
		{"inmr_bo_flag"}
	};

	int inmr_no_fields = 15;

	struct {
		char 	mr_co_no[3];
		char 	mr_item_no[17];
		long 	mr_hhbr_hash;
		long 	mr_hhsi_hash;
		char 	mr_alpha_code[17];
		char 	mr_super_no[17];
		char 	mr_maker_no[17];
		char 	mr_alternate[17];
		char 	mr_class[2];
		char 	mr_description[41];
		char 	mr_category[12];
		char	mr_serial_item[2];
		char	mr_pack_size[6];
		float	mr_outer_size;
		char	mr_bo_flag[2];
	} inmr_rec;

	/*========================================
	| Kit invoice/ packing slip detail file. |
	========================================*/
	struct dbview sokd_list[] ={
		{"sokd_co_no"},
		{"sokd_type"},
		{"sokd_hhbr_hash"},
		{"sokd_line_no"},
		{"sokd_text"}
	};

	int sokd_no_fields = 5;

	struct {
		char	kd_co_no[3];
		char	kd_type[2];
		long	kd_hhbr_hash;
		int		kd_line_no;
		char	kd_text[61];
	} sokd_rec;

	/*===========================
	| Sales Order kitting file. |
	===========================*/
	struct dbview sokt_list[] ={
		{"sokt_co_no"},
		{"sokt_hhbr_hash"},
		{"sokt_line_no"},
		{"sokt_mabr_hash"},
		{"sokt_matl_qty"},
		{"sokt_due_date"},
		{"sokt_bonus"}
	};

	int sokt_no_fields = 7;

	struct {
		char	kt_co_no[3];
		long	kt_hhbr_hash;
		int		kt_line_no;
		long	kt_mabr_hash;
		float	kt_matl_qty;
		long	kt_due_date;
		char	kt_bonus[2];
	} sokt_rec;

	char	*data = "data",
			*comm = "comm",
			*ccmr = "ccmr",
			*cmcd = "cmcd",
			*cmhr = "cmhr",
			*cmit = "cmit",
			*cmrd = "cmrd",
			*cmrh = "cmrh",
			*inmr = "inmr",
			*sokd = "sokd",
			*sokt = "sokt";
	
	int		lpno;
	int		prev_lpno;
	int		pipe_open;
	int		part_printed = FALSE;
	int		line_spacing = 1;
	int		page_no;
	int		line_no;
	int		multiple;
	
/*-----------------------------
| Structures for linked list. |
-----------------------------*/
struct WH_LIST
{
	int		lpno;
	long	hhcc_hash;
	char	wh_no[3];
	char	wh_name[41];
	struct	DTL_LIST *head_ptr;
	struct	DTL_LIST *tail_ptr;

	struct	WH_LIST	*next;
};
#define	WH_NULL	((struct WH_LIST *) NULL)

struct	DTL_LIST
{
	long	hhbr_hash;
	char	_class[2];
	char	item_no[17];
	char	item_desc[41];
	float	qty_ord;
	float	qty_bord;
	char	serial_item[2];
	char	ser_no[26];
	char	location[11];

	struct	DTL_LIST *next;
};
#define	DTL_NULL ((struct DTL_LIST *) NULL)
struct	WH_LIST	*wh_head = WH_NULL;
struct	WH_LIST	*wh_tail = WH_NULL;

struct
{
	char	item_desc[41];
	float	qty_ord;
	float	qty_bord;
	char	serial_no[26];
	char	location[11];
	long	j_date;
	char	systemDate[11];
	char	sys_time[6];
	char	message[41];
	char	full_supply[31];
	int	lpno;
} local_rec;

/*===========================
| Local function prototypes |
===========================*/
void				OpenDB			(void);
void				CloseDB		(void);
int					process			(long hash);
int					load_list		(void);
int					clear_list		(void);
struct WH_LIST *	find_wh_node	(long hhcc_hash);
int					process_wh		(struct WH_LIST *wh_ptr);
void				page_head		(struct WH_LIST *wh_ptr);
void				page_trail		(int prt_sign);
void				blank_dtls		(void);
int					proc_phantom	(struct WH_LIST *wh_ptr, struct DTL_LIST *dtl_ptr);
void				open_output		(int pipe_lpno);
void				close_output	(void);
struct DTL_LIST *	dtl_node		(int node_act, struct DTL_LIST *lst_ptr);
struct WH_LIST *	wh_node			(int node_act, struct WH_LIST *lst_ptr);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{
	long	hhrq_hash;
	char	_line[128],
			*sptr;

	if (argc != 2 && argc != 3)
	{
		print_at(0,0, mlStdMess036,argv[0]);
		print_at(1,0, mlCmMess747, argv[0]);
		return (EXIT_FAILURE);
	}

	OpenDB ();

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));

	prev_lpno = 0;
	if (argc == 3)
	{
		/*-------------------------------
		| Process a single requisition. |
		-------------------------------*/
		lpno = atoi (argv[1]);
		hhrq_hash = atol (argv[2]);
		process (hhrq_hash);
	}
	else
	{
		lpno = atoi(argv[1]);

		/*----------------------------------
		| Process a multiple requisitions. |
		----------------------------------*/
		sptr = gets(_line);
		if (!sptr)
			return (EXIT_FAILURE);

		hhrq_hash = atol(sptr);
		while (sptr && !feof(stdin) && hhrq_hash != 0L)
		{
			strcpy (local_rec.sys_time, TimeHHMM ());
			process(hhrq_hash);
	
			sptr = gets(_line);
			if (sptr)
				hhrq_hash = atol(sptr);
		}
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
	abc_dbopen(data);

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, ccmr_no_fields, "ccmr_hhcc_hash");
	open_rec (cmcd, cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cmit, cmit_list, CMIT_NO_FIELDS, "cmit_hhit_hash");
	open_rec (cmhr, cmhr_list, CMHR_NO_FIELDS, "cmhr_hhhr_hash");
	open_rec (cmrd, cmrd_list, CMRD_NO_FIELDS, "cmrd_id_no");
	open_rec (cmrh, cmrh_list, CMRH_NO_FIELDS, "cmrh_hhrq_hash");
	open_rec (inmr, inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec (sokd, sokd_list, sokd_no_fields, "sokd_id_no");
	open_rec (sokt, sokt_list, sokt_no_fields, "sokt_id_no");
}

/*========================	
| Close ALL open tables. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (ccmr);
	abc_fclose (cmcd);
	abc_fclose (cmhr);
	abc_fclose (cmit);
	abc_fclose (cmrd);
	abc_fclose (cmrh);
	abc_fclose (inmr);
	abc_fclose (sokd);
	abc_fclose (sokt);
	abc_dbclose (data);
}

/*----------------------
| Print a requisition. |
----------------------*/
int
process (
 long	hash)
{
	struct	WH_LIST	*lcl_ptr;

	/*---------------------
	| Find header record. |
	---------------------*/
	cc = find_hash(cmrh, &cmrh_rec, COMPARISON, "u", hash);
	if (cc)
		return(FALSE);

	/*-----------------------------------------------
	| Ignore (C)losed, (F)orward and (B)ackordered. |
	-----------------------------------------------*/
	if (cmrh_rec.stat_flag[0] != 'R')
	{
		abc_unlock(cmrh);
		return(FALSE);
	}

	/*------------------------------
	| Find contract header record. |
	------------------------------*/
	cc = find_hash(cmhr, &cmhr_rec, COMPARISON, "r", cmrh_rec.hhhr_hash);
	if (cc)
	{
		abc_unlock(cmrh);
		return(FALSE);
	}
	
	/*-----------------------------
	| Read first description line |
	-----------------------------*/
	cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	strcpy (cmcd_rec.stat_flag, "D");
	cmcd_rec.line_no = 0;
	cc = find_rec(cmcd, &cmcd_rec, COMPARISON, "r");
	if (cc)
		sprintf (cmcd_rec.text, "%-70.70s", "Description Not Found.");

	/*-------------------------
	| Lookup issue to record. |
	-------------------------*/
	if (cmhr_rec.hhit_hash != 0L)
	{
		cc = find_hash(cmit, &cmit_rec, COMPARISON, "r", cmhr_rec.hhit_hash);
		if (cc)
		{
			strcpy (cmit_rec.issto, "          ");
			sprintf (cmit_rec.iss_name, "%-40.40s", " ");
		}
	}
	else
	{
		strcpy (cmit_rec.issto, "          ");
		sprintf (cmit_rec.iss_name, "%-40.40s", " ");
	}

	/*-------------------------------------------
	| Load linked list for current requisition. |
	-------------------------------------------*/
	load_list();

	/*------------------------------------------------------
	| For each WH open a pipe to pformat and print details |
	------------------------------------------------------*/
	lcl_ptr = wh_head;
	while (lcl_ptr != WH_NULL)
	{
		/*---------------------------
		| Change in printer number. |
		---------------------------*/
		if (prev_lpno != lcl_ptr->lpno)
		{
			/*---------------------
			| Close pipe if open. |
			---------------------*/
			if (pipe_open)
			{
				fprintf (fout, ".EOF\n");
				pclose(fout);
			}
			/*----------------
			| Open new pipe. |
			----------------*/
			open_output(lcl_ptr->lpno);
			prev_lpno = lcl_ptr->lpno;
		}

		/*--------------------------------------------------
		| Print requisition details for current warehouse. |
		--------------------------------------------------*/
		process_wh (lcl_ptr);

		lcl_ptr = lcl_ptr->next;
	}
	
	/*----------------------
	| Update printed flag. |
	----------------------*/
	strcpy (cmrh_rec.printed, "Y");
	cc = abc_update(cmrh, &cmrh_rec);
	if (cc)
		file_err(cc, cmrh, "DBUPDATE");

	/*--------------------
	| Clear linked list. |
	--------------------*/
	clear_list ();

	return (FALSE);
}

/*-----------------------------------------------------------------
| Load linked list for current requisition.                       |
|                                                                 |
| The structure of the list is as follows:                        |
|                                                                 |
|      HEAD POINTER                                               |
|           |                                                     |
|           |                                                     |
|           v                                                     |
|      +----------+                                               |
|      | WH_LIST  |   +--------+   +--------+                     |
|      |  NODE    |-->| DETAIL |-->| DETAIL |-->DTL_NULL          |
|      |          |   +--------+   +--------+                     |
|      +----------+                                               |
|           |                                                     |
|           |                                                     |
|           v                                                     |
|      +----------+                                               |
|      | WH_LIST  |   +--------+                                  |
|      |  NODE    |-->| DETAIL |-->DTL_NULL                       |
|      |          |   +--------+                                  |
|      +----------+                                               |
|           |                                                     |
|           |                                                     |
|           v                                                     |
|        WH_NULL                                                  |
-----------------------------------------------------------------*/
int
load_list (
 void)
{
	struct	DTL_LIST *lcl_ptr;
	struct	WH_LIST  *wh_ptr;

	/*--------------------------
	| Clear any existing list. |
	--------------------------*/

	cmrd_rec.hhrq_hash = cmrh_rec.hhrq_hash;
	cmrd_rec.line_no = 0;
	cc = find_rec(cmrd, &cmrd_rec, GTEQ, "r");
	while (!cc && cmrd_rec.hhrq_hash == cmrh_rec.hhrq_hash)
	{
		/*--------------------------
		| Lookup inventory record. |
		--------------------------*/
		cc = find_hash(inmr, &inmr_rec,COMPARISON, "r", cmrd_rec.hhbr_hash);
		if (cc)
		{
			cc = find_rec(cmrd, &cmrd_rec, NEXT, "r");
			continue;
		}

		/*----------------------------------
		| Allocate node for detail record. |
		----------------------------------*/
		lcl_ptr = dtl_node (ALLC_ND, DTL_NULL);
		strcpy (lcl_ptr->serial_item, inmr_rec.mr_serial_item);
		sprintf (lcl_ptr->ser_no, "%-25.25s", cmrd_rec.serial_no);
		sprintf (lcl_ptr->location, "%-10.10s", cmrd_rec.location);
		sprintf (lcl_ptr->_class, "%-1.1s", inmr_rec.mr_class);
		sprintf (lcl_ptr->item_no, "%-16.16s", inmr_rec.mr_item_no);
		sprintf (lcl_ptr->item_desc, "%-40.40s",inmr_rec.mr_description);
		lcl_ptr->hhbr_hash 	= inmr_rec.mr_hhbr_hash;
		lcl_ptr->qty_ord 	= cmrd_rec.qty_ord;
		lcl_ptr->qty_bord 	= cmrd_rec.qty_bord;
		lcl_ptr->next = DTL_NULL;

		/*----------------------
		| Find warehouse node. |
		----------------------*/
		wh_ptr = find_wh_node(cmrd_rec.hhcc_hash);

		/*---------------------------------------------
		| Append after last record on warehouse list. |
		---------------------------------------------*/
		if (wh_ptr->tail_ptr == DTL_NULL)
		{
			wh_ptr->head_ptr = lcl_ptr;
			wh_ptr->tail_ptr = lcl_ptr;
		}
		else
		{
			wh_ptr->tail_ptr->next = lcl_ptr;
			wh_ptr->tail_ptr = lcl_ptr;
		}

		cc = find_rec(cmrd, &cmrd_rec, NEXT, "r");
	}

	return(TRUE);
}

/*--------------------
| Clear linked list. |
--------------------*/
int
clear_list (
 void)
{
	struct	WH_LIST  *wh_ptr;

	wh_ptr = wh_head;
	while (wh_ptr != WH_NULL)
	{
		/*---------------------------
		| Clear detail list for WH. |
		---------------------------*/
		dtl_node(FREE_LST, wh_ptr->head_ptr);

		wh_ptr = wh_ptr->next;
	}

	/*---------------------------
	| Clear detail list for WH. |
	---------------------------*/
	wh_node(FREE_LST, wh_head);
	wh_head = WH_NULL;
	wh_tail = WH_NULL;

	return (EXIT_SUCCESS);
}

/*----------------------
| Find warehouse node. |
----------------------*/
struct WH_LIST *
find_wh_node (
 long	hhcc_hash)
{
	int	pos_found;
	struct	WH_LIST *lcl_ptr, *wrk_ptr, *pos_ptr;

	lcl_ptr = wh_head;
	while (lcl_ptr != WH_NULL)
	{
		/*-----------------------
		| Found warehouse node. |
		-----------------------*/
		if (lcl_ptr->hhcc_hash == hhcc_hash)
			return(lcl_ptr);

		lcl_ptr = lcl_ptr->next;
	}

	/*-------------------------------------------
	| Warehouse not in list yet so add to list. |
	| Insert in lpno order.                     |
	-------------------------------------------*/
	cc = find_hash(ccmr, &ccmr_rec, COMPARISON, "r", hhcc_hash);
	if (cc)
		file_err(cc, ccmr, "DBFIND");
	lcl_ptr = wh_node(ALLC_ND, WH_NULL);
	lcl_ptr->hhcc_hash = hhcc_hash;
	sprintf (lcl_ptr->wh_no, "%2.2s", ccmr_rec.mr_cc_no);
	sprintf (lcl_ptr->wh_name, "%-40.40s", ccmr_rec.mr_name);
	lcl_ptr->lpno = (ccmr_rec.mr_lpno == 0) ? lpno : ccmr_rec.mr_lpno;
	lcl_ptr->head_ptr = DTL_NULL;
	lcl_ptr->tail_ptr = DTL_NULL;
	lcl_ptr->next = WH_NULL;

	/*---------------------------------------------
	| Find position in linked list based on lpno. |
	---------------------------------------------*/
	pos_found = FALSE;
	wrk_ptr = wh_head;
	pos_ptr = wh_head;
	while (wrk_ptr != WH_NULL)
	{
		if (lcl_ptr->lpno <= wrk_ptr->lpno)
		{
			pos_found = TRUE;
			break;
		}

		pos_ptr = wrk_ptr;
		wrk_ptr = wrk_ptr->next;
	}

	if (pos_found)
	{
		if (pos_ptr == wh_head)
		{
			/*-------------
			| Head insert |
			-------------*/
			lcl_ptr->next = wh_head;
			wh_head = lcl_ptr;
		}
		else
		{
			lcl_ptr->next = pos_ptr->next;
			pos_ptr->next = lcl_ptr;
		}
	}
	else
	{
		/*--------
		| Append |
		--------*/
		if (wh_head == WH_NULL)
		{
			wh_head = lcl_ptr;
			wh_tail = lcl_ptr;
		}
		else
		{
			wh_tail->next = lcl_ptr;
			wh_tail = lcl_ptr;
		}
	}
	
	return(lcl_ptr);
}

/*----------------------------------
| Main process for a requisition.  |
----------------------------------*/
int
process_wh (
 struct WH_LIST *	wh_ptr)
{
	struct  DTL_LIST *lcl_ptr;

	/*---------------------
	| Print heading info. |
	---------------------*/
	page_no = 0;
	page_head(wh_ptr);
	line_no = 0;

	/*---------------------
	| Print Detail Lines. |
	---------------------*/
	lcl_ptr = wh_ptr->head_ptr;
	while (lcl_ptr != DTL_NULL)
	{
		if (++line_no > TAB_PG_LEN)
		{
			page_trail(FALSE);
			page_head(wh_ptr);
			line_no = 1;
		}

		/*-------------------------------
		| Make sure serial item and its |
		| serial no are on same page.   |
		-------------------------------*/
		if (SERIAL(lcl_ptr) && (line_no + 1) > TAB_PG_LEN)
		{
			/*------------------------------------
			| Print blank line for current line. |
			------------------------------------*/
			fprintf (fout, "|     |%-10.10s", " ");
			fprintf (fout, "|%-16.16s",       " ");
			fprintf (fout, "|%-40.40s",       " ");
			fprintf (fout, "|%-12.12s",       " ");
			fprintf (fout, "|%-12.12s",       " ");
			fprintf (fout, "|%-12.12s| %-40.40s |\n", " ", " ");

			/*-------------------------------------
			| Print blank lines for rest of page. |
			-------------------------------------*/
			blank_dtls ();

			/*-------------
			| Page break. |
			-------------*/
			page_trail(FALSE);
			page_head(wh_ptr);
		}

		/*---------------------
		| Print line details. |
		---------------------*/
		fprintf (fout, "|     |%-10.10s", lcl_ptr->location);
		fprintf (fout, "|%-16.16s",       lcl_ptr->item_no);
		fprintf (fout, "|%-40.40s",       lcl_ptr->item_desc);
		fprintf (fout, "|%12.2f",         lcl_ptr->qty_ord);
		fprintf (fout, "|%12.2f",         lcl_ptr->qty_bord);
		fprintf (fout, "|%-12.12s| ", " ");

		if (PHANTOM(lcl_ptr))
		{
			fprintf (fout, "PHANTOM KIT%-30.30s|\n", " ");
			proc_phantom (wh_ptr, lcl_ptr);
		}
		else
			fprintf (fout, "%-40.40s |\n", " ");

		/*----------------------
		| Print serial number. |
		----------------------*/
		if (SERIAL(lcl_ptr))
		{
			fprintf (fout, "|     |%-10.10s", " ");
			fprintf (fout, " %-18.18s", "SERIAL NUMBER");
			fprintf (fout, "%-25.25s", lcl_ptr->ser_no);
			fprintf (fout, "%-14.14s", " ");
			fprintf (fout, "|%-12.12s",       " ");
			fprintf (fout, "|%-12.12s",       " ");
			fprintf (fout, "|%-12.12s| %-40.40s |\n", " ", " ");
			line_no++;
		}

		lcl_ptr = lcl_ptr->next;
	}

	/*--------------------------------------
	| Print empty part of tabular section. |
	--------------------------------------*/
	if (line_no < TAB_PG_LEN)
		blank_dtls ();

	/*---------------------
	| Print Page Trailer. |
	---------------------*/
	page_trail(TRUE);

	return (EXIT_SUCCESS);
}

/*-----------------------
| Print header section. |
-----------------------*/
void
page_head (
 struct WH_LIST *	wh_ptr)
{
	page_no++;
	line_no = 1;

	fprintf (fout, 
		"REQUISITION NUMBER : %06ld %120.120s %3d\n", 
		cmrh_rec.req_no,
		"PAGE NO", 
		page_no);

	fprintf (fout, "%s\n", RULE_OFF);
	fprintf (fout, "| DELIVERY NAME : %-40.40s", cmrh_rec.del_name);
	fprintf (fout, "      CONTRACT NO : %-6.6s", cmhr_rec.cont_no);
	fprintf (fout, "  %-70.70s |\n", cmcd_rec.text);

	fprintf (fout, "| DELIVERY ADDRESS : %-40.40s", cmrh_rec.del_adr[0]);
	fprintf (fout, "   REQUESTED BY     : %-20.20s ", cmrh_rec.req_by);
	fprintf (fout, "%-50.50s   |\n", " ");

	fprintf (fout, "|                  : %-40.40s", cmrh_rec.del_adr[1]);
	fprintf (fout, "   ISSUE TO         : %-10.10s", cmit_rec.issto);
	fprintf (fout, "   %-40.40s", cmit_rec.iss_name);
	fprintf (fout, "%-20.20s |\n", " ");

	fprintf (fout, "|                  : %-40.40s", cmrh_rec.del_adr[2]);
	fprintf (fout, "   REQUISITION DATE :%s", DateToString(cmrh_rec.req_date));
	fprintf (fout, "%-65.65s|\n", " ");

	fprintf (fout, "|%-63.63s", " ");
	fprintf (fout, "REQUIRED DATE   :%s", DateToString(cmrh_rec.rqrd_date));
	fprintf (fout, "%-65.65s |\n", " ");

	fprintf (fout, "%s\n", MID_RULE);
	fprintf (fout, "%s\n", MID_BLNK);

	fprintf (fout, 
		"|     WAREHOUSE : %2.2s %-40.40s %95.95s|\n", 
		wh_ptr->wh_no, 
		wh_ptr->wh_name,
		" ");
	fprintf (fout, "%s\n", MID_BLNK);

	/*-------------------------------
	| Headings for tabular section. |
	-------------------------------*/
	fprintf (fout, "|     -----------");
	fprintf (fout, "-----------------");
	fprintf (fout, "-----------------------------------------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------- %-40.40s |\n", " ");

	fprintf (fout, "|     | LOCATION ");
	fprintf (fout, "|  ITEM NUMBER   ");
	fprintf (fout, "|          ITEM  DESCRIPTION             ");
	fprintf (fout, "|  QUANTITY  ");
	fprintf (fout, "|QTY B/ORDER ");
	fprintf (fout, "| QTY ISSUED | %-40.40s |\n", " ");

	fprintf (fout, "|     +----------");
	fprintf (fout, "+----------------");
	fprintf (fout, "+----------------------------------------");
	fprintf (fout, "+------------");
	fprintf (fout, "+------------");
	fprintf (fout, "+------------+ %-40.40s |\n", " ");
}

/*------------------------
| Print trailer section. |
------------------------*/
void
page_trail (
 int prt_sign)
{
	fprintf (fout, "|     -----------");
	fprintf (fout, "-----------------");
	fprintf (fout, "-----------------------------------------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------- %-40.40s |\n", " ");

	fprintf (fout, "%s\n", MID_BLNK);

	fprintf (fout, 
		"|     ADDITIONAL INSTRUCTIONS : %-40.40s %-80.80s    |\n",
		cmrh_rec.add_int[0], 
		" ");
	fprintf (fout, 
		"|                             : %-40.40s %-80.80s    |\n",
		cmrh_rec.add_int[1], 
		" ");
	fprintf (fout, 
		"|                             : %-40.40s",
		cmrh_rec.add_int[2]);

	/*------------------
	| Print signature. |
	------------------*/
	if (prt_sign)
	{
		fprintf (fout, 
			"%-30.30sDATE : __/__/__      ", " ");
		fprintf (fout, 
			"SIGNATURE : _____________         |\n");
	}
	else
		fprintf (fout, " %-80.80s    |\n", " ");

	fprintf (fout, "%s\n", RULE_OFF);
	fprintf (fout, ".PA\n");
	fflush(fout);
}

/*--------------------------------------
| Print empty part of tabular section. |
--------------------------------------*/
void
blank_dtls (
 void)
{
	for(; line_no < TAB_PG_LEN; line_no++)
	{
		fprintf (fout, "|     |%-10.10s", " ");
		fprintf (fout, "|%-16.16s",       " ");
		fprintf (fout, "|%-40.40s",       " ");
		fprintf (fout, "|%-12.12s",       " ");
		fprintf (fout, "|%-12.12s",       " ");
		fprintf (fout, "|%-12.12s| %-40.40s |\n", " ", " ");
	}
}

/*-------------------------------
| Process BOM for phantom items |
-------------------------------*/
int
proc_phantom (
 struct WH_LIST *	wh_ptr,
 struct DTL_LIST *	dtl_ptr)
{
	/*-----------
	| Print BOM |
	-----------*/
	strcpy (sokt_rec.kt_co_no, cmrh_rec.co_no);
	sokt_rec.kt_hhbr_hash = dtl_ptr->hhbr_hash;
	sokt_rec.kt_line_no = 0;
	cc = find_rec(sokt, &sokt_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (sokt_rec.kt_co_no, cmrh_rec.co_no) &&
	       sokt_rec.kt_hhbr_hash == dtl_ptr->hhbr_hash)
	{
		cc = find_hash(inmr, &inmr_rec, COMPARISON, "r", 
			       sokt_rec.kt_mabr_hash);
		if (cc)
		{
			fprintf (fout, "%-34.34sITEM NOT FOUND ON FILE\n", " ");
			cc = find_rec(sokt, &sokt_rec, NEXT, "r");
			continue;
		}

		/*-------------
		| Page break. |
		-------------*/
		if (++line_no > TAB_PG_LEN)
		{
			page_trail (FALSE);
			page_head (wh_ptr);
			line_no = 1;
		}

		/*--------------------
		| Print detail line. |
		--------------------*/
		fprintf (fout,"|     |            %-16.16s",inmr_rec.mr_item_no);
		fprintf (fout, " %-40.40s", inmr_rec.mr_description);
		fprintf (fout, 
			"%12.2f|%-12.12s|%-12.12s|%-42.42s|\n",
			cmrd_rec.qty_ord * sokt_rec.kt_matl_qty,
			" ", " ", " ");

		cc = find_rec(sokt, &sokt_rec, NEXT, "r");
	}

	/*---------------------------
	| Print Instruction Details |
	---------------------------*/
	strcpy (sokd_rec.kd_co_no, cmrh_rec.co_no);
	strcpy (sokd_rec.kd_type,  "P");
	sokd_rec.kd_hhbr_hash = dtl_ptr->hhbr_hash;
	sokd_rec.kd_line_no = 0;
	cc = find_rec(sokd, &sokd_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (sokd_rec.kd_co_no, cmrh_rec.co_no) &&
	       sokd_rec.kd_hhbr_hash == dtl_ptr->hhbr_hash &&
	       sokd_rec.kd_type[0] == 'P' )
	{
		/*-------------
		| Page break. |
		-------------*/
		if (++line_no > TAB_PG_LEN)
		{
			page_trail (FALSE);
			page_head (wh_ptr);
			line_no = 1;
		}
		fprintf (fout, "|     |            ");
		fprintf (fout, 
			" %-40.40s   %-25.25s|%-12.12s|%-12.12s|%-42.42s|\n",
			sokd_rec.kd_text,
			" ", " ", " ", " ");

		cc = find_rec(sokd, &sokd_rec, NEXT, "r");
	}

	return (EXIT_SUCCESS);
}

/*-------------------------------------------------------
| Routine to open output pipe to the spooler.           |
-------------------------------------------------------*/
void
open_output (
 int pipe_lpno)
{
	if ((fout = popen ("pformat", "w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);
	pipe_open = TRUE;

	sprintf (err_str, "%s<%s>", local_rec.systemDate, PNAME);
	fprintf (fout, ".START%s\n", clip (err_str));
	fprintf (fout, ".LP%d\n", pipe_lpno);
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".OP\n");
	fprintf (fout, ".2\n");
	fprintf (fout, 
		".E %s REQUISITION DOCKET\n", 
		(cmrh_rec.printed[0] == 'Y') ? "REPRINT" : "");
	fprintf (fout, ".B1\n");
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
void
close_output (
 void)
{
	fprintf (fout, ".EOF\n");
	pclose (fout);
}

/*-------------------------------
| Allocate or Free memory for a |
| node OR Free The Whole List   |
-------------------------------*/
struct DTL_LIST *
dtl_node (
 int				node_act,
 struct DTL_LIST *	lst_ptr)
{
	int	i = 0;
	struct	DTL_LIST *lcl_ptr = (struct DTL_LIST *) 0;

	switch (node_act)
	{
	case ALLC_ND:
		/*-----------------
		| Allocate A Node |
		-----------------*/
		while (i < 100)
		{
			lcl_ptr = (struct DTL_LIST *)malloc (sizeof (struct DTL_LIST));
			if (lcl_ptr != DTL_NULL)
				break;
			i++;
			sleep (sleepTime);
		}
		if (lcl_ptr == DTL_NULL)
			sys_err("Error in dtl_node() During (MALLOC)",12,PNAME);

		return (lcl_ptr);

	case FREE_LST:
		/*------------------
		| Free entire list |
		------------------*/
		while (lst_ptr != DTL_NULL)
		{
			lcl_ptr = lst_ptr;
			lst_ptr = lst_ptr->next;
			free(lcl_ptr);
		}

		lst_ptr = DTL_NULL;
		return (DTL_NULL);
	}
	return (DTL_NULL);
}

/*-------------------------------
| Allocate or Free memory for a |
| node OR Free The Whole List   |
-------------------------------*/
struct WH_LIST *
wh_node (
 int				node_act,
 struct WH_LIST *	lst_ptr)
{
	int	i = 0;
	struct	WH_LIST *lcl_ptr = (struct WH_LIST *) 0;

	switch (node_act)
	{
	case ALLC_ND:
		/*-----------------
		| Allocate A Node |
		-----------------*/
		while (i < 100)
		{
			lcl_ptr = (struct WH_LIST *)malloc (sizeof (struct WH_LIST));
			if (lcl_ptr != WH_NULL)
				break;
			i++;
			sleep (sleepTime);
		}
		if (lcl_ptr == WH_NULL)
			sys_err("Error in wh_node() During (MALLOC)", 12,PNAME);
		
		return (lcl_ptr);

	case FREE_LST:
		/*------------------
		| Free entire list |
		------------------*/
		while (lst_ptr != WH_NULL)
		{
			lcl_ptr = lst_ptr;
			lst_ptr = lst_ptr->next;
			free(lcl_ptr);
		}

		lst_ptr  = WH_NULL;
		return (WH_NULL);
	}
	return (WH_NULL);
}

/*
dbg_prt()
{
	struct	WH_LIST	 *lcl_wh;
	struct	DTL_LIST *lcl_dtl;

	open_output();

	lcl_wh = wh_head;
	while (lcl_wh != WH_NULL)
	{
		fprintf (fout, "+---------------------------------------------------+\n");
		fprintf (fout, 
			"| hhcc[%5ld]%39s|\n", 
			lcl_wh->hhcc_hash, " ");
		fprintf (fout, 
			"| wh_no[%-40.40s]%3s|\n", 
			lcl_wh->wh_no, " ");
		fprintf (fout, 
			"| wh_name[%-40.40s]%1s|\n", 
			lcl_wh->wh_name, " ");
		fprintf (fout, 
			"| lpno[%d]%43s|\n", 
			lcl_wh->lpno, " ");
		fprintf (fout, "+---------------------------------------------------+\n");
		fflush(fout);

		----------------------
		| Print details for WH |
		----------------------
		lcl_dtl = lcl_wh->head_ptr;
		while (lcl_dtl != DTL_NULL)
		{
			fprintf (fout, "      +-----------------\n");
			fprintf (fout, "      | hhbr[%5ld]\n", lcl_dtl->hhbr_hash);
			fprintf (fout, "      | qty_ord[%f]\n", lcl_dtl->qty_ord);
			fprintf (fout, "      | qty_bord[%f]\n", lcl_dtl->qty_bord);
			fprintf (fout, "      +-----------------\n");
	
			lcl_dtl = lcl_dtl->next;
		}

		fprintf (fout, "\n");

		lcl_wh = lcl_wh->next;
	}

	fprintf (fout, ".EOF\n");
	pclose(fout);
}
*/
