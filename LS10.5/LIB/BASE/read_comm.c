/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : read_comm.c                                    |
|  Source Desc       : Reads database file comm.                      |
|                                                                     |
|  Library Routines  : read_comm()                                    |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|---------------------------------------------------------------------|
|  Date Modified     : 10/03/93 | Modified by : Jonathan Chen         |
|                                                                     |
|  Comments          :                                                |
|         (10/03/93) : GV_fiscal now set in a more anonymous manner   |
|                    :                                                |
|                                                                     |
$Log: read_comm.c,v $
Revision 5.0  2001/06/19 06:59:38  cha
LS10-5.0 New Release as of 19 JUNE 2001

Revision 4.0  2001/03/09 00:52:39  scott
LS10-4.0 New Release as at 10th March 2001

Revision 3.0  2000/10/12 13:34:26  gerry
Revision No. 3 Start
<after Rel-10102000>

Revision 2.0  2000/07/15 07:17:18  gerry
Forced revision no. to 2.0 - Rel-15072000

Revision 1.4  1999/10/04 03:45:20  jonc
Update comm_rec buffer as a pointer to anything.

Revision 1.3  1999/09/13 09:36:33  scott
Updated for Copyright

Revision 1.2  1999/09/13 06:20:49  alvin
Check-in all ANSI modifications made by Trev.

Revision 1.1.1.1  1999/06/10 11:56:34  jonc
Initial cutover from SCCS.

 * Revision 1.2  93/05/13  12:00:54  jonc
 * Removed calls to dbfind() and replaced with std interfaces calls
 * 
 * Revision 1.1  93/05/13  11:47:27  jonc
 * Initial revision
 * 
=====================================================================*/
#include	<std_decs.h>

int	GV_fiscal = 0;

/*
.function
	Function	:	read_comm ()

	Description	:	Get common info from commom database file.

	Notes		:	Read_comm reads the comm record for the terminal
				running the calling application.
				If the comm_fiscal field appears in the view
				passed to this routine the global variable
				GV_fiscal is set.

	Parameters	:	comm_view - Informix dbview.
				comm_no	  - Number of fields in view.
				obj	  - Pointer to object into which data
					    is to be loaded.

	Globals		:	GV_fiscal - End month of the fiscal year.
					    (1 - 12)
.end
*/
void
read_comm (
 struct dbview *comm_view,
 int comm_no,
 void * obj)
{
	int	i;
	char	*comm = "comm",
		*comm_term = "comm_term";

	/* Find comm_term field and set it
	*/
	for (i = 0; i < comm_no; i++)
		if (!strcmp (comm_view [i].vwname, comm_term))
		{
			*(int *) ((char *) obj + comm_view [i].vwstart) = ttyslt ();
			break;
		}
	if (i >= comm_no)
		file_err (i, comm, "No comm_term field");

	/* Extract comm record
	*/
	open_rec (comm, comm_view, comm_no, comm_term);
	if ((i = find_rec (comm, obj, EQUAL, "r")))
		file_err (i, comm, "find_rec");
	abc_fclose (comm);

	/* Set GV_fiscal
	*/
	for (i = 0; i < comm_no; i++)
		if (!strcmp (comm_view [i].vwname, "comm_fiscal"))
		{
			GV_fiscal = *(int *) ((char *) obj + comm_view [i].vwstart);
			break;
		}
}
