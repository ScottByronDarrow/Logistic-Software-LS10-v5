#ident	"$Id: dbutils.c,v 5.2 2001/08/20 23:07:46 scott Exp $"
/*
 *
 *
 *******************************************************************************
 *	$Log: dbutils.c,v $
 *	Revision 5.2  2001/08/20 23:07:46  scott
 *	Updated from scott's machine
 *	
 *	Revision 5.1  2001/08/06 22:47:56  scott
 *	RELEASE 5.0
 *	
 *	Revision 5.0  2001/06/19 07:07:43  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:53:53  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 2.1  2000/08/16 02:14:09  scott
 *	Updated for small warnings
 *	
 *	Revision 2.0  2000/07/15 07:32:18  gerry
 *	Forced Revision No start to 2.0 - Rel-15072000
 *	
 *	Revision 1.3  1999/09/30 04:57:29  jonc
 *	Tightened the argument to use const char * where applicable.
 *	
 */
#include	"dbcisam.h"
#include	<aud_lib.h>
#include	<std_decs.h>
#ifdef	LINUX
#include	<linux/stddef.h>
#else	// LINUX
#include	<stddef.h>
#endif	// LINUX

/*
 * Globals
 */
char	*_dbpath = NULL;

/*	Conversion structure
*/
static struct
{
	int	_sqltype;
	int	_isamtype;
	int	_ctype;
	int	_csize;
} _type_conv [] =
{
	{ SQLCHAR,    CHARTYPE,   CHARTYPE,   -1	     },	/* SQLCHAR    */
	{ SQLSMINT,   INTTYPE,    INTTYPE,    sizeof(int)    },	/* SQLSMINT   */
	{ SQLINT,     LONGTYPE,   LONGTYPE,   sizeof(long)   },	/* SQLINT     */
	{ SQLFLOAT,   DOUBLETYPE, DOUBLETYPE, sizeof(double) },	/* SQLFLOAT   */
	{ SQLSMFLOAT, FLOATTYPE,  FLOATTYPE,  sizeof(float)  },	/* SQLSMFLOAT */
	{ SQLDECIMAL, MONEYTYPE,  CHARTYPE,   -1	     },	/* SQLDECIMAL */
	{ SQLSERIAL,  SERIALTYPE, LONGTYPE,   sizeof(long)   },	/* SQLSERIAL  */
	{ SQLDATE,    DATETYPE,   LONGTYPE,   sizeof(long)   },	/* SQLDATE    */
	{ SQLMONEY,   MONEYTYPE,  CHARTYPE,   -1	     },	/* SQLMONEY   */
	{ -1,	      -1,	  -1,	      -1	     }
};

/*
 * Real Code
 */
char *_get_dbpath (
 const char *dbname)
{
	char	*base_path	= NULL;
	int		found = 0;
	char	*dptr;
	char	*sptr;
	char	*tptr;
	char	buffer[201];
	char	read_buf[81];
	struct	stat	buf;
	FILE	*dbase;
	static int	called = FALSE;

	/*
	 * Already Called		
	 */
	iserrno = 0;
	if (called)
		free(_dbpath);
	/*
	 * load DBPATH variable	
	 */
	dptr = strdup (getenv ("DBPATH"));
	sptr = dptr;
	/*
	 * check current directory first	
	 */
	if (*sptr == ':')
	{
		sprintf(buffer,"./%s.dbs",dbname);
		if (!access(buffer,07))
		{
			base_path = strdup (".");
			found = 1;
		}
		else
		{
			if (errno == 13)
				iserrno = 6045;
			sptr++;
		}
	}
	/*
	 * check all other directories in DBPATH
	 */
	while (!found && strlen(sptr))
	{
		/*
		 * position of separator in DBPATH
		 */
		tptr = strchr (sptr,':');
		if (tptr != (char *)0)
		{
			*tptr = '\0';
			tptr++;
		}
		/*
		 * check if system file exists	
		 */
		sprintf(buffer,"%s/%s.dbs",sptr,dbname);
		if (!access(buffer,07))
		{
			found = 1;
			base_path = strdup (sptr);
			break;
		}
		else
		{
			if (errno == 13)
				iserrno = 6045;
		}
		if (tptr != (char *)0)
			sptr = tptr;
		else
			break;
	}
	free(dptr);
	/*
	 * check for redirection		
	 */
	if (found)
	{
		sprintf(buffer,"%s/%s.dbs",base_path,dbname);
		if (stat(buffer,&buf))
			return((char *)0);
		/*
		 * if filename			
		 */
		if (buf.st_mode & S_IFREG)
		{
			/*
			 * open file			
			 */
			if ((dbase = fopen(buffer,"r")) == 0)
				return((char *)0);
			/*
			 * read first line in file
			 */
			sptr = fgets(read_buf,80,dbase);
			if (sptr == (char *)0 || strlen(sptr) <= 1)
			{
				fclose(dbase);
				return((char *)0);
			}
			fclose(dbase);
			/*
			 * get "real" database name	
			 */
			*(sptr + strlen(sptr) - 1) = '\0';
			if (*sptr == '/')
				strcpy(buffer,sptr);
			else
				sprintf(buffer,"%s/%s",base_path,sptr);
		}
		free(base_path);
		base_path = strdup (buffer);
	}
	else
		return((char *)0);
	/*
	 * return path name of database	
	 */
	called = TRUE;
	return(base_path);
}

