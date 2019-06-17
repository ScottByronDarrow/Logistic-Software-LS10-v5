/*=====================================================================
|  Copyright (C) 1996 - 2001 Logistic Software Limited   .            |
|=====================================================================|
| $Id: modsel.c,v 5.2 2001/08/09 05:13:38 scott Exp $
|  Program Name  : (modsel.c)
|  Program Desc  : (Module Select Input Program)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: modsel.c,v $
| Revision 5.2  2001/08/09 05:13:38  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:31  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:40  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/05/02 08:47:25  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to perform routine maintenance to ensure standards are maintained.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: modsel.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/modsel/modsel.c,v 5.2 2001/08/09 05:13:38 scott Exp $";

#define		NO_SCRGEN
#include 	<pslscr.h>	
#include 	<ml_std_mess.h>	
#include 	<ml_menu_mess.h>	

#define	MASTER		 (!strcmp (envCoMstBr, comm_rec.est_no))
#define	CO_DB		 (envCoClose [0] == '1')
#define	CO_CR		 (envCoClose [1] == '1')
#define	CO_SK		 (envCoClose [2] == '1')
#define	CO_GL		 (envCoClose [4] == '1')
#define	ALL_COMPANY	 (CO_DB && CO_CR && CO_SK && CO_GL)

#define	DB_MOD		0
#define	CR_MOD		1
#define	SK_MOD		2
#define	GL_MOD		3

#define	NOT_ACTIVE	0
#define	OPEN		1
#define	TO_CLOSE	2
#define	CLOSING		3
#define	CLOSED		4

	struct	{
		int		module;
		char	mod_code  [3];
		char	mod_desc  [21];
		long	mod_date;
		int		lst_mth;
		int		lst_yr;
		int		nxt_mth;
		int		nxt_yr;
		int		status;
		int		cur_mth;
		int		cur_yr;
		int		new_status;
	} MOD_STAT  [4];

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	int		workMonth;
	char	envCoClose  [6];
	char	envCoMstBr  [3];

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct esmrRecord	esmr_rec;
struct mectRecord	mect_rec;
struct mectRecord	mect2_rec;

	char	*mect2	=	"mect2";

	/*---------------------------------------
	| Set up Array to hold Months of Year . |
	---------------------------------------*/
	static char *mth  [] = {
		"Jan",	"Feb",	"Mar",	"Apr", 	"May",	"Jun",
		"Jul",	"Aug", 	"Sep",	"Oct",	"Nov",	"Dec"
	};

