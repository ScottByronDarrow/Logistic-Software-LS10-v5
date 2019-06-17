static	int	MoveAdd ( char *, char *, char *, long, long, long, long, int, char *, char *, char *, char *, char *, float, double, double );

	/*================+
	 | Movement file. |
	 +================*/
#define	MOVE_NO_FIELDS	20

	struct dbview	move_list [MOVE_NO_FIELDS] =
	{
		{"move_move_hash"},
		{"move_co_no"},
		{"move_br_no"},
		{"move_wh_no"},
		{"move_hhbr_hash"},
		{"move_hhcc_hash"},
		{"move_hhum_hash"},
		{"move_date_tran"},
		{"move_type_tran"},
		{"move_batch_no"},
		{"move_class"},
		{"move_category"},
		{"move_ref1"},
		{"move_ref2"},
		{"move_qty"},
		{"move_cost_price"},
		{"move_sale_price"},
		{"move_op_id"},
		{"move_time_create"},
		{"move_date_create"}
	};

	struct tag_moveRecord
	{
		long	move_hash;
		char	co_no [3];
		char	br_no [3];
		char	wh_no [3];
		long	hhbr_hash;
		long	hhcc_hash;
		long	hhum_hash;
		Date	date_tran;
		int		type_tran;
		char	batch_no [8];
		char	move_class [2];
		char	category [12];
		char	ref1 [16];
		char	ref2 [16];
		float	qty;
		Money	cost_price;
		Money	sale_price;
		char	op_id [15];
		char	time_create [6];
		Date	date_create;
	}	move_rec;
