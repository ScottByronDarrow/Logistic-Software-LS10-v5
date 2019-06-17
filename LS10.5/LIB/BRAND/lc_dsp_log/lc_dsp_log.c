/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: lc_dsp_log.c,v 5.0 2002/05/08 01:46:00 scott Exp $
|  Program Name  : (lc_dsp_log.c)
|  Program Desc  : (Logistic License logfile printout)
|---------------------------------------------------------------------|
|  Date Written  : (10/05/90)      | Author      : Trevor van Bremen  |
|---------------------------------------------------------------------|
| $Log: lc_dsp_log.c,v $
| Revision 5.0  2002/05/08 01:46:00  scott
| CVS administration
|
| Revision 4.2  2001/08/09 09:31:22  scott
| Updated to add FinishProgram () function
|
| Revision 4.1  2001/08/07 00:08:03  scott
| RELEASE 5.0
|
| Revision 4.0  2001/04/02 04:49:22  scott
| Updated new program to release 4.(x)
|
| Revision 1.1  2001/04/02 04:48:20  scott
| Updated to version 9.10.3
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lc_dsp_log.c,v $",
		*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LIB/BRAND/lc_dsp_log/lc_dsp_log.c,v 5.0 2002/05/08 01:46:00 scott Exp $";

#define		X_OFF	0
#define		Y_OFF	2

#include 	<pslscr.h>
#include 	<get_lpno.h>
#include 	<license2.h>

void		Process 		(void);
void		ProcessLines 	(int);
int	 		OpenLog 		(void);
void 		CloseLog 		(int);
void		shutdown_prog	(void);

extern	int	errno;

struct	lic_log
{
	long	brand_key;
	char	brand_password [12];
	char	brand_user [15];	
	long	brand_date;
	char	brand_time [6];
	char	brand_client [41];
	int		brand_logins;
	long	brand_expire;
	int		brand_terms;
} _log_rec;

struct	DES_REC	des_rec;

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	init_scr ();
	set_tty  ();
	clear 	 ();
	swide 	 ();
	crsr_off ();
	Process  ();
	shutdown_prog 	();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence . |
=========================*/
void
shutdown_prog (
 void)
{
	clear ();
	snorm ();
	rset_tty ();
}

/*======================
| Process Log Display. |
======================*/
void
Process (void)
{
	int	fd = OpenLog ();

	strcpy (err_str, "LS10 LICENSE AUDIT FILE.");

	print_at (0,30, "%R LS10  LICENSE LOG DISPLAY ");
	Dsp_prn_open (0, 2, 14, err_str, (char *) 0, (char *) 0, 
			                 (char *) 0, (char *) 0, 
			                 (char *) 0, (char *) 0);

	Dsp_saverec ("  LICENCED BY  |                CLIENT NAME.             |  KEY.  |  SERIAL NO  |   DATE   |  TIME | NUMBER | NUMBER |  EXPIRY  ");
	Dsp_saverec ("               |                                         |        |             |  CREATED |CREATED|TERMINAL| LOGINS |   DATE.  ");

	Dsp_saverec (" [PRINT] [EDIT/END]");
	ProcessLines (fd);
			
	CloseLog (fd);

	Dsp_srch ();
	Dsp_close ();
}

void
ProcessLines (
	int		fd)
{
	char	disp_str [300],
			brandDate [11],
			expiryDate [11];

	if (RF_REWIND (fd))
		return;

	cc = RF_READ (fd, (char *)&_log_rec);
	while (!cc)
	{
		strcpy (brandDate, DateToString (_log_rec.brand_date));
		strcpy (expiryDate, DateToString (_log_rec.brand_expire));

		sprintf 
		(
			disp_str,
			" %c%s^E %40.40s^E^1%08ld^6^E^1 %11.11s ^6^E%s^E %5.5s ^E^1 %05d  ^6^E^1  %04d  ^6^E^1%10.10s^6",

			toupper (_log_rec.brand_user [0]),
			_log_rec.brand_user + 1,
			_log_rec.brand_client,
			_log_rec.brand_key,
			_log_rec.brand_password,
			brandDate,
			_log_rec.brand_time,
			_log_rec.brand_terms,
			_log_rec.brand_logins,
			expiryDate
		);

		Dsp_saverec (disp_str);
		cc = RF_READ (fd, (char *)&_log_rec);
	}
	return;
}
/*=================
| Open audit log. |
=================*/
int
OpenLog (void)
{
	int		fd;
	char	*basepath = getenv ("BASE_PATH");
	char	filename [100];

	sprintf (filename, "%s/ver.etc/BRAND/BRAND.LOG",
		basepath ? basepath : "/usr");

	/*---------------------------------------
	| If variable file doesn't exist	|
	---------------------------------------*/
	if (access (filename,00) < 0)
		return (EXIT_FAILURE);

	_cc = RF_OPEN (filename,sizeof (struct lic_log),"r",&fd);
	if (_cc)
		file_err (_cc, "BRAND.LOG", "WKOPEN");

	return (fd);
}

/*==================
| Close Audit log. |
==================*/
void
CloseLog (
	int	fd)
{
	_cc = RF_CLOSE (fd);
	if (_cc)
		file_err (_cc, "BRAND.LOG", "WKCLOSE");
}
