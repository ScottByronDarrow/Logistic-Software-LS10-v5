/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
| $Id: so_lsmaint.c,v 5.6 2002/04/15 08:38:27 robert Exp $ 
|  Program Name  : ( so_lsmaint.c )                                   |
|  Program Desc  : ( Add / Maintain Lost Sales Records. )   		  |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|---------------------------------------------------------------------|
| $Log: so_lsmaint.c,v $
| Revision 5.6  2002/04/15 08:38:27  robert
| SC00772 - added default value for demand update field
|
| Revision 5.5  2002/04/15 08:00:18  robert
| SC00772- Updated LS10-GUI display issue
|
| Revision 5.4  2002/04/03 07:25:50  robert
| Updated to modify description display to avoid overlap on LS10-GUI
|
| Revision 5.3  2002/02/22 07:23:46  kaarlo
| Updated to convert to app.schema and perform a general code clean.
|
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_lsmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_lsmaint/so_lsmaint.c,v 5.6 2002/04/15 08:38:27 robert Exp $";

#include <pslscr.h>
#include <minimenu.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>

#include "schema"

#define	SEL_UPDATE	0
#define	SEL_IGNORE	1
#define	SEL_DELETE	2
#define	SEL_DEFAULT	99

/*================================================================
| Special fields and flags  ################################## . |
================================================================*/
int  	new_item = 0;

struct commRecord   comm_rec;
struct exlsRecord   exls_rec;
struct inlsRecord   inls_rec;

char	*data = "data";

char	badFileName [5];

MENUTAB upd_menu [] =
	{
		{ " 1. UPDATE RECORD WITH CHANGES MADE.   ",
		  "" },
		{ " 2. IGNORE CHANGES JUST MADE TO RECORD.",
		  "" },
		{ " 3. DELETE RECORD.                     ",
		  "" },
		{ ENDMENU }
	};

/*============================
| Local & Screen Structures. |
============================*/

struct {
	char 	dummy [11];
	char	DemandCode [2];
	char	DemandDesc [26];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "ls_code",	 4, 2, CHARTYPE,
		"UU", "          ",
		" ", "", "Lost sale code  ", "Lost Sale Code ",
		YES, NO,  JUSTLEFT, "", "", exls_rec.code},
	{1, LIN, "description",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description.    ", " ",
		YES, NO,  JUSTLEFT, "", "", exls_rec.description},
	{1, LIN, "DemandCode",	 6, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Demand updated  ", "Y(es) to indicate code will update lost sales demand.",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.DemandCode},
	{1, LIN, "DemandDesc",	 6, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.DemandDesc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

extern	int	TruePosition;

/*=======================
| Function Declarations |
=======================*/

void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int  spec_valid (int field);
int  update (void);
int  ExlsDelOk (void);
void save_page (char *key_val);
int  heading (int scn);

/*===========================
| Main Processing Routine . |
===========================*/

int
main (
	int 	argc,
	char 	* argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	/*---------------------------
	| Stup required parameters. |
	---------------------------*/
	
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
	
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
	
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
	
		heading(1);
		scn_display(1);
		edit(1);

		if (restart)
			continue;

		if (update ())
			/* exit_prog(); <-- old line */
            break; /* <-- replacement line */

	}	/* end of input control loop	*/
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/

void
OpenDB (void)
{
	abc_dbopen(data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec );
	open_rec (exls, exls_list, EXLS_NO_FIELDS, "exls_id_no");
}

/*=========================
| Close data base files . |
=========================*/

void
CloseDB (void)
{
	abc_fclose (exls);
	abc_dbclose (data);
}

int
spec_valid (
	int 	field)
{
	if (LCHECK ("ls_code"))
	{
		if (SRCH_KEY)
		{
			save_page (temp_str);
			return (0);
		}
		
		strcpy (exls_rec.co_no,comm_rec.co_no);
		
		cc = find_rec(exls, &exls_rec, COMPARISON, "u");
		
		if (!cc)
		{
			if (exls_rec.demand_ok[0] == 'Y')
				sprintf (local_rec.DemandDesc, "%-25s", "[Y]es - Demand Updated");
			else
				sprintf (local_rec.DemandDesc, "%-25s", "[N]o - Demand Not Updated");

			strcpy (local_rec.DemandCode, exls_rec.demand_ok);

			new_item = FALSE;
			entry_exit = 1;
			DSP_FLD ("DemandCode");
			DSP_FLD ("DemandDesc");
			DSP_FLD ("description");
		}
		else
			new_item = TRUE;
		return(0);
	}
			
	if (LCHECK ("DemandCode"))
	{
		if (local_rec.DemandCode[0] == 'Y')
			sprintf (local_rec.DemandDesc, "%-25s", "[Y]es - Demand Updated");
		else
			sprintf (local_rec.DemandDesc, "%-25s", "[N]o - Demand Not Updated");

		DSP_FLD ("DemandDesc");

		return (0);
	}

	return (0);
}

int
update (void)
{
	int		exitLoop;

	if (new_item)
	{
		sprintf (exls_rec.demand_ok, "%-1.1s", local_rec.DemandCode);
		
		cc = abc_add (exls, &exls_rec);
		
		if (cc)
			file_err (cc, exls, "DBADD");
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N .   ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case 	SEL_DEFAULT :
			case 	SEL_UPDATE :
					sprintf (exls_rec.demand_ok, "%-1.1s", local_rec.DemandCode);
					cc = abc_update (exls, &exls_rec);
					if (cc) 
						file_err (cc, exls, "DBSEL_UPDATE");
					exitLoop = TRUE;
					break;
			case 	SEL_IGNORE :
					abc_unlock (exls);
					exitLoop = TRUE;
					break;
			case 	SEL_DELETE :
					if (ExlsDelOk ())
					{
						clear_mess ();
						cc = abc_delete (exls);
						if (cc)
							file_err (cc, exls, "DBSEL_UPDATE");
					}
					else
					{
						sprintf (err_str, ML(mlSoMess178), badFileName);
						print_mess (err_str);
						sleep (sleepTime);
						clear_mess ();
					}
					exitLoop = TRUE;
					break;
			default :
					break;
	    	}

			if (exitLoop)
				break;
		}
	}

	return(cc);
}

