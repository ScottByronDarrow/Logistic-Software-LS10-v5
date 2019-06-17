#ident	"$Id: WorkFile.C,v 5.0 2001/06/19 08:16:40 cha Exp $"
/*
 *	Logistic WorkFile (ugh).
 *
 *	Logistic WorkFiles are fixed length binary data-files which
 *	contain the length of the record-size in the first sizeof(long)
 *	bytes of the file, stored as a machine-dependant binary long.
 *
 *************************************************************
 *	$Log: WorkFile.C,v $
 *	Revision 5.0  2001/06/19 08:16:40  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 01:02:24  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 04:59:19  scott
 *	*** empty log message ***
 *	
 *	Revision 3.0  2000/10/12 13:40:11  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 1.1  1999/11/30 21:23:11  nz
 *		Copy three programs from ver10 LIB++
 *	
 *	Revision 1.1.1.1  1998/01/22 00:58:44  jonc
 *	Version 10 start
 *	
 *	Revision 2.4  1996/07/30 01:32:38  jonc
 *	Added #ident directive
 *
 *	Revision 2.3  1996/03/21 21:49:45  jonc
 *	Stopped core dump on empty pathname
 *
 *	Revision 2.2  1996/03/18 04:55:32  jonc
 *	Typo fixed.
 *
 *	Revision 2.1  1996/03/11 21:32:04  jonc
 *	Initial revision
 *
 */
#include	<assert.h>
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<stddef.h>
#include	<string.h>
#include	<unistd.h>

#include	<osdeps.h>
#include	<liberr.h>

#include	<WorkFile.h>

/*
 *	Magic stuff
 */
#define	FILELEADER_SZ	sizeof (long)

/*
 *	Local functions
 */
static int	OpenWorkFile (const char *, long);

/*
 *	Public interface
 */
WorkFile::WorkFile (
 const char *	pathname,
 long			recordsize,
 bool			create_it) :
	fname (NULL),
	recsz (recordsize),
	fdesc (-1)
{
	/*
	 *	Sanity checks
	 */
	assert (recsz > 0);
	if (!pathname)
		(*app_error_handler) ("WorkFile::WorkFile", "Empty workfile name");

	fname = strcpy (new char [strlen (pathname) + 1], pathname);

	if (Exists ())
		fdesc = OpenWorkFile (fname, recsz);
	else if (create_it)
		Create ();
}

WorkFile::~WorkFile ()
{
	if (fdesc >= 0)
		close (fdesc);
	delete [] fname;
}

bool
WorkFile::Exists () const
{
	struct stat	info;

	if (stat (fname, &info) || !S_ISREG (info.st_mode))
		return (false);
	return (true);
}

int
WorkFile::Count ()
{
	if (fdesc < 0)
	{
		if (Exists ())
			fdesc = OpenWorkFile (fname, recsz);
		else
			(*app_error_handler) ("WorkFile::Count",
				"Accessing non-existent file \"%s\"", fname);
	}

	assert (fdesc >= 0);

	/*
	 *	Find the number of records by looking at the filesize
	 *	and then dividing it by the record-size (allowing for the
	 *	offset, of course)
	 */
	struct stat	info;

	if (stat (fname, &info) || !S_ISREG (info.st_mode))
		(*lib_error_handler) ("WorkFile::Count",
			"WorkFile (%s) has disappeared", fname);

	return ((info.st_size - FILELEADER_SZ) / recsz);
}

bool
WorkFile::Create ()
{
	if (fdesc >= 0)
		return (true);

	if ((fdesc = open (fname, O_RDWR | O_TRUNC | O_CREAT, 0666)) < 0)
		return (false);

	fcntl (fdesc, F_SETFD, 1);		// set close-on-exec

	/*
	 *	Write the record size
	 */
	if (write (fdesc, &recsz, sizeof (long)) != sizeof (long))
		(*lib_error_handler) ("WorkFile::Create",
			"Initial write (%s) failed with %d", fname, errno);

	return (true);
}

