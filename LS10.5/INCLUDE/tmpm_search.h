#include	<wild_search.h>

void	tmpm_search (char *key_val);

/*===================================
| Search for Prospect master file.  |
===================================*/
void
tmpm_search (char *key_val)
{
	int	valid = 1;
	int	break_out;
	int	_db_find = atoi(get_env("DB_FIND"));
	int	_co_owned = atoi(get_env("DB_CO"));
	char	type_flag[2];
	char	_estab[3];
	char	*stype = chk_env("DB_SER");
	char	*sptr = (*key_val == '*') ? (char *)0 : key_val;

	strcpy(_estab,(_co_owned) ? comm_rec.test_no : " 0");

	switch (search_key)
	{
	case	FN9:
		strcpy(type_flag,"N");
		break;

	case	FN10:
		strcpy(type_flag,"A");
		break;

	case	FN12:
		strcpy(type_flag,"D");
		break;

	default:
		sprintf(type_flag,"%-1.1s",(stype != (char *)0) ? stype : "N");
		break;
	}

	if (type_flag[0] == 'D' || type_flag[0] == 'd')
		sptr = (char *)0;

	work_open();

	if (type_flag[0] == 'A' || type_flag[0] == 'a')
	{
		abc_selfield("tmpm",(_db_find) ? "tmpm_id_no4" : "tmpm_id_no2");
		save_rec("#Number","# Acronym   Prospect Name.");
	}
	else
	{
		abc_selfield("tmpm",(_db_find) ? "tmpm_id_no3" : "tmpm_id_no");
		save_rec("#Number","# Prospect Name.");
	}
	strcpy(tmpm_rec.pm_co_no,comm_rec.tco_no);
	strcpy(tmpm_rec.pm_br_no,_estab);
	sprintf(tmpm_rec.pm_acronym,"%-9.9s",(sptr != (char *)0) ? sptr : " ");
	sprintf(tmpm_rec.pm_pro_no,"%-6.6s",(sptr != (char *)0) ? sptr : " ");
	
	cc = find_rec("tmpm",&tmpm_rec,GTEQ,"r");
	while (!cc && !strcmp(tmpm_rec.pm_co_no,comm_rec.tco_no))
	{
		/*--------------------------------------------
		| If Debtors Branch Owned && Correct Branch. |
		--------------------------------------------*/
		if ( !_db_find && strcmp(tmpm_rec.pm_br_no,_estab))
			break;

		switch (type_flag[0])
		{
		case	'A':
		case	'a':
			valid = check_search(tmpm_rec.pm_acronym,key_val,&break_out);
			break;

		case	'D':
		case	'd':
			valid = check_search(tmpm_rec.pm_name,key_val,&break_out);
			break_out = 0;
			break;

		default:
			valid = check_search(tmpm_rec.pm_pro_no,key_val,&break_out);
			break;
		}

		if (valid)
		{
			sprintf(err_str," (%s) %-40.40s",
				tmpm_rec.pm_acronym,
				tmpm_rec.pm_name);
			cc = save_rec(tmpm_rec.pm_pro_no,err_str);
			if (cc)
				break;
		}
		else
		{
			if (break_out)
				break;
		}

		cc = find_rec("tmpm",&tmpm_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	abc_selfield("tmpm",(_db_find) ? "tmpm_id_no3" : "tmpm_id_no");
	if (cc)
	{
		sprintf(tmpm_rec.pm_acronym,"%-9.9s"," ");
		sprintf(tmpm_rec.pm_name,"%-40.40s"," ");
		return;
	}
	
	strcpy(tmpm_rec.pm_co_no,comm_rec.tco_no);
	strcpy(tmpm_rec.pm_br_no,_estab);
	sprintf(tmpm_rec.pm_pro_no,"%-6.6s",temp_str);
	sprintf(tmpm_rec.pm_acronym,"%-9.9s",temp_str);
	cc = find_rec("tmpm",&tmpm_rec,GTEQ,"r");
	if (cc)
		sys_err("Error in tmpm during (DBFIND)",cc,PNAME);
}
