#include	<std_decs.h>

void
ArrAlloc (
 DArray	*state,
 void	*arr,			/* pointer to start of array */
 size_t	rowsz,
 size_t	rows)
{
	*(void **) arr = malloc (rowsz * rows);

	if (!*(void **) arr)
	{
		fprintf (stderr,
			"ArrAlloc: Can't allocate enough memory\n");
		exit (EXIT_FAILURE);
	}
	memset (*(void **) arr, 0, rowsz * rows);
	memset (state, 0, sizeof (DArray));

	state -> array = arr;
	state -> rowsz = rowsz;
	state -> init = rows > 2 ? rows : 2;
	state -> rows = rows;
}

void
ArrDelete (
 DArray	*state)
{
	free (*(void **) state -> array);		/* free up the memory */
	*(void **) state -> array = NULL;		/* zap to NULL to discourage use */
}

int
ArrChkLimit (
 DArray	*state,
 void	*arr,
 int	idx)
{
	/*
	 *	Check index against the array, reallocate a larger
	 *	size if necessary
	 */
	void	*newblk;
	int		newrowcnt;

	if (arr != *(void **) state -> array)	/* sanity check */
	{
		fprintf (stderr,
			"ArrChkLimit: inconsistency between array and DArray\n");
		exit (EXIT_FAILURE);
	}
	if (idx < state -> rows)
		return (TRUE);

	/*
	 *	Allocate half again the original amount
	 */
	newrowcnt = state -> rows + state -> init / 2;
	if (newrowcnt <= idx)
		newrowcnt = idx + 1;
	newblk = realloc (arr, newrowcnt * state -> rowsz);
	if (!newblk)
		return (FALSE);

	*(void **) state -> array = newblk;		/* shove the new array in */
	state -> rows = newrowcnt;
	return (TRUE);
}