bool
WorkFile::Delete ()
{
	if (fdesc >= 0)
	{
		close (fdesc);
		fdesc = -1;
	}
	return (unlink (fname) == 0 ? true : false);
}

bool
WorkFile::First ()
{
	if (fdesc < 0)
		(*app_error_handler) ("WorkFile::First",
			"Non-existent WorkFile (%s)", fname);

	return (lseek (fdesc, FILELEADER_SZ, SEEK_SET) < 0 ? false : true);
}

bool
WorkFile::Last ()
{
	if (fdesc < 0)
		(*app_error_handler) ("WorkFile::Last",
			"Non-existent WorkFile (%s)", fname);

	return (lseek (fdesc, 0, SEEK_END) < 0 ? false : true);
}

bool
WorkFile::Next (
 void *	buffer)
{
	if (fdesc < 0)
		(*app_error_handler) ("WorkFile::Next",
			"Non-existent WorkFile (%s)", fname);

	return (read (fdesc, buffer, (unsigned) recsz) == recsz ? true : false);
}

bool
WorkFile::Prev (
 void *	buffer)
{
	if (fdesc < 0)
		(*app_error_handler) ("WorkFile::Prev",
			"Non-existent WorkFile (%s)", fname);

	/*
	 *	We've to check whether we back past the beginning of
	 *	the file
	 */
	off_t	mark = lseek (fdesc, 0, SEEK_CUR);

	if (mark <= FILELEADER_SZ)
		return (false);

	assert (mark >= recsz);				// weirdo file otherwise

	if (lseek (fdesc, mark - recsz, SEEK_SET) < 0)
		(*lib_error_handler) ("WorkFile::Prev",
			"lseek (%s) fails with %d", fname, errno);

	if (read (fdesc, buffer, (unsigned) recsz) != recsz)
		(*lib_error_handler) ("WorkFile::Prev",
			"Assured read (%s) fails with %d", fname, errno);

	/*
	 *	Position back one record again
	 */
	if (lseek (fdesc, mark - recsz, SEEK_SET) < 0)
		(*lib_error_handler) ("WorkFile::Prev",
			"Assured lseek (%s) fails with %d", fname, errno);

	return (true);
}

bool
WorkFile::Get (
 unsigned	recno,
 void *		buffer)
{
	if (fdesc < 0)
		(*app_error_handler) ("WorkFile::Get",
			"Non-existent WorkFile (%s)", fname);

	if (lseek (fdesc, FILELEADER_SZ + recno * recsz, SEEK_SET) < 0)
		return (false);
	return (read (fdesc, buffer, (unsigned) recsz) == recsz ? true : false);
}

bool
WorkFile::Write (
 void *	buffer)
{
	if (fdesc < 0)
		(*app_error_handler) ("WorkFile::Write",
			"Non-existent WorkFile (%s)", fname);

	return (write (fdesc, buffer, (unsigned) recsz) == recsz ? true : false);
}

bool
WorkFile::Put (
 unsigned	recno,
 void *		buffer)
{
	if (fdesc < 0)
		(*app_error_handler) ("WorkFile::Put",
			"Non-existent WorkFile (%s)", fname);

	if (lseek (fdesc, FILELEADER_SZ + recno * recsz, SEEK_SET) < 0)
		return (false);
	return (write (fdesc, buffer, (unsigned) recsz) == recsz ? true : false);
}

/*
 *	Local functions
 */
static int
OpenWorkFile (
 const char *	pathname,
 long			recsize)
{
	int	fd = open (pathname, O_RDWR);

	if (fd < 0)
		(*lib_error_handler) ("WorkFile::WorkFile",
			"open (%s) failed with %d", pathname, errno);

	fcntl (fd, F_SETFD, 1);		// set close-on-exec

	/*
	 *	Verify the record size
	 */
	long	frecsize;

	if (read (fd, &frecsize, sizeof (long)) != sizeof (long) ||
		frecsize != recsize)
	{
		(*app_error_handler) ("WorkFile::WorkFile",
			"File \"%s\" is not a WorkFile", pathname);
	}

	return (fd);
}