/*===========================
| Check whether it is OK to |
| delete the exsf record.   |
| Files checked are :       |
| inls                      |
===========================*/

int
ExlsDelOk (void)
{
	/*-------------
	| Check inls. |
	-------------*/

	print_mess (ML (mlStdMess035));
	sleep (sleepTime);
	open_rec (inls, inls_list, INLS_NO_FIELDS, "inls_id_no");
	strcpy (inls_rec.co_no,  comm_rec.co_no);
	strcpy (inls_rec.est_no, "  ");
	cc = find_rec (inls, &inls_rec, GTEQ, "r");
	while (!cc && !strcmp (inls_rec.co_no, comm_rec.co_no))
	{
		if (!strcmp (inls_rec.res_code, exls_rec.code))
		{
			abc_fclose (inls);
			strcpy (badFileName, inls);
			return (FALSE);
		}
		cc = find_rec (inls, &inls_rec, NEXT, "r");
	}
	abc_fclose (inls);

	return (TRUE);
}

void
save_page (
	char 	* key_val)
{
	_work_open (1, 0, 40);
	save_rec ("#Ls", "#Lost Sale Description");

	strcpy (exls_rec.co_no, comm_rec.co_no);
	sprintf (exls_rec.code, "%-2.2s", key_val);

	cc = find_rec (exls, &exls_rec, GTEQ, "r");

	while (!cc && 
		   !strncmp (exls_rec.code, key_val, strlen (key_val)) && 
		   !strcmp (exls_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (exls_rec.code, exls_rec.description);
		if (cc)
			break;

		cc = find_rec (exls, &exls_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exls_rec.co_no, comm_rec.co_no);
	sprintf (exls_rec.code, "%-2.2s", temp_str);

	cc = find_rec (exls, &exls_rec, COMPARISON, "r");

	if (cc)
		file_err (cc, exls, "DBFIND");
}

int
heading (
	int 	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlSoMess179), 28, 0, 1);
		move (0,1);
		line (80);

		box (0, 3, 80, 3);

		move (0, 20);
		line (80);
		print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		move (0, 22);
		line (80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
