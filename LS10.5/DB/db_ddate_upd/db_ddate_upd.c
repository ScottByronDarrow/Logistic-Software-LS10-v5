/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
| $Id: db_ddate_upd.c,v 5.2 2001/08/09 08:23:46 scott Exp $
-----------------------------------------------------------------------
| $Log: db_ddate_upd.c,v $
| Revision 5.2  2001/08/09 08:23:46  scott
| Added FinishProgram ();
|
| Revision 5.1  2001/08/06 23:21:57  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:04:26  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:24:53  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:13:43  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:52:26  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.2  1999/11/25 04:20:06  scott
| Updated from old program for NZ client upgrades.
|
*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_ddate_upd.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_ddate_upd/db_ddate_upd.c,v 5.2 2001/08/09 08:23:46 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

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
	};

	int comm_no_fields = 7;

	struct {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tco_short[16];
		char	test_no[3];
		char	test_name[41];
		char	test_short[16];
	} comm_rec;

	/*================================================
	| Customer Invoice Accounting Invoice/Credit file |
	================================================*/
	struct dbview cuin_list [] =
	{
		{"cuin_hhci_hash"},
		{"cuin_inv_no"},
		{"cuin_date_of_inv"},
		{"cuin_pay_terms"},
		{"cuin_due_date"},
	};

	int	cuin_no_fields = 5;

	struct tag_cuinRecord
	{
		long	hhci_hash;
		char	inv_no [9];
		long	inv_date;
		char	pay_terms [4];
		long	due_date;
	} cuin_rec;

	char	*data	= "data",
			*cuin	= "cuin";
		
	void	OpenDB (void);
	void	CloseDB (void);
	void	ProcessFile (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	OpenDB ();

	read_comm(comm_list, comm_no_fields, &comm_rec);

	dsp_screen
	(
		"Processing : Updating Invoice Due Dates.", 
		comm_rec.tco_no,
		comm_rec.tco_name
	);
	ProcessFile ();

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}
/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);
	open_rec (cuin, cuin_list, cuin_no_fields, "cuin_hhci_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose( cuin);
	abc_dbclose (data);
}

void
ProcessFile (void)
{
	cuin_rec.hhci_hash = 0L;
	cc = find_rec(cuin, &cuin_rec, GTEQ, "u");
	while (!cc)
	{
		dsp_process("Customer No. : ", cuin_rec.inv_no);

		cuin_rec.due_date = CalcDueDate 
							(
								cuin_rec.pay_terms, 
								cuin_rec.inv_date
							);
		cc = abc_update (cuin, &cuin_rec);
		if (cc)
			file_err (cc, cuin, "DBUPDATE");

		cc = find_rec(cuin, &cuin_rec, NEXT, "u");
	}
}
