/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_rec_gl.c    )                                 |
|  Program Desc  : ( Stock Receipts Suspended Audit Print.        )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm,                                             |
|  Database      : (stck)                                             |
|---------------------------------------------------------------------|
|  Author        : Huon Butterworth| Date Written  : 28/04/89         |
|---------------------------------------------------------------------|
|  Comments      :                                                    |
|  (29/02/96)    : Modified by Andy Yuen                              |
|                  Modified heading comments and removed unused       |
|                  db structure.  No program changes.                 |
| $Log: sk_sprint.c,v $
| Revision 5.1  2001/08/09 09:14:24  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:09:56  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:04  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:52  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:02:27  gerry
| Forced Revision Start No 2.0 Rel-15072000
|
| Revision 1.6  1999/09/20 05:51:25  scott
| Updated from Ansi Project.
|
| Revision 1.5  1999/09/10 02:11:54  gerry
| SC 1772 - ANSI Compliance
|
| Revision 1.4  1999/06/15 09:39:18  scott
| Updated for log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_sprint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/OL/sk_sprint/sk_sprint.c,v 5.1 2001/08/09 09:14:24 scott Exp $";

#define		NO_SCRGEN
#include <pslscr.h>
/*
#include <sys/types.h>
#include <sys/stat.h>
*/

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_cc_name"},
		{"comm_cc_short"},
		{"comm_inv_date"},
		{"comm_gl_date"}
	};
	
	int comm_no_fields = 12;
	
	struct {
		int  	termno;
		char 	tco_no[3];
		char 	tco_name[41];
		char 	tco_short[16];
		char 	tes_no[3];
		char 	tes_name[41];
		char 	tes_short[16];
		char 	tcc_no[3];
		char 	tcc_name[41];
		char 	tcc_short[10];
		long 	tinv_date;
		long 	tgl_date;
	} comm_rec;

#include	<std_decs.h>
void pr_audit (void);

int
main (
	int argc,
	char *argv [])
{
	abc_dbopen ("data");
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);
	pr_audit ();
	abc_dbclose ("data");
	return (EXIT_SUCCESS);
}

void
pr_audit (
	void)
{
	char	froot [16],
		fname [128],
		tmp_co [3],
		tmp_est [3];
	char	sys_str [BUFSIZ];

	strcpy (tmp_co, comm_rec.tco_no);
	strcpy (tmp_est, comm_rec.tes_no);
	sprintf (froot, "CO%sBR%s", lclip (tmp_co), lclip (tmp_est));
	sprintf (fname, "/tmp/%s.pr", froot);

	sprintf (sys_str, "cat %s | pformat", fname);

	system (sys_str);
}

