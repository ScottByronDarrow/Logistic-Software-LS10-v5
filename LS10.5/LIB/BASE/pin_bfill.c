/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : pin_bfill.c                                    |
|  Source Desc       : Byte initialisation routine.                   |
|                                                                     |
|  Library Routines  : pin_bfill()                                    |
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
/*
.function
	Function	:	pin_bfill ()

	Description	:	Byte initialisation routine.

	Notes		:	Pin_bfill initialises the destination with a
				specified number of the designated
				initialisation character.
			
	Parameters	:	to     - Destination.
				f_char - Initialisation character.
				siz    - Number of bytes to initialise.
.end
*/
void
pin_bfill (char *to, char f_char, unsigned int siz)
{
	while (siz--)
		*to++ = f_char;
}
