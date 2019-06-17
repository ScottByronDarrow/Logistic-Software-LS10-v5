/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : file_err.c                                     |
|  Source Desc       : Another version of sys_err, Saves on typeing.  |
|                                                                     |
|  Library Routines  : file_err()                                     |
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
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

void
file_err (int err_no, const char *fn, const char *op)
{
	char	tmp_str [512];

	if (!err_no)
		return;

	sprintf (tmp_str, "Error in %s during (%s)", fn, op);

	sys_err (tmp_str, err_no, PNAME);
}
