/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: auto_des.c,v 5.1 2001/08/09 09:20:41 scott Exp $
|  Program Name  : (so_auto_des.c )                               
|  Program Desc  : (Despatch Confirmation.                      )
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow. | Date Written  : 19/10/88         |
|---------------------------------------------------------------------|
| $Log: auto_des.c,v $
| Revision 5.1  2001/08/09 09:20:41  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:18:39  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/05/02 04:05:29  scott
| Updated to use so_AutoDesConf as silly to maintain two automatic dispatch programs
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: auto_des.c,v $", 
		*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_auto_des/auto_des.c,v 5.1 2001/08/09 09:20:41 scott Exp $";

#include	<pslscr.h>

#include 	<dsp_screen.h>
#include 	<dsp_process.h>

	int		dispatchOpen			= FALSE;
	char	createFlag [2];

#include	"schema"

struct commRecord	comm_rec;
struct cohrRecord	cohr_rec;

	char	*data 	= "data";

	FILE	*soAutoDesConf;

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ProcessCohr 		(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc < 2)
	{
		print_at (0,0,"Usage <CreateFlag>",argv [0]);
		return (EXIT_FAILURE);
	}

	sprintf (createFlag, "%-1.1s", argv [1]);

	/*---------------------------
	| open main database files. |
	---------------------------*/
	OpenDB ();

	dsp_screen ("Automatic Sales Order Update",
					comm_rec.co_no, comm_rec.co_name);
	ProcessCohr ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence . |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open Database Files . |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no2");
}

/*========================
| Close Database Files . |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (cohr);
	abc_dbclose (data);
}

void
ProcessCohr (
 void)
{
	strcpy (cohr_rec.co_no, comm_rec.co_no);
	strcpy (cohr_rec.br_no, comm_rec.est_no);
	strcpy (cohr_rec.type, "P");
	strcpy (cohr_rec.inv_no, "        ");

	cc = find_rec (cohr, &cohr_rec, GTEQ, "r");
	while (!cc && !strcmp (cohr_rec.co_no, comm_rec.co_no) && 
		      !strcmp (cohr_rec.br_no, comm_rec.est_no) && 
		      cohr_rec.type [0] == 'P')
	{
		if (dispatchOpen == FALSE)
		{
			/*-----------------------------------
			| Start Out Put To Standard Print . |
			-----------------------------------*/
			if ((soAutoDesConf = popen ("so_autoDesConf", "w")) == 0)
				file_err (errno, "so_autoDesConf", "POPEN");

			dispatchOpen = TRUE;
		}
		dsp_process ("Packing Slip", cohr_rec.inv_no);
		
		fprintf (soAutoDesConf, "%s\n", createFlag);
		fprintf (soAutoDesConf, "%ld\n", cohr_rec.hhco_hash);
		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}
	if (dispatchOpen == TRUE)
		pclose (soAutoDesConf);
}
