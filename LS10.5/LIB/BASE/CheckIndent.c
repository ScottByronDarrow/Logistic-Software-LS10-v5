/*
 *  Copyright (C) 1999 - 2000 LogisticSoftware
 *
 * $Id: CheckIndent.c,v 5.0 2001/06/19 06:59:10 cha Exp $
 *
 * Create all necessary records for an INDENT item
 * $Log: CheckIndent.c,v $
 * Revision 5.0  2001/06/19 06:59:10  cha
 * LS10-5.0 New Release as of 19 JUNE 2001
 *
 * Revision 4.0  2001/03/09 00:52:33  scott
 * LS10-4.0 New Release as at 10th March 2001
 *
 * Revision 3.0  2000/10/12 13:34:16  gerry
 * Revision No. 3 Start
 * <after Rel-10102000>
 *
 * Revision 2.0  2000/07/15 07:17:11  gerry
 * Forced revision no. to 2.0 - Rel-15072000
 *
 * Revision 1.3  2000/05/22 05:35:31  scott
 * S/C USL-16005 / LSDI-2531
 * Updated as some fields created for indent items no updated.
 * Problem we that indent maintenance did not maintain all fields held on
 * indent file (inin).
 * Updated indent maintenance (sk_inin_inp.c) and changed library (CheckIndent.c)
 * to update surcharge flag as should be set to "0" and not blank.
 *
 * Revision 1.2  2000/02/17 10:10:31  scott
 * Updated to change class to srsk_class as class is reserved in C++;
 *
 * Revision 1.1  2000/02/17 06:48:27  scott
 * S/C LSANZ-16005 / LSDI-2531
 * Updated to include missing fields in files related to indents.
 * Implemented library based routine from version 10.
 *
 *	
 */

#define	TRUE	1
#define	FALSE	0

#include	<std_decs.h>
#include	<CheckIndent.h>

#define	ININ_ITEM_NO_LEN	16				/* col length of inin_item_no */

static const char *	INDENT = "INDENT";

static const char
	*esmr	= "_esmr_checkindent",
	*incc	= "_incc_checkindent",
	*inin	= "_inin_checkindent",
	*inmr	= "_inmr_checkindent",
	*inei	= "_inei_checkindent",
	*inex	= "_inex_checkindent",
	*inuv	= "_inuv_checkindent",
	*srsk	= "_srsk_checkindent";

#define	ESMR_NO_FIELDS	2

	static struct dbview esmr_list [ESMR_NO_FIELDS] =
	{
		{"esmr_co_no"},
		{"esmr_est_no"}
	};

	struct esmrRecord
	{
		char	co_no [3];
		char	est_no [3];
	};

	/*==================================
	| Inventory EXtra description file |
	==================================*/
#define	INEX_NO_FIELDS	3

	static struct dbview inex_list [INEX_NO_FIELDS] =
	{
		{"inex_hhbr_hash"},
		{"inex_line_no"},
		{"inex_desc"}
	};

	struct inexRecord
	{
		long	hhbr_hash;
		int		line_no;
		char	desc [41];
	};

	/*================================+
	 | Inventory Indent Control File. |
	 +================================*/
#define	ININ_NO_FIELDS	31

	static struct dbview inin_list [ININ_NO_FIELDS] =
	{
		{"inin_co_no"},
		{"inin_item_no"},
		{"inin_next_indent"},
		{"inin_class"},
		{"inin_category"},
		{"inin_serial_item"},
		{"inin_abc_code"},
		{"inin_abc_update"},
		{"inin_ff_option"},
		{"inin_ff_method"},
		{"inin_allow_repl"},
		{"inin_reorder"},
		{"inin_active_status"},
		{"inin_costing_flag"},
		{"inin_gst_pc"},
		{"inin_tax_pc"},
		{"inin_tax_amount"},
		{"inin_lot_ctrl"},
		{"inin_sale_unit"},
		{"inin_pack_size"},
		{"inin_dec_pt"},
		{"inin_std_uom"},
		{"inin_alt_uom"},
		{"inin_source"},
		{"inin_uom_cfactor"},
		{"inin_outer_size"},
		{"inin_sellgrp"},
		{"inin_buygrp"},
		{"inin_qc_reqd"},
		{"inin_qc_time"},
		{"inin_stat_flag"}
	};

	struct ininRecord
	{
		char	co_no [3];
		char	item_no [17];
		long	next_indent;
		char	inin_class [2];
		char	category [12];
		char	serial_item [2];
		char	abc_code [2];
		char	abc_update [2];
		char	ff_option [2];
		char	ff_method [2];
		char	allow_repl [2];
		char	reorder [2];
		char	active_status [2];
		char	costing_flag [2];
		float	gst_pc;
		float	tax_pc;
		double	tax_amount;
		char	lot_ctrl [2];
		char	sale_unit [5];
		char	pack_size [6];
		int		dec_pt;
		long	std_uom;
		long	alt_uom;
		char	source [3];
		float	uom_cfactor;
		float	outer_size;
		char	sellgrp [7];
		char	buygrp [7];
		char	qc_reqd [2];
		float	qc_time;
		char	stat_flag [2];
	};

