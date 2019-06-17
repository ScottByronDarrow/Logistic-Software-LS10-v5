/*=====================================================================
|  Copyright (C) 1988 - 1992 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( dsp_brand.c         )                            |
|  Program Desc  : ( User licence branding.                       )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Date Written  : (05/12/90)      | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
|  Date Modified : (05/12/90)      | Modified  by : Scott Darrow.     |
|  Date Modified : (21/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (12/09/1997)    | Modified  by : Jiggs A Veloz     |
|  Date Modified : (05/10/1999)    | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  Comments      :                                                    |
|  (21/08/92)    : Changes for Concurrent Logins. S/C PSL 7646        |
|  (12/09/1997)  : SEL Multilingual Conversion. Changed dates from 	  |
|  (05/10/1999)  : Ported to ANSI standards.                          |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: dsp_brand.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/dsp_brand/dsp_brand.c,v 5.2 2001/08/09 09:26:51 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<get_lpno.h>
#include	<license2.h>

struct	DES_REC	des_rec;
struct	LIC_REC	lic_rec;
	
	char	systemDate[11];
	long	lsystemDate = 0L;

	int	chk_brand = FALSE;

/*===========================
| Local function prototypes |
===========================*/
void	process	(struct LIC_REC *lic_rec);


int
main (
 int	argc,
 char *	argv [])
{
	if (!strncmp (argv[0], "chk_brand", 9))
		chk_brand = TRUE;
	else
		chk_brand = FALSE;

	sprintf (systemDate, "%-10.10s", DateToString (TodaysDate()));
	lsystemDate = StringToDate (systemDate);

	init_scr ();
	set_tty ();
	crsr_off ();

	lc_check (&des_rec, &lic_rec);
	process (&lic_rec);

	rset_tty ();
	crsr_on ();
	return (EXIT_SUCCESS);
}

void
process (
 struct LIC_REC *lic_rec)
{
	if (chk_brand)
		if (lic_rec->expiry - 30 > lsystemDate)
			return;
	
	sprintf (err_str, "LOGISTIC LICENSE TO ( %s )", lic_rec->user_name);

	Dsp_prn_open (0, 0, 14, err_str, (char *) 0, (char *) 0, 
			                 (char *) 0, (char *) 0, 
			                 (char *) 0, (char *) 0);

	Dsp_saverec ("            U S E R    L I C E N S E    D I S P L A Y.           ");
	Dsp_saverec ("");
	Dsp_saverec ("[PRINT] [EDIT/END]");

	sprintf (err_str," User Name      : %40.40s", lic_rec->user_name);
	Dsp_saverec (err_str);

	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");

	sprintf (err_str, " User Address 1 : %40.40s", lic_rec->user_add1);
	Dsp_saverec (err_str);

	sprintf (err_str, " User Address 2 : %40.40s", lic_rec->user_add2);
	Dsp_saverec (err_str);

	sprintf (err_str, " User Address 3 : %40.40s", lic_rec->user_add3);
	Dsp_saverec (err_str);
	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");

	sprintf (err_str, " User Password  : %11.11s", lic_rec->passwd);
	Dsp_saverec (err_str);

	sprintf (err_str, " User License  : %5d Terminals %4d Logins", lic_rec->max_trm, lic_rec->max_usr);
	Dsp_saverec (err_str);

	sprintf (err_str, " Expiry Date   : %10.10s", DateToString (lic_rec->expiry));
	Dsp_saverec (err_str);
	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");

	sprintf (err_str, " Machine Make  : %10.10s", lic_rec->mch_make);
	Dsp_saverec (err_str);

	sprintf (err_str, " Machine Model : %10.10s", lic_rec->mch_modl);
	Dsp_saverec (err_str);

	sprintf (err_str, " Machine Ser # : %25.25s", lic_rec->mch_serl);
	Dsp_saverec (err_str);
	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");

	Dsp_srch ();
	Dsp_close ();
}

