/*=======================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
| Program Name : ( account.c   )                                        |
| Program Desc : ( Handling of menu accounting bits & pieces      )     |
|                (                                                )     |
|-----------------------------------------------------------------------|
| Authors      : Unknown                                                |
| Date Written : ??/??/??                                               |
|-----------------------------------------------------------------------|
| Date Modified : (02/07/1999) Modified by : Trevor van Bremen          |
| Date Modified : (31/08/1999) Modified by : Alvin Misalucha            |
|                                                                       |
| Comments      :                                                       |
| (02/07/99)    : Fully restructured.  Moved code fragments to library  |
| (31/08/1999)  : Added account.h include.                              |
|                                                                       |
| $Log: account.c,v $
| Revision 5.1  2001/08/06 22:40:54  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 06:59:14  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:52:35  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/12 13:34:18  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/11 23:48:37  scott
| Updated to fix term_slot problem with Linux.
|
| Revision 2.0  2000/07/15 07:17:12  gerry
| Forced revision no. to 2.0 - Rel-15072000
|
| Revision 1.3  1999/09/29 00:02:04  jonc
| Corrected "long" vs "time_t" usage; very apparent on AIX and Alpha.
|
| Revision 1.2  1999/09/13 09:36:28  scott
| Updated for Copyright
|
=======================================================================*/
#include	<std_decs.h>
#include	<account.h>

struct	acc_type	acc_rec;

int
UserAccountOpen (char *fname, char *mode)
{
	int	errc;
	int	fd;
	char	*sptr = getenv("PROG_PATH");
	char	*user = getenv("LOGNAME");
	char	account[15];
	char	filename[100];

	sprintf(filename,"%s/BIN/ACCOUNT",(sptr != (char *)0) ? sptr : "/usr/LS10.5");

	if (access(filename,00) == -1)
		return(-1);

	if (fname == (char *)0)
#if defined(LINUX)
    {
	    char	*slot;

	    slot = getenv("TERM_SLOT");
		sprintf(account,"%s.%03d",user,atoi(slot));
	}
#else /* LINUX */
		sprintf(account,"%s.%03d",user,ttyslt());
#endif /* LINUX */

	else
		sprintf(account,"%-.14s",fname);

	sprintf(filename,"%s/BIN/ACCOUNT/%s",(sptr != (char *)0) ? sptr : "/usr/LS10.5",account);

	/*---------------------------------------
	| If variable file doesn't exist	|
	---------------------------------------*/
	if (access(filename,00) < 0)
	{
		if ((errc = RF_OPEN (filename, sizeof (struct acc_type), "w", &fd)))
			sys_err ("Error in ACCOUNT during (WKCREAT)", errc, PNAME);

		if ((errc = RF_CLOSE (fd)))
			sys_err ("Error in ACCOUNT during (WKCLOSE)", errc, PNAME);
	}

	if ((errc = RF_OPEN (filename, sizeof (struct acc_type), mode, &fd)))
		sys_err ("Error in ACCOUNT during (WKOPEN)", errc, PNAME);

	return(fd);
}

int
UserAccountAdd (char *moption)
{
	int	fd = UserAccountOpen((char *)0,"a");

	if (fd < 0)
		return(errno);

	sprintf(acc_rec._desc,"%-60.60s",moption);
	acc_rec._time = time (NULL);

	errno = RF_ADD(fd,(char *) &acc_rec);

	RF_CLOSE(fd);
	
	return(errno);
}