#define INCC_NO_FIELDS 9

	static struct dbview incc_list [INCC_NO_FIELDS] =
	{
		{"incc_hhcc_hash"},
		{"incc_hhbr_hash"},
		{"incc_sort"},
		{"incc_stocking_unit"},
		{"incc_ff_option"},
		{"incc_ff_method"},
		{"incc_abc_code"},
		{"incc_abc_update"},
		{"incc_stat_flag"}
	};

	struct inccRecord
	{
		long	hhcc_hash;
		long	hhbr_hash;
		char	sort [29];
		char	stocking_unit [5];
		char	ff_option [2];
		char	ff_method [2];
		char	abc_code [2];
		char	abc_update [2];
		char	stat_flag [2];
	};

#define INMR_NO_FIELDS 29

	static struct dbview inmr_list [INMR_NO_FIELDS] =
	{
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_alpha_code"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_category"},
		{"inmr_abc_code"},
		{"inmr_abc_update"},
		{"inmr_serial_item"},
		{"inmr_lot_ctrl"},
		{"inmr_costing_flag"},
		{"inmr_sale_unit"},
		{"inmr_pack_size"},
		{"inmr_source"},
		{"inmr_dec_pt"},
		{"inmr_bo_flag"},
		{"inmr_bo_release"},
		{"inmr_sellgrp"},
		{"inmr_buygrp"},
		{"inmr_gst_pc"},
		{"inmr_std_uom"},
		{"inmr_alt_uom"},
		{"inmr_uom_cfactor"},
		{"inmr_outer_size"},
		{"inmr_active_status"},
		{"inmr_schg_flag"},
		{"inmr_qc_reqd"},
		{"inmr_stat_flag"},
	};

	/* Record structure for inmr table */
	struct inmrRecord
	{
		char	co_no [3];
		char	item_no [17];
		long	hhbr_hash;
		char	alpha_code [17];
		char	inmr_class [2];
		char	description [41];
		char	category [12];
		char	abc_code [2];
		char	abc_update [2];
		char	serial_item [2];
		char	lot_ctrl [2];
		char	costing_flag [2];
		char	sale_unit [5];
		char	pack_size [6];
		char	source [3];
		int		dec_pt;
		char	bo_flag [2];
		char	bo_release [2];
		char	sellgrp [7];
		char	buygrp [7];
		float	gst_pc;
		long	std_uom;
		long	alt_uom;
		float	uom_cfactor;
		float	outer_size;
		char	active_status [2];
		char	schg_flag [2];
		char	qc_reqd [2];
		char	stat_flag [2];
	};

	/*========================+
	 | SeaRch file for Stock. |
	 +========================*/
#define	SRSK_NO_FIELDS	10

	static struct dbview srsk_list [SRSK_NO_FIELDS] =
	{
		{"srsk_co_no"},
		{"srsk_hhbr_hash"},
		{"srsk_item_no"},
		{"srsk_class"},
		{"srsk_active_status"},
		{"srsk_alpha_code"},
		{"srsk_alternate"},
		{"srsk_barcode"},
		{"srsk_maker_no"},
		{"srsk_description"}
	};

	struct srskRecord
	{
		char	co_no [3];
		long	hhbr_hash;
		char	item_no [17];
		char	srsk_class [2];
		char	active_status [2];
		char	alpha_code [17];
		char	alternate [17];
		char	barcode [17];
		char	maker_no [17];
		char	description [41];
	};

	/*======+
	 | inuv |
	 +======*/
