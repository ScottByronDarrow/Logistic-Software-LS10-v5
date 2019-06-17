/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : dflt_env.c                                     |
|  Source Desc       : Get environment variable or default.           |
|                                                                     |
|  Library Routines  : dflt_env()                                     |
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
	Function	:	dflt_env ()

	Description	:	Get environment variable or default.

	Notes		:	Gets the specified environment variable or, if
				the variable doesn't exist returns the default.

	Parameters	:	vname - Pointer to the variable name.
			 	vdflt - Pointer to the variable default.
	
	Returns		:	v_ptr - Pointer to the value or the default.
.end
*/
char	*dflt_env (char *vname, char *vdflt)
{
	char	*v_ptr,	*getenv (const char *);

	if (!(v_ptr = getenv (vname)))
		v_ptr = vdflt;

	return (v_ptr);
}
