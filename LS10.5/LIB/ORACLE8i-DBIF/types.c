/*====================================================================|
|  Copyright (C) 1999 - 1999 Logistic Software Limited.               |
|=====================================================================|
| $Id: types.c,v 5.0 2002/05/08 01:30:09 scott Exp $
|  Program Name  : (types.c)
|  Program Desc  : (Internal/external type conversion routines)
|---------------------------------------------------------------------|
| $Log: types.c,v $
| Revision 5.0  2002/05/08 01:30:09  scott
| CVS administration
|
| Revision 1.2  2002/03/11 02:31:56  scott
| Updated to perform code check and comment lineups.
|
| Revision 1.1  2002/02/05 02:39:52  kaarlo
| Initial check-in for ORACLE8i porting.
|
| Revision 5.1  2001/06/21 08:01:02  cha
| Updated to handle correctly the Money datatype.
|
| Revision 5.0  2001/06/19 07:10:28  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/06 02:09:53  cha
| Updated to check in changes made to the Oracle DBIF Library
|
| Revision 1.1  2000/11/20 06:11:52  jason
| Initial update.
|
| Revision 2.2  2000/07/31 06:05:35  raymund
| Corrected Date (Century) conversion bug
|
| Revision 2.1  2000/07/28 06:10:08  raymund
| Implemented CURRENT in find_rec. Provided a patch for bug in disp_srch().
|
| Revision 2.0  2000/07/15 07:33:51  gerry
| Forced Revision No. Start to 2.0 Rel-15072000
|
| Revision 1.3  1999/10/29 03:02:28  jonc
| Added date-conversion support.
|
| Revision 1.2  1999/10/28 01:58:36  jonc
| Added support for generic-catalog access. Damnably slow, though.
|
| Revision 1.1  1999/10/21 21:47:05  jonc
| Alpha level checkin:
| Done: database queries, updates.
| Todo: date conversion, locking and error-handling.
=====================================================================*/

#include	<ptypes.h>
#include	<pDate.h>
#include	<twodec.h>

#include	"oracledbif.h"

/*
 * Constants 
 */
#define	ROWID_SIZE	255			/* ORACLE docs specify max 255 byte array */

#define	FLOAT_FLOAT_PREC	63			/* approx 19 digits */
#define	FLOAT_DOUBLE_PREC	126			/* approx 38 digits */

#define	NUMBER_MONEY_PREC	15
#define	NUMBER_MONEY_SCALE	2

/*
 * Structures 
 */
struct align_date	{char a; ORA_Date	b;	};
struct align_long	{char a; long		b;	};
struct align_float	{char a; float   	b;	};
struct align_double	{char a; double		b;	};

/*================
| _DecodeORAType |
================*/

void
_DecodeORAType (
	const char * col_name,
	const char * seq_name,
	const char * data_type,
	int data_length,
	int data_precision,
	int data_scale,
	sb2 i_data_precision,
	sb2 i_data_scale,
	enum ORAType * type,		/* output buffer */
	int * length)				/* output buffer */
{
	/*
	 * Determine type: 
	 * 	CHAR		  
	 *	DATE		 
	 *	FLOAT		
	 *	NUMBER	
	 */

	if (!strcmp ("CHAR", data_type))
	{
		*type = OT_Chars;
		*length = data_length + 1;
	} else if (!strcmp ("DATE", data_type))
	{
		*type = OT_Date;
	} else if (!strcmp ("FLOAT", data_type))
	{
		/*
		 * Decide on whether it should be a 				  
		 * 	- float		  								  
		 *	- double		  							 
		 *			  									
		 * We kludge this by looking at the data-precision 
		 */

		if (i_data_precision == -1)
			oracledbif_error ("data-precision NULL on FLOAT %s", col_name);

		switch (data_precision)
		{
			case 	FLOAT_FLOAT_PREC:
					*type = OT_Float;
					break;
			case 	FLOAT_DOUBLE_PREC:
					*type = OT_Double;
					break;
			default:
					oracledbif_error (
						"data-precision (%d) on FLOAT %s not recognised",
						data_precision,
						col_name);
		}
	} else if (!strcmp ("NUMBER", data_type))
	{
		/*
		 * Decide on whether it should be 
		 * 	- number					 
		 *	- money	  					
		 *	- serial	  			
		 */

		if (!strcmp (seq_name, col_name))
		{
			*type = OT_Serial;
		} else
		{
			int recognised = FALSE;

			if (i_data_precision == -1)
			{
				if (i_data_scale == -1 ||
				   (i_data_scale != -1 && data_scale == 0))
				{
					*type = OT_Number;
					recognised = TRUE;
				}
			} else if (i_data_precision != -1 &&
					   data_precision == NUMBER_MONEY_PREC &&
					   i_data_scale != -1 &&
					   data_scale == NUMBER_MONEY_SCALE)
			{
				*type = OT_Money;
				recognised = TRUE;
			}
			if (!recognised)
				oracledbif_error ("NUMBER %s not recognised", col_name);
		}
	} else
		oracledbif_error ("Don't know type: %s", data_type);

	/*
	 * All types aside from strings have their have a fixed data-type size
	 */
	if (*type != OT_Chars)
		*length = _internal_size (*type);
}

