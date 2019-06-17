#ifndef	_Array_h
#define	_Array_h
/*	$Id: Array.h,v 5.0 2002/05/08 01:50:42 scott Exp $
 *
 *	Template for simple dynamic Array class :
 *		- array of pointers to the given type
 *		- array grows dynamically
 *
 *******************************************************************************
 *	$Log: Array.h,v $
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
#include	<assert.h>

#define	DEFAULTCHUNKSZ	10

template <class T>
class Array
{
private:
	const unsigned	ChunkSz;	// Size of each Chunk
	T				*chunk;		// Contiguous chunk
	int				rows;		// current limit

public:
	//	Constructors, Destructors
	Array (
	 unsigned	suggested = DEFAULTCHUNKSZ) :
		ChunkSz (suggested),
		chunk (0),
		rows (0)
	{
	}

	virtual ~Array ()
	{
		delete [] chunk;
	}

	/*
	 *	Mutators
	 */
	virtual T &
	operator [] (
	 int i)
	{
		if (i >= rows)
		{
			unsigned	expanded = ((i / ChunkSz) + 1) * ChunkSz;
			T			*newchunk = new T [expanded];

			for (int j = 0; j < rows; j++)
				newchunk [j] = chunk [j];
			delete [] chunk;

			chunk = newchunk;
			rows = expanded;
		}

		return (chunk [i]);
	}

	/*
	 *	Accessors
	 *		- provided for const functions
	 */
	virtual T &
	Elem (
	 int	i) const
	{
		assert (i < rows);
		return (chunk [i]);
	}
};

#endif	//	_Array_h
