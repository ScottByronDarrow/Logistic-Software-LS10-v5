/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (ol_upd.c       )                                  |
|  Program Desc  : (Display Branch Status (Online / Offline).    )    |
|                  (                                             )    |
|---------------------------------------------------------------------|
|  Access files  :  esmr,                                             |
|  Database      : (oldb)                                             |
|---------------------------------------------------------------------|
|  Author        : Huon Butterworth| Date Written  : 06/04/89         |
|---------------------------------------------------------------------|
|  Date Modified : (06/04/89)      | Modified  by  : Huon Butterworth |
|                : (03/03/1993)    | Modified  by  : Jonathan Chen    |
|                : (11/09/1997)    | Modified  by  : Roanna Marcelino |
|                                                                     |
|  Comments      :                                                    |
|   (03/03/1993) : PSL 8260 - replaced outdated lib calls.            |
|                  Also mods to put app in line with std psl app stds |
|   (11/09/1997) : Modify for Multilingual Conversion.                |
|                                                                     |
| $Log: ol_upd.c,v $
| Revision 5.1  2001/08/09 09:14:23  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:09:54  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:03  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:52  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:02:26  gerry
| Forced Revision Start No 2.0 Rel-15072000
|
| Revision 1.7  1999/11/08 08:42:27  scott
| Updated due to warning errors when compiling using -Wall flag.
|
| Revision 1.6  1999/09/29 10:11:29  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/20 05:51:24  scott
| Updated from Ansi Project.
|
| Revision 1.4  1999/09/10 02:11:27  gerry
| SC 1772 - ANSI Compliance
|
| Revision 1.3  1999/06/15 09:39:18  scott
| Updated for log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ol_upd.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/OL/ol_upd/ol_upd.c,v 5.1 2001/08/09 09:14:23 scott Exp $";

#define	NO_SCRGEN
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_ol_mess.h>

/*==========================
| Common Record Structure. |
==========================*/
static	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"}
	};

static	int comm_no_fields = 6;

static	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		long	t_dbt_date;
	} comm_rec;

	/*=====================
	| Branch Master file. |
	=====================*/
	struct dbview esmr_list[] ={
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_online"}
	};

	int esmr_no_fields = 3;

	struct {
		char	br_co_no[3];
		char	br_br_no[3];
		int	br_online;
	} esmr_rec;

/*==========
 Table Names
===========*/
static char
	*data	= "data",
	*comm	= "comm",
	*esmr	= "esmr";

/*=====================
 Function declarations
=======================*/
#include	<std_decs.h>
int main (int argc, char *argv []);
void OpenDB (void);
void CloseDB (void);
void process (char *u_type);

int
main 
(
	int argc, 
	char *argv []
)
{
	if (argc != 2)
	{
		print_at (0,0,mlOlMess032, argv[0]);
		return (EXIT_FAILURE);
	}

	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	process (argv[1]);

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
OpenDB 
(
	void
)
{
	abc_dbopen (data);
	open_rec (esmr,esmr_list,esmr_no_fields,"esmr_id_no");
	open_rec (comm,comm_list,comm_no_fields,"comm_term");
}

void
CloseDB 
(
	void
)
{
	abc_fclose (esmr);
	abc_fclose (comm);
	abc_dbclose (data);
}

void
process 
(
	char *u_type
)
{
	strcpy (esmr_rec.br_co_no, comm_rec.tco_no);
	strcpy (esmr_rec.br_br_no, comm_rec.test_no);

	if ((cc = find_rec (esmr,&esmr_rec,GTEQ,"u")))
		file_err (cc, "esmr", "DBFIND");

	else if (*u_type == 'D' && esmr_rec.br_online <= 0)
		file_err (1,"Error during esmr Cannot decrease below 0",PNAME);

	esmr_rec.br_online += (*u_type == 'I') ? 1 : -1;

	abc_unlock (esmr);
	if ((cc = abc_update (esmr, &esmr_rec)))
		file_err (cc, "esmr", "DBUPDATE");
}

