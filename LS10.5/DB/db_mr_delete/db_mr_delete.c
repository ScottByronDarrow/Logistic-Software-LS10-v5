/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_mr_delete.c,v 5.2 2001/11/26 06:16:06 scott Exp $
|  Program Name  : (db_mr_delete.c)
|  Program Desc  : (Delete Customers From Customer Master File)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_mr_delete.c,v $
| Revision 5.2  2001/11/26 06:16:06  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.1  2001/11/26 05:44:44  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_mr_delete.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_mr_delete/db_mr_delete.c,v 5.2 2001/11/26 06:16:06 scott Exp $";

#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_db_mess.h>

	/*
	 * Special fields and flags.
	 */
	int		pid, 
			workNo;
					
	char	filename [100];

#include	"schema"

struct cumrRecord	cumr_rec;
struct srdbRecord	srdb_rec;

	/*
	 * Work file record. 
	 */
	struct {
		long	hhcuHash;
	} customerRec;

/*
 * Local Function Prototypes.
 */
int 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	shutdown_prog 	(void);

/*
 * Main Processing Routine.
 */
int
main (
	int		argc, 
	char	*argv [])
{
	int 	i = 0, 
			deleteCnt = 0;

	char	displayCustomer [17];

	if (argc < 2) 
	{
		print_at (0, 0, mlStdMess046, argv [0]);
		return (EXIT_FAILURE);
	}
	pid = atoi (argv [1]);

	if (OpenDB ())
    {
		shutdown_prog ();
        return (EXIT_FAILURE);
    }

	init_scr ();
	set_tty ();
	clear ();

	crsr_off ();

	box (0, 1, 80, 1);
	box (7, 5, 28, 12);
	box (47, 5, 28, 12);
	line_at (7, 8, 27);
	line_at (7, 48, 27);

	sprintf (err_str, " %s ", ML (mlDbMess111));
	centre_at (2, 80, err_str);
	sprintf (err_str, " %s ", ML (mlDbMess111));
	centre_at (2, 80, err_str);

	print_at (6, 9, "%R %s ", ML (mlDbMess112));
	print_at (6, 51, "%R %s ", ML (mlDbMess113));

	i = 1, 
	deleteCnt = 1;

	cc = RF_READ (workNo, (char *) &customerRec);
    while (!cc)
    {
		cumr_rec.hhcu_hash = customerRec.hhcuHash;
		if (find_rec (cumr, &cumr_rec, COMPARISON, "r"))
		{
			cc = RF_READ (workNo, (char *) &customerRec);
			continue;
		}
		srdb_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cc = find_rec (srdb, &srdb_rec, EQUAL, "r");
		if (!cc)
		{
			cc = abc_delete (srdb);
			if (cc)
			 	file_err (cc, srdb, "DBDELETE");
		}	

		sprintf (displayCustomer, "%-6.6s %-9.9s", 
					cumr_rec.dbt_no, cumr_rec.dbt_acronym);

		sprintf (err_str, "%02d. %-12.12s", deleteCnt, displayCustomer);

		us_pr (err_str, 12, i + 7, 1);

		cc = abc_delete (cumr);
		if (cc)
			file_err (cc, cumr, "DBDELETE");
		
		us_pr (err_str, 12, i + 7, 0);
		us_pr (err_str, 53, i + 7, 1);
		line_at (i + 7, 35, 13);
		deleteCnt++;
		i++;
		if (i > 10)
		{
			for (i = 0 ; i < 10 ; i++)
			{
				print_at (i + 8, 12, "                 ");
				print_at (i + 8, 53, "                 ");
			}
			i = 1;
		}
		cc = RF_READ (workNo, (char *) &customerRec);
		
    }	/* end of input control loop	*/
	
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

int
OpenDB (void)
{
	char	*sptr = getenv ("PROG_PATH");

	sprintf (filename, "%s/WORK/cudl%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pid);

	abc_dbopen ("data");

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (srdb, srdb_list, SRDB_NO_FIELDS, "srdb_hhcu_hash");
	
	return (RF_OPEN (filename, sizeof (customerRec), "r", &workNo));
}

void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (srdb);
	abc_dbclose ("data");

	RF_DELETE (workNo);
}