/*============================
| Local function prototypes  |
============================*/
void	PrintScreen		 (void);
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB		 	 (void);
void	SetAll			 (void);
int		Update			 (void);
void	PrintModuleStat	 (int, int, int, int, int, int, int, char, int, int);
int		SetControl		 (void);
void	PrintStatus		 (int, int);
void	ChangeStatus	 (int);
void	MainHeading		 (void);
void	FlashMessage	 (char *);
int		ChangeDate		 (long, long);
long	LastDay			 (long);
int		AllClosed		 (char *);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv  [])
{
	char *	sptr;

	init_scr ();
	set_tty ();

	OpenDB ();

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
	{
		shutdown_prog ();
		return (EXIT_FAILURE);
	}
	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *)0)
		sprintf (envCoClose, "%-5.5s", "11111");
	else
		sprintf (envCoClose, "%-5.5s", sptr);

	sptr = chk_env ("CO_MST_BR");
	if (sptr == (char *)0)
		strcpy (envCoMstBr, " 1");
	else
		sprintf (envCoMstBr, "%-2.2s", sptr);

	if (strcmp (comr_rec.master_br, "  "))
		sprintf (envCoMstBr, comr_rec.master_br);

	clear ();
	MainHeading ();
	SetAll ();

	PrintScreen ();

	if (SetControl () == -1)
		return (EXIT_FAILURE);

	if (Update ())
	{
		shutdown_prog ();
		return (EXIT_FAILURE);
	}
	
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
PrintScreen (
 void)
{
	PrintModuleStat 
	 (
		MOD_STAT  [DB_MOD].module,
	   	0,
	   	2,
		MOD_STAT  [DB_MOD].lst_mth,
		MOD_STAT  [DB_MOD].lst_yr,
		MOD_STAT  [DB_MOD].nxt_mth,
		MOD_STAT  [DB_MOD].nxt_yr,
		MOD_STAT  [DB_MOD].status,
		MOD_STAT  [DB_MOD].cur_mth,
		MOD_STAT  [DB_MOD].cur_yr
	);
	PrintModuleStat 
	 (
		MOD_STAT  [CR_MOD].module,
		40,
		2,
		MOD_STAT  [CR_MOD].lst_mth,
		MOD_STAT  [CR_MOD].lst_yr,
		MOD_STAT  [CR_MOD].nxt_mth,
		MOD_STAT  [CR_MOD].nxt_yr,
		MOD_STAT  [CR_MOD].status,
		MOD_STAT  [CR_MOD].cur_mth,
		MOD_STAT  [CR_MOD].cur_yr
	);
	PrintModuleStat 
	 (
		MOD_STAT  [SK_MOD].module,
		0,
		11,
		MOD_STAT  [SK_MOD].lst_mth,
		MOD_STAT  [SK_MOD].lst_yr,
		MOD_STAT  [SK_MOD].nxt_mth,
		MOD_STAT  [SK_MOD].nxt_yr,
		MOD_STAT  [SK_MOD].status,
		MOD_STAT  [SK_MOD].cur_mth,
		MOD_STAT  [SK_MOD].cur_yr
	);
	PrintModuleStat 
	 (
		MOD_STAT  [GL_MOD].module,
		40,
		11,
		MOD_STAT  [GL_MOD].lst_mth,
		MOD_STAT  [GL_MOD].lst_yr,
		MOD_STAT  [GL_MOD].nxt_mth,
		MOD_STAT  [GL_MOD].nxt_yr,
		MOD_STAT  [GL_MOD].status,
		MOD_STAT  [GL_MOD].cur_mth,
		MOD_STAT  [GL_MOD].cur_yr
	);
}
/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	abc_alias (mect2, mect);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (mect,  mect_list, MECT_NO_FIELDS, "mect_id_no");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (mect2, mect_list, MECT_NO_FIELDS, "mect_id_no2");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (mect);
	abc_fclose (comr);
	abc_fclose (esmr);
	abc_fclose (mect2);
	abc_dbclose ("data");
}

