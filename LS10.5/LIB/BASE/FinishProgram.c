/*
 *
 *******************************************************************************
 * FinishProgram () 
 * Does the stuff when a program completes.
 *******************************************************************************
 *	$Id: FinishProgram.c,v 5.0 2002/05/07 10:13:55 scott Exp $
 *	$Log: FinishProgram.c,v $
 *	Revision 5.0  2002/05/07 10:13:55  scott
 *	Updated to bring version number to 5.0
 *	
 *	Revision 1.1  2001/08/09 10:40:37  scott
 *	New function
 *	
 *	
 */
#include	<std_decs.h>

void	FinishProgram	(void);

void
FinishProgram (void)
{
	if (foreground())
	{
#ifndef	GVISION
		clear();
#endif
		if (_wide)
			snorm();

		rset_tty();
		crsr_on();
	}
}
