/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : (cr_gl_eom.c  )
|  Program Desc  : (Update General Ledger Control Totals for EOM) 
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: cr_gl_eom.c,v $
| Revision 5.3  2001/12/06 09:04:15  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.2  2001/12/06 08:54:17  kaarlo
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_gl_eom.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_gl_eom/cr_gl_eom.c,v 5.3 2001/12/06 09:04:15 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>

#include    "schema"

struct commRecord   comm_rec;
struct gljcRecord   gljc_rec;

/*
 * Local function prototypes
 */
void	OpenDB		 (void);
void	CloseDB	 	 (void);
void	UpdateGljc	 (void);

int
main (
 int	argc,
 char *	argv [])
{
	OpenDB ();

	sprintf (err_str,"Clearing G/L Control Totals %s",comm_rec.co_no);
	dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

	UpdateGljc ();
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (gljc, gljc_list, GLJC_NO_FIELDS, "gljc_co_no");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (gljc);
	abc_dbclose ("data");
}

/*
 * Set control totals to zero. 
 */
void
UpdateGljc (void)
{
	int	j_type;

	strcpy (gljc_rec.co_no,comm_rec.co_no);

	cc = find_rec (gljc,&gljc_rec,GTEQ,"u");

	while (!cc && !strcmp (gljc_rec.co_no,comm_rec.co_no))
	{
		j_type = atoi (gljc_rec.journ_type);

		if (j_type > 6 && j_type < 10) 
		{
		    gljc_rec.tot_1 = 0.0;
		    gljc_rec.tot_2 = 0.0;
		    gljc_rec.tot_3 = 0.0;
		    gljc_rec.tot_4 = 0.0;
		    gljc_rec.tot_5 = 0.0;
		    gljc_rec.tot_6 = 0.0;

		    cc = abc_update (gljc,&gljc_rec); 
		    if (cc)
			   file_err (cc, gljc, "DBUPDATE");
		}
		cc = find_rec (gljc,&gljc_rec,NEXT,"u");
	}
	abc_unlock (gljc);
}