/*
 * _internal_size 
 */
int
_internal_size (
 	enum ORAType t)
{
	/*
	 * Internal buffer datatype sizes 
	 */
	switch (t)
	{
		case 	OT_Chars:
				return 1;
		case 	OT_Date:
				return sizeof (ORA_Date);
		case 	OT_Serial:
		case 	OT_Number:
				return sizeof (long);
				break;
		case 	OT_Float:
				return sizeof (float);
		case 	OT_Money:
		case 	OT_Double:
				return sizeof (double);
		case 	OT_RowId:
				return ROWID_SIZE;
		default:
				oracledbif_error ("userdatasize: unknown data-type %d", t);
	}

	return 0;
}

/*
 * _internal_align 
 */
int
_internal_align (
 	int orig,
 	enum ORAType t)
{
	/*
	 * Aligns offsets for internal-data buffer 			  
	 * This *must* agree with the internal-datatypes used.
	 */
	int align_on;
	int offset;

	switch (t)
	{
		case 	OT_Chars:
		case 	OT_RowId:
				return 0;
		case 	OT_Date:
				align_on = offsetof (struct align_date, b);
				break;
		case 	OT_Serial:
		case 	OT_Number:
				align_on = offsetof (struct align_long, b);
				break;
		case 	OT_Float:
				align_on = offsetof (struct align_float, b);
				break;
		case 	OT_Money:
		case 	OT_Double:
				align_on = offsetof (struct align_double, b);
				break;
		default:
				oracledbif_error ("align: unknown data-type %d", t);
	}

	if (!(offset = orig % align_on))
		return 0;
	return align_on - offset;
}

/*
 * _application_align 
 */
int
_application_align (
 int orig,
 enum ORAType t)
{
	/*
	 * Aligns offsets for user-application buffer     
	 * This *must* agree with the userdatatypes used.
	 */
	int align_on;
	int offset;

	switch (t)
	{
		case 	OT_Chars:
				return 0;
		case 	OT_Date:
		case 	OT_Serial:
		case 	OT_Number:
				align_on = offsetof (struct align_long, b);
				break;
		case 	OT_Float:
				align_on = offsetof (struct align_float, b);
				break;
		case 	OT_Money:
		case 	OT_Double:
				align_on = offsetof (struct align_double, b);
				break;
		default:
				oracledbif_error ("align: unknown data-type %d", t);
	}

	if (!(offset = orig % align_on)) 
		return 0;
	return align_on - offset;
}

/*
 * _application_size 
 */
int
_application_size (
 enum ORAType t)
{
	/*
	 * Application datatype sizes 
	 */
	switch (t)
	{
		case 	OT_Chars:
				return 1;
		case 	OT_Date:
		case 	OT_Serial:
		case 	OT_Number:
				return sizeof (long);
				break;
		case 	OT_Float:
				return sizeof (float);
		case 	OT_Money:
		case 	OT_Double:
				return sizeof (double);
		default:
				oracledbif_error ("userdatasize: unknown data-type %d", t);
	}

