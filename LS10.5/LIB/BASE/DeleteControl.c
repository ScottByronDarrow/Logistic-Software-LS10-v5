/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: DeleteControl.c,v 5.1 2001/08/14 02:34:04 scott Exp $
|  Program Name  : (DeleteControl.c)
|  Program Desc  : (Delete Control Function for delete programs.)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 13th Aug 2001.   |
|---------------------------------------------------------------------|
| $Log: DeleteControl.c,v $
| Revision 5.1  2001/08/14 02:34:04  scott
| Added DeleteControl.c - Required for new Delete wizard
|
=====================================================================*/
#include	<std_decs.h>
#include	<DeleteControl.h>

/*====================================
| Multi Lingual convershion routine. |
====================================*/
int
FindDeleteControl (
	char	*coNo,
	char	*controlCode)
{
	int	delErrCode	=	0;
	open_rec (delh, delh_list, DELH_NO_FIELDS, "delh_id_no");

	sprintf (delhRec.co_no, "%-2.2s", 	coNo);
	sprintf (delhRec.code,  "%-20.20s", controlCode);
	delErrCode = find_rec (delh, &delhRec, COMPARISON, "r");
	if (delErrCode)
	{
		abc_fclose (delh);
		return (delErrCode);
	}
	abc_fclose (delh);
	return (delErrCode);
}
