/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: ser_msg.c,v 5.2 2002/06/24 05:07:18 scott Exp $
|  Program Name  :  (ser_msg.c) 
|---------------------------------------------------------------------|
|  Date Written  : 10/05/86        |  Author     : Scott Darrow.      |
|---------------------------------------------------------------------|
| $Log: ser_msg.c,v $
| Revision 5.2  2002/06/24 05:07:18  scott
| Updated to change year from 2001 to 2002
|
| Revision 5.1  2001/08/06 22:40:56  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 06:59:38  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/12 09:08:58  scott
| Updated to use chk_env instead of get_env. Allows for better control.
|
=====================================================================*/
#include	<std_decs.h>

/*
 *	Function declarations
 */
static void	StandardHead  	(char *, char *, char *, char *),
			Scroll  		(int, int, int, char *);

/*
 *	External interface
 */
void
ser_msg  (
	int		error_no,
	struct 	LIC_REC	*lic_rec,
	int		show)
{
	if  (error_no < LICENSE_OK)
	{
		int	quit = TRUE;

		StandardHead  
		(
			lic_rec -> user_name,
			lic_rec -> user_add1, 
			lic_rec -> user_add2,
			lic_rec -> user_add3);

		switch  (error_no)
	    {
		case LICENSE_BAD :
			print_at  (15, 5,
				"%R Y O U R   S Y S T E M   H A S   A N  I N V A L I D   L I C E N S E  ");
			print_at  (17, 4,
				"%R LOGISTIC SOFTWARE SYSTEM WILL NEED TO BE LICENSED FOR YOUR MACHINE. ");
			break;

		case LICENSE_DEAD :
			print_at  (15, 8,
				"%R Y O U R   S Y S T E M   L I C E N S E   H A S   E X P I R E D. ");
			print_at  (17, 5,
				"%R EXPIRY DATE WILL NEED TO BE UPDATED BEFORE PROCESSING CAN CONTINUE. ");
			break;

		case LICENSE_DYING	:
			print_at  (15, 8,
				"%R Y O U R   S Y S T E M   L I C E N S E  I S   E X P I R I N G. ");
			print_at  (17, 8,
				"%R EXPIRY DATE WILL NEED TO BE UPDATED WITHIN THE NEXT 2 WEEKS. ");
			sleep (10);
			quit = FALSE;
			break;

		case -4	:
			print_at  (15, 14,
				"%R SORRY YOUR TERMINAL HAS AN INVALID TERMINAL NUMBER ");
			break;

		case LICENSE_OFLOW	:
			print_at  (15, 11,
				"%R SORRY YOUR SYSTEM IS ONLY LICENSED FOR %5d CONCURRENT USERS ",
				lic_rec->max_usr);
			print_at  (17, 2,
				"%R PLEASE TRY TO LOGIN AGAIN WHEN ANOTHER LOGISTIC SOFTWARE USER LOGS OUT. ");
			break;
		}
		print_at  (19, 10,
		"%R PLEASE CONTACT YOUR SOFTWARE SUPPLIER FOR FURTHER DETAILS. ");

		/*
		 *	Force an exit for fatal licensing errors
		 */
		if  (quit)
			exit  (1);
	}

	if  (!show)
		return;

	/*
	 * Display licence
	 */
	StandardHead  (lic_rec -> user_name, lic_rec -> user_add1, 
			lic_rec -> user_add2, lic_rec -> user_add3);

	print_at  (15, 4, "Serial number     : %s", lic_rec->passwd);
	print_at  (16, 4, "User License      : %d Terminals, %d Users",
		lic_rec->max_trm, lic_rec->max_usr);
	if  (lic_rec->expiry <= 0L)
		print_at  (17, 4, "Expiry Date       : NEVER");
	else
		print_at  (17, 4, "Expiry Date       : %s", DateToString  (lic_rec->expiry));
	print_at  (18, 4, "Machine Make      : %10.10s",lic_rec->mch_make);
	print_at  (19, 4, "Machine Model     : %10.10s",lic_rec->mch_modl);
	print_at  (20, 4, "Machine Serial #  : %25.25s",lic_rec->mch_serl);
	sleep  (1);
}

static void
StandardHead  (
	char	*_user,
	char	*_adr1,
	char	*_adr2,
	char	*_adr3)
{
	char	msg_str [81];
	char	*envSysRelease;
	char	*envCopyright;

	envSysRelease	=	chk_env  ("SYS_RELEASE");

	clear  ();
	crsr_off ();
	box (0,0,80,20);
	sprintf (msg_str, "Logistic Software Distribution System Version %s  ",
			 (envSysRelease ==  (char *)0) ? " " : envSysRelease);

	Scroll ((80 -  (int) strlen (msg_str)) / 2,2,strlen (msg_str) + 2,msg_str);

	envCopyright = getenv ("COPYRIGHT");

	sprintf (msg_str,"Copyright  (c) 1996 - 2002 %s  ", 
			(envCopyright == (char *)0) ? " " : envCopyright);
	Scroll ((80 -  (int) strlen (msg_str)) / 2,5,strlen (msg_str) + 2,msg_str);

	line_at (9,1,79);
	print_at (10,4, "Licensed to       : %s", _user);
	print_at (11,4, "                  : %s", _adr1);
	print_at (12,4, "                  : %s", _adr2);
	print_at (13,4, "                  : %s", _adr3);
	line_at (14,1,79);
}

/*==========================================
| Produces nice Scroll, looks good anyway. |
==========================================*/
static void
Scroll  (
 int	x,
 int	y,
 int	w,
 char	*s)
{
	int	ll = strlen (s);
	int	l =  (w - ll) / 2;
	int	r =  (w - l - ll);
	char	buffer [133];

	sprintf (buffer,"%*s%s%*s",l," ",s,r," ");

	box (x,y,w,1);

	rv_pr (buffer,x + 1,y + 1,1);

	move (x - 2,y - 1);
	PGCHAR (0);
	PGCHAR (6);
	PGCHAR (6);
	PGCHAR (1);

	move (x - 2,y);
	PGCHAR (4);
	move (x + 1,y);
	PGCHAR (9);

	move (x - 2,y + 1);
	PGCHAR (2);
	PGCHAR (6);
	PGCHAR (11);

	move (x + w - 2,y - 1);
	PGCHAR (0);
	PGCHAR (6);
	PGCHAR (6);
	PGCHAR (1);

	move (x + w - 2,y);
	PGCHAR (9);
	move (x + w + 1,y);
	PGCHAR (4);

	move (x + w - 1,y + 1);
	PGCHAR (10);
	PGCHAR (6);
	PGCHAR (3);
}
