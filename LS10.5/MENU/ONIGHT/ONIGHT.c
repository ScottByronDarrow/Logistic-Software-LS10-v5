/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( ONIGHT.c       )                                 |
|  Program Desc  : ( Over Night Processing Processor.             )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written  : (10/05/86)      | Author       : Scott Darrow.     | 
|---------------------------------------------------------------------|
|  Date Modified : (10/05/86)      | Modified  by : Scott B. Darrow.  |
|  Date Modified : (07/04/88)      | Modified  by : Roger Gibbison.   |
|  Date Modified : (23/05/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (02/10/92)      | Modified  by : Trevor van Bremen |
|                                                                     |
|  Comments      : Added call to re_select                            |
|  (23/05/92)    : Updated for compatibility with RiscOs 5.0          |
|                : Also, generally tidy up.                           |
|  (02/10/92)    : Rename output file to night_repts.sh in SCRIPT dir |
|                : S/C PSL 7834.                                      |
|                                                                     |
| $Log: ONIGHT.c,v $
| Revision 5.1  2001/08/09 05:13:13  scott
| Updated to use FinishProgram ();
|
| Revision 5.0  2001/06/19 08:07:54  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:13  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:48  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:00  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.7  2000/02/18 01:56:17  scott
| Updated to fix small warnings found when compiled under Linux
|
| Revision 1.6  1999/12/06 01:47:06  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.5  1999/11/16 09:41:53  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.4  1999/09/29 10:10:55  scott
| Updated to be consistant on function names.
|
| Revision 1.3  1999/09/17 07:26:49  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.2  1999/06/15 02:31:44  scott
| update to add log file + change database name etc.
|
=====================================================================*/
#define	CCMAIN

char	*PNAME = "$RCSfile: ONIGHT.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/ONIGHT/ONIGHT.c,v 5.1 2001/08/09 05:13:13 scott Exp $";

#include	<pslscr.h>

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_est_no"},
		{"comm_cc_no"},
		{"comm_dp_no"},
	};

	int comm_no_fields = 5;

	struct
	{
		int	termno;
		char	tco_no[3];
		char	test_no[3];
		char	tcc_no[3];
		char	tdp_no[3];
	} comm_rec;

	char	*comm	= "comm",
			*data	= "data";

FILE	*stat_out;
void	OpenDB(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	int	i;

	OpenDB ();
	/*-------------------------------------------------
	| Open Night Reports File and add to end of file. |
	-------------------------------------------------*/
	if ((stat_out = fopen ("SCRIPT/night_repts.sh", "a")) == NULL)
	{
		fclose (stat_out);
		return EXIT_SUCCESS;
	}

	/*-------------------------------------------------
	| Send Program Description To First Line of file. |
	-------------------------------------------------*/
	fputs ("# ", stat_out);

	strcpy (temp_str, argv[argc - 1]);
 		fputs (temp_str, stat_out);

	/*----------------------
	| Send a New Line Out. |
	----------------------*/
	fputs ("\n", stat_out);

	/*-------------------------------
	| Write "re_select" bit to file	|
	-------------------------------*/
	sprintf
	(
		temp_str, "re_select \"%s\" \"%s\" \"%s\" \"%s\" ; ",
		comm_rec.tco_no,
		comm_rec.test_no,
		comm_rec.tcc_no,
		comm_rec.tdp_no
	);

	fputs (temp_str, stat_out);

	/*----------------------------------
	| Send Program Name First to file. |
	----------------------------------*/
	fputs (argv[1], stat_out);

	/*-------------------------------
	| Send Argument List in Quotes. |
	-------------------------------*/
	for (i = 2 ; i < argc - 1; i++)
	{
		sprintf (temp_str," \"%s\"", argv[i]);
 		fputs (temp_str, stat_out);
	}

	/*----------------------
	| Send a New Line Out. |
	----------------------*/
	fputs ("\n", stat_out);

	/*----------------------------
	| Close Night Reports File . |
	----------------------------*/
	fclose (stat_out);
	return EXIT_SUCCESS;
}

/*============================================ 
| Get common info from commom database file. |
============================================*/
void
OpenDB(void)
{
	abc_dbopen (data);

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	abc_dbclose (data);
}
