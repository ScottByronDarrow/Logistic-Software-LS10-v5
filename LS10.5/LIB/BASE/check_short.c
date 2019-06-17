/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( check_short.c  )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (  /  /  )      | Modified  by  :                  |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
/*-----------------------------------------------
| Returns 0 if srch on short code required	|
-----------------------------------------------*/
int
check_short(char *item_no)
{
	char	*clip();

	clip(item_no);
	if (strlen(item_no) > 8 || strlen(item_no) == 0)
		return(1);
	return(0);
}
/*------------------------------------
| Returns 0 if class valid 1 if not. |
------------------------------------*/
int
check_class(char *class)
{
	if ( class[0] == 'N' || class[0] == 'Z' )
		return(1);

	return(0);
}
