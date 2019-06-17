#ifndef SEARCH_UTILS_H

#define SEARCH_UTILS_H

/*==================
| Global functions |
==================*/
int		work_open (void);
int		_work_open (int, int, int);
int		save_rec (char * field1, char *	field2);
void	save_column_heading (char *	field1, char * field2);
int		save_file_header (unsigned short keylen, unsigned short	desclen);

int		display_item (int idxItem, char * key, char * desc);

int		extractHeaders (char * line[], int numItems, char *	key, char *	buffer);
int 	Logistic_work_open (void * fn);
int		Logistic_RF_OPEN (void);
void	Logistic_work_close (void);
int		Logistic_save_rec (char * field1, char * field2);
int		Logistic_disp_srch (void);

#endif /*SEARCH_UTILS_H*/
