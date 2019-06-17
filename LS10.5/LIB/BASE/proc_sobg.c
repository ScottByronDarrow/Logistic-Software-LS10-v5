/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
| Program Name : ( proc_sobg.c )                                      |
| Program Desc : ( Includes code for processing sobg records.   )     |
|                (                                              )     |
|---------------------------------------------------------------------|
|  Date Written  : (13/03/91)      | Author      : Trevor van Bremen  |
|---------------------------------------------------------------------|
|  Date Modified : (xx/xx/91)      | Modified by : Scott Darrow.      |
|  Date Modified : (16/06/93)      | Modified by : Trevor van Bremen  |
|  Date Modified : (07/07/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (11/05/94)      | Modified by : Campbell Mander.   |
|  Date Modified : (07/02/95)      | Modified by : Basil Wood         |
|  Date Modified : (09/02/95)      | Modified by : Basil Wood         |
|  Date Modified : (04/04/95)      | Modified by : Basil Wood         |
|                                                                     |
|  Comments      : (xx/xx/91) -                                       |
|  (16/06/93)    : PSL 9086. Remove malloc.h                          |
|  (07/07/93)    : PSL 8948 Moved from header code to lib module.     |
|                : Forced a system crash should it fail malloc()      |
|  (11/05/94)    : HGP 10565. Change to add_hash () to allow passing  |
|                : of a PID to be used for Real-time Committal of     |
|                : stock.                                             |
|  (07/02/95)    : PSM-V9 11674. Add add_hash_RC                      |
|  (09/02/95)    : PSM-V9 11674. sobg_list[] must be static           |
|  (04/04/95)    : PSM-V9 11802. Allow duplicate RC and RP sobg recs. |
|                :                                                    |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

#define	MAX_SOBG	500

	/*=============================+
	 | Background Processing file. |
	 +=============================*/
#define	SOBG_NO_FIELDS	9

	static struct dbview	sobg_list [SOBG_NO_FIELDS] =
	{
		{"sobg_co_no"},
		{"sobg_br_no"},
		{"sobg_type"},
		{"sobg_lpno"},
		{"sobg_hash"},
		{"sobg_hash2"},
		{"sobg_pid"},
		{"sobg_value"},
		{"sobg_last_line"}
	};

	struct tag_sobgRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	type [3];
		int		lpno;
		long	hash;
		long	hash2;
		long	pid;
		double	value;
		int		last_line;
	};

static struct tag_sobgRecord	sobg_rec;

static char	*sobg = "sobg";

struct BG_PTR
{
	struct tag_sobgRecord	rec [MAX_SOBG];
	struct BG_PTR			*next;
};

#define	BG_NULL	((struct BG_PTR *) 0)

static struct BG_PTR	*sobg_head = BG_NULL;
static int				BG_next = 0;


static struct BG_PTR	*BG_alloc (void)
{
	return ((struct BG_PTR *) malloc (sizeof (struct BG_PTR)));
}

