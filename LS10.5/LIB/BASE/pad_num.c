/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: pad_num.c,v 5.1 2001/08/06 22:40:55 scott Exp $
|  Module Name   : (pad_num.c)
|  Program Desc  : (Zero pad strings.)
|---------------------------------------------------------------------|
| $Log: pad_num.c,v $
| Revision 5.1  2001/08/06 22:40:55  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 06:59:36  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/02 04:08:48  scott
| Updated to usage of zero_pad with new string greater than existing string.
|
=====================================================================*/
#include	<ctype.h>
#include	<string.h>

#define	BAD_CHAR	'*'
#define	CUSTOMER_LEN	6
#define	BATCH_LEN	5

//====================================================================
// This Routine is used for Any Number to pad zeros into the front of    
// numbers if they don't contain any alphanumeric characters.            
//
char *
zero_pad (
 char *	pad_str,
 int	pad_len)
{
	int		len;
//	char	buffer [15];
	char	*str, *end;

	//--------------------------------------------------------------------
	//	Make sure "pad_str" only contains spaces and numeric characters
	//
	for (str = pad_str; *str; str++)
		if (!isdigit (*str) && *str != ' ')
			return (pad_str);

	//-----------------------------------
	//	Look for first numeric character
	//
	for (str = pad_str; *str; str++)
		if (isdigit (*str))
			break;

	//---------------------------------
	//	Truncate end of numeric string
	//
	for (end = str; *end; end++)
		if (!isdigit (*end))
		{
			*end = '\0';		// end it right here
			break;
		}

	if ((len = strlen (str)) > pad_len)
	{
		//--------------------------------------------------------------------
		//	Application bug. Fill up pad_str with BAD_CHAR to indicate bug
		//
		for (str = pad_str; *str; str++)
			*str = BAD_CHAR;
	}
	else if (len < pad_len)			// only do it if we have to 
	{
		//--------------------------------------------------------------------
		//	Work from end of string, and then top up with 0's
		//
		int	i;

		pad_str [pad_len--] = '\0';				// terminate end of string
		for (i = len - 1; i >= 0; i--, pad_len--)
			pad_str [pad_len] = str [i];

		while (pad_len >= 0)
			pad_str [pad_len--] = '0';
	}

	return (pad_str);
}

//=========================================================================
// This Routine is used for Customer and creditor Numbers to pad zeros into 
// the front of numbers if they don't contain any alphanumeric characters.
//
char *
pad_num (
 char *	istring)
{
	return (zero_pad (istring, CUSTOMER_LEN));
}

//=======================================================================
// This Routine is used for Batch Number to pad zeros into the front of  
// numbers if they don't contain any alphanumeric characters.            

char *
pad_batch (
 char *	bstring)
{
	return (zero_pad (bstring, BATCH_LEN));
}