/*================================
| Set up all relevent structures |
================================*/
void
SetAll (
 void)
{
	int		cr_dmy [3],
			db_dmy [3],
			tgl_dmy [3],
			inv_dmy [3];

	/*-------------------------
	| Process debtors module. |
	-------------------------*/
	strcpy (mect_rec.co_no, comm_rec.co_no);
	strcpy (mect_rec.br_no, (CO_DB) ? "  " : comm_rec.est_no);
	strcpy (mect_rec.module_type, "DB");
	if (find_rec (mect, &mect_rec, COMPARISON, "r"))
		MOD_STAT [DB_MOD].status = NOT_ACTIVE;
	else
	{
		/*--------------------
		| Get current month. |
		--------------------*/
		DateToDMY (comm_rec.dbt_date , &db_dmy [0],&db_dmy [1],&db_dmy [2]);
		workMonth = db_dmy [1]; 

		if ( (workMonth - 1) != (mect_rec.closed_mth) % 12)
			MOD_STAT [DB_MOD].status = CLOSED;
		else
			MOD_STAT [DB_MOD].status = OPEN;
	
		MOD_STAT [DB_MOD].module = DB_MOD;
		MOD_STAT [DB_MOD].lst_mth = mect_rec.closed_mth - 1;
		MOD_STAT [DB_MOD].lst_yr  = (mect_rec.closed_mth == 12) 
												? db_dmy [2] - 1 : db_dmy [2];
		MOD_STAT [DB_MOD].nxt_yr  = db_dmy [2];
		MOD_STAT [DB_MOD].nxt_mth = (mect_rec.closed_mth == 12) 
				     	? 0 : mect_rec.closed_mth;


		MOD_STAT [DB_MOD].cur_mth = db_dmy [1] - 1;
		MOD_STAT [DB_MOD].cur_yr  = db_dmy [2];
	
		MOD_STAT [DB_MOD].new_status = MOD_STAT [DB_MOD].status;
		strcpy (MOD_STAT [DB_MOD].mod_desc, "CUSTOMERS");
		strcpy (MOD_STAT [DB_MOD].mod_code, "DB");
		MOD_STAT [DB_MOD].mod_date = comm_rec.dbt_date;
	}

	/*---------------------------
	| Process creditors module. |
	---------------------------*/
	strcpy (mect_rec.co_no, comm_rec.co_no);
	strcpy (mect_rec.br_no, (CO_CR) ? "  " : comm_rec.est_no);
	strcpy (mect_rec.module_type, "CR");
	if (find_rec (mect, &mect_rec, COMPARISON, "r"))
		MOD_STAT [CR_MOD].status = NOT_ACTIVE;
	else
	{
		/*--------------------
		| Get current month. |
		--------------------*/
		DateToDMY	 (comm_rec.crd_date , &cr_dmy [0],&cr_dmy [1],&cr_dmy [2]);
		workMonth = cr_dmy [1]; 

		if ( (workMonth - 1) != (mect_rec.closed_mth) % 12)
			MOD_STAT [CR_MOD].status = CLOSED;
		else
			MOD_STAT [CR_MOD].status = OPEN;
	
		MOD_STAT [CR_MOD].module = CR_MOD;
		MOD_STAT [CR_MOD].lst_mth = mect_rec.closed_mth - 1;
		MOD_STAT [CR_MOD].lst_yr  = (mect_rec.closed_mth == 12) 
						? cr_dmy [2] - 1: cr_dmy [2];
		MOD_STAT [CR_MOD].nxt_yr  = cr_dmy [2];
		MOD_STAT [CR_MOD].nxt_mth = (mect_rec.closed_mth == 12) 
				     	? 0 : mect_rec.closed_mth;
	
	
		MOD_STAT [CR_MOD].cur_mth = cr_dmy [1] - 1;
		MOD_STAT [CR_MOD].cur_yr  = cr_dmy [2];
		MOD_STAT [CR_MOD].new_status = MOD_STAT [CR_MOD].status;
		strcpy (MOD_STAT [CR_MOD].mod_code, "CR");
		strcpy (MOD_STAT [CR_MOD].mod_desc, "SUPPLIERS");
		MOD_STAT [CR_MOD].mod_date = comm_rec.crd_date;
	}

	/*---------------------------
	| Process inventory module. |
	---------------------------*/
	strcpy (mect_rec.co_no, comm_rec.co_no);
	strcpy (mect_rec.br_no, (CO_SK) ? "  " : comm_rec.est_no);
	strcpy (mect_rec.module_type, "SK");
	if (find_rec (mect, &mect_rec, COMPARISON, "r"))
		MOD_STAT [SK_MOD].status = NOT_ACTIVE;
	else
	{
		/*--------------------
		| Get current month. |
		--------------------*/
		DateToDMY (comm_rec.inv_date , &inv_dmy [0],&inv_dmy [1],&inv_dmy [2]);
		workMonth = inv_dmy [1]; 

		if ( (workMonth - 1) != (mect_rec.closed_mth) % 12)
			MOD_STAT [SK_MOD].status = CLOSED;
		else
			MOD_STAT [SK_MOD].status = OPEN;

		MOD_STAT [SK_MOD].module = SK_MOD;
		MOD_STAT [SK_MOD].lst_mth = mect_rec.closed_mth - 1;
		MOD_STAT [SK_MOD].lst_yr  = (mect_rec.closed_mth == 12) 
										? inv_dmy [2] - 1: inv_dmy [2];
		MOD_STAT [SK_MOD].nxt_yr  = inv_dmy [2];
		MOD_STAT [SK_MOD].nxt_mth = (mect_rec.closed_mth == 12) 
				     	? 0 : mect_rec.closed_mth;
	

		MOD_STAT [SK_MOD].cur_mth = inv_dmy [1] - 1;
		MOD_STAT [SK_MOD].cur_yr  = inv_dmy [2];
		MOD_STAT [SK_MOD].new_status = MOD_STAT [SK_MOD].status;
		strcpy (MOD_STAT [SK_MOD].mod_code, "SK");
		strcpy (MOD_STAT [SK_MOD].mod_desc, "INVENTORY");
		MOD_STAT [SK_MOD].mod_date = comm_rec.inv_date;
	}

	/*--------------------------------
	| Process General Ledger module. |
	--------------------------------*/
	strcpy (mect_rec.co_no, comm_rec.co_no);
	strcpy (mect_rec.br_no, (CO_GL) ? "  " : comm_rec.est_no);
	strcpy (mect_rec.module_type, "GL");
	if (find_rec (mect, &mect_rec, COMPARISON, "r"))
		MOD_STAT [GL_MOD].status = NOT_ACTIVE;
	else
	{
		/*--------------------
		| Get current month. |
		--------------------*/
		DateToDMY (comm_rec.gl_date, &tgl_dmy [0],&tgl_dmy [1],&tgl_dmy [2]);
		workMonth = tgl_dmy [1];


		if ( (workMonth - 1) != (mect_rec.closed_mth) % 12)
			MOD_STAT [GL_MOD].status = CLOSED;
		else
			MOD_STAT [GL_MOD].status = OPEN;

		MOD_STAT [GL_MOD].module = GL_MOD;
		MOD_STAT [GL_MOD].lst_mth = mect_rec.closed_mth - 1;
		MOD_STAT [GL_MOD].lst_yr  = (mect_rec.closed_mth == 12) 
						? tgl_dmy [2] - 1: tgl_dmy [2];
		MOD_STAT [GL_MOD].nxt_yr  = tgl_dmy [2];
		MOD_STAT [GL_MOD].nxt_mth = (mect_rec.closed_mth == 12) 
				     	? 0 : mect_rec.closed_mth;
	
	
		MOD_STAT [GL_MOD].cur_mth = tgl_dmy [1] - 1;
		MOD_STAT [GL_MOD].cur_yr  = tgl_dmy [2];
		MOD_STAT [GL_MOD].new_status = MOD_STAT [GL_MOD].status;
		strcpy (MOD_STAT [GL_MOD].mod_code, "GL");
		strcpy (MOD_STAT [GL_MOD].mod_desc, "GENERAL LEDGER");
		MOD_STAT [GL_MOD].mod_date = comm_rec.gl_date;
	}
}
/*================================
| Execute modmom with arguments. |
================================*/
int
Update (
 void)
{
	char	run_prog [100];
	int	run_ok = FALSE;

	clear ();
	print_at (0,0, ML (mlMenuMess211));

	strcpy (run_prog, "modmon ");

	if (MOD_STAT [DB_MOD].new_status == TO_CLOSE)
	{
		run_ok = TRUE;
		strcat (run_prog, "1 ");
	}

	if (MOD_STAT [SK_MOD].new_status == TO_CLOSE)
	{
		run_ok = TRUE;
		strcat (run_prog, "3 ");
	}

	if (MOD_STAT [CR_MOD].new_status == TO_CLOSE)
	{
		run_ok = TRUE;
		strcat (run_prog, "2 ");
	}

	if (MOD_STAT [GL_MOD].new_status == TO_CLOSE)
	{
		run_ok = TRUE;
		strcat (run_prog, "5 ");
	}
	if (!run_ok)
		return (EXIT_FAILURE);

	sys_exec (clip (run_prog));
	return (EXIT_SUCCESS);
}
/*==========================
| Print / Process modules. |
==========================*/
void
PrintModuleStat (
 int	mod,
 int	x,
 int	y,
 int	lst_mth,
 int	lst_yr,
 int	nxt_mth,
 int	nxt_yr,
 char	stat,
 int	cur_mth,
 int	cur_yr)
{
	int		offset;
	char	stat_desc  [11];

	box (x, y, 35, 6);
	move (x + 1, y + 5);
	line (34);

	switch (stat)
	{
	case	NOT_ACTIVE:
		strcpy (stat_desc, "NOT ACTIVE");
		break;

	case	OPEN:
		strcpy (stat_desc, "OPEN");
		break;

	case	TO_CLOSE:
		strcpy (stat_desc, "TO CLOSE");
		break;

	case	CLOSED:
		strcpy (stat_desc, "CLOSED");
		break;
	}
	switch (mod)
	{
	case	DB_MOD:
		strcpy (err_str, ML (mlMenuMess038));
		break;

	case	CR_MOD:
		strcpy (err_str, ML (mlMenuMess039));
		break;

	case	GL_MOD:
		strcpy (err_str, ML (mlMenuMess040));
		break;

	case	SK_MOD:
		strcpy (err_str, ML (mlMenuMess041));
		break;

	}
	offset = (35 - strlen (err_str)) / 2;

	print_at (y , x + offset, "%R %s", err_str);
	print_at (y + 1, x + 2, ML (mlMenuMess042), mth [lst_mth] , lst_yr);
	print_at (y + 2, x + 2, ML (mlMenuMess043), mth [nxt_mth] , nxt_yr);
	print_at (y + 3, x + 2, ML (mlMenuMess044), stat_desc);
	print_at (y + 4, x + 2, ML (mlMenuMess045), mth [cur_mth] , cur_yr);
}

