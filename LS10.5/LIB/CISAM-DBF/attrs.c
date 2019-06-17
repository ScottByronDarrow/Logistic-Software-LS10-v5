#ident	"$Id: attrs.c,v 5.0 2001/06/19 07:07:42 cha Exp $"
/*
 *	Database SQL interface attributes
 *
 *******************************************************************************
 *	$Log: attrs.c,v $
 *	Revision 5.0  2001/06/19 07:07:42  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:53:53  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 2.0  2000/07/15 07:32:17  gerry
 *	Forced Revision No start to 2.0 - Rel-15072000
 *	
 *	Revision 1.3  1999/11/17 19:48:27  jonc
 *	Fixed: float/double attributes.
 *	
 *	Revision 1.2  1999/11/15 19:50:22  nz
 *	Fixed small/long SQL definitions
 *	
 *	Revision 1.1  1999/10/28 23:24:31  jonc
 *	Added DbIfAttribute interface.
 *	
 */
#include	<stddef.h>
#include	<string.h>

#include	<dbio.h>
#include	<ProtosIF.h>

struct PropertyList
{
	const char * property, * value;
};

static struct PropertyList
list [] =
	{
		{"has-serial",				"yes"},
		{"has-sequence",			NULL},
		{"has-column-placement",	"yes"},

		{"date-type",				"date"},
		{"serial-type",				"serial"},
		{"short-type",				"smallint"},
		{"long-type",				"integer"},
		{"float-type",				"smallfloat"},
		{"double-type",				"float"},
		{"money-type",				"money"},

		{NULL, NULL}							/* terminator */
	};


const char *
DbIfAttribute (
 const char * property)
{
	int i;

	for (i = 0; list [i].property; i++)
		if (!strcmp (property, list [i].property))
			return list [i].value;
	return NULL;
}
