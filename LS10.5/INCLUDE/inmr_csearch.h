/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( inmr_csearch.h   )                                |
|  Program Desc  : ( Inventory Master File Search Include File    )   |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written :   /  /            |
|---------------------------------------------------------------------|
|  Date Modified : (06/01/1994)    | Modified by  : Campbell Mander.  |
|                                  | Modified by  : Alvin Misalucha.  |
|                                                                     |
|  (07/11/1993)  : HGP 9501. Remove ability to have price search.     |
|                : The overhead involved using the new pricing        |
|                : routines is prohibitive.  SBD has OK'd the removal |
|                : of this feature.                                   |
|                :                                                    |
|  (17/08/1999)  : Converted to ANSI format.                          |
|                                                                     |
=====================================================================*/

/*===================================
| Function prototypes               |
===================================*/
#ifdef	INMR_PRICE_SEARCH
void	stck_search (char *	key_val, int price_type, char *	item_type);
#else
void	stck_search (char *	key_val, char * item_type);
#endif


/*===================================
| Search for inventory master file. |
===================================*/
#ifdef	INMR_PRICE_SEARCH
void
stck_search (
 char *	key_val,
 int	price_type,
 char *	item_type)
{
	int	p_type = (price_type < 0 || price_type > 4) ? 1 : price_type;
#else
	void
stck_search (
 char *	key_val,
 char * item_type)
{
#endif
	int		valid = 1;
	int		break_out;
	char	type_flags[6];
	char	type_flag[2];
	char	*stype = chk_env("SK_SER");
	char	*sptr = (*key_val == '*') ? (char *)0 : key_val;

	sprintf(type_flags, "%-5.5s", (stype == (char *)0) ? "IAMLD" : stype);

	switch (search_key)
	{
	case	FN4:
		sprintf(type_flag, "%-1.1s", type_flags);
		break;

	case	FN9:
		sprintf(type_flag, "%-1.1s", type_flags + 1);
		break;

	case	FN10:
		sprintf(type_flag, "%-1.1s", type_flags + 2);
		break;

	case	FN11:
		sprintf(type_flag, "%-1.1s", type_flags + 3);
		break;

	case	FN12:
		sprintf(type_flag, "%-1.1s", type_flags + 4);
		break;

	default:
		return;
	}

	work_open();

	switch (type_flag[0])
	{
	case	'L':
	case	'l':
		abc_selfield("inmr", "inmr_id_no_5");
		save_rec("#Item Number.", "# Alternate No      Description.");
		break;

	case	'A':
	case	'a':
		abc_selfield("inmr", "inmr_id_no_2");
		save_rec("#Item Number.", "# Alpha Code.       Description.");
		break;

	case	'M':
	case	'm':
		abc_selfield("inmr", "inmr_id_no_4");
		save_rec("#Item Number.", "# Maker No.         Description.");
		break;

	case	'D':
	case	'd':
		sptr = (char *)0;
	default:
		abc_selfield("inmr", "inmr_id_no");
		save_rec("#Item Number.", "# Description.");
		break;
	}

	strcpy(inmr_rec.mr_co_no,      comm_rec.tco_no);
	sprintf(inmr_rec.mr_item_no,   "%-16.16s",(sptr != (char *)0) ? sptr : " ");
	sprintf(inmr_rec.mr_alpha_code,"%-16.16s",(sptr != (char *)0) ? sptr : " ");
	sprintf(inmr_rec.mr_maker_no,  "%-16.16s",(sptr != (char *)0) ? sptr : " ");
	sprintf(inmr_rec.mr_alternate, "%-16.16s",(sptr != (char *)0) ? sptr : " ");
	cc = find_rec("inmr", &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp(inmr_rec.mr_co_no, comm_rec.tco_no))
	{
		valid = 1;
		break_out = 0;
		/*---------------------------------------
		| check inmr_serial_item flag		|
		---------------------------------------*/
		if (item_type[0] != ' ' && item_type[0] != inmr_rec.mr_serial_item[0])
			valid = 0;

		/*---------------------------------------
		| check inmr_costing_flag flag		|
		---------------------------------------*/
		if (item_type[1] != ' ' && item_type[1] != inmr_rec.mr_costing_flag[0])
			valid = 0;

		switch (type_flag[0])
		{
		case	'L':
		case	'l':
			if (!valid)
				break;

			valid = check_search(inmr_rec.mr_alternate, key_val, &break_out);
			sprintf(err_str,
					"(%s) %-36.36s",
					inmr_rec.mr_alternate,
					inmr_rec.mr_description);
			break;

		case	'A':
		case	'a':
			if (!valid)
				break;

			valid = check_search(inmr_rec.mr_alpha_code, key_val, &break_out);
			sprintf(err_str,
					"(%s) %-36.36s",
					inmr_rec.mr_alpha_code,
					inmr_rec.mr_description);
			break;

		case	'M':
		case	'm':
			if (!valid)
				break;

			valid = check_search(inmr_rec.mr_maker_no, key_val, &break_out);
			sprintf(err_str,
					"(%s) %-36.36s",
					inmr_rec.mr_maker_no,
					inmr_rec.mr_description);
			break;

		case	'D':
		case	'd':
			if (!valid)
				break;

			valid = check_search(inmr_rec.mr_description, key_val, &break_out);
			break_out = 0;
			strcpy(err_str, inmr_rec.mr_description);
			break;

		default:
			if (!valid)
				break;

			valid = check_search(inmr_rec.mr_item_no, key_val, &break_out);
			strcpy(err_str, inmr_rec.mr_description);
			break;
		}
#ifdef	INMR_BOM
#include <inmr_bom.h>
#endif
		if (valid)
		{
			cc = save_rec(inmr_rec.mr_item_no, err_str);
			if (cc)
				break;
		}
		else
		{
			if (break_out)
				break;
		}

		cc = find_rec("inmr",&inmr_rec,NEXT,"r");
	}
	abc_selfield("inmr", "inmr_id_no");
	cc = disp_srch();
	work_close();
	if (cc)
	{
		sprintf(inmr_rec.mr_item_no,     "%-16.16s"," ");
		sprintf(inmr_rec.mr_alpha_code,  "%-16.16s"," ");
		sprintf(inmr_rec.mr_maker_no,    "%-16.16s"," ");
		sprintf(inmr_rec.mr_alternate,   "%-16.16s"," ");
		sprintf(inmr_rec.mr_description, "%-40.40s"," ");
		return;
	}

	strcpy(inmr_rec.mr_co_no, comm_rec.tco_no);
	sprintf(inmr_rec.mr_item_no, "%-16.16s", temp_str);
	cc = find_rec("inmr", &inmr_rec, COMPARISON, "r");
	if (cc)
		sys_err("Error in inmr during (DBFIND)", cc, PNAME);
}

#include	<wild_search.h>