int
SetControl (
 void)
{
	int	cur_sel = 0;
	int	c;

	PrintStatus (0, TRUE);
	PrintStatus (1, FALSE);
	PrintStatus (2, FALSE);
	PrintStatus (3, FALSE);

	while ( (c = getkey ()) != EOI)
	{
		switch (c)
		{
		case	DOWN_KEY:
		case	RIGHT_KEY:
		case	' ':
			PrintStatus (cur_sel, FALSE);
			cur_sel++;
			if (cur_sel > 3)
				cur_sel = 0;

			PrintStatus (cur_sel, TRUE);
			
		break;

		case	UP_KEY:
		case	LEFT_KEY:
		case	'\b':
			PrintStatus (cur_sel, FALSE);
			cur_sel--;
			if (cur_sel < 0)
				cur_sel = 3;

			PrintStatus (cur_sel, TRUE);
		break;

		case	'\r':
		case	'\n':
			ChangeStatus (cur_sel);
			PrintStatus (cur_sel, TRUE);
		break;

		case	REDRAW:
			MainHeading ();
			PrintScreen ();
			PrintStatus (0, FALSE);
			PrintStatus (1, FALSE);
			PrintStatus (2, FALSE);
			PrintStatus (3, FALSE);
			PrintStatus (cur_sel, TRUE);
		break;

		case	RESTART:
			shutdown_prog ();
			return (-1);
		break;
		}
	}
	return (EXIT_SUCCESS);
}

