/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : pin_bcopy.c                                    |
|  Source Desc       : Byte copy routine.                             |
|                                                                     |
|  Library Routines  : pin_bcopy()                                    |
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
	Function	:	pin_bcopy ()

	Description	:	Byte copy routine.

	Notes		:	This routine is used to copy a specified number
				of bytes from on location to another.
				NULL characters do no terminate the copy.
			
	Parameters	:	to   - Pointer to destination.
				from - Pointer to source.
				siz  - Number of bytes to copy.
.end
*/
void
pin_bcopy (char *to, char *from, unsigned int siz)
{
	while (siz--)
		*to++ = *from++;
}
