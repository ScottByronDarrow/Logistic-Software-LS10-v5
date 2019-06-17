#ident	"$Id: types.c,v 5.0 2001/06/19 07:08:20 cha Exp $"
/*
 *	Internal/external type conversion routines
 *
 *******************************************************************************
 *	$Log: types.c,v $
 *	Revision 5.0  2001/06/19 07:08:20  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:27:56  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 1.4  2000/10/12 13:31:29  gerry
 *	Removed Ctrl-Ms
 *	
 *	Revision 1.3  2000/09/25 09:48:46  gerry
 *	DB2 Release 2 - After major fixes
 *	
 *
 *
 *	
 *	
 */
#include	<string.h>
#include	<ptypes.h>
#include	<pDate.h>
#include	<stddef.h>
#include	<stdio.h>
#include	<sqlcli.h>

#include	"db2dbif.h"

/*
 *	Magic stuff
 */

#define	FLOAT_FLOAT_PREC	63			/* approx 19 digits */
#define	FLOAT_DOUBLE_PREC	126			/* approx 38 digits */

#define	NUMBER_DECIMAL_PREC		19			/*DB2 suggested size*/
#define	NUMBER_DECIMAL_SCALE	4

/* DB2 suggested size of date is 10 yyyy-mm-dd */
/* Changed from 11 to 10, July 4, 2K EPF*/
/* Let's try 6, we are concerned in the dates storage rather that displaying it*/
#define DATE_SIZE	6 

/*#define DEC_SIZE 25*/ /* epf: by RKA, I think this is the display size "+ 3" */
#define DEC_SIZE 19		/* epf: precision is 19 + 2; '+' or '-' and the '.' */
		
/*
 */

typedef struct DATE_STRUCT DB2_Date;
typedef SQL_NUMERIC_STRUCT db2_decimal;

struct align_date	 {	char a;		Date	b;	};
struct align_long	 {	char a;		int	b;	};
struct align_float	 {	char a;		float	b;	};
struct align_double	 {	char a;		double	b;	};
struct align_decimal {  char a;		double	b;  };

/*******************************************************************/
struct align_db2date	 {	char a;		DB2_Date	b;	};
struct align_db2int		 {	char a;		long int 	b;	};
struct align_db2double	 {	char a;		double	b;	};
struct align_db2float	 {	char a;		double	b;	};
struct align_db2decimal  {  char a;		db2_decimal	b;  };
struct align_db2real	 {  char a;		float	b;  };
/*******************************************************************/

int
_application_align (
 int orig,
 enum DB2Type t)
{
	/*
	 *	Aligns offsets for user-application buffer
	 *	This *must* agree with the userdatatypes used.
	 */
	int align_on;
	int offset;

	switch (t)
	{
	case DB2_CHAR:
		return 0;

	case DB2_INTEGER:
		align_on = offsetof (struct align_long, b);
		break;

	case DB2_DATE:
		align_on = offsetof (struct align_date, b); /* epf: use to be like DB2_NUMERIC, give it it's own alignment structure */
		break;

	case DB2_REAL:
		align_on = offsetof (struct align_float, b);
		break;
	
	case DB2_FLOAT:
		align_on = offsetof (struct align_double, b);
		break;

	case DB2_DECIMAL:
	case DB2_NUMERIC:
		align_on = offsetof (struct align_decimal, b);
		break;	
	
	case DB2_DOUBLE:
		align_on = offsetof (struct align_double, b);
		break;

	default:
		printf("align: unknown data-type %d", t);
	}

	if (!(offset = orig % align_on))
		return 0;
	return align_on - offset;
}

int
_application_size (
 enum DB2Type t)
{
	/*
	 *	Application datatype sizes
	 */
	switch (t)
	{
	case DB2_CHAR:
		return 1;

	case DB2_DATE:
		return sizeof (Date);			  /* epf: Date is long */ 
	
	case DB2_INTEGER:
		return sizeof (int);
	
	case DB2_DOUBLE:
		return sizeof (double);

	case DB2_REAL:
		return sizeof (float);

	case DB2_FLOAT:
		return sizeof (double);		 /* epf: float size of DB2_FLOAT is size of C double */
		
	case DB2_NUMERIC:
	case DB2_DECIMAL:
		return sizeof(double);

	default:
		printf ("userdatasize: unknown data-type %d", t);
	}

