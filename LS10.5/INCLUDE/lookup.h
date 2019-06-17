#ifndef	LOOKUP_H
#define	LOOKUP_H

#ifndef IHOT_KEYS
#	include <hot_keys.h>
#endif

#define	LOOKUP		_LOOK_start (
#define	INDEX		); _LOOK_index (
#define	HEADING		); _LOOK_heading (
#define	USING		); _LOOK_fields (
#define	VIEW		); _LOOK_view (
#define	SIZE		); _LOOK_size (
#define	OBJECT		); _LOOK_object ((char *) 
#define	PRE		); _LOOK_pre (
#define	FUNCTION	); _LOOK_function (
#define	ROW		); _LOOK_row (
#define	COL		); _LOOK_col (
#define	DEPTH		); _LOOK_depth (
#define	HOTKEYS		); _LOOK_hots (
#define	PERMANENT	); _LOOK_keep (-1
#define	TEMPORARY	); _LOOK_keep (FALSE
#define	NOSCAN		); _LOOK_scan (FALSE
#define	SCAN		); _LOOK_scan (TRUE
#define	ENDLOOK		); _LOOK_up ();

#define CLEAN_LOOKUPS	_LOOK_clean ();

#define	KEY_PARTS	8

typedef	struct
{
	char	*lk_name;			/* database file name	*/
	char	*lk_index;			/* file index to use	*/
	char	*lk_list;			/* fields to display	*/
	char	*lk_head;			/* heading		*/
	char	*lk_obuf;			/* initial value buffer	*/
	char	*lk_buf;			/* work buffer		*/
	struct	dbview	*lk_view;		/* view of file		*/
	struct	dbview	*lk_key [KEY_PARTS];	/* key flds in view	*/
	int	lk_siz;				/* 			*/
	int	lk_ksiz;			/* 			*/
	int	lk_rlen;			/* 			*/
	int	lk_row;				/* 			*/
	int	lk_col;				/* 			*/
	int	lk_depth;			/* 			*/
	int	lk_keep;			/* 			*/
	int	(*lk_pre) (char *);		/* 			*/
	int	(*lk_func) (char *, char *, int *);
	struct	dbview	**lk_args;		/* 			*/
	int	lk_argc;			/* argcount		*/
	KEY_PTR lk_hots;			/*			*/

}	LOOK_STRUCT;
typedef	LOOK_STRUCT	*LOOK_PTR;

extern void	_LOOK_start (char *),
		_LOOK_index (char *),
		_LOOK_heading (char *),
		_LOOK_fields (char *),
		_LOOK_view (struct dbview *),
		_LOOK_size (int),
		_LOOK_object (char *),
		_LOOK_pre (int (*) (char *)),
		_LOOK_function (int (*) (char *, char *, int *)),
		_LOOK_row (int),
		_LOOK_col (int),
		_LOOK_depth (int),
		_LOOK_hots (KEY_PTR),
		_LOOK_keep (int),
		_LOOK_scan (int),
		_LOOK_up (void),
		_LOOK_clean (void);

#endif	/*LOOKUP_H*/
