/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : toggle_crsr.c                                  |
|  Source Desc       : Turn cursor ON/OFF and return previous state.  |
|                                                                     |
|  Library Routines  : toggle_crsr()                                  |
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
#include <stdio.h>
#include <tcap.h>

/*
.function
	Function	:	crsr_toggle ()

	Description	:	Turn cursor ON/OFF and return previous state.

	Notes		:	Crsr_toggle is used to switch the cursor into
				the state needed by the calling routine and then
				just before the calling routine is exited to
				switch the cursor back into it's previous state.
			
	Parameters	:	stat	 - TRUE  - Turn cursor on.
					   FALSE - Turn cursor off.

	Returns		:	tmp_stat - Integer holding the previous status.
.end
*/
int
crsr_toggle (int stat)
{
	static	int	old_stat = 1;
	int	tmp_stat;

	tmp_stat = old_stat;
	
	if (stat)
	{
		crsr_on ();
		old_stat = 1;
	}
	else
	{
		crsr_off ();
		old_stat = 0;
	}

	fflush (stdout);
	return (tmp_stat);
}