int
_isam_type(int sql_type)
{
	int	i;

	for (i = 0;_type_conv[i]._sqltype != -1;i++)
		if (_type_conv[i]._sqltype == sql_type)
			return(_type_conv[i]._isamtype);
	return(-1);
}

/*
 * Get the 'C' type from the SQL type.
 */
int
_c_type(int sql_type)
{
	int	i;

	for (i = 0;_type_conv[i]._sqltype != -1;i++)
		if (_type_conv[i]._sqltype == sql_type)
			return(_type_conv[i]._ctype);
	return(-1);
}

int
_c_size(int sql_type)
{
	int	i;

	for (i = 0;_type_conv[i]._sqltype != -1;i++)
		if (_type_conv[i]._sqltype == sql_type)
			return(_type_conv[i]._csize);
	return(-1);
}

int
_col_len(int t, int l)
{
	return (t == SQLMONEY || t == SQLDECIMAL) ? DECLENGTH(l) : l;
}

int
_set_offsets(struct dbview *fieldlist, int n)
{
	int	i = 0;
	int	voffset = 0;

	for (i = 0;i < n;i++)
	{
		/*
		 * calc. word length of implementation
		 */
		voffset += _align (voffset, fieldlist [i].vwtype);

		/*
		 * Offset in recbuf		
		 */
		fieldlist[i].vwstart = voffset;
		/*
		 * Calculate the offset into the record
		 */
		switch (fieldlist[i].vwtype)
		{
		case	CHARTYPE:
			voffset += fieldlist[i].vwlen + 1;
			break;

		case	INTTYPE:
			voffset += sizeof(int);
			break;

		case	LONGTYPE:
		case	DATETYPE:
		case	SERIALTYPE:
			voffset += sizeof(long);
			break;

		case	FLOATTYPE:
			voffset += sizeof(float);
			break;

		case	DOUBLETYPE:
		case	MONEYTYPE:
			voffset += sizeof(double);
			break;

		default:
			return(-1);
		}
	}

	return(0);
}

/*
 *	Word alignment stuff
 */
struct	align_int		{	char a;		int		b;	};
struct	align_long		{	char a;		long	b;	};
struct	align_float		{	char a;		float	b;	};
struct	align_double	{	char a;		double	b;	};

int
_align (int x, int t)
{
	/*	Word alignment function
	 */
	int		align_on	=	0;

	switch (t)
	{
	case	CHARTYPE:
		return (EXIT_SUCCESS);

	case	INTTYPE:
		align_on = offsetof (struct align_int, b);
		break;

	case	LONGTYPE:
	case	SERIALTYPE:
	case	DATETYPE:
	case	EDATETYPE:
	case	YDATETYPE:
		align_on = offsetof (struct align_long, b);
		break;

	case	FLOATTYPE:
		align_on = offsetof (struct align_float, b);
		break;

	case	DOUBLETYPE:
	case	MONEYTYPE:
		align_on = offsetof (struct align_double, b);
		break;
	}

	if (!(x % align_on))
		return (EXIT_SUCCESS);
	return (align_on - (x % align_on));
}
