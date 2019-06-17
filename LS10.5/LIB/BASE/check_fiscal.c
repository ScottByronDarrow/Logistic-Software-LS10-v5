/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : check_fiscal.c                                 |
|  Source Desc       :	hecks fiscal year end.                        |
|                                                                     |
|  Library Routines  : check_fiscal()                                 |
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
/*
.function
	Function	:	check_fiscal ()

	Description	:	Checks fiscal year end.

	Notes		:	Checks that the fiscal year end has been set for
				use by the General Ledger routines.

	Globals		:	GV_fiscal - Variable which should contain the
					    fiscal year end month.
.end
*/
void
check_fiscal (void)
{
	extern	int	GV_fiscal;
	extern	char	*PNAME;

	if (!GV_fiscal)
		sys_err ("Fiscal year not set - Check comm_list/comm_rec.",
								-1, PNAME);
}
