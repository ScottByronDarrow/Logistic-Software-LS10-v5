/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: lc_input.c,v 5.3 2002/07/26 05:48:07 robert Exp $
|  Program Name  : ( lc_input.c   )                                   |
|  Program Desc  : ( Logistic License Maintenance Program         )   |
|---------------------------------------------------------------------|
|  Date Written  : (10/05/90)      | Author      : Trevor van Bremen  |
|---------------------------------------------------------------------|
| $Log: lc_input.c,v $
| Revision 5.3  2002/07/26 05:48:07  robert
| Fixed number overflow on license encrption (make password key longer)
|
| Revision 5.2  2001/08/09 09:26:59  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:58:41  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:22:55  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:43:54  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/03/01 08:29:48  scott
| General Clean up
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lc_input.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/lc_input/lc_input.c,v 5.3 2002/07/26 05:48:07 robert Exp $";

#include	<pslscr.h>
#include	<ml_utils_mess.h>
#include	<fcntl.h>
#include	<unistd.h>

#include	<license2.h>
#include    <errno.h>

enum
{
	SCN_License = 1
};

static char	*ProgPath = "PROG_PATH",	/*const*/
			*License  = "BIN/LICENSE";	/*const*/

static int	enteredPassword;
static long	user_key;

static struct DES_REC	des_rec;
static struct LIC_REC	lic_rec, new_lc;

/*----------------------------
| Local & Screen Structure   |
----------------------------*/

static	struct	var	vars[] =
{
	{SCN_License, LIN, "orgpasswd",	9, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Original       : ", "",
		NA, NO,  JUSTLEFT, "", "", lic_rec.passwd},
	{SCN_License, LIN, "serialno",	10, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Serial Number  : ", "Please make sure you key Upper/Lower characters as supplied.",
		YES, NO,  JUSTLEFT, "", "", new_lc.passwd},
	{SCN_License, LIN, "usr_name",	12, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "User Name      : ", " ",
		YES, NO,  JUSTLEFT, "", "", new_lc.user_name},
	{SCN_License, LIN, "usr_add1",	13, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "User Address 1 : ", " ",
		YES, NO,  JUSTLEFT, "", "", new_lc.user_add1},
	{SCN_License, LIN, "usr_add2",	14, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "User address 2 : ", " ",
		YES, NO,  JUSTLEFT, "", "", new_lc.user_add2},
	{SCN_License, LIN, "usr_add3",	15, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "User address 3 : ", " ",
		YES, NO,  JUSTLEFT, "", "", new_lc.user_add3},
	{SCN_License, LIN, "max_user",	16, 20, INTTYPE,
		"NNNN", "          ",
		"0", "0", "Max. Logins    : ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *) &new_lc.max_usr},
	{SCN_License, LIN, "max_term",	17, 20, INTTYPE,
		"NNNNN", "          ",
		"0", "0", "Max. Terminals : ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *) &new_lc.max_trm},
	{SCN_License, LIN, "expr_dte",	18, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Expiry Date    : ", " ",
		 NA, NO,  JUSTLEFT, "", "", (char *) &new_lc.expiry},
	{SCN_License, LIN, "mch_make",	19, 20, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", " ", "Machine make   : ", "Please enter make of machine, e.g HP",
		YES, NO,  JUSTLEFT, "", "", new_lc.mch_make},
	{SCN_License, LIN, "mch_modl",	20, 20, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", " ", "Machine model  : ", "Please enter model of machine, e.g 9000/867",
		YES, NO,  JUSTLEFT, "", "", new_lc.mch_modl},
	{SCN_License, LIN, "mch_serl",	21, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Machine serial : ", "Please enter machine serial number",
		YES, NO,  JUSTLEFT, "", "", new_lc.mch_serl},
	{0}
};

/*============================
| Local Function Prototypes. |
============================*/
int 	heading 				(int);
int 	spec_valid 				(int);
static 	long	GetUserKey 		(void);
static 	int		GoodPasswd 		(struct LIC_REC *);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int      argc,
	char	*argv[])
{
	SETUP_SCR (vars);

	init_scr 	();
	clear 		();
	set_tty 	();
	set_masks 	();
	init_vars 	(1);

	/*	Variable initialisation
	 */
	memset (&new_lc, 0, sizeof (struct LIC_REC));
	memset (&lic_rec, 0, sizeof (struct LIC_REC));
	enteredPassword	= FALSE;
	user_key 		= GetUserKey ();

	/*----------------------
	| Reset Control flags  |
	----------------------*/
	entry_exit = FALSE;
	prog_exit  = FALSE;
	restart    = FALSE;
	init_ok    = FALSE;
	search_ok  = TRUE;

	/*--------------------- 
	| Entry Screen input  |
	---------------------*/
	heading (SCN_License);
	scn_display (SCN_License);
	entry (SCN_License);
	if (!restart && (prog_exit || enteredPassword))
	{
		if (GoodPasswd (&new_lc))
		{
			lc_write (&new_lc);
			puts (ML(mlUtilsMess043));
		}
		else
		{
			puts (ML(mlUtilsMess044));
			sleep(2);
			clear_mess();
			puts (ML(mlUtilsMess045));
			sleep(2);
			clear_mess();
		}
	}
	else
		puts (ML(mlUtilsMess044));

	rset_tty ();
	crsr_on ();
	return (EXIT_SUCCESS);
}

int
heading (
 int                scn)
{
	clear ();

	centre_at (0, 80, ML(mlUtilsMess046));

	box (0, 4, 80, 17);
	line_at (1, 0, 80);
	line_at (8, 1, 78);
	line_at (11, 1, 78);
	print_at (3, 25, ML(mlUtilsMess047), user_key);
	print_at (5, 4, ML(mlUtilsMess048));
	print_at (6, 4, ML(mlUtilsMess049));
	print_at (7, 4, ML(mlUtilsMess050));
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int                field)
{
	if (LCHECK ("serialno"))
	{
		enteredPassword = FALSE;
		if (!GoodPasswd (&new_lc))
		{
			print_err (ML(mlUtilsMess051));
			return (EXIT_FAILURE);
		}
		enteredPassword = TRUE;
		new_lc.max_usr = des_rec.max_usr;
		new_lc.max_trm = des_rec.max_trm;
		new_lc.expiry  = des_rec.expiry;
		scn_display (SCN_License);
	}
	return (EXIT_SUCCESS);
}

/*
 *	Create a new LICENSE file if necessary,
 *	returning client license key
 */
static long
GetUserKey (void)
{
	char	*sptr = getenv (ProgPath);
	char	lic_nam [64];
	int		lic_fd;

	sprintf (lic_nam, "%s/%s", sptr ? sptr : "/usr/LS10.5", License);
	if (access (lic_nam, F_OK))
	{
		/*	Abort on unexpected error
		 */
		if (errno != ENOENT)
			sys_err ("access (1)", errno, PNAME);

		umask (0);
		if ((lic_fd = creat (lic_nam, 0644)) < 0)
			sys_err ("creat (1)", errno, PNAME);
		close (lic_fd);
	}
	else
	{
		if (lc_check (&des_rec, &lic_rec) == LICENSE_BAD)
			memset (&lic_rec, 0, sizeof (struct LIC_REC));
		else
			new_lc = lic_rec;
	}
	return (lc_i_no ());
}

static int
GoodPasswd (
 struct LIC_REC	*lic)
{
	strcpy (des_rec.passwd, lic -> passwd);
	psl_decrypt (&des_rec);
	return (des_rec.user_key == user_key);
}
