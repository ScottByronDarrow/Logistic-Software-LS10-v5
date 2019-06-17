/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( cudi_search.h )                                  |
|  Program Desc  : (                                              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Updates files :                                                    |
|---------------------------------------------------------------------|
|  Author        : ?               | Date Written  : ??/??/????       |
|---------------------------------------------------------------------|
|  Date Modified : (18/08/1999)    | Modified by : Eumir Que Camara.  |
|                :                                                    |
|  Comments      :                                                    |
|  (18/08/1999)  : Ported to ANSI standard.                           |
=====================================================================*/
#ifndef __CUDI_SEARCH_H__
#define __CUDI_SEARCH_H__

int
cudi_search (
 int	indx)
{
	char	save_str[170];

        work_open();
	save_rec("#No","#Details");
	cudi_rec.di_hhcu_hash = cumr_rec.cm_hhcu_hash;
	cudi_rec.di_del_no = 0;
	cc = find_rec(cudi,&cudi_rec,GTEQ,"r");
        while (!cc && cudi_rec.di_hhcu_hash == cumr_rec.cm_hhcu_hash)
    	{                        
		sprintf(save_str,"%s, %s, %s, %s",
			clip(cudi_rec.di_name),
			clip(cudi_rec.di_adr[0]),
			clip(cudi_rec.di_adr[1]),
			clip(cudi_rec.di_adr[2]));

		save_str[80] = '\0';
		sprintf(err_str,"%3d",cudi_rec.di_del_no);
		cc = save_rec(err_str,save_str); 
		if (cc)
			break;
		cc = find_rec(cudi,&cudi_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	        return(-1);
	cudi_rec.di_hhcu_hash = cumr_rec.cm_hhcu_hash;
	cudi_rec.di_del_no = atoi(temp_str);
	cc = find_rec(cudi,&cudi_rec,COMPARISON,"r");
	if (cc)
 	        sys_err("Error in cudi During (DBFIND)",cc,PNAME);

	switch (indx)
	{
	case	0:
		sprintf(temp_str,"%-40.40s",cudi_rec.di_name);
		break;

	case	1:
	case	2:
	case	3:
		sprintf (temp_str,"%-40.40s",cudi_rec.di_adr[indx - 1]);
		break;

	default:
		break;
	}

	return (cudi_rec.di_del_no);
}

#endif /* #ifndef __CUDI_SEARCH_H__ */
