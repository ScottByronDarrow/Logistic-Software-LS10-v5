/*	$Id: LpNumber.h,v 5.0 2002/05/08 01:50:43 scott Exp $
 *
 *	List of Printers (cross-ref'd prntype & LP_SECURE)
 *
 *	(adopted from old , with bothersome design)
 *
 *******************************************************************************
 * $Log: LpNumber.h,v $
 * Revision 5.0  2002/05/08 01:50:43  scott
 * CVS administration
 *
 * Revision 4.0  2001/03/09 01:02:10  scott
 * LS10-4.0 New Release as at 10th March 2001
 *
 * Revision 3.1  2000/11/10 04:06:43  scott
 * Updated to clean code while working in format-p
 *
 * Revision 3.0  2000/10/12 13:39:02  gerry
 * Revision No. 3 Start
 * <after Rel-10102000>
 *
 * Revision 1.1.1.1  1999/07/14 23:58:50  jonc
 * Initial C++ sources (adopted minimally from V10)
 *
 */
#ifndef	_LpNumber_h
#define	_LpNumber_h

#include	<CArray.h>
#include	<String.h>

struct	tagLpInfo
{
	int		lpno;
	String	pType;
	String	qName;
	String	pDesc;
};

class LpNumber
{
	friend	class LpNumberIter;

	private:
		CArray <tagLpInfo>	lpSecured,
							lpHidden;

	public:
		LpNumber ();

		bool	Valid (int) const;

		const tagLpInfo &	LpInfo (int lpno) const;	// info for a lpno
		const tagLpInfo &	operator [] (int) const;	// access into lpList
};

/*
 *	Iterator thru' valid printers
 */
class LpNumberIter
{
	private:
		int	idx;
		const LpNumber &	src;

	public:
		LpNumberIter (const LpNumber &);

		/*
		 *
		 */
		operator			bool () const;
		const tagLpInfo &	lpInfo (void) const;

		/*
		 *	Iterator mutators
		 */
		void				Reset (void);
		void				operator ++ (int),
							operator -- (int);
};

#endif	// LpNumber_h
