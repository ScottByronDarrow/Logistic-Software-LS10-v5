/*******************************************************************************
 *
 *	Global vars required by DSO libraries
 *
 *	These are usually defined in the application by <pslscr.h> (yech!),
 *	but if an application makes use of a library routine without using
 *	<pslscr.h> the symbols are undefined. To resolve them, there must be
 *	defined here
 *
 ******************************************************************************/
#include <psizes.h>

int		cc, search_key, search_ok, dflt_used, restart, max_work, last_char,
		prog_exit;
char	temp_str [LS_SIZEOF_TEMP_STR], err_str [LS_SIZEOF_ERR_STR];