	return 0;
}

int
_internal_align (
 int orig,
 enum DB2Type t)
{
	/*
	 *	Aligns offsets for internal-data buffer
	 *	This *must* agree with the internal-datatypes used.
	 */
	int align_on;
	int offset;

	switch (t)
	{
	case DB2_CHAR:
		return 0;

	case DB2_DATE:
		align_on = offsetof (struct align_db2date, b);
		break;

	case DB2_NUMERIC:
	case DB2_DECIMAL:
		return 0;

	case DB2_INTEGER:
		align_on = offsetof (struct align_db2int, b);
		break;

	case DB2_REAL:
		align_on = offsetof (struct align_db2real, b);
		break;

	case DB2_FLOAT:
		align_on = offsetof (struct align_db2float, b);
		break;

	case DB2_DOUBLE:
		align_on = offsetof (struct align_db2double, b);
		break;

	default:
		printf("align: unknown data-type %d", t);
	}

	/*
	 */
	if (!(offset = orig % align_on))
		return 0;
	return align_on - offset;
}

int
_internal_size (
 enum DB2Type t)
{
	/*
	 *	Internal buffer datatype sizes
	 */
	switch (t)
	{
	case DB2_CHAR:
		return 1;

	case DB2_DATE:
		/*return sizeof (DB2_Date);*/ /* epf: returns 6 by RKA      */
		/*return DATE_SIZE;*/			  /* epf: lets changed it */
		/*return sizeof(Date);*/			  /* epf:  				  */
		/*return sizeof(DB2_Date);*/                  /* epf: doesn't seem to return 4, let's try native name type */
		return sizeof(DB2_Date);

	case DB2_INTEGER:
		return sizeof (long);

	case DB2_REAL:
		/*return sizeof (float);*/ /*epf: by RKA */
		return sizeof(float);	   /*epf: length of SQL_FLOAT & SQL_DOUBLE (8)*/

	case DB2_DOUBLE:
		return sizeof (double);

	case DB2_FLOAT:
		return sizeof (double);

	case DB2_DECIMAL:
	case DB2_NUMERIC:
		/*return sizeof(db2_decimal);*/
		return DEC_SIZE;

	default:
		printf ("userdatasize: unknown data-type %d", t);
	}

	return 0;
}

int
_internal_sqltype (
 enum DB2Type t)
{
	/*
	 *	Returns equivalent external C data-types for ORACLE
	 *	given internal database type
	 */
	switch (t)
	{
	case DB2_CHAR:
		return SQL_CHAR;

	case DB2_DATE:
		return SQL_TYPE_DATE;

	case DB2_NUMERIC:
	case DB2_DECIMAL:
		return SQL_CHAR;

	case DB2_INTEGER:
		return SQL_INTEGER;
	
	case DB2_REAL:
		return SQL_REAL;

	case DB2_FLOAT:
	case DB2_DOUBLE:
		return SQL_DOUBLE;

	default:
		printf ("Type conversion error");
	}
	return 0;
}

int
_external_sqltype (
 enum DB2Type t)
{
	/*
	 *	Returns equivalent external C data-types for DB2
	 *	given internal database type
	 */
	switch (t)
	{
	case DB2_CHAR:
		return SQL_CHAR;

	case DB2_DATE:
		return SQL_TYPE_DATE;

	case DB2_NUMERIC:
	case DB2_DECIMAL:
		return SQL_DOUBLE;

	case DB2_INTEGER:
		return SQL_INTEGER;
	
	case DB2_REAL:
		return SQL_REAL;

	case DB2_FLOAT:
	case DB2_DOUBLE:
		return SQL_DOUBLE;

	default:
		printf ("external sqltype: Type conversion error");
	}
	return 0;
}