void
PrintStatus (
 int	_module,
 int	rv_flag)
{
	char	stat_desc [25];

	switch (MOD_STAT [_module].new_status)
	{
	case	NOT_ACTIVE:
		strcpy (stat_desc, ML (mlMenuMess046));
		break;

	case	OPEN:
		strcpy (stat_desc, ML (mlMenuMess047));
		break;

	case	TO_CLOSE:
		strcpy (stat_desc, ML (mlMenuMess048));
		break;

	case	CLOSED:
		strcpy (stat_desc, ML (mlMenuMess049));
		break;
	}

	switch (_module)
	{
	case	DB_MOD:
		rv_pr (stat_desc, 5, 8, rv_flag);
		break;
		
	case	CR_MOD:
		rv_pr (stat_desc, 45, 8, rv_flag);
		break;

	case	SK_MOD:
		rv_pr (stat_desc, 5, 17, rv_flag);
		break;

	case	GL_MOD:
		rv_pr (stat_desc, 45, 17, rv_flag);
		break;
	}
}

void
ChangeStatus (
 int _cur_sel)
{
	int	i;

	/*----------------------------------------------------
	| Module is not active so nothing can be done to it. |
	----------------------------------------------------*/
	if (MOD_STAT [_cur_sel].new_status == NOT_ACTIVE)
	{
		sprintf (err_str, ML (mlMenuMess050) , clip (MOD_STAT [_cur_sel].mod_desc));
		FlashMessage (err_str);
		return;
	}

	/*------------------------------------------------
	| Module is closed so nothing can be done to it. |
	------------------------------------------------*/
	if (MOD_STAT [_cur_sel].new_status == CLOSED)
	{
		sprintf (err_str, ML (mlMenuMess051), clip (MOD_STAT [_cur_sel].mod_desc));
		FlashMessage (err_str);
		return;
	}

	/*-----------------------------------------------------------
	| Module is company owned and current branch is not master. |
	-----------------------------------------------------------*/
	if (envCoClose [_cur_sel] == '1' && !MASTER)
	{
		sprintf (err_str, ML (mlMenuMess052) , clip (MOD_STAT [_cur_sel].mod_desc));
		FlashMessage (err_str);
		return;
	}
	
	/*-------------------------------------------
	| Module is currently open so set to close. |
	-------------------------------------------*/
	if (MOD_STAT [_cur_sel].new_status == OPEN)
		MOD_STAT [_cur_sel].new_status = TO_CLOSE;

	/*-------------------------------------------------
	| else Module is currently closed so set to open. |
	-------------------------------------------------*/
	else if (MOD_STAT [_cur_sel].new_status == TO_CLOSE)
		MOD_STAT [_cur_sel].new_status = OPEN;

	/*---------------------------------------------------
	| Module is set to close and is not general ledger. |
	---------------------------------------------------*/
	if (MOD_STAT [_cur_sel].new_status == TO_CLOSE &&
	     MOD_STAT [_cur_sel].module != GL_MOD)
		return;

	/*-----------------------------------------------
	| Module is set to close and is general ledger. |
	-----------------------------------------------*/
	if (MOD_STAT [_cur_sel].module == GL_MOD)
	{
		if (MOD_STAT [GL_MOD].new_status == TO_CLOSE)
		{
			for (i = 0; i < 3; i++)
			{
				if (envCoClose [i] == '0')
				{
					if (AllClosed (MOD_STAT [i].mod_code))
					{
						sprintf (err_str, ML (mlMenuMess053) , clip (MOD_STAT [i].mod_desc),mect2_rec.br_no);
						FlashMessage (err_str);
						MOD_STAT [i].new_status = OPEN;
						PrintStatus (i, FALSE);
						return;
					}
				}
			} 
			for (i = 0; i < 4; i++)
			{
				if (ChangeDate (MOD_STAT [GL_MOD].mod_date, 
			      	       	       MOD_STAT [i].mod_date))
				{
					MOD_STAT [i].new_status = TO_CLOSE;
					PrintStatus (i, FALSE);
				}
			}
		}
		return;
	}
	/*----------------------------------------------------------------
	| GL is set to close so check status of current selected module. |
	----------------------------------------------------------------*/
	if (MOD_STAT [GL_MOD].new_status == TO_CLOSE)
	{
		if (ChangeDate (MOD_STAT [GL_MOD].mod_date, 
			       MOD_STAT [_cur_sel].mod_date))
		{
			MOD_STAT [_cur_sel].new_status = TO_CLOSE;
			sprintf (err_str, ML (mlMenuMess054) , clip (MOD_STAT [_cur_sel].mod_desc));
			FlashMessage (err_str);
			PrintStatus (_cur_sel, FALSE);
			return;
		}
	}
	return;
}

