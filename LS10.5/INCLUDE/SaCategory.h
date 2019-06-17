#include	<malloc.h>

struct listRec {
	char	catStart [12];
	char	catEnd [12];
	double	sumData [8];
	double	sumUnit [4];
	char	catStatus [2];
	struct	listRec	*nextRecord;
} listRec;

struct	listRec		*headPtr;
struct	listRec		*currPtr;
void	LoadListStruct 		(void);
void 	EndListStruct 		(char *, char *);
int		FindListStruct 		(char *, int);
void	PrintListStruct 	(void);

struct	listRec	*ListAllocate (void)
{
	return ( (struct listRec *) malloc (sizeof (struct listRec)));
}

/*
 * Loads sasr records into list
 */
void
LoadListStruct (void)
{
	int	err;

	open_rec (sasr, sasr_list, SASR_NO_FIELDS, "sasr_id_no");
	strcpy (sasr_rec.co_no, comm_rec.co_no);
	sprintf (sasr_rec.start_cat, "%-11.11s", " ");
	sprintf (sasr_rec.end_cat, "%-11.11s", " ");
	err = find_rec (sasr, &sasr_rec, GTEQ, "r");
	while (!err && !strcmp (sasr_rec.co_no, comm_rec.co_no))
	{
		EndListStruct (sasr_rec.start_cat, sasr_rec.end_cat);
		err = find_rec (sasr, &sasr_rec, NEXT, "r");
	}
	abc_fclose (sasr);
}

/*
 * Put categories a alphabetically sorted, linked list.	
 */
void
EndListStruct (
	char	*start_cat, 
	char	*end_cat)
{
	struct	listRec	*tempPtr;
	struct	listRec	*listPtr = ListAllocate ();
	int	i;

	sprintf (listPtr->catStart,	"%-11.11s", start_cat);
	sprintf (listPtr->catEnd, 	"%-11.11s", end_cat);
	strcpy (listPtr->catStatus, "Y");
	for (i = 0;i < 8;i++)
	{
		listPtr->sumData [i] = 0.00;
		if (i < 4)
			listPtr->sumUnit [i] = 0.00;
	}

	/*
	 * First addition	
	 */
	if (headPtr == NULL)
	{
		listPtr->nextRecord = NULL;
		headPtr = listPtr;
	}
	else
	{
		/*
		 * Put at end of list- records already sorted	
		 */
		tempPtr = headPtr;
		while (tempPtr->nextRecord != NULL)
			tempPtr = tempPtr->nextRecord;

		tempPtr->nextRecord = listPtr;
		listPtr->nextRecord = NULL;
	}
}

/*
 * find ftype record in list where curr_cat is in subrange
 * ie curr_list->catStart <= curr_cat <= curr_list->catEnd	
 * returns:							
 *	FALSE	- not found (or end of list).			
 *	TRUE	- record found, curr_list points to it		
 */
int
FindListStruct (char *curr_cat, int ftype)
{
	if (ftype == FIRST)
		currPtr = headPtr;
	else
		currPtr = currPtr->nextRecord;

	while (currPtr != NULL && (strcmp (curr_cat, currPtr->catStart) < 0  || strcmp (curr_cat, currPtr->catEnd) > 0))
		currPtr = currPtr->nextRecord;

	if (currPtr == NULL)
		return (1);

	/*
	 * Not in subrange	
	 */
	if (strcmp (curr_cat, currPtr->catStart) < 0)
		return (2);

	if (strcmp (currPtr->catEnd, curr_cat) < 0)
		return (3);

	return (0);
}

/*
 * Print the contents for the list	- generally for debugging only.	
 */
void
PrintListStruct (void)
{
	struct	listRec	*tempPtr = currPtr;

	while (tempPtr != NULL)
	{
		printf (" [%s] [%s]\n", tempPtr->catStart, tempPtr->catEnd);
		fflush (stdout);
		tempPtr = tempPtr->nextRecord;
	}
	printf ("**************************\n");
}
