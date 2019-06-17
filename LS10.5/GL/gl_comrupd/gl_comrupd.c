/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_comrupd.c,v 5.2 2001/08/09 09:13:39 scott Exp $
|  Program Name  : ( gl_comrupd.c   )                                 |
|  Program Desc  : ( Update last year end date after year end run.   )|
|---------------------------------------------------------------------|
|  Date Written  : (24/08/89)      | Author       : Huon Butterworth  |
|---------------------------------------------------------------------|
| $Log: gl_comrupd.c,v $
| Revision 5.2  2001/08/09 09:13:39  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/07/25 02:17:43  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_comrupd.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_comrupd/gl_comrupd.c,v 5.2 2001/08/09 09:13:39 scott Exp $";

/*
 *   Include file dependencies  
 */
#include <pslscr.h>
#include <dsp_screen.h>

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;

/*
 *   Constants, defines and stuff   
 */
	char	*data = "data";

/*
 *   Local function prototypes  
 */
void	OpenDB 		(void);
void 	CloseDB 	(void);

/*
 * Main Processing Loop.
 */
int
main (
 int argc,
 char *argv[])
{
	OpenDB ();

	dsp_screen ("G/L Year End Date Stamping", 
                 comm_rec.co_no,
                 comm_rec.co_name);
	
	strcpy (comr_rec.co_no, comm_rec.co_no);
    cc = find_rec (comr, &comr_rec, COMPARISON, "u");
	if (cc)
        file_err (cc, comr, "DBFIND");

	comr_rec.yend_date = get_ybeg (comr_rec.gl_date) - 1;

    cc = abc_update (comr, &comr_rec);
	if (cc)
        file_err (cc, comr, "DBUPDATE");

	crsr_on ();
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
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (comr);
	abc_dbclose (data);
}

/* [ end of file ] */