	return 0;
}

/*
 * _internal_sqltype 
 */
int
_internal_sqltype (
	enum ORAType t)
{
	/*
	 * Returns equivalent external C data-types for ORACLE
	 *	given internal database type					 
	 */
	switch (t)
	{
		case 	OT_Chars:
				return SQLT_STR;
		case 	OT_Date:
				return SQLT_DAT;
		case 	OT_Serial:
		case 	OT_Number:
				return SQLT_INT;
		case 	OT_Money:
		case 	OT_Double:
		case 	OT_Float:
				return SQLT_FLT;
		case 	OT_RowId:
				return SQLT_CHR;
		default:
				oracledbif_error ("Type conversion error");
	}
	return 0;
}

/*
 * _ConvApp2Raw 
 */
void
_ConvApp2Raw (
	ColumnDef * col,
	int appoffset,
	void * raw_buffer)
{
	/*
	 * Convert from Application user-buffer to Internal data-buffer 
	 */
	int d, m, y;
	ORA_Date ora_date;
	char * appbuf = (char *) raw_buffer + appoffset;

	switch (col -> type)
	{
		case 	OT_Chars:
				/*
				 * If we have a NULL string, we have to supply a string 
				 * with 1 space instead, otherwise ORACLE treats this 
				 * as a NULL - with unexpected consequences          
				 */
				if (appbuf [0])
				{
					/*
					 * Make sure that appbuf is NULL terminated at the length 
					 * expected by Oracle. This is to solve temporarily a bug
					 * in disp_srch                                         
					 */
					
					appbuf[col->length - 1] = '\0';
					strcpy (col -> data, appbuf);
        		}
				else
					strcpy (col -> data, " ");
				break;
		case 	OT_Date:
				DateToDMY (*(Date *) appbuf, &d, &m, &y);
				ora_date.day = d;
				ora_date.month = m;
				ora_date.year = 100 + y % 100;
				ora_date.century = 100 + y / 100;
				ora_date.hour =
				ora_date.minute =
				ora_date.second = 1;
				*(ORA_Date *) col -> data = ora_date;
				break;
		case 	OT_Serial:
		case 	OT_Number:
				*(long *) col -> data = *(long *) appbuf;
				break;
		case 	OT_Money:
		  		*(double *) col -> data = no_dec(*(double *) appbuf);
		  		break; 
		case 	OT_Double:
				*(double *) col -> data = *(double *) appbuf;
				break;
		case 	OT_Float:
				*(float *) col -> data = *(float *) appbuf;
				break;
		case 	OT_RowId:
		default:
				oracledbif_error ("_ConvApp2Raw failed on %s", col -> name);
	}
}

/*
 * _ConvRaw2App 
 */
void
_ConvRaw2App (
	ColumnDef * col,
	int appoffset,
	void * raw_buffer)
{
	/*
	 * Convert from Internal data-buffer to Application user-buffer 
	 * The NULL indicators need to be checked as well.			   
	 */
	ORA_Date ora_date;
	int isnull = col -> null_ind == -1;
	char * appbuf = (char *) raw_buffer + appoffset;

	switch (col -> type)
	{
		case 	OT_Chars:
				if (isnull)
					appbuf [0] = '\0';
				else
					strcpy (appbuf, col -> data);
				break;
		case 	OT_Date:
				ora_date = *(ORA_Date *) col -> data;
				*(Date *) appbuf =
				DMYToDate (ora_date.day,
						   ora_date.month,
						   (ora_date.century - 100) * 100 + ora_date.year - 100);
				break;
		case 	OT_Serial:
		case 	OT_Number:
				*(long *) appbuf = isnull ? 0 : *(long *) col -> data;
				break;
		case 	OT_Money:
		case 	OT_Double:
				*(double *) appbuf = isnull ? 0.0 : *(double *) col -> data;
				break;
		case 	OT_Float:
				*(float *) appbuf = isnull ? 0.0 : *(float *) col -> data;
				break;
		case 	OT_RowId:
		default:
				oracledbif_error ("_ConvRaw2App failed on %s", col -> name); 
	}
}
