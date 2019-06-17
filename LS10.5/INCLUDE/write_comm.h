#ifndef	_write_comm_h
#define	_write_comm_h
/*	$Id: write_comm.h,v 5.1 2001/08/06 22:49:52 scott Exp $
 *
 *	Writing to MENUSYS/COMM workfile
 *
 *******************************************************************************
 *	$Log: write_comm.h,v $
 *	Revision 5.1  2001/08/06 22:49:52  scott
 *	RELEASE 5.0
 *	
 *	Revision 5.0  2001/06/19 06:51:55  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:59:36  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/12 13:29:04  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:15:53  gerry
 *	Force revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.4  1999/11/17 22:54:58  jonc
 *	Fixed crash on absense of MENUSYS/COMM
 *	
 */

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview com_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_cc_name"},
		{"comm_cc_short"},
		{"comm_env_name"},
		{"comm_dbt_date"},
		{"comm_crd_date"},
		{"comm_inv_date"},
		{"comm_gl_date"},
		{"comm_fiscal"}
	};

	int com_no_fields = 16;

	struct	com_type {
		int		termno;
		char	CONO[3];
		char	CONAME[41];
		char	COSHORT[16];
		char	BRNO[3];
		char	BRNAME[41];
		char	BRSHORT[16];
		char	WHNO[3];
		char	WHNAME[41];
		char	WHSHORT[10];
		char	ENV_NAME[61];
		long	dbt_date;
		long	crd_date;
		long	inv_date;
		long	gl_date;
		int	FIS;
	} com_rec;

void
WriteComm (int term_no)
{
	int errc, found;
	int	fd = OpenComm ();
	static	int	aliased;

	if (!aliased)
		abc_alias ("com","comm");

	aliased = 1;

	open_rec ("com",com_list,com_no_fields,"comm_term");

	memset (&com_rec, 0, sizeof (com_rec));
	while (!RF_READ (fd, (char *) &com_rec) && com_rec.termno != term_no);

	found = com_rec.termno == term_no;

	memset (&com_rec, 0, sizeof (com_rec));
	com_rec.termno = term_no;
	if ((errc = find_rec ("com", &com_rec, EQUAL, "u")))
		file_err (errc, "comm", "DBFIND");

	if (found)
		RF_UPDATE (fd, (char *) &com_rec);
	else
		RF_ADD (fd, (char *) &com_rec);

	RF_CLOSE (fd);
	abc_fclose ("com");
}

int
OpenComm (void)
{
	int		errc, fd;
	char	*sptr = getenv ("PROG_PATH");
	char	filename[101];

	sprintf (filename,"%s/BIN/MENUSYS/COMM", (sptr != (char *)0) ? sptr : "/usr/DB");

	if (access (filename,00) < 0)
	{
		if ((errc = RF_OPEN (filename, sizeof (struct com_type), "w", &fd)))
			file_err (errc, "wkcomm", "WKOPEN");

		if ((errc = RF_CLOSE (fd)))
			file_err (errc, "wkcomm", "WKCLOSE");
	}

	if ((errc = RF_OPEN (filename, sizeof (struct com_type), "u", &fd)))
		file_err (errc, "wkcomm", "WKOPEN");

	return (fd);
}

#endif	/* _write_comm_h */
