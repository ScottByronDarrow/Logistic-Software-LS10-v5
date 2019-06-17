/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : mod_env.c                                      |
|  Source Desc       : Modify system environment variable.            |
-----------------------------------------------------------------------
	$Log: mod_env.c,v $
	Revision 5.0  2001/06/19 06:59:35  cha
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.0  2001/03/09 00:52:38  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 3.0  2000/10/12 13:34:24  gerry
	Revision No. 3 Start
	<after Rel-10102000>
	
	Revision 2.0  2000/07/15 07:17:17  gerry
	Forced revision no. to 2.0 - Rel-15072000
	
	Revision 1.4  1999/11/10 03:56:11  jonc
	Removed conflicting internal decl. of strdup which stops compile on
	RS6000 boxes.
	
=====================================================================*/
#include	<std_decs.h>

extern	char	**environ;

/*
.function
	Function	:	mod_env ()

	Description	:	Modify an environment variable.

	Notes		:	Mod_env modifies a variable in the environment
				of the current process. The modified variable
				is then available to any processes created by
				the modifying process.
			
	Parameters	:	e_var   - Pointer to string containing the name
					  of the variable to modify.
				e_val   - New value to place in variable.

	Globals		:	environ - Pointer to a table of pointers to 
					  the environment variables.

	Returns		:	e_ptr   - A pointer to the modified variable if
					  the variable was found.
				NULL	- A NULL character pointer if the
					  named variable cannot be found.
					  Now superceded. Only returns NULL
					  if it fails to create a new environ.
.end
*/

char	*mod_env (char *e_var, char *e_val)
{
	register char	**e_ptr;
	int	 len;
	static 	 char	 tmp_str [256];

	len = strlen (e_var);
	for (e_ptr = environ; e_ptr && *e_ptr; e_ptr++)
		if (!strncmp (*e_ptr, e_var, len))
		{
			sprintf (tmp_str, "%s=%s", e_var, e_val);
			if (!(*e_ptr = strdup (tmp_str)))
				return ((char *) NULL);

			return (*e_ptr);
		}

	return ((char *) NULL);
}
