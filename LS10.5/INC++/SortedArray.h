#ifndef	_SortedArray_h
#define	_SortedArray_h
/*
 *	Sorted Contiguous Array.
 *
 *		An increasing ordered array of elements
 *		sorted via Insertion sort
 *
 *******************************************************************************
 *	$Log: SortedArray.h,v $
 *	Revision 5.0  2002/05/08 01:50:43  scott
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
 */
#include	<CArray.h>

template <class T>
class SortedArray :
	public CArray <T>
{
	private:
		/*
		 *	Changed the access permission on this to prevent
		 *	its use by applications
		 */
		virtual SortedArray &
		Insert (
		 int		slot,
		 const T &	elem)
		{
			CArray <T>::Insert (slot, elem);
			return (*this);
		}

	public:
		virtual SortedArray &
		Add (
		 const T &	elem)
		{
			/*
			 *	Find the appropriate slot to add this element
			 */
			int	slot, count = Count ();

			for (slot = 0; slot < count; slot++)
				if (Elem (slot) > elem)
					break;
			return (Insert (slot, elem));
		}
};

#endif	//_SortedArray_h
