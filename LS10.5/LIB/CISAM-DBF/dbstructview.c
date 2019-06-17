#ident	"$Id: dbstructview.c,v 5.1 2001/08/20 23:07:46 scott Exp $"
/*
 *	$Log: dbstructview.c,v $
 *	Revision 5.1  2001/08/20 23:07:46  scott
 *	Updated from scott's machine
 *	
 *	Revision 5.0  2001/06/19 07:07:43  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:53:53  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 2.0  2000/07/15 07:32:18  gerry
 *	Forced Revision No start to 2.0 - Rel-15072000
 *	
 *	Revision 1.7  1999/11/10 06:13:43  scott
 *	Updated to force error when fields are to long.
 *	
 *	Revision 1.6  1999/10/26 03:11:36  jonc
 *	Replaced some more magic numbers with manifest constants
 *	
 *	Revision 1.5  1999/09/30 04:57:28  jonc
 *	Tightened the argument to use const char * where applicable.
 *	
 *	Revision 1.4  1999/09/13 06:28:15  alvin
 *	Check-in all ANSI modifications made by Trev.
 *	
 *	Revision 1.3  1999/07/19 00:15:08  scott
 *	Updated to clip fields greater than 18 characters. Needs to be looked at in the future as supporting tools now clip to 18 characters so only effects old programs.
 *	
 *	Revision 1.2  1999/07/14 23:47:26  jonc
 *	Added generic access support.
 *	
 */
#include	"dbcisam.h"
#include	<std_decs.h>

/*	Function declarations
*/
static void	init_buf (LLIST *tptr, int offset, int sql_type, int len);

/*
 * Real Code
 */
