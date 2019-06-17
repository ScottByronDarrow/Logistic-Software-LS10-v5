#include	<malloc.h>

struct list_rec {
	char	_catg_start[12];
	char	_catg_end[12];
	double	_sum_data[8];
	double	_sum_unit[4];
	char	_catg_status[2];
	struct	list_rec	*_next;
} list_rec;

struct	list_rec	*head_ptr;
struct	list_rec	*curr_ptr;
void	load_list(void);
void 	enlist(char *, char *);
int		find_list(char *, int);
void	print_list(void);

struct	list_rec	*list_alloc (void)
{
	return((struct list_rec *) malloc(sizeof(struct list_rec)));
}

/*===============================
| Loads sasr records into list	|
===============================*/
void
load_list(void)
{
	int	err;

	open_rec("sasr",sasr_list,sasr_no_fields,"sasr_id_no");
	strcpy(sasr_rec.sr_co_no,comm_rec.tco_no);
	sprintf(sasr_rec.sr_start_cat,"%-11.11s"," ");
	sprintf(sasr_rec.sr_end_cat,"%-11.11s"," ");
	err = find_rec("sasr",&sasr_rec,GTEQ,"r");
	while (!err && !strcmp (sasr_rec.sr_co_no, comm_rec.tco_no))
	{
		enlist(sasr_rec.sr_start_cat,sasr_rec.sr_end_cat);
		err = find_rec("sasr",&sasr_rec,NEXT,"r");
	}
	abc_fclose("sasr");
}

/*=======================================================
| Put categories a alphabetically sorted, linked list.	|
=======================================================*/
void
enlist(char *start_cat, char *end_cat)
{
	struct	list_rec	*temp_ptr;
	struct	list_rec	*list_ptr = list_alloc();
	int	i;

	sprintf(list_ptr->_catg_start,"%-11.11s",start_cat);
	sprintf(list_ptr->_catg_end,"%-11.11s",end_cat);
	strcpy(list_ptr->_catg_status,"Y");
	for (i = 0;i < 8;i++)
	{
		list_ptr->_sum_data[i] = 0.00;
		if (i < 4)
			list_ptr->_sum_unit[i] = 0.00;
	}

	/*-----------------------
	| First addition	|
	-----------------------*/
	if (head_ptr == NULL)
	{
		list_ptr->_next = NULL;
		head_ptr = list_ptr;
	}
	else
	{
		/*-----------------------------------------------
		| Put at end of list- records already sorted	|
		-----------------------------------------------*/
		temp_ptr = head_ptr;
		while (temp_ptr->_next != NULL)
			temp_ptr = temp_ptr->_next;

		temp_ptr->_next = list_ptr;
		list_ptr->_next = NULL;
	}
}

/*===============================================================
| find ftype record in list where curr_cat is in subrange	|
| ie curr_list->_catg_start <= curr_cat <= curr_list->_catg_end	|
| returns:							|
|	FALSE	- not found (or end of list).			|
|	TRUE	- record found, curr_list points to it		|
===============================================================*/
int
find_list(char *curr_cat, int ftype)
{
	if (ftype == FIRST)
		curr_ptr = head_ptr;
	else
		curr_ptr = curr_ptr->_next;

	while (curr_ptr != NULL && (strcmp(curr_cat,curr_ptr->_catg_start) < 0  || strcmp(curr_cat,curr_ptr->_catg_end) > 0))
		curr_ptr = curr_ptr->_next;

	if (curr_ptr == NULL)
		return(1);

	/*-----------------------
	| Not in subrange	|
	-----------------------*/
	if (strcmp(curr_cat,curr_ptr->_catg_start) < 0)
		return(2);

	if (strcmp(curr_ptr->_catg_end,curr_cat) < 0)
		return(3);

	return(0);
}

/*=======================================
| Print the contents for the list	|
| - generally for debugging only.	|
=======================================*/
void
print_list(void)
{
	struct	list_rec	*temp_ptr = curr_ptr;

	while (temp_ptr != NULL)
	{
		printf("[%s][%s]\n",temp_ptr->_catg_start,temp_ptr->_catg_end);
		fflush(stdout);
		temp_ptr = temp_ptr->_next;
	}
	printf("**************************\n");
}