#define	INUV_NO_FIELDS	2

	static struct dbview inuv_list [INUV_NO_FIELDS] =
	{
		{"inuv_hhbr_hash"},
		{"inuv_hhum_hash"},
	};

	struct inuvRecord
	{
		long	hhbr_hash;
		long	hhum_hash;
	};

#define INEI_NO_FIELDS 5

	static struct dbview inei_list [INEI_NO_FIELDS] =
	{
		{"inei_hhbr_hash"},
		{"inei_est_no"},
		{"inei_abc_code"},
		{"inei_abc_update"},
		{"inei_stat_flag"}
	};

	/* Record structure for inei table */
	struct ineiRecord
	{
		long	hhbr_hash;
		char	est_no [3];
		char	abc_code [2];
		char	abc_update [2];
		char	stat_flag [2];
	};

/*
 */
extern char *	clip (char *),
			*	txt_gval (int, int );

static void		InexMaint (struct inmrRecord *);
static void		table_setup (),
				table_teardown ();

/*
 */
int
check_indent (
	const 	char *co_no,
	const 	char *br_no,
	long 	hhcc_hash,
	char 	*item_no)			/* will be overwritten on return */
{
	int		err;
	size_t	len_itemno = strlen (clip (item_no));
	int		len_suffix = ININ_ITEM_NO_LEN - len_itemno;
	char	indent_no [64];			/* should last until i'm long dead */

	struct esmrRecord	esmrRec;
	struct inccRecord	inccRec;
	struct ineiRecord	ineiRec;
	struct ininRecord	ininRec;
	struct inmrRecord	inmrRec;
	struct srskRecord	srskRec;
	struct inuvRecord	inuvRec;


	/*
	 * check if item_no contains "INDENT"
	 */
	if (!strstr (item_no, INDENT))
		return IndentErr_BadName;

	table_setup ();

	/*
	 * check if item_no in indent file
	 */
	memset (&ininRec, 0, sizeof (struct ininRecord));
	strcpy (ininRec.co_no, co_no);
	strcpy (ininRec.item_no, item_no);
	if ((err = find_rec (inin, &ininRec, COMPARISON, "u")))
	{
		table_teardown ();
		return IndentErr_NoSuchIndent;
	}

	/*
	 * create item number for indent
	 */
	do
	{
		sprintf (indent_no, "%0*ld", len_suffix, ++ininRec.next_indent);
		if (strlen (indent_no) + len_itemno > ININ_ITEM_NO_LEN)
		{
			table_teardown ();
			return IndentErr_Overflow;
		}

		/*
		 * check if item already exists
		 */
		memset (&inmrRec, 0, sizeof (struct inmrRecord));
		strcpy (inmrRec.co_no, co_no);
		sprintf (inmrRec.item_no, "%s%s", item_no, indent_no);

	}	while (find_rec (inmr, &inmrRec, EQUAL, "r") == 0);

	/*
	 *	Initialize the record to be added with
	 *		- supplied parameters
	 *		- defaults off ininRec
	 *		- other pieces of magic
	 */
	memset (&inmrRec, 0, sizeof (struct inmrRecord));

	strcpy (inmrRec.co_no, co_no);
	sprintf (inmrRec.item_no, "%s%s", item_no, indent_no);
	strcpy (inmrRec.alpha_code,	item_no);

	strcpy (inmrRec.inmr_class,		ininRec.inin_class);
	strcpy (inmrRec.category,		ininRec.category);
	strcpy (inmrRec.serial_item,	ininRec.serial_item);
	strcpy (inmrRec.abc_code,		ininRec.abc_code);
	strcpy (inmrRec.abc_update,		ininRec.abc_update);
	strcpy (inmrRec.costing_flag,	ininRec.costing_flag);
	strcpy (inmrRec.lot_ctrl,		ininRec.lot_ctrl);
	strcpy (inmrRec.sale_unit,		ininRec.sale_unit);
	strcpy (inmrRec.pack_size,		ininRec.pack_size);
	strcpy (inmrRec.source,			ininRec.source);
	strcpy (inmrRec.sellgrp,		ininRec.sellgrp);
	strcpy (inmrRec.buygrp,			ininRec.buygrp);
	strcpy (inmrRec.qc_reqd,		ininRec.qc_reqd);
	strcpy (inmrRec.active_status,	ininRec.active_status);
	strcpy (inmrRec.schg_flag,		"0");
	inmrRec.dec_pt 		= ininRec.dec_pt;
	inmrRec.uom_cfactor = ininRec.uom_cfactor;
	inmrRec.outer_size  = ininRec.outer_size;
	inmrRec.std_uom 	= ininRec.std_uom;
	inmrRec.alt_uom 	= ininRec.alt_uom;
	inmrRec.gst_pc 		= ininRec.gst_pc;

	strcpy (inmrRec.stat_flag,	"0");
	strcpy (inmrRec.bo_flag,	"Y");
	strcpy (inmrRec.bo_release,	"A");

	/*
	 *	Update the tables
	 */
	if ((err = abc_add (inmr, &inmrRec)))
		file_err (err, "inmr", "abc_add in check_indent");
	if ((err = abc_update (inin, &ininRec)))
		file_err (err, "inin", "abc_update in check_indent");

	/*
	 *	refind item for hhbr_hash
	 */
	if ((err = find_rec (inmr, &inmrRec, COMPARISON, "u")))
		file_err (err, "inmr", "find_rec in check_indent");

	/*
	 */
	InexMaint (&inmrRec);
	if (clip (inmrRec.description) [0])
	{
		/*
		 *	Update only if description has been added
		 */
		if ((err= abc_update (inmr, &inmrRec)))
			file_err (err, "inmr", "abc_update in check_indent");
	} else
		abc_unlock (inmr);

	/*
	 *	Add inei for each branch
	 */
	memset (&esmrRec, 0, sizeof (struct esmrRecord));
	strcpy (esmrRec.co_no, co_no);
	for (err = find_rec (esmr, &esmrRec, GTEQ, "r");
		!err && !strcmp (co_no, esmrRec.co_no);
		err = find_rec (esmr, &esmrRec, NEXT, "r"))
	{
		/*
		 *	Check for existence before proceeding
		 */
		memset (&ineiRec, 0, sizeof (struct ineiRecord));
		ineiRec.hhbr_hash = inmrRec.hhbr_hash;
		strcpy (ineiRec.est_no, esmrRec.est_no);
		if (!find_rec (inei, &ineiRec, EQUAL, "r"))
			continue;	/* it's already there, don't add another */

		/*
		 *	Attempt an add
		 */
		memset (&ineiRec, 0, sizeof (struct ineiRecord));
		ineiRec.hhbr_hash = inmrRec.hhbr_hash;
		strcpy (ineiRec.est_no, esmrRec.est_no);
		strcpy (ineiRec.abc_code, ininRec.abc_code);
		strcpy (ineiRec.abc_update, ininRec.abc_update);

   		if ((err = abc_add (inei, &ineiRec)))
	 		file_err (err, "inei", "abc_add in check_indent");
	}

	/*
	 *	Add incc for current branch
	 */
	inccRec.hhcc_hash = hhcc_hash;
	inccRec.hhbr_hash = inmrRec.hhbr_hash;
	if (find_rec (incc, &inccRec, EQUAL, "r"))
	{
		memset (&inccRec, 0, sizeof (struct inccRecord));
		inccRec.hhcc_hash = hhcc_hash;
		inccRec.hhbr_hash = inmrRec.hhbr_hash;
		sprintf (inccRec.sort,
					"%s%s%s",
					inmrRec.inmr_class, inmrRec.category, inmrRec.item_no);
		strcpy (inccRec.stocking_unit, 	ininRec.sale_unit);
		strcpy (inccRec.ff_option, 	   	ininRec.ff_option);
		strcpy (inccRec.ff_method, 		ininRec.ff_method);
		strcpy (inccRec.abc_code, 		ininRec.abc_code);
		strcpy (inccRec.abc_update, 	ininRec.abc_update);
		strcpy (inccRec.stat_flag, "0");

		if ((err = abc_add (incc, &inccRec)))
			file_err (err, "incc", "abc_add in check_indent");
	}
	/*
	 *	Add srsk 
	 */
	srskRec.hhbr_hash = inmrRec.hhbr_hash;
	if (find_rec (srsk, &srskRec, EQUAL, "r"))
	{
		char	workDesc [41];
		memset (&srskRec, 0, sizeof (struct srskRecord));
		srskRec.hhbr_hash = inmrRec.hhbr_hash;
		strcpy (srskRec.co_no, 		    inmrRec.co_no);
		strcpy (srskRec.item_no, 		inmrRec.item_no);
		strcpy (srskRec.srsk_class, 	inmrRec.inmr_class);
		strcpy (srskRec.active_status, 	inmrRec.active_status);
		strcpy (srskRec.alpha_code, 	inmrRec.alpha_code);
		strcpy (srskRec.alternate, 		" ");
		strcpy (srskRec.barcode, 		" ");
		strcpy (srskRec.maker_no, 		" ");
		strcpy (workDesc, inmrRec.description);
		strcpy (srskRec.description, 	upshift (workDesc));
		if ((err = abc_add (srsk, &srskRec)))
			file_err (err, "srsk", "abc_add in check_indent");
	}
	/*
	 *	Add inuv for current branch/warehouse
	 */
	inuvRec.hhbr_hash = inmrRec.hhbr_hash;
	inuvRec.hhum_hash = inmrRec.std_uom;
	if (find_rec (inuv, &inuvRec, EQUAL, "r"))
	{
		memset (&inuvRec, 0, sizeof (struct inuvRecord));

		inuvRec.hhbr_hash = inmrRec.hhbr_hash;
		inuvRec.hhum_hash = inmrRec.std_uom;
		if ((err = abc_add (inuv, &inuvRec)))
			file_err (err, "inuv", "abc_add in check_indent");
	}
	/*
	 *	Clean up
	 */
	table_teardown ();

	strcpy (item_no, inmrRec.item_no);
	return IndentErr_Ok;
}