int
dbstructview (
	const 	char * filename,
	struct 	dbview *fieldlist,
	int 	n)
{
	int	colcount;
	int	j = 0;
	int	i = 0;
	int	ioffset = 0;
	int	voffset = 0;
	int	sql_type;
	int	len = n * sizeof(int);
	char	fieldname[20];
	LLIST	*tptr;
	LLIST	*sptr;
	/*
	 * find node entry for table	
	 */
	if (!(tptr = _GetNode (filename)))
		return NOFILENAME;
	/*
	 * allocate pointers to isam buf
	 */
	tptr -> _start = malloc (len);
	if (!tptr -> _start)
		return MAXFIELDS;
	/*
	 * set view & no_fields in view	
	 */
	tptr -> _no_fields = n;
	tptr -> _view = fieldlist;
	/*
	 * pointer to syscolumns table	
	 */
	sptr = _GetSysNode (SYS_COL);
	/*
	 * set key for syscolumns table	
	 */
	stlong (tptr -> _tabid, sptr -> _buffer + sptr -> _key.k_part [0].kp_start);
	stint (0, sptr -> _buffer + sptr -> _key.k_part [1].kp_start);
	/*
	 * perform read on syscolumns	
	 */
	if (isread (sptr -> _fd, sptr -> _buffer, ISGTEQ))
		return (iserrno);
	/*
	 * Initialise the view		
	 */
	colcount = 0;
	while (ldlong (sptr -> _buffer + sptr -> _key.k_part [0].kp_start) ==
		tptr -> _tabid)
	{
		assert (colcount < tptr -> columncount);
		memset (tptr -> columnlist + colcount, 0, sizeof (struct ColumnInfo));

		/*
		 * copy colname into fieldname	
		 */
		ldchar (sptr -> _buffer, 18, fieldname);
		strcpy (tptr -> columnlist [colcount].name, fieldname);

		if (i < n && strlen (fieldlist[i].vwname) > NAME_LEN)
			return NOFIELD;
		
		/*
		 * get coltype			
		 */
		sql_type = ldint (sptr -> _buffer + SYSCOL_COLTYPE) & SQLTYPE;
		/*
		 * and get collength		
		 */
		len = ldint (sptr -> _buffer + SYSCOL_COLLENGTH);
		/*
		 * if using packed type format	
		 */
		if (sql_type == SQLDECIMAL || sql_type == SQLMONEY)
			len = DECLENGTH(len);

		switch (sql_type)
		{
		case SQLCHAR:
			tptr -> columnlist [colcount].type = CT_Chars;
			tptr -> columnlist [colcount].size = len;
			break;

		case SQLSMINT:
			tptr -> columnlist [colcount].type = CT_Short;
			break;

		case SQLINT:
			tptr -> columnlist [colcount].type = CT_Long;
			break;

		case SQLFLOAT:
			tptr -> columnlist [colcount].type = CT_Double;
			break;

		case SQLSMFLOAT:
			tptr -> columnlist [colcount].type = CT_Float;
			break;

		case SQLDECIMAL:
			tptr -> columnlist [colcount].type = CT_Number;
			tptr -> columnlist [colcount].size = len;
			break;

		case SQLSERIAL:
			tptr -> columnlist [colcount].type = CT_Serial;
			break;

		case SQLDATE:
			tptr -> columnlist [colcount].type = CT_Date;
			break;

		case SQLMONEY:
			tptr -> columnlist [colcount].type = CT_Money;
			tptr -> columnlist [colcount].size = len;
			break;
		}
		tptr -> columncisam [colcount] = voffset;
		colcount++;

		/*
		 * initialise isam init buffer	
		 */
		if (tptr -> _init)
			init_buf (tptr, voffset, sql_type, len);
		voffset += len;
		/*
		 * Offset for serial field	
		 */
		if (sql_type == SQLSERIAL)
			tptr -> _serial = ioffset;
		/*
		 * Field is in the required view	
		 */
		j = _check_view(fieldname,tptr);
		if (j != -1)
		{
			/*
			 * convert sqltype to isam type	
			 */
			fieldlist[j].vwtype = _isam_type(sql_type);
			/*
			 * conversion failed.
			 */
			if (fieldlist[j].vwtype == -1)
				return(-1);
			/*
			 * calculate size (native)	
			 */
			fieldlist[j].vwlen = _c_size(sql_type);
			if (fieldlist[j].vwlen < 0)
				fieldlist[j].vwlen = ldint (sptr -> _buffer + 26);
			/*
			 * offset in isam record.	
			 */
			tptr -> _start [j] = ioffset;
			i++;
		}
		/*
		 * offset in isam buffer
		 */
		ioffset += _col_len (sql_type, ldint (sptr -> _buffer + 26));
		/*
		 * read next column			
		 */
		if (isread (sptr -> _fd, sptr -> _buffer, ISNEXT))
			break;
	}

	assert (colcount == tptr -> columncount);

	/*
	 * could not load all columns.	
	 */
	if (i < n)
		return NOFIELD;
	/*
	 * set offsets in non isam buf.	
	 */
	if (_set_offsets(fieldlist,n) == -1)
		return NOVIEWSET;
	return(0);
}

static void	init_buf (LLIST *tptr, int offset, int sql_type, int len)
{
	switch (sql_type)
	{
	case	SQLCHAR:
		memset (tptr -> _init + offset, ' ', len);
		break;

	case	SQLSMINT:
		stint (0, tptr -> _init + offset);
		break;

	case	SQLINT:
	case	SQLSERIAL:
		stlong (0L, tptr -> _init + offset);
		break;

	case	SQLDATE:
		stlong (LONGNULL, tptr -> _init + offset);
		break;

	case	SQLFLOAT:
		stdbl (0.00, tptr -> _init + offset);
		break;

	case	SQLSMFLOAT:
		stfloat (0.00, tptr -> _init + offset);
		break;

	case	SQLDECIMAL:
	case	SQLMONEY:
	{
		dec_t	v;

		deccvint (0, &v);
		stdecimal (&v, tptr -> _init + offset,len);
		break;
	}
	}
}

int
_check_view(char *fieldname, LLIST *tptr)
{
	register	int	i;

	for (i = 0; i < tptr -> _no_fields; i++)
	{
		if (!strcmp (fieldname, tptr -> _view [i].vwname))
			return(i);
	}
	return(-1);
}
