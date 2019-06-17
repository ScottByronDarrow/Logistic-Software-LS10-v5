/*
 *	Reading and writing the license file
 */
#include	<std_decs.h>

/*
 *	Trivial mask for license
 */
#define	LIC_MASK(off)	((0x4f + off) & 0xff)

/*
 *	Local functions
 */
static char	*lic_name (void);

/*
 *	External interface
 */
void
lc_read (
 struct LIC_REC	*license)
{
	int		lic_fd, lic_ptr;
	char	*lic_nam = lic_name ();

	/*	Scub license record
	 */
	memset (license, 0, sizeof (struct LIC_REC));

	/*	Read in license file - it doesn't matter about how much we've read in
	 */
	if ((lic_fd = open (lic_nam, O_RDONLY)) < 0)
		file_err (errno, PNAME, "open (LICENSE)");

	read (lic_fd, license, sizeof (struct LIC_REC));
	close (lic_fd);

	/*
	 * Remove trivial mask
 	 */
	for (lic_ptr = 0; lic_ptr < sizeof (struct LIC_REC); lic_ptr++)
		((char *) license) [lic_ptr] ^= LIC_MASK (lic_ptr);
}

void
lc_write (
 struct LIC_REC	*license)
{
	char	*lic_nam = lic_name ();
	int		lic_ptr, lic_fd;

	if ((lic_fd = open (lic_nam, O_WRONLY)) < 0)
		file_err (errno, PNAME, "open (LICENSE)");

	memset (&license -> tokens, 0, sizeof (license -> tokens));

	/*
	 *	Apply trivial mask
	 */
	for (lic_ptr = 0; lic_ptr < sizeof (struct LIC_REC); lic_ptr++)
		((char *) license) [lic_ptr] ^= LIC_MASK (lic_ptr);

	write (lic_fd, license, sizeof (struct LIC_REC));
	close (lic_fd);

	chmod (lic_nam, 0660);
}

/*
 *	Support functions
 */
static char *
lic_name (void)
{
	char	*sptr = getenv ("PROG_PATH");
	static char	filename [64];

	if (!sptr)
		sys_err ("getenv (PROG_PATH)", -1, PNAME);
	sprintf (filename, "%s/BIN/LICENSE", sptr);
	return (filename);
}
