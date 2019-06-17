/*=====================================================================
|  Copyright (C) 2000 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: GlUtils.h,v 5.2 2001/08/20 23:15:22 scott Exp $
|  Program Name  : (GlUtils.h) 
|  Program Desc  : (Standard General Ledger Utilities header)
|=====================================================================|
| $Log: GlUtils.h,v $
| Revision 5.2  2001/08/20 23:15:22  scott
| Updated for development related to bullet proofing
|
| Revision 5.1  2001/07/25 01:01:56  scott
| Updated for 10.5
|
=====================================================================*/
#ifndef	_GlUtils_h
#define	_GlUtils_h

#include	<GlStructure.h>

#ifdef	CCMAIN
#	define	GLOB_GL
#else
#	define	GLOB_GL	extern
#endif

#ifdef	CCMAIN

	extern	int		GV_max_level,
					GV_cur_level,
					GV_link_cnt;
	extern	long	GV_post_time;
	extern	int	 	_SpecialRedraw;

	/*
	 * General ledger Budget header.
	 */
	struct dbview	glbd_list [] =
	{
		{"glbd_co_no"},
		{"glbd_budg_no"},
		{"glbd_desc"},
		{"glbd_status"},
		{"glbd_mod_date"},
		{"glbd_stat_flag"}
	};

	/*
	 * General Ledger batch control file header.
	 */
	struct dbview	glbh_list [] =
	{
		{"glbh_co_no"},
		{"glbh_br_no"},
		{"glbh_jnl_type"},
		{"glbh_batch_no"},
		{"glbh_hhbh_hash"},
		{"glbh_user"},
		{"glbh_date"},
		{"glbh_time"},
		{"glbh_stat_flag"},
		{"glbh_mth"},
		{"glbh_other_module"}
	};

	/*
	 * General Ledger batch control line item file.
	 */
	struct dbview	glbl_list [] =
	{
		{"glbl_hhbh_hash"},
		{"glbl_line_no"},
		{"glbl_acc_no"},
		{"glbl_acronym"},
		{"glbl_name"},
		{"glbl_chq_inv_no"},
		{"glbl_ci_amt"},
		{"glbl_o1_amt"},
		{"glbl_o2_amt"},
		{"glbl_o3_amt"},
		{"glbl_o4_amt"},
		{"glbl_hhgl_hash"},
		{"glbl_tran_type"},
		{"glbl_sys_ref"},
		{"glbl_batch_no"},
		{"glbl_tran_date"},
		{"glbl_period_no"},
		{"glbl_narrative"},
		{"glbl_alt_desc1"},
		{"glbl_alt_desc2"},
		{"glbl_alt_desc3"},
		{"glbl_user_ref"},
		{"glbl_fx_amt"},
		{"glbl_local_amt"},
		{"glbl_dc_flag"},
		{"glbl_currency"},
		{"glbl_exch_rate"},
		{"glbl_stat_flag"}
	};

	/*
	 * General ledger chart of accounts record.
	 */
	struct dbview	glca_list [] =
	{
		{"glca_co_no"},
		{"glca_level_no"},
		{"glca_acc_no"},
		{"glca_hhca_hash"},
		{"glca_acc_desc"},
		{"glca_mod_date"},
		{"glca_stat_flag"}
	};

	/*
	 * General ledger control record.
	 */
	struct dbview	glct_list [] =
	{
		{"glct_format"},
		{"glct_fiscal"},
		{"glct_history"},
		{"glct_link_max"},
		{"glct_ptype"},
		{"glct_max_budg"},
		{"glct_nxt_budg"},
		{"glct_mod_date"},
		{"glct_stat_flag"}
	};

	/*
	 * General Ledger Interface Detail File. 
	 */
	struct dbview	glid_list [] =
	{
		{"glid_hhih_hash"},
		{"glid_br_no"},
		{"glid_wh_no"},
		{"glid_acct_no"}
	};

	/*
	 * General Ledger Interface Header File.
	 */
	struct dbview	glih_list [] =
	{
		{"glih_co_no"},
		{"glih_hhih_hash"},
		{"glih_int_code"},
		{"glih_int_desc"},
		{"glih_cat_no"},
		{"glih_class_type"},
		{"glih_stat_flag"}
	};

	/*
	 * General Ledger Journal Control Record.
	 */
	struct dbview	gljc_list [] =
	{
		{"gljc_co_no"},
		{"gljc_journ_type"},
		{"gljc_run_no"},
		{"gljc_jnl_desc"},
		{"gljc_nxt_pge_no"},
		{"gljc_tot_1"},
		{"gljc_tot_2"},
		{"gljc_tot_3"},
		{"gljc_tot_4"},
		{"gljc_tot_5"},
		{"gljc_tot_6"},
		{"gljc_stat_flag"},
		{"gljc_rep_prog1"},
		{"gljc_rep_prog2"}
	};

	/*
	 * General Ledger Log file.
	 */
	struct dbview	gllg_list [] =
	{
		{"gllg_co_no"},
		{"gllg_est_no"},
		{"gllg_jnl_type"},
		{"gllg_pid"},
		{"gllg_desc"},
		{"gllg_mod_date"},
		{"gllg_stat_flag"}
	};

	/*
	 * Link between parent/child accounts.
	 */
	struct dbview	glln_list [] =
	{
		{"glln_parent_hash"},
		{"glln_child_hash"}
	};

	/*
	 * General Ledger Master File - Account Header
	 */
	struct dbview	glmr_list [] =
	{
		{"glmr_co_no"},
		{"glmr_acc_no"},
		{"glmr_curr_code"},
		{"glmr_desc"},
		{"glmr_class1"},
		{"glmr_class2"},
		{"glmr_class3"},
		{"glmr_hhca_hash"},
		{"glmr_hhmr_hash"},
		{"glmr_parent_cnt"},
		{"glmr_child_cnt"},
		{"glmr_mod_date"},
		{"glmr_system_acc"},
		{"glmr_stat_flag"}
	};

	/*
	 * General ledger narrative
	 */
	struct dbview	glna_list [] =
	{
		{"glna_gltr_hash"},
		{"glna_line_no"},
		{"glna_narrative"}
	};

	/*
	 * General Ledger Period Balance file.
	 */
	struct dbview	glpd_list [] =
	{
		{"glpd_hhmr_hash"},
		{"glpd_prd_no"},
		{"glpd_year"},
		{"glpd_budg_no"},
		{"glpd_mod_time"},
		{"glpd_user_id"},
		{"glpd_balance"},
		{"glpd_fx_balance"},
		{"glpd_hhgp_hash"}
	};

	/*
	 * General Ledger Transaction Consolidation File.
	 */
	struct dbview	gltc_list [] =
	{
		{"gltc_gltr_hash"},
		{"gltc_tran_type"},
		{"gltc_sys_ref"},
		{"gltc_batch_no"},
		{"gltc_tran_date"},
		{"gltc_post_date"},
		{"gltc_narrative"},
		{"gltc_user_ref"},
		{"gltc_amount"},
		{"gltc_stat_flag"},
		{"gltc_amt_origin"},
		{"gltc_currency"},
		{"gltc_exch_rate"}
	};
	/*
	 * General Ledger Transaction File.
	 */
	struct dbview	gltr_list [] =
	{
		{"gltr_gltr_hash"},
		{"gltr_hhmr_hash"},
		{"gltr_tran_type"},
		{"gltr_sys_ref"},
		{"gltr_batch_no"},
		{"gltr_tran_date"},
		{"gltr_post_date"},
		{"gltr_narrative"},
		{"gltr_user_ref"},
		{"gltr_amount"},
		{"gltr_stat_flag"},
		{"gltr_amt_origin"},
		{"gltr_currency"},
		{"gltr_exch_rate"}
	};

	/*
	 * General Ledger Transactions Work File.
	 */
	struct dbview	glwk_list [] =
	{
		{"glwk_acc_no"},
		{"glwk_co_no"},
		{"glwk_est_no"},
		{"glwk_acronym"},
		{"glwk_name"},
		{"glwk_chq_inv_no"},
		{"glwk_ci_amt"},
		{"glwk_o1_amt"},
		{"glwk_o2_amt"},
		{"glwk_o3_amt"},
		{"glwk_o4_amt"},
		{"glwk_hhgl_hash"},
		{"glwk_tran_type"},
		{"glwk_sys_ref"},
		{"glwk_batch_no"},
		{"glwk_tran_date"},
		{"glwk_period_no"},
		{"glwk_post_date"},
		{"glwk_narrative"},
		{"glwk_alt_desc1"},
		{"glwk_alt_desc2"},
		{"glwk_alt_desc3"},
		{"glwk_user_ref"},
		{"glwk_amount"},
		{"glwk_loc_amount"},
		{"glwk_jnl_type"},
		{"glwk_currency"},
		{"glwk_exch_rate"},
		{"glwk_stat_flag"},
		{"glwk_run_no"}
	};

	/*
	 * Currency File Record.
	 */
	struct dbview	pocr_list [] =
	{
		{"pocr_co_no"},
		{"pocr_code"},
		{"pocr_description"},
		{"pocr_prime_unit"},
		{"pocr_sub_unit"},
		{"pocr_ex1_factor"},
		{"pocr_ex2_factor"},
		{"pocr_ex3_factor"},
		{"pocr_ex4_factor"},
		{"pocr_ex5_factor"},
		{"pocr_ex6_factor"},
		{"pocr_ex7_factor"},
		{"pocr_ldate_up"},
		{"pocr_gl_ctrl_acct"},
		{"pocr_gl_exch_var"},
		{"pocr_stat_flag"},
		{"pocr_operator"}
	};

	/*
	 * General ledger user security record.
	 */
	struct dbview	glus_list [] =
	{
		{"glus_co_no"},
		{"glus_user_name"},
		{"glus_acc_hdr_code"},
		{"glus_super_user"}
	};

