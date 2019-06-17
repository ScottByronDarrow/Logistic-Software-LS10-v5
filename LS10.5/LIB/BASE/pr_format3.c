/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( pr_format.c    )                                 |
|  Program Desc  : ( Formatted print routines.                    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Date Written  : 10/04/87        | Author      : Roger Gibbison     |
|---------------------------------------------------------------------|
|  Date Modified : (10/04/87)      | Modified by : Roger Gibbison.    |
|  Date Modified : (20/08/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (24/08/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (05/04/94)      | Modified by : Campbell Mander.   |
|  Date Modified : (25.01.95)      | Modified by : Jonathan Chen      |
|  Date Modified : (02/07/99)      | Modified by : Trevor van Bremen  |
|                                                                     |
|  Comments      : Valid mask characters in input file are:           |
|                : ^AAAAA^         Character String - (%-4.4s)        |
|                : ^FFFF.FF^       Double/Float     - (%7.2f)         |
|                : ^DD/DD/DD^      Edate            - (%8.8s)         |
|                :    Note: This mask is tested for. ie ^DD/MM/YY^    |
|                :          is incorrect.                             |
|                :                                                    |
|                : ^III^           Integer          - (%3d)           |
|                : ^LLLLL^         Long Integer     - (%5ld)          |
|                : ^MMMMMM.MM^     Money            - (%9.2f)         |
|                :                                                    |
|                : The '^' characters are replaced by ' ' to retain   |
|                : the format of the document.                        |
|                :                                                    |
|                : label is a 10 character label at the beginning of  |
|                : each line of format.                               |
|                :                                                    |
|                : fields are numbered 1 - n                          |
|                :  a fld_no of 0 prints the line as it is read.      |
|                :                                                    |
|     (23/08/93) : HGP 9649 Fix to handle data argument for pr_format |
|                : and also moving code to library.                   |
|     (24/08/93) : Fix bad bug on label name                          |
|     (05/04/94) : HGP 10469. Removal of $ signs.                     |
|     (25.01.95) : Modified to access MENUPATH over PROG_PATH         |
|     (02/07/99) : Try to FORCE use of stdarg.h over vararg.h         |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

/*
| Use the preferred variable arg interface if available
*/
#ifdef	__STDC__
#include	<stdarg.h>
#else
#error Trevor van Bremen 02July1999 For prototyping reasons, I wanna AVOID this
#include	<varargs.h>
#endif	/*__STDC__*/

/*
| char position in .p file at which line info begins (count from 0)
*/
#define	DATA_OFFSET	10

/*
| structures and other support types
*/
struct list_rec
{
	char	label [DATA_OFFSET + 1];
	char	*format;
	struct list_rec	*next;
};

enum FieldType
{
	PR_Bad,
	PR_Alpha,
	PR_Int,
	PR_Float,
	PR_Date,
	PR_Long,
	PR_Money,
	PR_LongZero,
	PR_IntZero
};

struct tagPR_Arg
{
	enum FieldType	fldType;
	int		len, dec;

	char	*arg_char;
	int		arg_int;
	long	arg_long;
	double	arg_double;
};
typedef	struct tagPR_Arg	PR_Arg;

static	struct list_rec	*head_ptr = NULL;

/*==============
| Local function declarations
================*/

static void				load_format (char *);
static struct list_rec	*list_alloc (char *);
static void				PrintField (FILE *, PR_Arg	*);
static char				*DecodeField (FILE *, PR_Arg *, char *);

FILE *
pr_open (
 char	*filename)
{
	/*
	 *	Look in the following directories for the print file
	 *		1. . (development copy)
	 *		2. ./PR_FILE (local copy)
	 *		3. PSL_MENU_PATH/PR_FILE (user specific)
	 *		4. PROG_PATH/BIN/PR_FILE (std copy)
	 */
	char	cwd [80],
			pathname [80];
	char	*env,
			*subdir = "PR_FILE",
			*mode = "r";
	char	*ProgPath = "PROG_PATH",
			*MenuPath = "PSL_MENU_PATH";

	/*
	 *	Check for local copy
	 */
	if (!access (filename, F_OK))
		return (fopen (filename, mode));

	/*
	 *	Use ./PR_FILE if currently not in PROG_PATH/BIN
	 */
	sprintf (pathname, "%s/BIN", getenv (ProgPath));
	if (strcmp (getcwd (cwd, sizeof (cwd)), pathname))
	{
		sprintf (pathname, "%s/%s", subdir, filename);
		if (!access (pathname, F_OK))
			return (fopen (pathname, mode));
	}

	/*
	 *	Check user-specific copy
	 */
	if ((env = getenv (MenuPath)))
	{
		sprintf (pathname, "%s/%s/%s", env, subdir, filename);
		if (!access (pathname, F_OK))
			return (fopen (pathname, mode));
	}

	/*
	 *	Std copy?
	 */
	if ((env = getenv (ProgPath)))
	{
		sprintf (pathname, "%s/BIN/%s/%s", env, subdir, filename);
		if (!access (pathname, F_OK))
			return (fopen (pathname, mode));
	}
	return (NULL);		/* abject failure */
}

#ifdef	__STDC__
int
pr_format (
 FILE	*fin,
 FILE	*fout,
 char	*label,
 int	fld_no,
 ...)							/* using <stdarg.h> */
{
#else
pr_format (va_alist) va_dcl		/* using <varargs.h> */
{
	FILE	*fin,
			*fout;
	char	*label;
	int		fld_no;
#endif	/*__STDC__*/

	int		i;
	int		rc = 0;
	int		screen_print;
	char	line [300];
	char	*sptr;
	char	*cptr;
	char	*dptr;
	struct	list_rec	*temp_ptr;

	va_list	ap;
	PR_Arg	argument;

#ifdef	__STDC__
	/*
	| Argument setup for <stdarg.h> interface
	*/
	va_start (ap, fld_no);
#else
	/*
	| Argument setup for <varargs.h>
	| Need to set up arguments individually
	*/
	va_start (ap);

	fin		= va_arg (ap, FILE *);
	fout	= va_arg (ap, FILE *);
	label	= va_arg (ap, char *);
	fld_no	= va_arg (ap, int);
#endif	/*__STDC__*/

	if (!head_ptr)
	{
		/*---------------------------------------
		| read through file to line [0-9] = label|
		---------------------------------------*/
		while ((sptr = fgets (line, sizeof (line), fin)))
			load_format (sptr);
	}

	screen_print = (fileno (stdout) == fileno (fout));

	/*
	| search for label node
	*/
	for (temp_ptr = head_ptr; temp_ptr; temp_ptr = temp_ptr -> next)
		if (!strcmp (label, temp_ptr -> label))
			break;

	if (!temp_ptr)
	{
		fprintf (fout, "Unexpected end of input : label %s not found\n", label);
		fflush (fout);
		return (EXIT_FAILURE);		/*	Bad return */
	}

	sptr = strcpy (line, temp_ptr -> format);

	/*---------------
	| no parameter	|
	---------------*/
	if (fld_no == 0)
	{
		if (screen_print)
		{
			/*---------------
			| dot command	|
			---------------*/
			if (*sptr == '.')
			{
				switch (* (sptr + 1))
				{
				case	'B':
					for (i = 1;i < atoi (sptr + 2);i++)
					{
						rc = check_page ();
						if (rc)
							return (rc);
						fprintf (fout, "\n\r");
					}
					break;

				case	'E':
					rc = check_page ();
					if (rc)
						return (rc);
					fprintf (fout, "%s\r", sptr + 2);
					break;

				default:
					rc = check_page ();
					if (rc)
						return (rc);
					fprintf (fout, "%s\r", sptr);
					break;
				}
			}
			else
			{
				rc = check_page ();
				if (rc)
					return (rc);
				fprintf (fout, "%s\r", sptr);
			}
			fflush (fout);
		}
		else
		{
			rc = check_page ();
			if (rc)
				return (rc);
			fprintf (fout, "%s", sptr);
			fflush (fout);
		}
		return (EXIT_SUCCESS);
	}

	/*-------------------
	| find first mask	|
	--------------------*/
	if (!(cptr = strchr (sptr, '^')))
	{
		fprintf (fout, "No mask in format line (%s)\n", label);
		return (EXIT_FAILURE);
	}
	dptr = cptr + 1;

	/*-----------------------
	| print 1st bit of line	|
	-----------------------*/
	if (fld_no == 1)
	{
		*cptr = '\0';

		rc = check_page ();
		if (rc)
			return (rc);

		fprintf (fout, screen_print ? "%s\r" : "%s", sptr);

		fflush (fout);
		*cptr = '^';
	}

	/*
	| Verify existence of field
	*/
	for (i = 1; i < fld_no; i++)
	{
		/*-----------------------
		| find start of mask	|
		-----------------------*/
		if (!(cptr = strchr (dptr, '^')))
		{
			fprintf (fout, " No such field %d label %s \n", fld_no, label);
			fflush (fout);
			return (EXIT_FAILURE);
		}
		dptr = cptr + 1;

		/*-------------------
		| find end of mask	|
		--------------------*/
		if (!(cptr = strchr (dptr, '^')))
		{
			fprintf (fout, " No such field %d label %s \n", fld_no, label);
			fflush (fout);
			return (EXIT_FAILURE);
		}
		dptr = cptr + 1;
	}

	/*
	| Decide on type of argument being put thru
	*/
	if (!(dptr = DecodeField (fout, &argument, dptr)))
		return (EXIT_FAILURE);

	switch (argument.fldType)
	{
	case PR_Alpha		:
		argument.arg_char	= va_arg (ap, char *);
		break;
	case PR_Int			:
	case PR_IntZero		:
		argument.arg_int	= va_arg (ap, int);
		break;
	case PR_Float		:
	case PR_Money		:
		argument.arg_double	= va_arg (ap, double);
		break;
	case PR_Date		:
	case PR_Long		:
	case PR_LongZero	:
		argument.arg_long	= va_arg (ap, long);
		break;

	default	:
		return (EXIT_FAILURE);
	}
	va_end (ap);
	PrintField (fout, &argument);

	/*-------------------------------
	| find start of next mask	|
	-------------------------------*/
	if ((cptr = strchr (dptr, '^')))
	{
		/*----------------------
		| fld_no not last field on line
		| - print to next mask
		-----------------------*/
		*cptr = '\0';
		fprintf (fout, "%s", dptr);
	}
	else
	{
		/*-----------------------
		| print to end of line	|
		-----------------------*/
		fprintf (fout, screen_print ? "%s\r" : "%s", dptr);
	}
	fflush (fout);
	return (EXIT_SUCCESS);
}

/*===============================================
|	procedure to print data for field			|
|	parameters:									|
|		fout:	output file						|
|		line:	the line containing the mask	|
|		arg:	the data to be printed			|
=================================================*/
static void
PrintField (
 FILE	*fout,
 PR_Arg	*arg)
{
	switch (arg -> fldType)
	{
	/*-----------
	| Character	|
	-------------*/
	case PR_Alpha	:
		fprintf (fout, "%-*.*s", arg -> len, arg -> len, arg -> arg_char);
		break;

	/*---------------
	| Zero Padded	|
	---------------*/
	case PR_LongZero	:
		fprintf (fout, "%0*ld", arg -> len, arg -> arg_long);
		break;
	case PR_IntZero		:
		fprintf (fout, "%0*d", arg -> len, arg -> arg_int);
		break;

	/*-----------
	| Integer	|
	-------------*/
	case PR_Int	:
		fprintf (fout, "%*d", arg -> len, arg -> arg_int);
		break;

	/*-----------
	| Money		|
	------------*/
	case PR_Money	:
		fprintf (fout, "%*.*f",
			arg -> len, arg -> dec,
			arg -> arg_double / 100);
		break;

	/*-------------------------------
	| Floating Point (Float/Double)	|
	-------------------------------*/
	case PR_Float	:
		fprintf (fout, "%*.*f", arg -> len, arg -> dec, arg -> arg_double);
		break;

	/*-----------
	| Date		|
	-------------*/
	case PR_Date	:
		fprintf (fout, "%10.10s", DateToString (arg -> arg_long));
		break;

	/*-----------
	| Long		|
	------------*/
	case PR_Long	:
		fprintf (fout, "%*ld", arg -> len, arg -> arg_long);
		break;

	default:
		fprintf (fout, "Internal Error : pr_format3\n");
	}
}

/*
| Given a line, determine field type, len, and possible decimal points
*/
static char *
DecodeField (
 FILE	*fout,
 PR_Arg	*arg,
 char	*line)
{
	char	*sptr, *dptr;

	memset (arg, 0, sizeof (PR_Arg));		/* Flush */

	/*
	| Find field terminators
	*/
	for (sptr = line; *sptr && *sptr != ' ' && *sptr != '^'; sptr++);
	if (*sptr != '^')
	{
		fprintf (fout, "No mask terminator\n");
		return (NULL);
	}
	*sptr = '\0';

	/*---------------
	| field length	|
	---------------*/
	arg -> len = strlen (line);
	switch (*line)
	{
	/*-----------
	| Character	|
	-------------*/
	case 'A':
		arg -> fldType = PR_Alpha;
		break;

	/*---------------
	| Zero Padded	|
	---------------*/
	case 'Z':
		arg -> fldType = *(line + 1) == 'L' ? PR_LongZero : PR_IntZero;
		break;

	/*-----------
	| Integer	|
	-------------*/
	case 'I':
		arg -> fldType = PR_Int;
		break;

	/*-----------
	| Money		|
	------------*/
	case 'M':
		arg -> fldType = PR_Money;
		arg -> dec = (dptr = strchr (line, '.')) ? strlen (dptr + 1) : 0;
		break;

	/*-------------------------------
	| Floating Point (Float/Double)	|
	-------------------------------*/
	case 'F':
		arg -> fldType = PR_Float;
		arg -> dec = (dptr = strchr (line, '.')) ? strlen (dptr + 1) : 0;
		break;

	/*-----------
	| Date		|
	-------------*/
	case 'D':
		if ((arg -> len != 8 && arg-> len != 10) || (strncmp (line, "DD/DD/DD", arg -> len) && strncmp (line, "DD/DD/DDDD", arg -> len)))
		{
			arg -> fldType = PR_Bad;
			fprintf (fout, "Invalid Date Mask [%10.10s] \n", line);
		}
		else
			arg -> fldType = PR_Date;
		break;

	/*-----------
	| Long		|
	------------*/
	case 'L':
		arg -> fldType = PR_Long;
		break;

	default:
		arg -> fldType = PR_Bad;
		fprintf (fout, "Invalid Mask characters\n");
		return (NULL);
	}
	return (sptr + 1);
}

static void
load_format (
 char	*line)
{
	struct	list_rec	*temp_ptr = list_alloc (line);

	if (head_ptr)
		temp_ptr -> next = head_ptr;
	head_ptr = temp_ptr;
}

static struct list_rec *
list_alloc (
 char	*line)
{
	int	i;
	struct list_rec	*rec = malloc (sizeof (struct list_rec));

	if (!rec)
		sys_err ("Out of Memory", 116, "pr_format3");

	memset (rec, 0, sizeof (struct list_rec));
	for (i = 0; i < DATA_OFFSET && line [i] != ' '; i++)
		rec -> label [i] = line [i];
	rec -> format = malloc (strlen (line + DATA_OFFSET) + 1);
	strcpy (rec -> format, line + DATA_OFFSET);

	return (rec);
}

/*
_print_format (fout)
FILE	*fout;
{
	struct	list_rec	*temp_ptr = head_ptr;
	int	screen_print = (fileno (stdout) == fileno (fout));

	fprintf (fout, "*********** format list ***********%s",
		 (screen_print) ? "\n\r" : "\n");

	if (temp_ptr == (struct list_rec *) NULL)
		fprintf (fout, " List is Empty%s",
			 (screen_print) ? "\n\r" : "\n");

	while (temp_ptr != (struct list_rec *) NULL)
	{
		fprintf (fout, "[%s][%s]%s",
			temp_ptr -> label,
			temp_ptr -> format,
			(screen_print) ? "\n\r" : "\n");
		temp_ptr = temp_ptr -> next;
	}
}
*/
