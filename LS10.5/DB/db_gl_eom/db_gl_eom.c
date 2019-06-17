/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_gl_eom.c,v 5.3 2001/11/22 08:11:23 scott Exp $
|  Program Name  : (db_gl_eom.c)
|  Program Desc  : (Update General Ledger Controls At End Of Month)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_gl_eom.c,v $
| Revision 5.3  2001/11/22 08:11:23  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_gl_eom.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_gl_eom/db_gl_eom.c,v 5.3 2001/11/22 08:11:23 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>

#include	"schema"

struct commRecord	comm_rec;
struct gljcRecord	gljc_rec;

	double	*gljc_totals	=	&gljc_rec.tot_1;

	char	*data = "data";

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	GljcUpdate 		(void);
void 	shutdown_prog 	(void);

int
main (
	int		argc,
	char	*argv [])
{
	OpenDB ();

	sprintf (err_str, "Clearing G/L Controls Totals For Company %s",comm_rec.co_no);
	dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

	GljcUpdate ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

    read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (gljc, gljc_list, GLJC_NO_FIELDS, "gljc_co_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (gljc);
	abc_dbclose (data);
}

/*===============================
| Set control totals to zero  . |
===============================*/
void
GljcUpdate (void)
{
	int	j_type;

	strcpy (gljc_rec.co_no,comm_rec.co_no);

	cc = find_rec (gljc, &gljc_rec, GTEQ, "u");
	while (!cc && !strcmp (gljc_rec.co_no,comm_rec.co_no))
	{
		j_type = atoi (gljc_rec.journ_type);

		if (( (j_type > 3 && j_type < 7) || j_type == 20))
		{
		    gljc_totals [0] = 0.0;
		    gljc_totals [1] = 0.0;
		    gljc_totals [2] = 0.0;
		    gljc_totals [3] = 0.0;
		    gljc_totals [4] = 0.0;
		    gljc_totals [5] = 0.0;
		
		    dsp_process ("Control Total # ", gljc_rec.journ_type);

		    if ((cc = abc_update (gljc, &gljc_rec)))
				file_err (cc, gljc, "DBUPDATE");
		}
		cc = find_rec (gljc, &gljc_rec, NEXT, "u");
	}
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}