const		char	*glbd	=	"glbd",
					*glbh	=	"glbh",
					*glbl	=	"glbl",
					*glca	=	"glca",
					*glct	=	"glct",
					*glid	=	"glid",
					*glih	=	"glih",
					*gljc	=	"gljc",
					*gllg	=	"gllg",
					*glln	=	"glln",
					*glmr	=	"glmr",
					*glna	=	"glna",
					*glpd	=	"glpd",
					*gltc	=	"gltc",
					*gltr	=	"gltr",
					*glus	=	"glus",
					*glwk	=	"glwk",
					*pocr	=	"pocr";

	char	GlMask		[17],
			GlDesc		[17],
			GlfDesc		[17],
			GlFormat	[17],
			GL_Account  [17];
#else
	extern	struct dbview	glbd_list [],
							glbh_list [],
							glbl_list [],
							glca_list [],
							glct_list [],
							glih_list [],
							glid_list [],
							gljc_list [],
							gllg_list [],
							glln_list [],
							glmr_list [],
							glna_list [],
							glpd_list [],
							gltc_list [],
							gltr_list [],
							glus_list [],
							glwk_list [],
							pocr_list [];

	extern	const	char	*glbd,
							*glbh,
							*glbl,
							*glca,
							*glct,
							*glih,
							*glid,
							*gljc,
							*gllg,
							*glln,
							*glmr,
							*glna,
							*glpd,
							*gltc,
							*gltr,
							*glus,
							*glwk,
							*pocr;

	extern	char	GlMask		[],
					GlDesc		[],
					GlfDesc		[],
					GlFormat	[],
					GL_Account	[];