static void
table_setup ()
{
	/*
	 *	Open all the necessary tables et al
	 */
	static int	done_this_before = FALSE;

	if (!done_this_before)
	{
		done_this_before = TRUE;

		abc_alias (esmr, "esmr");
		abc_alias (incc, "incc");
		abc_alias (inei, "inei");
		abc_alias (inex, "inex");
		abc_alias (inin, "inin");
		abc_alias (inmr, "inmr");
		abc_alias (srsk, "srsk");
		abc_alias (inuv, "inuv");
	}

	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_co_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (inin, inin_list, ININ_NO_FIELDS, "inin_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (srsk, srsk_list, SRSK_NO_FIELDS, "srsk_hhbr_hash");
	open_rec (inuv, inuv_list, INUV_NO_FIELDS, "inuv_id_no");
}

static void
table_teardown ()
{
	abc_fclose (esmr);
	abc_fclose (incc);
	abc_fclose (inei);
	abc_fclose (inex);
	abc_fclose (inin);
	abc_fclose (inmr);
	abc_fclose (srsk);
	abc_fclose (inuv);
}

static void
InexMaint (
 struct inmrRecord * inmrRec)
{
	/*
	 *	Add description + inex text for new INDENT item
	 */
	int		i;
	int		tx_window;
	int		tx_lines;
	char	item_desc [61];

	sprintf (item_desc, "DESCRIPTION FOR ITEM %s", inmrRec -> item_no);

	tx_window = txt_open (2, 1, 2, 40, 100, item_desc);
	if ((tx_lines = txt_edit (tx_window)))
	{
		strcpy (inmrRec -> description, txt_gval (tx_window, 1));

		for (i = 2; i <= tx_lines; i++)
		{
			int	err;
			struct inexRecord	inexRec;

			memset (&inexRec, 0, sizeof (struct inexRecord));
			inexRec.hhbr_hash = inmrRec -> hhbr_hash;
			inexRec.line_no = i - 2;
			if (find_rec (inex, &inexRec, EQUAL, "r"))
			{
				strcpy (inexRec.desc, txt_gval (tx_window, i));

				if ((err = abc_add (inex, &inexRec)))
					file_err (err, "inex", "InexMaint in check_indent");
			} else
			{
				/*
				 *	This is unlikely, but we put the code in anyway
				 */
				strcpy (inexRec.desc, txt_gval (tx_window, i));
				
				if ((err = abc_update (inex, &inexRec)))
					file_err (err, "inex", "InexMaint in check_indent");
			}
		}
	}

	txt_close (tx_window, FALSE);

	/*
	 *	Kludge to clear area where text-box was drawn,
	 *	it's icky - but that's the hangover from bad code
	 */
	for (i = 0; i < 4; i++)
		print_at (i + 2, 0, "%*s", 43, "");
}