void
_ConvApp2Raw (ColumnDef * col,
 int appoffset,
 void * raw_buffer, const char * dml)
{
	/*
	 *	Convert from Application user-buffer to Internal data-buffer
	 */
	int d, m, y;
	DB2_Date db2_date;
	char * appbuf = (char *) raw_buffer + appoffset;

	switch (col -> type)
	{
	case DB2_CHAR:
		/*
		 *	If we have a NULL string, we have to supply a string
		 *	with 1 space instead, otherwise DB2 treats this
		 *	as a NULL - with unexpected consequences
		 */
		if (appbuf [0])
			strncpy (col -> data, appbuf,col -> length - 1);
		else 
			strcpy (col -> data, " ");

		/*
		 * assign length to length/indicator buffers 
		 * length for char type is 1 less than.
		 */
		/*col->null_ind = col->length - 1;*/
		col->null_ind = strlen(col->data);

		break;

	case DB2_DATE:
		DateToDMY (*(Date *) appbuf, &d, &m, &y);
		/*
		*
		*	If we have a NULL date or a date with any of the 
		*	fields as zero, January 1, 1900 would be its value
		*	
		*/
		if (!d || !m || !y)
		{
		db2_date.day = 1;
		db2_date.month = 1;
		/*db2_date.year = 100 + 1900 % 1000;*/ /* no need to extract the year portion 1900 is already the year */
		db2_date.year = 1900;
		*(DB2_Date *) col -> data = db2_date;
		}
		else
		{
		db2_date.day = d;
		db2_date.month = m;
		/*db2_date.year = 100 + y % 1000;*/  /* no need to extract the year portion y is already the year */
		db2_date.year = y;
		*(DB2_Date *) col -> data = db2_date;
		}

		/*
		 * assign length to length/indicator buffers 
		 * length for date is same - 6
		 */
		col->null_ind = col->length;
		break;


	case DB2_INTEGER:
		/**(long *) col -> data = *(long *) appbuf; */ /*epf: by RKA */
		*(long int *) col -> data = *(long int *) appbuf;	   /*epf: long int for clarity */	

		/*
		 * assign length to length/indicator buffers 
		 * length for integer is same - 4
		 */
		col->null_ind = col->length;
		break;

	case DB2_NUMERIC:
	case DB2_DECIMAL:
		/**(double *) col -> data = *(double *) appbuf;*/	/*epf: by RKA */
		if (strcmp("u",dml) == 0)
			{
			sprintf(col -> data,"%.4f",*(double*)appbuf);	
			col->null_ind = strlen(col->data);
			}
		else
			{
			*(double *) col -> data = *(double *) appbuf;
			col->null_ind =6;
			}
		break;

	case DB2_FLOAT:
		/**(float *) col -> data = *(float *) appbuf;*/ /*epf: by RKA */
		*(double *) col -> data = *(double *) appbuf;   /*epf: float is treated as double */

		/*
		 * assign length to length/indicator buffers 
		 * length for float is same - 8
		 */
		col->null_ind = col->length;
		break;

	case DB2_REAL:
		*(float *) col -> data = *(float *) appbuf;

		/*
		 * assign length to length/indicator buffers 
		 * length for real is same - 4
		 */
		col->null_ind = col->length;
		break;

	case DB2_DOUBLE:
		*(double *) col -> data = *(double *) appbuf;
		/*
		 * assign length to length/indicator buffers 
		 * length for double is same - 8
		 */
		col->null_ind = col->length;
		break;

	default:
		printf ("_ConvApp2Raw failed on %s", col -> name);
	}
}

