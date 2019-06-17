#ident	"$Id: attrs.c,v 5.0 2001/06/19 07:10:27 cha Exp $"
/*
 *	Database SQL interface attributes
 *
 *******************************************************************************
 *	$Log: attrs.c,v $
 *	Revision 5.0  2001/06/19 07:10:27  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.1  2001/04/06 02:09:52  cha
 *	Updated to check in changes made to the Oracle DBIF Library
 *	
 *	Revision 1.1  2000/11/20 06:11:52  jason
 *	Initial update.
 *	
 *	Revision 2.0  2000/07/15 07:33:50  gerry
 *	Forced Revision No. Start to 2.0 Rel-15072000
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
		{"has-serial",				NULL},
		{"has-sequence",			"yes"},
		{"has-column-placement",	NULL},

		{"date-type",				"date"},
		{"serial-type",				"integer"},
		{"short-type",				"integer"},
		{"long-type",				"integer"},
		{"float-type",				"float(63)"},
		{"double-type",				"float(126)"},
		{"money-type",				"number(15,2)"},

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
