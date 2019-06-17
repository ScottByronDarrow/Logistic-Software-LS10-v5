#ifndef	DICT_H
#define	DICT_H
/*******************************************************************************
$Header: /usr/LS10/REPOSITORY/LS10.5/LIB/SQLIF/dict.h,v 5.0 2001/06/19 07:14:40 cha Exp $

	Prototypes for misc functions

$Log: dict.h,v $
Revision 5.0  2001/06/19 07:14:40  cha
LS10-5.0 New Release as of 19 JUNE 2001

Revision 4.0  2001/03/09 02:28:06  scott
LS10-4.0 New Release as at 10th March 2001

Revision 2.0  2000/07/15 07:35:04  gerry
Forced Revision No. Start to Rel-15072000

Revision 1.1.1.1  1999/06/10 11:56:34  jonc
Initial cutover from SCCS.

 * Revision 1.1  93/05/05  16:31:50  jonc
 * Initial revision
 * 

*******************************************************************************/

struct tagDictionary
{
	char	*entry,			/*	entered as	*/
		*really;		/*	really means	*/

	struct tagDictionary	*next;
};
typedef	struct tagDictionary	Dictionary;

extern int	AddEntry (char *, char *);
extern void	DelDictionary ();
extern char	*GetMeaning (char *);

#endif	/*DICT_H*/