void
_ConvRaw2App (
 ColumnDef * col,
 int appoffset,
 void * raw_buffer)
{
	/*
	 *	Convert from Internal data-buffer to Application user-buffer
	 *	The NULL indicators need to be checked as well.
	 */
	DB2_Date db2_date;
	/*db2_decimal db2_decimal_var;*/
	
	int isnull = col -> null_ind == -1;
	char * appbuf = (char *) raw_buffer + appoffset;
	char *endptr;

	switch (col -> type)
	{
	case DB2_CHAR:
		if (isnull)
		{
			appbuf [0] = ' ';
		}
		else
		{
			strcpy(appbuf, col -> data);
		}
		break;

	case DB2_DATE:
		db2_date = *(DB2_Date *) col -> data;
		if (!db2_date.day || !db2_date.month || !db2_date.year)
		{
			db2_date.day = 1;
			db2_date.month = 1;
			db2_date.year = 1900;
		}
		*(Date *) appbuf =
			DMYToDate (
				db2_date.day,
				db2_date.month,
				db2_date.year);
		break;

	case DB2_NUMERIC:
	case DB2_DECIMAL:
		/* epf: convert to SQL_C_CHAR */
		/**(unsigned char *) appbuf = isnull ?  (unsigned char) "0.0" : (unsigned char) col -> data;*/
		if (isnull)                                         
		{
			strcpy(appbuf,"0.00");
			*(double *)appbuf = strtod(appbuf,&endptr);
		}
		else
		{
			/*db2_decimal_var = *(db2_decimal*) col -> data;*/
			strcpy(appbuf,col -> data);
			/*strcpy(appbuf,db2_decimal_var.val);*/
			*(double *)appbuf = strtod(appbuf,&endptr);
		}
		break;
	
	case DB2_INTEGER:
		/**(long int*) appbuf = isnull ? 0 : *(long int*) col -> data;*/
		*(int*) appbuf = isnull ? 0 : *(int*) col -> data;
		break;

	case DB2_REAL:
		*(float *) appbuf = isnull ? 0 : *(float *) col -> data;
		break;

	case DB2_FLOAT:
	case DB2_DOUBLE:
		/**(double *) appbuf = isnull ? 0.0 : *(double *) col -> data;*/
		/**(float *) appbuf = isnull ? (float)0.0 : *(float *) col -> data;*/
		if (isnull)                                         
		{
			*(double *)appbuf = 0;
		}
		else
		{
			*(double *)appbuf = *(double*)(col->data);
		}
		break;

	/*case DB2_FLOAT:
		*(float *) appbuf = isnull ? 0.0 : *(float *) col -> data;
		break;*/

	default:
		printf ("_ConvRaw2App failed on %s", col -> name);
	}
}

