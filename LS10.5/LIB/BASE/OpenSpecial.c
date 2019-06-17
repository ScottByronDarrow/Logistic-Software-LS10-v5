/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: OpenSpecial.c,v 5.0 2001/06/19 06:59:13 cha Exp $
-----------------------------------------------------------------------
| $Log: OpenSpecial.c,v $
| Revision 5.0  2001/06/19 06:59:13  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:52:34  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.7  2001/02/14 09:58:48  scott
| Opps removed define not required
|
| Revision 3.6  2001/02/14 09:57:06  scott
| Updated to use const
|
| Revision 3.5  2001/02/14 09:46:35  scott
| Updated comments
|
| Revision 3.4  2001/02/14 09:34:21  scott
| Updated after testing
|
| Revision 3.3  2001/02/14 09:29:17  scott
| Updated for changed to OpenSpecial
|
| Revision 3.2  2001/02/14 09:15:33  scott
| Added comment section
|
*/
#include	<std_decs.h>

char *
OpenSpecial (
	const	char	*subDir,
	const	char	*fileName)
{
	/*
	 *	Look in the following directories for the print file
	 *		1. . (development copy)
	 *		2. ./subDir (local copy)
	 *		3. PSL_MENU_PATH/subDir (user specific)
	 *		4. PROG_PATH/BIN/subDir (std copy)
	 */
	static	char	cwd [80],
					pathname [80];
	char	*env,
			*ProgPath = "PROG_PATH",
			*MenuPath = "PSL_MENU_PATH";

	/*
	 *	Check for local copy
	 */
	strcpy (pathname, fileName);
	if (!access (pathname, F_OK))
		return (pathname);

	/*
	 *	Use ./subDir if currently not in PROG_PATH/BIN
	 */
	sprintf (pathname, "%s/BIN", getenv (ProgPath));
	if (strcmp (getcwd (cwd, sizeof (cwd)), pathname))
	{
		sprintf (pathname, "%s/%s", subDir, fileName);
		if (!access (pathname, F_OK))
			return (pathname);
	}

	/*
	 *	Check user-specific copy
	 */
	if ((env = getenv (MenuPath)))
	{
		sprintf (pathname, "%s/%s/%s", env, subDir, fileName);
		if (!access (pathname, F_OK))
			return (pathname);
	}

	/*
	 *	Standard copy
	 */
	if ((env = getenv (ProgPath)))
	{
		sprintf (pathname, "%s/BIN/%s/%s", env, subDir, fileName);
		if (!access (pathname, F_OK))
			return (pathname);
	}
	return ((char *)0);		/* abject failure */
}
