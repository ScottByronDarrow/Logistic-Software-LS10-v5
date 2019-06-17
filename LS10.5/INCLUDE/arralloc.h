#ifndef	_arralloc_h
#define	_arralloc_h

typedef
struct
{
	void	*array;
	size_t	init,
			rows,
			rowsz;
}	DArray;

/*
 *	Prototypes
 */
extern	void	ArrAlloc (DArray *, void *array, size_t rowsz, size_t rows),
				ArrDelete (DArray *);
extern int		ArrChkLimit (DArray	*, void *arr, int idx);

#endif	/*_arralloc_h*/
