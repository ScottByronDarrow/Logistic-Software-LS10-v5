#ifdef	CCMAIN
#	define	PS_GLOB
#else
#	define	PS_GLOB	extern
#endif

#define	OVERLAP	0x0001
#define	LATE	0x0002
#define	ORDER	0x0004

struct	RGRS_REC
{
	long	rs_hhrs_hash;
	char	rs_co_no[3];
	char	rs_br_no[3];
	char	rs_code[9];
	char	rs_desc[41];
	char	rs_type[2];
	char	rs_type_name[11];
	char	rs_cost_type[4];
	double	rs_rate;	/* money */
	int	rs_qty_avl;
	double	rs_ovhd_var;	/* money */
	double	rs_ovhd_fix;	/* money */
	char	rs_cal_sel[2];
};

struct	STP_LIST				/* INDIVIDUAL STEP LIST	*/
{
	struct	STP_LIST	*nxt_res;	/* Vertical		*/
	struct	STP_LIST	*prv_res;	/* Vertical		*/
	struct	STP_LIST	*nxt_job;	/* Horizontal		*/
	struct	STP_LIST	*prv_job;	/* Horizontal		*/
	struct	JOB_LIST	*job_dtl;	/* Pointer to job info.	*/
	struct	RSL_LIST	*rsl_ptr;	/* Pointer to resource.	*/
	int			seq_no;
	int			line_no;
	long			st_date_time;	/* NB: Include date	*/
	long			duration;
	long			setup;
	long			run;
	long			clean;
	char			can_split[2];	/* 'N' if uNsplittable	*/
						/* 'S' if Same resource	*/
						/* 'Y' if verY splittable */
	char			status;
};

struct	RSL_LIST				/* RGRS LINE INFO.	*/
						/* >= 1 per rgrs record	*/
{
	struct	RSL_LIST	*nxt_rsl;
	struct	RSL_LIST	*prv_rsl;
	struct	RGRS_REC	*rgrs_ptr;	/* Ptr to rgrs info	     */
	struct	STP_LIST	*fst_job;	/* Ptr to head of jobs	     */
	struct	STP_LIST	*lst_job;	/* Ptr to last job on rsrc   */
	struct	STP_LIST	*fst_stp;	/* Ptr to head of steps      */
	struct	STP_LIST	*lst_stp;	/* Ptr to last step on rsrc  */
	long			lst_dt_tm;	/* End date/time of last job */
};

struct	JOB_LIST				/* Holds job hdr info.	*/
{
	struct	JOB_LIST	*nxt_job;
	struct	JOB_LIST	*prv_job;
	struct	STP_LIST	*fst_stp;
	struct	STP_LIST	*lst_stp;
	long			hhwo_hash;
	int			rtg_seq; 
	long			hhbr_hash;
	char			order_no[8];
	char			item_no[17];
	char			description[41];
	float			prod_qty;
	int			priority;
	int			bom_alt;
	int			rtg_alt;
	long			create_date;
	long			reqd_date;
	int			is_hilite;
};

#define	JOB_NULL	((struct JOB_LIST *) 0)
#define	RSL_NULL	((struct RSL_LIST *) 0)
#define	STP_NULL	((struct STP_LIST *) 0)

PS_GLOB	struct	RSL_LIST	*rsl_head;	/* Resource head ptr	*/
PS_GLOB	struct	RSL_LIST	*rsl_tail;	/* Resource tail ptr	*/
PS_GLOB	struct	JOB_LIST	*job_head;	/* Job head pointer	*/
PS_GLOB	struct	JOB_LIST	*job_tail;	/* Job tail pointer	*/
PS_GLOB	struct	JOB_LIST	*job_free;	/* Job free pointer	*/
PS_GLOB	struct	STP_LIST	*stp_free;	/* Step free pointer	*/
