#ifndef	_CArray_h
#define	_CArray_h
/*
 *	Contiguous Array.
 *
 *		No holes allowed.
 *
 *******************************************************************************
 *	$Log: CArray.h,v $
 *	Revision 5.0  2002/05/08 01:50:42  scott
 *	CVS administration
 *	
 *	Revision 4.0  2001/03/09 01:02:10  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.1  2000/11/10 04:06:42  scott
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
#include	<Array.h>

template <class T>
class CArray :
	public Array <T>
{
	private:
		int	elemCount;

	public:
		CArray () :
			elemCount (0)
		{
		}

		CArray (
		 const CArray <T> &	src) :
			elemCount (0)
		{
			for (int i = 0; i < src.elemCount; i++)
				Add (src.Elem (i));
		}

		int
		Count () const
		{
			return (elemCount);
		}

		void
		Clear ()
		{
			elemCount = 0;
		}

		virtual CArray &
		Add (
		 const T &	elem)
		{
			/*
			 *	Add to the top
			 */
			return (Insert (elemCount, elem));
		}

		virtual CArray &
		Dec ()
		{
			/*
			 *	Decrement the number of itmes (ie remove the top one)
			 */
			if (elemCount)
				elemCount--;
			return (*this);
		}

		virtual CArray &
		Insert (
		 int		slot,
		 const T &	elem)
		{
			/*
			 *	Insert item at given index and shift everything up
			 */
			assert (slot <= elemCount);

			for (int i = elemCount++; i > slot; i--)
			{
				/*
				 *	Take a copy of the Element to be shifted
				 *	'cos the act of moving may destroy the Elem reference
				 *	during an Array expansion
				 */
				T	item (Elem (i - 1));

				Array<T>::operator [] (i) = item;
			}
			Array <T>::operator [] (slot) = elem;

			return (*this);
		}

		virtual CArray &
		Remove (
		 int	slot)
		{
			/*
			 *	Remove item at given index and shift everything down
			 *		- this is down by overwriting the slot
			 */
			assert (slot < elemCount);

			for (int i = slot; i < elemCount - 1; i++)
				Elem (i) = Elem (i + 1);
			elemCount--;

			return (*this);
		}

		CArray &
		operator = (
		 const CArray <T> &	src)
		{
			Clear ();
			for (int i = 0; i < src.elemCount; i++)
				Add (src.Elem (i));
			return (*this);
		}

		/*
		 *	Overridden virtuals
		 */
		virtual T &
		operator [] (
		 int i)
		{
			assert (i >= 0);					// application error
			assert (i < elemCount);				// application error
			return (Array <T>::operator [] (i));
		}

		virtual T &
		Elem (
		 int	i) const
		{
			assert (i >= 0);					// application error
			assert (i < elemCount);				// application error
			return (Array <T>::Elem (i));
		}
};

#endif	//_CArray_h