void
MainHeading (
 void)
{
	crsr_off ();
	print_at (0,28, ML (mlMenuMess055));

	line_at (1,0, 79);
	line_at (21,0,79);

	print_at (22,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (23,0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
	print_at (20,0, ML (mlMenuMess056));
}

void
FlashMessage (
 char *	mess)
{
	int	i;

	move (0,20);
	cl_line ();
	
	fflush (stdout);
	for (i = 0; i < 4; i++)
	{
		putchar (BELL);
		rv_pr (mess, (80 - strlen (mess)) / 2,20, (i % 2));
		sleep (sleepTime);
	}
	move (0,20);
	cl_line ();
	print_at (20,0, ML (mlMenuMess056));
}

/*============================================
| Check Modules against general ledger date. |
============================================*/
int
ChangeDate (
 long	generalLedgerDate,
 long	moduleDate)
{
	if (moduleDate && moduleDate <= MonthEnd (generalLedgerDate))
		return (EXIT_FAILURE);
	
	return (EXIT_SUCCESS);
}
/*============================================================
| Check if all records for module type / company are closed. |
============================================================*/
int
AllClosed (
 char *	_type)
{
	long	wk_date = 0L;

	strcpy (mect2_rec.co_no, comm_rec.co_no);
	sprintf (mect2_rec.module_type, "%-2.2s", _type);
	cc = find_rec (mect2, &mect2_rec, GTEQ, "r");
	while (!cc && !strcmp (mect2_rec.co_no, comm_rec.co_no) &&
		       !strcmp (mect2_rec.module_type, _type))
	{
		if (!strcmp (mect2_rec.co_no, comm_rec.co_no) &&
		     !strcmp (mect2_rec.br_no, comm_rec.est_no))
		{
			cc = find_rec (mect2, &mect2_rec, NEXT, "r");
			continue;
		}
		strcpy (esmr_rec.co_no, mect2_rec.co_no);
		strcpy (esmr_rec.est_no, mect2_rec.br_no);
		if (!find_rec (esmr, &esmr_rec, COMPARISON, "r"))
		{
			if (!strcmp (_type, "DB"))
				wk_date = esmr_rec.dbt_date;

			if (!strcmp (_type, "CR"))
				wk_date = esmr_rec.crd_date;

			if (!strcmp (_type, "SK"))
				wk_date = esmr_rec.inv_date;

			if (!strcmp (_type, "CR"))
				wk_date = esmr_rec.crd_date;

			if (!strcmp (_type, "GL"))
				wk_date = esmr_rec.gl_date;

			if (ChangeDate (MOD_STAT [GL_MOD].mod_date, wk_date))
				return (EXIT_FAILURE);
		}

		cc = find_rec (mect2, &mect2_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}