#endif

#define	GLBD_NO_FIELDS	6
#define	GLBH_NO_FIELDS	11
#define	GLBL_NO_FIELDS	28
#define	GLCA_NO_FIELDS	7
#define	GLCT_NO_FIELDS	9
#define	GLIH_NO_FIELDS	7
#define	GLID_NO_FIELDS	4
#define	GLJC_NO_FIELDS	14
#define	GLLG_NO_FIELDS	7
#define	GLLN_NO_FIELDS	2
#define	GLMR_NO_FIELDS	14
#define	GLNA_NO_FIELDS	3
#define	GLPD_NO_FIELDS	9
#define	GLTC_NO_FIELDS	13
#define	GLTR_NO_FIELDS	14
#define	GLUS_NO_FIELDS	4
#define	GLWK_NO_FIELDS	30
#define	POCR_NO_FIELDS	17
	
GLOB_GL	GLBD_STRUCT	glbdRec;
GLOB_GL	GLBH_STRUCT	glbhRec;
GLOB_GL	GLBL_STRUCT	glblRec;
GLOB_GL	GLCA_STRUCT	glcaRec;
GLOB_GL	GLCT_STRUCT	glctRec;
GLOB_GL	GLIH_STRUCT	glihRec;
GLOB_GL	GLID_STRUCT	glidRec;
GLOB_GL	GLJC_STRUCT gljcRec;
GLOB_GL	GLLG_STRUCT gllgRec;
GLOB_GL	GLLN_STRUCT gllnRec;
GLOB_GL	GLMR_STRUCT glmrRec;
GLOB_GL	GLNA_STRUCT glnaRec;
GLOB_GL	GLPD_STRUCT glpdRec;
GLOB_GL	GLTC_STRUCT gltcRec;
GLOB_GL	GLTC_STRUCT gltcRec;
GLOB_GL	GLTR_STRUCT gltrRec;
GLOB_GL	GLUS_STRUCT glusRec;
GLOB_GL	GLWK_STRUCT glwkRec;
GLOB_GL	POCR_STRUCT pocrRec;

