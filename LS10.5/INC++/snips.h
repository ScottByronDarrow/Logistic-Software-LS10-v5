#ifndef	_snips_h
#define	_snips_h
/*	$Id: snips.h,v 5.0 2002/05/08 01:50:44 scott Exp $
 *
 *	Template snippets
 *
 *	$Log: snips.h,v $
 *	Revision 5.0  2002/05/08 01:50:44  scott
 *	CVS administration
 *	
 *	Revision 4.0  2001/03/09 01:02:11  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 04:06:43  scott
 *	Updated to clean code while working in format-p
 *	
 *	Revision 3.0  2000/10/12 13:39:02  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 1.1.1.1  1999/07/14 23:58:50  jonc
 *	Initial C++ sources (adopted minimally from V10)
 *	
 *	Revision 1.1.1.1  1998/01/22 00:58:28  jonc
 *	Version 10 start
 *
 *	Revision 2.0  1996/02/13 03:45:06  jonc
 *	Updated to 2.0
 *
 *	Revision 1.1  1996/02/13 03:44:32  jonc
 *	Initial cut
 *
 *	Revision 1.1  1996/01/07 21:08:48  jonc
 *	Initial revision
 *
 */
template<class T> inline T
Abs (
 const T &	v)
{
	return (v > 0 ? v : -v);
}

template<class T> inline void
swap (
 T &	a,
 T &	b)
{
	T	v = a;

	a = b;
	b = v;
}

template<class T> inline const T &
max (
 const T &	a,
 const T &	b)
{
	return (a > b ? a : b);
}

#endif	//_snips_h
