#ifndef	_InventorySorting_h
#define	_InventorySorting_h
/*	$Id: InventorySorting.h,v 5.0 2001/06/19 06:51:17 cha Exp $
 *
 * Inventory sorting. Based on the standard sort file stuff.
 *
 * See below.
 *
 *******************************************************************************
 *	$Log: InventorySorting.h,v $
 *	Revision 5.0  2001/06/19 06:51:17  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 00:59:20  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/12 13:28:51  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 07:15:34  gerry
 *	Force revision no. to 2.0 - Rel-15072000
 *	
 *	Revision 1.1.1.1  1999/06/10 11:56:33  jonc
 *	Initial cutover from SCCS.
 *	
 *	Revision 2.1  1998/01/21 23:50:01  jonc
 *	Updated to version 2, no changes
 *	
 *	Revision 1.2  1998/01/21 23:46:42  colin
 *	Added.
 *
 */

/*
 *	Prototypes
 */
				/* add to OpenDb - to open tables etc */
extern void		OpenInventorySorting (void);

				/* create a new inventory sort file */
extern FILE *	NewInventorySortFile (char * fileName);

				/*	add the item to the sort file created
				 	with NewInventorySortFile()
				 */
extern void		AddToInventorySortFile (FILE *, long hhbrHash, char * text);

				/* returns the text used for
					sorting in case you need to do it yourself
				 */
extern char *	InventorySortValue (long);
extern FILE *	SortInventorySortFile (FILE * file, char * fileName);
extern long		ReadInventorySortFile (FILE * file, char * buffer);
extern void		DeleteInventorySortFile (FILE * file, char * fileName);
extern void		CloseInventorySorting (void);

#endif	/*_InventorySorting_h*/