/*******SEARCH*********/
int
_CompRaw2App (
 ColumnDef * col,
 int appoffset,
 void * raw_buffer,
 int mode)
{
	/*
	 *	Convert from Internal data-buffer to Application user-buffer
	 *	The NULL indicators need to be checked as well.
	 */
	DB2_Date db2_date;
	Date date_val;
	double numeric_val;
	int int_val;
	float float_val;
	double double_val;

	/*int isnull = col -> null_ind == -1;*/
	char * appbuf = (char *) raw_buffer + appoffset;
	char *endptr;

	switch (col -> type)
	{
	case DB2_CHAR:
		switch (mode)
		{
		case GREATER:
			if (0 < strcmp(col -> data, appbuf))
				return TRUE;
			break;
		case GTEQ:
			if (0 <= strcmp(col -> data, appbuf))
				return TRUE;
			break;
		case LT:
			if (0 > strcmp(col -> data, appbuf))
				return TRUE;
			break;
		case LTEQ:
			if (0 >= strcmp(col -> data, appbuf))
				return TRUE;
			break;
		}
		return FALSE;
	case DB2_DATE:
		db2_date = *(DB2_Date *) col -> data;
		if (!db2_date.day || !db2_date.month || !db2_date.year)
		{
			db2_date.day = 1;
			db2_date.month = 1;
			db2_date.year = 1900;
		}
		date_val = DMYToDate ( db2_date.day, db2_date.month, db2_date.year);
		switch (mode)
		{
		case GREATER:
			if (*(Date*) appbuf < date_val)
				return TRUE;
			break;
		case GTEQ:
			if (*(Date*) appbuf <= date_val)
				return TRUE;
			break;
		case LT:
			if (*(Date*) appbuf > date_val)
				return TRUE;
			break;
		case LTEQ:
			if (*(Date*) appbuf >= date_val)
				return TRUE;
			break;
		}
		return FALSE;

	case DB2_NUMERIC:
	case DB2_DECIMAL:
		numeric_val = strtod(col -> data, &endptr);
		switch (mode)
		{
		case GREATER:
			if (*(double *)appbuf < numeric_val)
				return TRUE;
			break;
		case GTEQ:
			if (*(double *)appbuf <= numeric_val)
				return TRUE;
			break;
		case LT:
			if (*(double *)appbuf > numeric_val)
				return TRUE;
			break;
		case LTEQ:
			if (*(double *)appbuf >= numeric_val)
				return TRUE;
			break;
		}
		return FALSE;
		
	case DB2_INTEGER:
		int_val = *(int*) col->data;
		switch (mode)
		{
		case GREATER:
			if (*(int *)appbuf < int_val)
				return TRUE;
			break;
		case GTEQ:
			if (*(int *)appbuf <= int_val)
				return TRUE;
			break;
		case LT:
			if (*(int *)appbuf > int_val)
				return TRUE;
			break;
		case LTEQ:
			if (*(int *)appbuf >= int_val)
				return TRUE;
			break;
		}
		return FALSE;
		
	case DB2_REAL:
		float_val = *(float*) col->data;
		switch (mode)
		{
		case GREATER:
			if (*(float *)appbuf < float_val)
				return TRUE;
			break;
		case GTEQ:
			if (*(float *)appbuf <= float_val)
				return TRUE;
			break;
		case LT:
			if (*(float *)appbuf > float_val)
				return TRUE;
			break;
		case LTEQ:
			if (*(float *)appbuf >= float_val)
				return TRUE;
			break;
		}
		return FALSE;

	case DB2_FLOAT:
	case DB2_DOUBLE:
		double_val = *(double*) col->data;
		switch (mode)
		{
		case GREATER:
			if (*(double *)appbuf < double_val)
				return TRUE;
			break;
		case GTEQ:
			if (*(double *)appbuf <= double_val)
				return TRUE;
			break;
		case LT:
			if (*(double *)appbuf > double_val)
				return TRUE;
			break;
		case LTEQ:
			if (*(double *)appbuf >= double_val)
				return TRUE;
			break;
		}
		return FALSE;

	default:
		printf ("\n_CompRaw2App failed on %s\n", col -> name);
		return FALSE;
	}
}
/*******SEARCH*********/

void
 _DecodeDB2Type (
 const char * col_name,
 const char * data_type,
 int data_length,
 int data_precision,
 int data_scale,
 SQLINTEGER i_data_scale,
 enum DB2Type * type,				/* output buffer */
 int * length)						/* output buffer */
{
	/*
	 *	Determine type:
	 *		CHARARACTER
	 *		DATE
	 *		FLOAT
	 *		REAL
	 *		INTEGER
	 *		DECIMAL
	 */
	
	
	if (!strcmp ("CHARACTER", data_type))
	{
		*type = DB2_CHAR;
		*length = data_length + 1;

	} 
	else if (!strcmp ("DATE", data_type))
	{
		*type = DB2_DATE;

	}
	else if (!strcmp ("FLOAT", data_type))
	{
			*type = DB2_FLOAT;
		
	}
	else if (!strcmp ("INTEGER", data_type))
	{	
		*type = DB2_INTEGER;
		
	} 
	else if (!strcmp ("DECIMAL", data_type))		
	{
			if (data_length == NUMBER_DECIMAL_PREC &&
				data_scale == NUMBER_DECIMAL_SCALE)
				{
				*type = DB2_DECIMAL;
				}
	}
	else if (!strcmp ("REAL", data_type))
	{
		*type = DB2_REAL;
	}
	else
		printf ("Don't know type: %s", data_type);
	/*
	 *	All types aside from strings have their
	 *	have a fixed data-type size
	 */
 if (*type != DB2_CHAR)
		*length = _internal_size (*type);
}