/*===========================================
| Add hash to stored record to update last. |
===========================================*/
static void
add_hash9 (
 char	*_co_no,
 char	*_br_no,
 char	*_type,
 int	_lpno,
 long	_hhbr_hash,
 long	_hhcc_hash,
 long	_pid,
 double	_value,
 int	_last_line)
{
	int	i, j;
	struct BG_PTR	*sobg_curr;

	if (!BG_next)
	{
		if (!(sobg_head = BG_alloc ()))
		{
			file_err (errno, __FILE__, "malloc()");
		}
		strcpy (sobg_head -> rec [0].co_no, _co_no);
		strcpy (sobg_head -> rec [0].br_no, _br_no);
		strcpy (sobg_head -> rec [0].type, _type);
		sobg_head -> rec [0].lpno	= _lpno;
		sobg_head -> rec [0].hash	= _hhbr_hash;
		sobg_head -> rec [0].hash2	= _hhcc_hash;
		sobg_head -> rec [0].pid	= _pid;
		sobg_head -> rec [0].value	= _value;
		sobg_head -> rec [0].last_line	= _last_line;
		BG_next++;
		return;
	}

	sobg_curr = sobg_head;
	for (i = 0; i < BG_next; i++)
	{
		j = i % MAX_SOBG;
		if (j == 0 && i > 0)
			sobg_curr = sobg_curr -> next;

		if (!strcmp (sobg_curr -> rec [j].co_no, _co_no)	&&
		    !strcmp (sobg_curr -> rec [j].br_no, _br_no)	&&
		    !strcmp (sobg_curr -> rec [j].type, _type)	&&
		    sobg_curr -> rec [j].lpno	== _lpno	&&
		    sobg_curr -> rec [j].hash	== _hhbr_hash	&&
		    sobg_curr -> rec [j].hash2	== _hhcc_hash &&
		    sobg_curr -> rec [j].pid	== _pid)
		{
			sobg_curr -> rec [j].value += _value;
		    if (sobg_curr -> rec [j].last_line < _last_line)
				sobg_curr -> rec [j].last_line = _last_line;
			return;
		}
	}

	j = BG_next % MAX_SOBG;
	if (j == 0)
	{
		sobg_curr -> next = BG_alloc ();
		if (sobg_curr -> next == BG_NULL)
		{
			recalc_sobg ();
			add_hash9 (_co_no, 
					  _br_no, 
					  _type, 
					  _lpno, 
					  _hhbr_hash, 
					  _hhcc_hash, 
					  _pid, 
					  _value,
					  _last_line);
			return;
		}
		sobg_curr = sobg_curr -> next;
	}
	strcpy (sobg_curr -> rec [j].co_no, _co_no);
	strcpy (sobg_curr -> rec [j].br_no, _br_no);
	strcpy (sobg_curr -> rec [j].type, _type);
	sobg_curr -> rec [j].lpno	= _lpno;
	sobg_curr -> rec [j].hash	= _hhbr_hash;
	sobg_curr -> rec [j].hash2	= _hhcc_hash;
	sobg_curr -> rec [j].pid	= _pid;
	sobg_curr -> rec [j].value	= _value;
	sobg_curr -> rec [j].last_line = _last_line;
	BG_next++;
}

/*================================
| Old add_hash()
| kept for backward compatibility
================================*/
void
add_hash (
 char	*_co_no,
 char	*_br_no,
 char	*_type,
 int	_lpno,
 long	_hhbr_hash,
 long	_hhcc_hash,
 long	_pid,
 double	_value)
{
	add_hash9 (_co_no,
			   _br_no,
			   _type,
			   _lpno,
			   _hhbr_hash,
			   _hhcc_hash,
			   _pid,
			   _value,
			   0);
}

/*================================
| 
================================*/
void
add_hash_RC (
 char *_co_no,
 char *_br_no,
 long _hhbr_hash,
 long _hhcc_hash,
 long _pid,
 int  _last_line)
{
	add_hash9 (_co_no,
			   _br_no,
			   "RC",
			   0,
			   _hhbr_hash,
			   _hhcc_hash,
			   _pid,
			   0.0,
			   _last_line);
}

/*===========================================
| Add record to background processing file. |
===========================================*/
static void	add_sobg (struct tag_sobgRecord *bg_rec)
{
	int	cc;

	sobg_rec = *bg_rec;

	if (!strcmp (sobg_rec.type, "RC")
	 || !strcmp (sobg_rec.type, "RP")
	 || (cc = find_rec (sobg, &sobg_rec, COMPARISON, "r")) != 0
	 || sobg_rec.hash2 != bg_rec -> hash2)
	{
		sobg_rec = *bg_rec;
		sobg_rec.value = 0;		/* redundant field */

		if ((cc = abc_add (sobg, &sobg_rec)))
			file_err (cc, sobg, "DBADD");
	}
}

/*=======================================
| Recalc all stored fields within sobg. |
=======================================*/
void
recalc_sobg (void)
{
	int	i, j;
	struct BG_PTR	*sobg_curr;

	if (!BG_next)
		return;

	open_rec (sobg, sobg_list, SOBG_NO_FIELDS, "sobg_id_no");

	sobg_curr = sobg_head;
	/*----------------------------
	| Recalc all records found . |
	----------------------------*/
	for (i = 0; i < BG_next; i++)
	{
		j = i % MAX_SOBG;
		if (j == 0 && i > 0)
		{
			sobg_head = sobg_curr -> next;
			free (sobg_curr);
			sobg_curr = sobg_head;
		}

		add_sobg (&sobg_curr -> rec [j]);
	}

	free (sobg_head);
	sobg_head = BG_NULL;

	abc_fclose (sobg);

	BG_next = 0;
}
