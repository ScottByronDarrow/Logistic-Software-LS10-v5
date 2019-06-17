/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( so_cucr_inp.c  )                                 |
|  Program Desc  : ( Maintain Customer Credit Return Description. )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cucr,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  cucr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 23/03/89         |
|---------------------------------------------------------------------|
|  Date Modified : (23/03/89)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (28/08/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (01/10/97)      | Modified  by  : Marnie Organo.   | 
|                                                                     |
|  Comments      : (28/08/90) - General Update for New Scrgen. S.B.D. |
|                : (01/10/96) - Updated for Multilingual Conversion.  | 
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cucr_inp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_cucr_inp/cucr_inp.c,v 5.5 2002/07/24 08:39:24 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
	};
	
	int comm_no_fields = 3;
	
	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
	} comm_rec;

	/*=
	|  |
	=*/
	struct dbview cucr_list[] ={
		{"cucr_co_no"},
		{"cucr_code"},
		{"cucr_desc"}
	};

	int cucr_no_fields = 3;

	struct {
		char	cr_co_no[3];
		char	cr_code[2];
		char	cr_desc[17];
	} cucr_rec;

	struct	storeRec {
		char	cucr_codes [2];
	} store [MAXLINES];

/*============================ 
| Local & Screen Structures. |
============================*/ 
struct {
	char	dummy[11];
} local_rec;

static	struct	var	vars[]	={	

	{1, TAB, "cucr_code", MAXLINES, 3, CHARTYPE, 
		"U", "          ", 
		" ", "", "Code", " ", 
		YES, NO, JUSTRIGHT, "", "", cucr_rec.cr_code}, 
	{1, TAB, "cucr_desc", 0, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Return Description", " ", 
		YES, NO, JUSTLEFT, "", "", cucr_rec.cr_desc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int  spec_valid (int field);
void load_cucr (void);
void update (void);
int  heading (int scn);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR (vars);


	init_scr();
	set_tty();
	set_masks();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (1, store, sizeof (struct storeRec));
#endif
	init_vars(1);

	OpenDB();

	entry_exit = 0;
	edit_exit = 0;
	prog_exit = 0;
	restart = 0;
	init_vars(1);

	load_cucr();

	if (lcount[1] == 0)
	{
		heading(1);
		entry(1);
        if (restart || prog_exit)  {
			shutdown_prog();
            return (EXIT_SUCCESS);
        }
	}
			
	heading(1);
	scn_display(1);
	edit(1);

    /* this logic needs to be reversed --
	if (restart)
		shutdown_prog();

	update(); 
    */
    if (!restart)
        update ();

	shutdown_prog();
    return (EXIT_SUCCESS);
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
	abc_dbopen("data");
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec("cucr",cucr_list,cucr_no_fields,"cucr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("cucr");
	abc_dbclose("data");
}

int
spec_valid (
 int field)
{
	int	i;
	int	high_val = (prog_status == ENTRY) ? line_cnt : lcount[1];
	char	*sptr;

	if (LCHECK("cucr_code"))
	{
		for (i = 0;i <= high_val;i++)
		{
			if (i == line_cnt)
				continue;

			if (store [i].cucr_codes[0] == cucr_rec.cr_code[0])
			{
				/*print_mess("Duplicate Code");*/
				print_mess(ML(mlSoMess313));
				sleep(2);
				return(1);
			}
		}

		strcpy (store [line_cnt].cucr_codes, cucr_rec.cr_code);
		return(0);
	}

	if (LCHECK("cucr_desc"))
	{
		strcpy(err_str,cucr_rec.cr_desc);
		sptr = clip(err_str);
		if (strlen(sptr) == 0)
		{
			/*print_mess(" Reason Description Cannot be Blank ");*/
			print_mess(ML(mlStdMess163));
			sleep(2);
			return(1);
		}
		return(0);
	}
	return(0);
}

void
load_cucr (
 void)
{
	scn_set(1);
	lcount[1] = 0;

	strcpy(cucr_rec.cr_co_no,comm_rec.tco_no);
	strcpy(cucr_rec.cr_code," ");
	cc = find_rec("cucr",&cucr_rec,GTEQ,"r");
	while (!cc && !strcmp(cucr_rec.cr_co_no,comm_rec.tco_no))
	{
		strcpy (store [lcount [1]].cucr_codes,cucr_rec.cr_code);
		putval (lcount[1]++);
		cc = find_rec("cucr",&cucr_rec,NEXT,"r");
	}
}

void
update (
 void)
{
	for (line_cnt = 0;line_cnt < lcount[1];line_cnt++)
	{
		getval(line_cnt);
		strcpy(cucr_rec.cr_co_no,comm_rec.tco_no);
		cc = find_rec("cucr",&cucr_rec,COMPARISON,"u");
		if (cc)
		{
			getval(line_cnt);
			strcpy(cucr_rec.cr_co_no,comm_rec.tco_no);
			cc = abc_add("cucr",&cucr_rec);
			if (cc) 
				sys_err("Error in cucr During (DBADD)",cc,PNAME);
		}
		else
		{
			getval(line_cnt);
			strcpy(cucr_rec.cr_co_no,comm_rec.tco_no);
			cc = abc_update("cucr",&cucr_rec);
			if (cc) 
				sys_err("Error in cucr During (DBUPDATE)",cc,PNAME);
			abc_unlock("cucr");
		}
	}
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		clear();
		
		rv_pr(ML(mlSoMess225),16,0,1);
		move(0,1);
		line(80);

		move(0,20);
		line(80);
		strcpy(err_str, ML(mlStdMess038));
		print_at(21,0,err_str,comm_rec.tco_no,comm_rec.tco_name);
		move(0,22);
		line(80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}
