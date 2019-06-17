/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( lockout_br.c   )                                 |
|  Program Desc  : ( Locks out branches once lock file created.   )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm,     ,     ,     ,     ,     ,     ,         |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (    )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 31/05/89         |
|---------------------------------------------------------------------|
|  Date Modified : (11/06/91)      | Modified  by  :                  |
|  Date Modified : (05/09/97)      | Modified  by  :Roanna Marcelino  |
|                                                                     |
|  Comments      : (11/06/91) - Updated to clean up.                  |
|                : (05/09/97) - Modified for Multilingual Conversion. |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: lockout_br.c,v $
| Revision 5.1  2001/08/09 09:14:14  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:09:40  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:52  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:44  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:02:19  gerry
| Forced Revision Start No 2.0 Rel-15072000
|
| Revision 1.5  1999/09/20 05:51:19  scott
| Updated from Ansi Project.
|
| Revision 1.4  1999/09/10 02:07:23  gerry
| SC 1772 - ANSI Compliance
|
| Revision 1.3  1999/06/15 09:39:12  scott
| Updated for log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lockout_br.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/OL/lockout_br/lockout_br.c,v 5.1 2001/08/09 09:14:14 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_ol_mess.h>

	/*=====================================
	| File comm	{System Common file}. |
	=====================================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"}
		};

	int comm_no_fields = 5;

	struct {
		int 	termno;
		char 	tco_no[3];
		char 	tco_name[41];
		char 	tes_no[3];
		char 	tes_name[41];
		} comm_rec;

#include	<std_decs.h>

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int argc,
	char *argv [])
{
	int	i;
	char	name[10];
	char	lock_type[2];

	if (argc != 2)
	{
		print_at(0,0,mlOlMess047,argv[0]);
		return (EXIT_FAILURE);
	}
	init_scr();

	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	abc_dbclose("data");

	sprintf(lock_type, "%-1.1s", argv[1]);
	
	sprintf(name, "LOCK.%02d%02d", atoi(comm_rec.tco_no),atoi(comm_rec.tes_no));
	if (lock_type[0] == 'L')
	{
		clear();
		print_at(0,0,ML(mlOlMess018),comm_rec.tes_name);
		i = creat(name,00777);
		close(i);
	}
	if (lock_type[0] == 'U')
	{
		clear();
		print_at(0,0,ML(mlOlMess019),comm_rec.tes_name);
		unlink(name);
	}
	if (lock_type[0] == 'C')
	{
		if (access(name,0) == -1) 
			return (EXIT_SUCCESS);

		clear();
		print_at(0,0,ML(mlOlMess020),clip(comm_rec.tes_name));
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

