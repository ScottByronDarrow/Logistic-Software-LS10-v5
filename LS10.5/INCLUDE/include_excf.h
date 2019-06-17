	/*================================+
	 | External Category File Record. |
	 +================================*/
#define	EXCF_NO_FIELDS 17	

	struct dbview	excf_list [EXCF_NO_FIELDS] =
	{
		{"excf_co_no"},
		{"excf_cat_no"},
		{"excf_hhcf_hash"},
		{"excf_ex_rate"},
		{"excf_cat_desc"},
		{"excf_max_disc"},
		{"excf_min_marg"},
		{"excf_ol_min_marg"},
		{"excf_ol_max_marg"},
		{"excf_gp_mkup"},
		{"excf_item_alloc"},
		{"excf_no_trans"},
		{"excf_no_days"},
		{"excf_review_prd"},
		{"excf_cont_drugs"},
		{"excf_ib_marg"},
		{"excf_stat_flag"},
	};

	struct tag_excfRecord
	{
		char	co_no [3];
		char	cat_no [12];
		long	hhcf_hash;
		float	ex_rate;
		char	cat_desc [41];
		float	max_disc;
		float	min_marg;
		float	ol_min_marg;
		float	ol_max_marg;
		float	gp_mkup;
		char	item_alloc [2];
		int		no_trans;
		int		no_days;
		float	review_prd;
		char	cont_drugs [2];
		float	ib_marg;
		char	stat_flag [2];
	}	excf_rec;