GLOB_GL	char * 	GL_AddBatch			(void);
GLOB_GL	char *	GL_AudGet			(FILE *);
GLOB_GL	char * 	GL_AudName			(char *, int);
GLOB_GL	char *	GL_AudName			(char *, int);
GLOB_GL	char * 	GL_GetAccount		(int);
GLOB_GL	char * 	GL_GetAccountNo		(int, char *);
GLOB_GL	char * 	GL_GetBit			(int);
GLOB_GL	char * 	GL_GetDfltEaccCode	(void);
GLOB_GL	char * 	GL_GetDfltEfaccCode	(void);
GLOB_GL	char * 	GL_GetDfltSaccCode	(void);
GLOB_GL	char * 	GL_GetDfltSfaccCode	(void);
GLOB_GL	char * 	GL_GetfUserCode		(void);
GLOB_GL	char *	GL_GetSecure		(char *);
GLOB_GL	char * 	GL_GetUserCode		(void);
GLOB_GL	char *	GL_SetAccWidth		(char *, int);
GLOB_GL	char *	GL_SetBitWidth		(int, int *, int *);
GLOB_GL	double	CurrencyFgnAmt		(double);
GLOB_GL	double	CurrencyLocAmt		(double);
GLOB_GL	double 	GL_FgnTotGlpd 		(long, int, int, int, int);
GLOB_GL	double 	GL_LocTotGlpd 		(long, int, int, int, int);
GLOB_GL	FILE *	GL_AudOpen			(char *, char *, int);
GLOB_GL	int	 	FindGlbh 			(char *, char *, char *, char *, char *);
GLOB_GL	int	 	FindGlct 			(void);
GLOB_GL	int	 	FindGljc 			(char *, char *, char *);
GLOB_GL	int	 	FindPocr 			(char *, char *, char *);
GLOB_GL	int		GL_AudTrailer		(FILE *);
GLOB_GL	int 	GL_CheckAccNo		(int, char *, GLMR_STRUCT *);
GLOB_GL	int 	GL_CheckLevel 		(int);
GLOB_GL	int 	GL_CheckLevel		(int);
GLOB_GL	int 	GL_FormAccNo		(char *, char *, int);
GLOB_GL	int 	GL_PostAccount		(char *, Date, char *, double, double);
GLOB_GL	int 	GL_PostControl		(int, int, char *, double, double);
GLOB_GL	int 	GL_PostLinks		(long, int, int, char *, double, double);
GLOB_GL	int 	GL_PostStamp		(void);
GLOB_GL	int 	GL_ReadGlmr			(char *, char *,GLMR_STRUCT *, int, char *);
GLOB_GL	int 	GL_ValidfUserCode	(char *);
GLOB_GL	int 	GL_ValidUserCode	(char *);
GLOB_GL	int 	_PostAccount		(char *, int, int, char *, double, double);
GLOB_GL	int 	ReadGlca			(char *, int, char *, char *,GLCA_STRUCT *);
GLOB_GL	int 	ReadGlmr			(char *, GLMR_STRUCT *, char *, int);
GLOB_GL	int 	SearchGlmr_C 		(char *, char *, char *, char *);
GLOB_GL	int 	SearchGlmr_CF 		(char *, char *, char *, char *);
GLOB_GL	int 	SearchGlmr	 		(char *, char *, char *);
GLOB_GL	int 	SearchGlmr_F 		(char *, char *, char *);
GLOB_GL	long 	GL_FormAccBit		(char *, int);
GLOB_GL	long 	GL_GetHhca			(int);
GLOB_GL	long	GL_GetHhca			(int);
GLOB_GL	long	GL_NextBatchNo		(char *, char *);
GLOB_GL	void	GL_AudClose			(FILE *);
GLOB_GL	void	GL_AudHeader		(FILE *, int, char *, char *);
GLOB_GL	void	GL_AudSave			(FILE *, char *, int, int, double, double, char *, double, char *);
GLOB_GL	void	GL_AudSetup			(char *, int);
GLOB_GL	void 	GL_CloseBatch 		(int);
GLOB_GL	void 	GL_Close 			(void);
GLOB_GL	void 	GL_GetDesc			(int, char *, int);
GLOB_GL	void 	GL_GLI				(char *,char *,char *,char *,char *,char *);
GLOB_GL	void 	GL_OpenBatch 		(char *, char *);
GLOB_GL	void 	GL_PostBudget 		(int);
GLOB_GL	void 	GL_PostSetup 		(char *, int);
GLOB_GL	void 	GL_PostTerminate	(void);
GLOB_GL	void	GL_SetMask			(char *);
GLOB_GL	void 	GL_StripForm 		(char *, char *);
GLOB_GL	void 	OpenGlbd 			(void);
GLOB_GL	void 	OpenGlbh 			(void);
GLOB_GL	void 	OpenGlbl 			(void);
GLOB_GL	void 	OpenGlca 			(void);
GLOB_GL	void 	OpenGlct 			(void);
GLOB_GL	void 	OpenGlid 			(void);
GLOB_GL	void 	OpenGlih 			(void);
GLOB_GL	void 	OpenGljc 			(void);
GLOB_GL	void 	OpenGllg 			(void);
GLOB_GL	void 	OpenGlln 			(void);
GLOB_GL	void 	OpenGlmr 			(void);
GLOB_GL	void 	OpenGlna 			(void);
GLOB_GL	void 	OpenGlpd 			(void);
GLOB_GL	void 	OpenGltc 			(void);
GLOB_GL	void 	OpenGltr 			(void);
GLOB_GL	void 	OpenGlus 			(void);
GLOB_GL	void 	OpenGlwk 			(void);
GLOB_GL	void 	OpenPocr 			(void);
GLOB_GL	void 	SearchPocr	 		(char *, char *);
#endif	/* _GlUtils_h	*/
