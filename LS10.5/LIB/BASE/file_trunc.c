/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : file_trunc.c                                   |
|  Source Desc       : Truncates an open file.                        |
|                                                                     |
|  Library Routines  : file_trunc()                                   |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|---------------------------------------------------------------------|
|  Date Modified     :   /  /     | Modified  by  :                   |
|                                                                     |
|  Comments          :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
| $Log: file_trunc.c,v $
| Revision 5.0  2001/06/19 06:59:16  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:52:36  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/12 13:34:20  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 07:17:13  gerry
| Forced revision no. to 2.0 - Rel-15072000
|
| Revision 1.7  1999/09/23 05:08:06  alvin
| Checking and removed comments,.
|
| Revision 1.6  1999/09/23 05:04:23  alvin
| Restored first parameter, work_tab.
|
| Revision 1.5  1999/09/23 04:54:00  alvin
| Removed fopen declaration
|
| Revision 1.4  1999/09/22 03:01:12  alvin
| Removed TAB_PTR parameter, which is not being used.
|
| Revision 1.3  1999/09/13 09:36:29  scott
| Updated for Copyright
|
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

int
file_trunc (TAB_PTR work_tab, long int size)
{
	int	c;
	long	fsize = 0L;
	FILE	*new_file;
	char	*tab_name (), tmp_buf [128];

	if (!(new_file = fopen(tab_name ("trunc"),"w")))
		return (-1);

	/*---------------------------------------
	| output from original file		|
	---------------------------------------*/
	while ( fsize < size )
	{
		if ((c = getc (TAB_ID)) != EOF)
			putc(c,new_file);
		else
			break;
		fsize++;
	}
	fclose(new_file);
	fclose(TAB_ID);

	strcpy (tmp_buf, tab_name (TAB_NAME));
	unlink (tmp_buf);

	link (tab_name ("trunc"), tmp_buf);
	unlink (tab_name ("trunc"));

	if (!(TAB_ID = fopen(tmp_buf,"r+")))
		return(-1);

	return(0);
}
