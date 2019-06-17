/*=====================================================================
|  Copyright (C) 1988 - 1997 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( pinform.c                   )                    |
|  Program Desc  : ( Replacement for ISQL sperform.               )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :   YES,     ,     ,     ,     ,     ,     ,         |
|  Database      : (stck)                                             |
|---------------------------------------------------------------------|
|  Updates Files :   YES,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Date Written  : (31/02/91)      | Author       : Trevor van Bremen |
|---------------------------------------------------------------------|
|  Date Modified : (09/11/92)      | Modified by : Trevor van Bremen  |
|  Date Modified : (29/01/93)      | Modified by : Trevor van Bremen  |
|  Date Modified : (01/03/93)      | Modified by : Trevor van Bremen  |
|  Date Modified : (02/04/93)      | Modified by : Trevor van Bremen  |
|  Date Modified : (26/08/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (06.03.94)      | Modified by : Jonathan Chen      |
|  Date Modified : (29/04/94)      | Modified by : Campbell Mander.   |
|  Date Modified : (04/08/97)      | Modified by : Campbell Mander.   |
|  Date Modified : (07/03/98)      | Modified by : Primo O. Esteria   |

|  Comments      :                                                    |
|  (09/11/92)    : Added FN03 option to main menu.                    |
|  (29/01/93)    : Added handling of AUDIT trail. DFT 8024.           |
|  (01/03/93)    : PSL 5784 (Allow for non-su dev people).            |
|  (02/04/93)    : PSL 5784 (Add KWL to above list)                   |
|  (26/08/93)    : PSL 9713 Removed hard-coded name list              |
|  (06.03.94)    : HGP 10469 Removing unpublished calls               |
|  (29/04/94)    : HGP 10469. Remove $ signs.                         |
|  (04/08/97)    : SEL 9.9.1. Fixed removal of successive records.    |
|                : Note that there is a corruption of isrecnum        |
|                : between the end of rmv_yes () and the (immediate)  |
|                : next invocation of rmv_yes ().  This fix gets      |
|                : around this problem by resetting isrecnum before   |
|                : the first dbfind () in rmv_yes ().                 |
|  (07/03/98)    : Accomodated wide fields to fit screen width in     |
|                : multiple lines                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pinform.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/pinform/pinform.c,v 5.3 2001/11/08 01:07:48 scott Exp $";
#define	NO_SCRGEN

#include	<pslscr.h>
#include	<ring_menu.h>
#include	<twodec.h>
#include	<sys/types.h>
#include	<signal.h>
#include    <dbio.h>
#include    "prototypes.h"

#ifndef	HAS_UID_T
typedef	int	uid_t;			/* it could be "short", but it don't matter */
typedef	int	gid_t;
#endif	/*HAS_UID_T*/

int	super_user = FALSE;

	/*===========
	| systables |
	===========*/
	struct dbview tables_list[] ={
		{"tabname"},
		{"owner"},
		{"dirpath"},
		{"tabid"},
		{"rowsize"},
		{"ncols"},
		{"nindexes"},
		{"nrows"},
		{"created"},
		{"version"},
		{"tabtype"},
		{"audpath"},
	};

	int	tables_no_fields = 12;

	struct	{
		char	tabname[19];
		char	owner[9];
		char	dirpath[65];
		long	tabid;
		int		rowsize;
		int		ncols;
		int		nindexes;
		long	nrows;
		long	created;
		long	version;
		char	tabtype[2];
		char	audpath[65];
	} tables_rec;

	/*============
	| syscolumns |
	============*/
	struct dbview columns_list[] ={
		{"colname"},
		{"tabid"},
		{"colno"},
		{"coltype"},
		{"collength"},
	};

	int	columns_no_fields = 5;

	struct	{
		char	colname[19];
		long	tabid;
		int	colno;
		int	coltype;
		int	collength;
	} columns_rec;

	/*============
	| sysindexes |
	============*/
	struct dbview indexes_list[] ={
		{"idxname"},
		{"owner"},
		{"tabid"},
		{"idxtype"},
		{"clustered"},
		{"part1"},
		{"part2"},
		{"part3"},
		{"part4"},
		{"part5"},
		{"part6"},
		{"part7"},
		{"part8"},
	};

	int	indexes_no_fields = 13;

	struct	IDX_REC {
		char	idxname[19];
		char	owner[9];
		long	tabid;
		char	idxtype[2];
		char	clustered[2];
		int	part[8];
	} indexes_rec;

/*
| Table names
*/
static char
	*data		= "data",
	*columns	= "columns",
	*indexes	= "indexes",
	*tables		= "tables";

struct	WIN_LIMITS
{
	int	max_lnth;
	int	max_item;
};

struct	TAB_STR
{
	struct	TAB_STR	*next;
	struct	TAB_STR	*prev;
	char	tabname[19];
	long	tabid;
};

#define	TAB_NULL	((struct TAB_STR *) NULL)
struct	TAB_STR	*tab_head = TAB_NULL;
struct	TAB_STR	*tab_curr = TAB_NULL;
struct	TAB_STR	*tab_tail = TAB_NULL;

int	del_pressed = FALSE;
char	wrk_name[19];
char	*wrk_rec;
char	*qry_rec;
int	cur_qry_meth;
int	wrk_cols;
struct	dbview	wrk_list[256];
char	sel_meth[257];
#define	SEL_NONE	0
#define	SEL_EQUAL	1
#define	SEL_LESS	2
#define	SEL_LTEQ	3
#define	SEL_GREATER	4
#define	SEL_GTEQ	5
#define	SEL_NEQUAL	6
char	slct_str[7][3]=
{
	{"  "},
	{" ="},
	{" <"},
	{"<="},
	{" >"},
	{">="},
	{"!="}
};
int	wrk_idxs;
struct	IDX_REC	wrk_idx[16];
int	scn_offset = 0;
long	tot_recs;
long	cur_rec;
#define	MAX_QRY	9999999
long	rec_nos[MAX_QRY];

char	*val_chrs;
char	inp_buf[256];
char	res_buf[256];		/* For FN02 restore field		*/
#define	INP_QRY	0
#define	INP_ADD	1
#define	INP_UPD	2

#define	SCN_START	2	/* 1st col appears on this screen line	*/
#define	SCN_DEPTH	19	/* The total no. columns per screen	*/
#define	PRMT_POS	0
#define	QRY_POS		18
#define	VAL_POS		21
#define	MAX_VAL_WID	79 - VAL_POS

int	re_draw (void);
int	query (void);
int	next (void);
int	previous (void);
int	goto_rec (void);
int	add (void);
int	update (void);
int	remove_rec (void);
int	screen (void);

int	workOffset	=	0;

menu_type	_main_menu[] =
{
	{	" PINFORM:", "",
		_no_option, "", 255, SHOW	},
	{	" ", "",
		re_draw, "", FN3, SELECT	},
	{	"Query", "Searches the active database table.",
		query, "Qq",	},
	{	"Next", "Shows the next row in the Current list.",
		next, "Nn",	},
	{	"Previous", "Shows the previous row in the Current list.",
		previous, "Pp",	},
	{	"Goto", "Jump to a record within the Current list.",
		goto_rec, "Gg",	},
	{	"Add", "Adds a row to the active database table.",
		add, "Aa",	},
	{	"Update", "Changes a row in the active database table.",
		update, "Uu",	},
	{	"Remove", "Deletes a row from the active database table.",
		remove_rec, "Rr",	},
	{	"Screen", "Shows the next page of the form.",
		screen, "Ss",	},
	{	"Exit", "Return to the Pinform Menu.",
		_no_option, "Ee", FN16, EXIT | SELECT | SHOW	},
	{	"",		},
};

/* used in wide column display control */

int multiple_lines_used;
int columns_used;


int	rmv_yes (void);
int rmv_no (void);
menu_type	_remove_menu[] =
{
	{	"REMOVE:", "",
		_no_option, "", 255, SHOW	},
	{	"Yes", "Removes this row from the active table.",
		rmv_yes, "Yy", 255, EXIT | SELECT | SHOW	},
	{	"No", "Does NOT remove this row from the active table.",
		rmv_no, "Nn", 255, EXIT | SELECT | SHOW	},
	{	"",		},
};

int	qry_none (void);
int	qry_equal (void);
int	qry_nequal (void);
int	qry_gteq (void);
int	qry_greater (void);
int	qry_lteq (void);
int	qry_less (void);

menu_type	_query_menu[] =
{
	{	"QUERY:", "",
		_no_option, "", 255, SHOW	},
	{	"=", "Only select rows which are EQUAL",
		qry_equal, "=", 255, EXIT | SELECT | SHOW	},
	{	"!=", "Only select rows which are NOT EQUAL",
		qry_nequal, "", 255, EXIT | SELECT | SHOW	},
	{	">=", "Only select rows which are NOT LESS THAN",
		qry_gteq, "", 255, EXIT | SELECT | SHOW	},
	{	">", "Only select rows which are GREATER THAN",
		qry_greater, ">", 255, EXIT | SELECT | SHOW	},
	{	"<=", "Only select rows which are NOT GREATER THAN",
		qry_lteq, "", 255, EXIT | SELECT | SHOW	},
	{	"<", "Only select rows which are LESS THAN",
		qry_less, "<", 255, EXIT | SELECT | SHOW	},
	{	"None", "Does NOT validate the current column.",
		qry_none, "Nn", 255, EXIT | SELECT | SHOW	},
	{	"",		},
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void do_menu (void);
struct TAB_STR *select_table (void);
void load_table_list (struct WIN_LIMITS *win_limits);
void prep_form (char *tabname);
int load_cols (long tabid);
int load_idxs (long tabid);
void run_form (void);
void set_del (int x);
void choose_idx (int *idx, int *prt);
int Is_wild (int colno);
void init_idx (int idx, int prt);
int chck_idx (int idx, int prt);
int valid_rec (void);
int wild_srch (char *search_str, char *key_val, int *break_out);
void dsp_rec (void);
void num_fmt (int len);
int input_rec (int mode);
void init_rec (char *rec);
void Getint (int x, int y, int *iptr, int *ires);
void Getlong (int x, int y, long *lptr, long *lres);
void Getfloat (int x, int y, float *fptr, float *fres);
void Getdouble (int x, int y, double *dptr, double *dres);
void Getmoney (int x, int y, double *dptr, double *dres, int i);
void GetDate (int x, int y, long *lptr, long *lres);
void Getalpha (int x, int y, char *cptr, char *cres, int len);
int get_inp (int x, int y, int len);
int qry_meth (int line);
void dsp_count (void);
static long unsigned int RecCount (char *table);

extern int _wide;
/*extern int isrecnum; */

int
main (
 int                argc, 
 char*              argv[])
{
	int	i;

	if (!getuid () || !geteuid ())
		super_user = TRUE;
	else
	{
		_main_menu[6].flag = DISABLED;
		_main_menu[7].flag = DISABLED;
		_main_menu[8].flag = DISABLED;
	}

	init_scr ();
	set_tty ();

	clear ();

	abc_dbopen (data);
	abc_alias (tables,	"systables");
	abc_alias (columns,	"syscolumns");
	abc_alias (indexes,	"sysindexes");
	open_rec (tables,	tables_list,	tables_no_fields,  "tabname");
	open_rec (columns,	columns_list,	columns_no_fields, "column");
	open_rec (indexes,	indexes_list,	indexes_no_fields, "idxtab");

	if (argc == 1)
		do_menu ();
	else
		for (i = 1; i < argc; i++)
			prep_form (clip (argv[i]));

	abc_fclose (indexes);
	abc_fclose (columns);
	abc_fclose (tables);
	abc_dbclose (data);

	clear ();
	crsr_on ();
	rset_tty ();
	return (EXIT_SUCCESS);
}

void
do_menu (void)
{
	struct	TAB_STR *       sptr;

	while (TRUE)
	{
		sptr = select_table ();
		if (sptr == TAB_NULL)
			break;
		prep_form (sptr->tabname);
	}
}

struct	TAB_STR	*
select_table (void)
{
	struct	TAB_STR	*       tmp_str;
	struct	WIN_LIMITS      win_limits;

	load_table_list (&win_limits);

	/*---------------------------------------
	| If none found, max_lnth will = 0	|
	---------------------------------------*/
	if (win_limits.max_lnth == 0)
		return ((struct	TAB_STR *) NULL);

	clear ();

	print_at (22, 0, "                   Use cursor keys to position highlight bar. ");
	print_at (23, 0, "                      Press <RET> to run or <FN16> to exit    ");

	tmp_str = (struct TAB_STR *) win_select
	(
		(super_user) ? "LOGISTIC DATABASE EDITOR" : "LOGISTIC DATABASE VIEWER",
		win_limits.max_lnth,
		(struct SEL_STR *) tab_head,
		0,
		0,
		80,
		20
	);
	return (tmp_str);
}

void
load_table_list (
 struct WIN_LIMITS *        win_limits)
{
	win_limits->max_lnth = 0;
	win_limits->max_item = 0;

	while (tab_head != TAB_NULL)
	{
	    tab_curr = tab_head;
	    tab_head = tab_head->next;
	    free (tab_curr);
	}
	tab_curr = TAB_NULL;
	tab_tail = TAB_NULL;
	sprintf (tables_rec.tabname, "%-18.18s", " ");
	sprintf (tables_rec.owner, "%-8.8s", " ");
	cc = find_rec (tables, &tables_rec, GTEQ, "r");
	while (!cc)
	{
        if (!super_user)
        {
            if (tables_rec.tabid < 100L)
            {
	            cc = find_rec (tables, &tables_rec, NEXT, "r");
	            continue;
            }
        }
        tab_curr = (struct TAB_STR *) malloc (sizeof (struct TAB_STR));
        if (tab_curr == TAB_NULL)
        {
	        sys_err ("Error in load_table_list() During (MALLOC)", errno, PNAME);
        }
        strcpy (tab_curr->tabname, tables_rec.tabname);
        if (strlen (clip (tab_curr->tabname)) > (unsigned int) win_limits->max_lnth)
        {
	        win_limits->max_lnth = strlen (tab_curr->tabname);
        }
        tab_curr->tabid = tables_rec.tabid;
        tab_curr->prev = tab_tail;
        tab_curr->next = TAB_NULL;
        if (tab_tail != TAB_NULL)
        {
            tab_tail->next = tab_curr;
        }
        else
        {
            tab_head = tab_curr;
        }
        tab_tail = tab_curr;
        win_limits->max_item++;
        cc = find_rec (tables, &tables_rec, NEXT, "r");
	}
}

/*********** here too  omirp *************/

void
prep_form (
 char*              tabname)
{
	sprintf (tables_rec.tabname, "%-18.18s", tabname);
	sprintf (tables_rec.owner, "%-8.8s", " ");
	if (find_rec (tables, &tables_rec, GTEQ, "r"))
		return;
	clip (tables_rec.tabname);
	if (strcmp (tabname, tables_rec.tabname))
 		return;

	strcpy (wrk_name, tabname);
	wrk_cols = load_cols (tables_rec.tabid);

	wrk_idxs = load_idxs (tables_rec.tabid);

	if (wrk_cols < 1)
		return;

	cc = _open_rec (wrk_name, wrk_list, wrk_cols, "", ACCSEQUENTIAL);
	if (cc)
		file_err (cc, wrk_name, "_open_rec");
	if (wrk_list[wrk_cols - 1].vwtype == CHARTYPE)
	{
		wrk_rec = (char *) malloc (1 + (unsigned) (wrk_list[wrk_cols - 1].vwstart + wrk_list[wrk_cols - 1].vwlen));
		qry_rec = (char *) malloc (1 + (unsigned) (wrk_list[wrk_cols - 1].vwstart + wrk_list[wrk_cols - 1].vwlen));
	}
	else
	{
		wrk_rec = (char *) malloc (16 + (unsigned) (wrk_list[wrk_cols - 1].vwstart));
		qry_rec = (char *) malloc (16 + (unsigned) (wrk_list[wrk_cols - 1].vwstart));
	}

	init_rec (wrk_rec);
	init_rec (qry_rec);

	tot_recs = 0L;
	run_form ();

	abc_fclose (wrk_name);
	free (wrk_rec);
	free (qry_rec);
	while (wrk_cols > 0)
	{
		wrk_cols--;
		free (wrk_list[wrk_cols].vwname);
	}
}

int
load_cols (
 long               tabid)
{
	int	count = 0;
	char	colname[19];

	columns_rec.tabid = tabid;
	columns_rec.colno = 0;
	cc = find_rec (columns, &columns_rec, GTEQ, "r");
	while
	(
		!cc &&
		columns_rec.tabid == tabid
	)
	{
		strcpy (colname, columns_rec.colname);
		clip (colname);
		wrk_list[count].vwname = strdup (colname);
		count++;
		cc = find_rec (columns, &columns_rec, NEXT, "r");
	}

	return (count);
}

int
load_idxs (
 long               tabid)
{
	int	count = 0;

	indexes_rec.tabid = tabid;
	cc = find_rec (indexes, &indexes_rec, GTEQ, "r");
	while
	(
		!cc &&
		indexes_rec.tabid == tabid &&
		count < 16
	)
	{
		strcpy (wrk_idx[count].idxname,	indexes_rec.idxname);
		strcpy (wrk_idx[count].owner,	indexes_rec.owner);
		strcpy (wrk_idx[count].idxtype,	indexes_rec.idxtype);
		wrk_idx[count].part[0]	=	indexes_rec.part[0];
		wrk_idx[count].part[1]	=	indexes_rec.part[1];
		wrk_idx[count].part[2]	=	indexes_rec.part[2];
		wrk_idx[count].part[3]	=	indexes_rec.part[3];
		wrk_idx[count].part[4]	=	indexes_rec.part[4];
		wrk_idx[count].part[5]	=	indexes_rec.part[5];
		wrk_idx[count].part[6]	=	indexes_rec.part[6];
		wrk_idx[count].part[7]	=	indexes_rec.part[7];
		count++;
		cc = find_rec (indexes, &indexes_rec, NEXT, "r");
	}

	return (count);
}

/*
Code inserted for mulitple lines of display for wide fields is just a
plain code insert.  No effort was made to fix code for whatever reson

omirp/ 07/06/98

*/
void
run_form (void)
{
	int	i,
		j;

    int _w =0,ix,x=0,x1;
	
	multiple_lines_used =0;
	columns_used =0;

	for (i = 0; i < wrk_cols; i++)
	{
	    sel_meth[i] = SEL_NONE;
	    if (wrk_list[i].vwtype == CHARTYPE && wrk_list[i].vwlen > MAX_VAL_WID)
	    {
			swide ();
			break;
	    }
	}

	clear ();
	print_at (0, _wide ? 43 : 17, "%R L O G I S T I C   T A B L E   %s ",
		super_user ? "E D I T O R" : "V I E W E R");
	move (0, 1);
	line (_wide ? 132 : 80);

	centre_at (1, _wide ? 132 : 80,
		" Total records: %-lu ",
		RecCount (wrk_name));


	scn_offset = 0;

	for (i = 0; i < SCN_DEPTH; i++)
	{
	    j = i + SCN_START+x;

		if (j > SCN_DEPTH +1)
		{
		   break;
		}

	    if (i < wrk_cols)
	    {
			_w = _wide ? 132 : 80;
			_w -= (VAL_POS+1);

			workOffset	=	sel_meth [i];
            if (wrk_list[i].vwlen <= _w || wrk_list[i].vwtype != CHARTYPE) 
            {
			    print_at (j, PRMT_POS, "%-18.18s%-2.2s[",
			    			wrk_list[i].vwname,
			    			slct_str[workOffset]);
            }

		switch (wrk_list[i].vwtype)
		{
		case	CHARTYPE:
			/**** omirp ******/
            _w = _wide ? 132 : 80;
            _w -= (VAL_POS+1); 
           
                           	        	
			if (wrk_list[i].vwlen <= _w)
			{ 
	            printf("%-*.*s]",wrk_list[i].vwlen,wrk_list[i].vwlen," ");
			}
			else
			{
				
				int _wx;

				multiple_lines_used = 1;
                columns_used++;

				workOffset	=	sel_meth [i];
                print_at(j,PRMT_POS,"%-18.18s%-2.2s[",
							wrk_list[i].vwname,
							slct_str[workOffset]);

				printf("%-*.*s]",_w,_w," ");
          
				x1 = wrk_list[i].vwlen/_w;
                _wx = wrk_list[i].vwlen - _w;

				for (ix = 0; ix < x1; ix++)
				{
					x++;
                    print_at(++j,PRMT_POS,"%18.18s%-2.2s["," "," ");

	 				printf ("%-*.*s]",
						  _wx,
						  _wx,
			    	      " ");
                    _wx -= _w;
				}
			}
           

 			break;

		case	INTTYPE:
			printf ("%-6.6s]",
			    " ");
			break;

		case	LONGTYPE:
			printf ("%-11.11s]",
			    " ");
			break;

 		case	DOUBLETYPE:
			printf ("%-17.17s]",
			    " ");
			break;

		case	FLOATTYPE:
			printf ("%-9.9s]",
			    " ");
			break;

		case	SERIALTYPE:
			printf ("%-11.11s]",
			    " ");
			break;

		case	DATETYPE:
			if (FullYear())
				printf ("%-10.10s]", " ");
			else
				printf ("%-8.8s]", " ");
			break;
 
		case	MONEYTYPE:
			printf ("%-17.17s] (Money)",
			    " ");
			break;
		}
	    }
	}

	move (0, 21);
	if (_wide)
		line (132);
	else
		line (80);

	run_menu (_main_menu, "", 22);

	if (_wide)
		snorm ();
	clear ();
}

int
query (void)
{
	int	i,
		idx_prts,
		idx_no;
	char	*tmp_rec,
		idxname[19];

	for (i = 0; i < wrk_cols; i++)
	    sel_meth[i] = SEL_NONE;

	if (scn_offset != 0)
	{
	    scn_offset = -SCN_DEPTH;
	    screen ();
	}

	tmp_rec = wrk_rec;
	wrk_rec = qry_rec;
	input_rec (INP_QRY);
	wrk_rec = tmp_rec;

	move (0, 22);
	cl_line ();
	move (0, 23);
	cl_line ();
	centre_at (22, _wide ? 132 : 80,
		"%R Executing Query.... Press <DEL> to abort ");

	choose_idx (&idx_no, &idx_prts);
	if (idx_no >= 0)
	{
		strcpy (idxname, wrk_idx [idx_no].idxname);
		clip (idxname);
		dbselfield (wrk_name, idxname, ACCKEYED);
		init_idx (idx_no, idx_prts);
		cc = find_rec (wrk_name, wrk_rec, GTEQ, "r");
	}
	else
		cc = find_rec (wrk_name, wrk_rec, FIRST, "r");

	rset_tty ();
	del_pressed = FALSE;
	signal (SIGINT, set_del);
	tot_recs = 0L;
	while (!cc && !del_pressed)
	{
		if (idx_no >= 0 && !chck_idx (idx_no, idx_prts))
			break;

		if (valid_rec ())
		{
			rec_nos[tot_recs] = abc_rowid (wrk_name);
			tot_recs++;
			if (tot_recs > MAX_QRY)
			{
				print_at (23, 0, "%R The limits of this initial version of PINFORM have been exceeded. ");
				break;
			}
		}
	    cc = find_rec (wrk_name, wrk_rec, NEXT, "r");
	}
	signal (SIGINT, SIG_IGN);
	set_tty ();

	if (idx_no >= 0)
	    dbselfield (wrk_name, (char *) 0, ACCSEQUENTIAL);

	if (tot_recs < 1L)
	{
	    init_rec (wrk_rec);
	    dsp_rec ();
	    cur_rec = -1L;
	    dsp_count ();
	    cur_rec++;
	    return (EXIT_SUCCESS);
	}

	cur_rec = 1L;

	previous ();
    return (EXIT_SUCCESS);
}

void
set_del (
 int                x)
{
	putchar (BELL);
	fflush (stdout);
	del_pressed = TRUE;
}

void
choose_idx (
 int*               idx,
 int*               prt)
{
	int	cidx,	/* Current Index			*/
		cprt,	/* Current Part within Index		*/
		ctyp;	/* Current Index type (TRUE=UNIQ)	*/

	*idx = -1;
	*prt = -1;
	ctyp = FALSE;
	cidx = 0;
	while (cidx < wrk_idxs)
	{
	    cprt = 0;
	    while (cprt < 8)
	    {
		/*-------------------------------------------------------
		| Just a note for those 'game enough' to change the	|
		| following line.....					|
		|							|
		| wrk_idx[cidx]			is the current index	|
		| wrk_idx[cidx].part[cprt]	is the current part	|
		| sel_meth[^^]			is the col. query flag	|
		-------------------------------------------------------*/
		if
		(
		    wrk_idx[cidx].part[cprt] < 1 ||
		    sel_meth[wrk_idx[cidx].part[cprt] - 1] != SEL_EQUAL ||
		    Is_wild (wrk_idx[cidx].part[cprt] - 1)
		)
		{
		    if (cprt > 0)
		    {
			if (cprt > *prt)
			{
			    *prt = cprt;
			    *idx = cidx;
			    ctyp = (wrk_idx[cidx].idxtype[0] == 'U') ? TRUE : FALSE;
			    break;
			}
			if (cprt == *prt && wrk_idx[cidx].idxtype[0] == 'U' && ctyp == FALSE)
			{
			    *prt = cprt;
			    *idx = cidx;
			    ctyp = TRUE;
			    break;
			}
		    }
		    break;
		}
		else
		{
		    if (cprt == 7)
		    {
			*idx = cidx;
			*prt = 8;
			return;
		    }
		}
		cprt++;
	    }
	    cidx++;
	}
}

/*-------------------------------
| Return true if wild-card(s)	|
| have been entered in colno.	|
-------------------------------*/
int
Is_wild (
 int                colno)
{
	union	_type_var
	{
		char	*cptr;
		int	*iptr;
		long	*lptr;
		float	*fptr;
		double	*dptr;
	} qry_ptr;

	if (wrk_list[colno].vwtype != CHARTYPE)
		return (FALSE);

	qry_ptr.cptr = qry_rec + wrk_list[colno].vwstart;
	if (strchr (qry_ptr.cptr, '*'))
		return (TRUE);

	return (FALSE);
}

void
init_idx (
 int                idx,
 int                prt)
{
	int		i			=	0,
			colno		=	0,
			tot_prts	=	0;

	union	_type_var
	{
		char	*cptr;
		int	*iptr;
		long	*lptr;
		float	*fptr;
		double	*dptr;
	} qry_ptr, rec_ptr;

	for (i = 0; i < 8; i++)
	    if (wrk_idx[idx].part[i] != 0)
		tot_prts = i + 1;
	    else
		break;

	for (i = 0; i < tot_prts; i++)
	{
	    colno = wrk_idx[idx].part[i] - 1;
	    qry_ptr.cptr = qry_rec + wrk_list[colno].vwstart;
	    rec_ptr.cptr = wrk_rec + wrk_list[colno].vwstart;
	    switch (wrk_list[colno].vwtype)
	    {
	    case	CHARTYPE:
		if (i < prt)
		    sprintf (rec_ptr.cptr, "%-*.*s",
			wrk_list[colno].vwlen,
			wrk_list[colno].vwlen,
			qry_ptr.cptr);
		else
		    sprintf (rec_ptr.cptr, "%-*.*s",
			wrk_list[colno].vwlen,
			wrk_list[colno].vwlen,
			" ");
		break;

	    case	INTTYPE:
		if (i < prt)
		    *rec_ptr.iptr = *qry_ptr.iptr;
		else
		    *rec_ptr.iptr = 0;
		break;

	    case	LONGTYPE:
	    case	SERIALTYPE:
	    case	DATETYPE:
		if (i < prt)
		    *rec_ptr.lptr = *qry_ptr.lptr;
		else
		    *rec_ptr.lptr = 0L;
		break;

	    case	DOUBLETYPE:
	    case	MONEYTYPE:
		if (i < prt)
		    *rec_ptr.dptr = *qry_ptr.dptr;
		else
		    *rec_ptr.dptr = (double) 0.00;
		break;

	    case	FLOATTYPE:
		if (i < prt)
		    *rec_ptr.fptr = *qry_ptr.fptr;
		else
		    *rec_ptr.fptr = (float) 0.00;
		break;
	    }
	}
}

int
chck_idx (
 int                idx,
 int                prt)
{
	int	i,
		colno;
	union	_type_var
	{
		char	*cptr;
		int	*iptr;
		long	*lptr;
		float	*fptr;
		double	*dptr;
	} qry_ptr, rec_ptr;

	for (i = 0; i < prt; i++)
	{
	    colno = wrk_idx[idx].part[i] - 1;
	    qry_ptr.cptr = qry_rec + wrk_list[colno].vwstart;
	    rec_ptr.cptr = wrk_rec + wrk_list[colno].vwstart;
	    switch (wrk_list[colno].vwtype)
	    {
	    case	CHARTYPE:
		if (strcmp (rec_ptr.cptr, qry_ptr.cptr))
		    return (FALSE);
		break;

	    case	INTTYPE:
		if (*rec_ptr.iptr != *qry_ptr.iptr)
		    return (FALSE);
		break;

	    case	LONGTYPE:
	    case	SERIALTYPE:
	    case	DATETYPE:
		if (*rec_ptr.lptr != *qry_ptr.lptr)
		    return (FALSE);
		break;

	    case	DOUBLETYPE:
	    case	MONEYTYPE:
		if (*rec_ptr.dptr != *qry_ptr.dptr)
		    return (FALSE);
		break;

	    case	FLOATTYPE:
		if (*rec_ptr.fptr != *qry_ptr.fptr)
		    return (FALSE);
		break;
	    }
	}

	return (TRUE);
}

int
valid_rec (void)
{
	int	i,
		j,
		k,
		l;
	union	_type_var
	{
		char	*cptr;
		int	*iptr;
		long	*lptr;
		float	*fptr;
		double	*dptr;
	} qry_ptr, rec_ptr;

	for (i = 0; i < wrk_cols; i++)
	{
	    if (sel_meth[i] != SEL_NONE)
	    {
		qry_ptr.cptr = (char *) qry_rec + wrk_list[i].vwstart;
		rec_ptr.cptr = (char *) wrk_rec + wrk_list[i].vwstart;
		j = SEL_EQUAL;
		switch (wrk_list[i].vwtype)
		{
		case	CHARTYPE:
		if (strchr (qry_ptr.cptr, '*'))
		{
			k = wild_srch (rec_ptr.cptr, qry_ptr.cptr, &l);
			if (k != 1)
			{
			    if (l < 0)
					j = SEL_LESS;
			    else if (l > 0)
				    j = SEL_GREATER;
				else
				    j = SEL_NEQUAL;
			}
		}
		else
		{
			k = strcmp (rec_ptr.cptr, qry_ptr.cptr);
			if (k < 0)
			    j = SEL_LESS;
			else if (k > 0)
				j = SEL_GREATER;
		}
		break;

		case	INTTYPE:
		    if (*(rec_ptr.iptr) < *(qry_ptr.iptr))
			j = SEL_LESS;
		    if (*(rec_ptr.iptr) > *(qry_ptr.iptr))
			j = SEL_GREATER;
		    break;

		case	LONGTYPE:
		case	SERIALTYPE:
		case	DATETYPE:
		    if (*(rec_ptr.lptr) < *(qry_ptr.lptr))
			j = SEL_LESS;
		    if (*(rec_ptr.lptr) > *(qry_ptr.lptr))
			j = SEL_GREATER;
		    break;

		case	DOUBLETYPE:
		case	MONEYTYPE:
		    if (*(rec_ptr.dptr) < *(qry_ptr.dptr))
			j = SEL_LESS;
		    if (*(rec_ptr.dptr) > *(qry_ptr.dptr))
			j = SEL_GREATER;
		    break;

		case	FLOATTYPE:
		    if (*(rec_ptr.fptr) < *(qry_ptr.fptr))
			j = SEL_LESS;
		    if (*(rec_ptr.fptr) > *(qry_ptr.fptr))
			j = SEL_GREATER;
		    break;
		}

		switch (sel_meth[i])
		{
		case	SEL_EQUAL:
		    if (j != SEL_EQUAL)
			return (FALSE);
		    break;

		case	SEL_LESS:
		    if (!(j & SEL_LESS))
			return (FALSE);
		    break;

		case	SEL_LTEQ:
		    if (!(j & SEL_EQUAL) && !(j & SEL_LESS))
			return (FALSE);
		    break;

		case	SEL_GREATER:
		    if (!(j & SEL_GREATER))
			return (FALSE);
		    break;

		case	SEL_GTEQ:
		    if (!(j & SEL_EQUAL) && !(j & SEL_GREATER))
			return (FALSE);
		    break;

		case	SEL_NEQUAL:
		    if (j == SEL_EQUAL)
			return (FALSE);
		    break;
		}
	    }
	}
	return (TRUE);
}

/*===============================
| Do a 'wild-card' search.	|
===============================*/
int
wild_srch (
 char*              search_str,
 char*              key_val,
 int*               break_out)
{
	int	i;
	char	*sptr;
	char	*pptr;	/* pointer to position in pattern string	*/
	char	*tptr;	/* pointer to position in pattern string	*/
	char	*t_string = strdup (search_str);
	char	*p_string = strdup (key_val);

	clip (p_string);
	sptr = clip(t_string);

	sptr = strchr (p_string,'*');
	/*-----------------------
	| No Wild Carding	|
	-----------------------*/
	if (sptr == (char *)0)
	{
		*break_out = strncmp(t_string,key_val,strlen(key_val));
		free (t_string);
		free (p_string);
		return(!*break_out);
	}

	if (p_string[0] != '*')
	{
		*sptr = '\0';
		/*---------------------------------------
		| Check Beginning of Target String	|
		---------------------------------------*/
		*break_out = strncmp(p_string,t_string,strlen(p_string));
		if (*break_out)
		{
			free (t_string);
			free (p_string);
			return(0);
		}

		tptr = t_string + strlen(p_string);
		pptr = sptr + 1;
		while (*pptr == '*')
			pptr++;
	}
	else
	{
		*break_out = 0;
		tptr = t_string;
		pptr = p_string;
		while (*pptr == '*')
			pptr++;
	}

	if (!strlen(pptr))
	{
		free (t_string);
		free (p_string);
		return(1);
	}

	sptr = strchr (pptr,'*');
	/*-------------------------------
	| Check Other Wild Cards	|
	-------------------------------*/
	while (sptr != (char *)0)
	{
		*sptr = '\0';

		if (!(tptr = strstr (tptr, pptr)))
		{
			free (t_string);
			free (p_string);
			return(0);
		}
		tptr += strlen(pptr);
		pptr = sptr + 1;
		sptr = strchr (pptr,'*');
	}

	/*-----------------------
	| Look For Match At End	|
	-----------------------*/
	if (strlen(pptr) > 0)
	{
		/*--------------
		| Cannot Match 	|
		--------------*/
		if (strlen(pptr) > strlen(tptr))
		{
			free (t_string);
			free (p_string);
			return(0);
		}

		sptr = tptr + strlen(tptr) - strlen(pptr);

		i = !strcmp(sptr,pptr);
		free (t_string);
		free (p_string);
		return (i);
	}
	free (t_string);
	free (p_string);
	return (EXIT_FAILURE);
}

int
next (void)
{
	if (tot_recs < 1L)
	{
		print_at (21, 0, "%R There are no rows in the current list \007");
		sleep (sleepTime);
		init_rec (wrk_rec);
		dsp_rec ();
		cur_rec = -1L;
		dsp_count ();
		cur_rec++;
		return (EXIT_SUCCESS);
	}
	cur_rec++;
	if (cur_rec >= tot_recs)
	{
		cur_rec--;
		print_at (21, 0, "%R There are no more rows in the direction you are going \007");
		sleep (sleepTime);
		dsp_count ();
		return (EXIT_SUCCESS);
	}
	isrecnum = rec_nos[cur_rec];
	cc = dbfind (wrk_name, EQUAL, (char *) 0, &length, (char *) wrk_rec);
	dsp_count ();
	dsp_rec ();
    return (EXIT_SUCCESS);
}

int
previous (void)
{
	if (tot_recs < 1L)
	{
		print_at (21, 0, "%R There are no rows in the current list \007");
		sleep (sleepTime);
		init_rec (wrk_rec);
		dsp_rec ();
		cur_rec = -1L;
		dsp_count ();
		cur_rec++;
		return (EXIT_SUCCESS);
	}

	if (cur_rec < 1)
	{
		print_at (21, 0, "%R There are no more rows in the direction you are going \007");
		sleep (sleepTime);
		dsp_count ();
		return (EXIT_SUCCESS);
	}
	cur_rec--;
	isrecnum = rec_nos[cur_rec];
	cc = dbfind (wrk_name, EQUAL, (char *) 0, &length, (char *) wrk_rec);
	dsp_count ();
	dsp_rec ();
    return (EXIT_SUCCESS);
}

int
goto_rec (void)
{
   	/* int	new_rec; */
	long new_rec;

	if (tot_recs < 1L)
	{
		print_at (21, 0, "%R There are no rows in the current list \007");
		sleep (sleepTime);
		init_rec (wrk_rec);
		dsp_rec ();
		cur_rec = -1L;
		dsp_count ();
		cur_rec++;
		return (EXIT_SUCCESS);
	}

	move (0, 22);
	cl_line ();
	move (0, 23);
	cl_line ();
	print_at (22, 0, "%R Please enter desired record number: ");
	new_rec = 0;
	Getlong (38, 22, &new_rec, &new_rec);
	move (0, 22);
	cl_line ();
	move (0, 23);
	cl_line ();
	if (new_rec > 0 && new_rec <= tot_recs)
	{
		cur_rec = new_rec - 1;
		isrecnum = rec_nos[cur_rec];
		cc = dbfind (wrk_name, EQUAL, (char *) 0, &length, (char *) wrk_rec);
		dsp_count ();
		dsp_rec ();
		return (EXIT_SUCCESS);
	}
	print_at (21, 0, "%R The selected record is NOT in the current list! \007");
	sleep (sleepTime);
	dsp_count ();
    return (EXIT_SUCCESS);
}

int
add (void)
{
	int 	i;

	for (i = 0; i < wrk_cols; i++)
		sel_meth[i] = SEL_NONE;
	scn_offset = -SCN_DEPTH;
	screen ();

	if (tot_recs > 0)
	{
		isrecnum = rec_nos[cur_rec];
		cc = dbfind (wrk_name, EQUAL, (char *) 0, &length, (char *) qry_rec);
	}
	else
	{
		init_rec (qry_rec);
		dsp_rec ();
	}

	cc = input_rec (INP_ADD);
	if (!cc)
	{
		cc = dbadd (wrk_name, (char *) wrk_rec);
		if (cc)
		{
			print_at (21, 0, "%R Cannot create a new row! Error is:%d! \007", cc);
			sleep (sleepTime);
			dsp_count ();
			return (EXIT_SUCCESS);
		}
	}
	else
	{
		cc = dbfind (wrk_name, EQUAL, (char *) 0, &length, (char *) wrk_rec);
		if (tot_recs < 1L)
		{
			cur_rec = -1L;
			dsp_count ();
			cur_rec++;
		}
		else
			dsp_count ();
		return (EXIT_SUCCESS);
	}

	rec_nos[tot_recs] = isrecnum;
	cur_rec = tot_recs;
	tot_recs++;
	dsp_count ();

	cc = dbunlock (wrk_name);
	if (cc)
		file_err (cc, wrk_name, "DBUNLOCK");

	centre_at (1, _wide ? 132 : 80,
		" Total records: %-lu ",
		RecCount (wrk_name));
    return (EXIT_SUCCESS);
}

int
update (void)
{
	int 	i;

	if (tot_recs < 1L)
	{
		print_at (21, 0, "%R There are no rows in the current list \007");
		sleep (sleepTime);
		init_rec (wrk_rec);
		dsp_rec ();
		cur_rec = -1L;
		dsp_count ();
		cur_rec++;
		return (EXIT_SUCCESS);
	}

	cc = dbfind (wrk_name, EQUAL, (char *) 0, &length, (char *) qry_rec);
	cc = dbfind (wrk_name, EQUAL + LOCK, (char *) 0, &length, (char *) wrk_rec);
	if (cc)
	{
		print_at (21, 0, "%R Cannot read record from file for update! \007");
		sleep (sleepTime);
		print_at (21, 0, "%R This row has been locked by another user -- try again later \007");
		sleep (sleepTime);
		dsp_count ();
		return (EXIT_SUCCESS);
	}

	for (i = 0; i < wrk_cols; i++)
		sel_meth[i] = SEL_NONE;
	scn_offset = -SCN_DEPTH;
	screen ();

	cc = input_rec (INP_UPD);
	if (!cc)
	{
		if ((cc = dbupdate (wrk_name, (char *) wrk_rec)))
		{
			print_at (21, 0, "%R Cannot update this row! Error is:%d! \007", cc);
			sleep (sleepTime);
			dsp_count ();
			return (EXIT_SUCCESS);
		}
	}

	dsp_count ();
	cc = dbunlock (wrk_name);
	if (cc)
		file_err (cc, wrk_name, "DBUNLOCK");
    return (EXIT_SUCCESS);
}

int
remove_rec (void)
{
	if (tot_recs < 1L)
	{
		print_at (21, 0, "%R There are no rows in the current list \007");
		sleep (sleepTime);
		init_rec (wrk_rec);
		dsp_rec ();
		cur_rec = -1L;
		dsp_count ();
		cur_rec++;
		return (EXIT_SUCCESS);
	}

	move (0, 22);
	cl_line ();
	move (0, 23);
	cl_line ();
	run_menu (_remove_menu, "", 22);

	centre_at (1, _wide ? 132 : 80,
		" Total records: %-lu ",
		RecCount (wrk_name));

    return (EXIT_SUCCESS);
}

int
rmv_yes (void)
{
	isrecnum = rec_nos[cur_rec];
	cc = dbfind (wrk_name, EQUAL + LOCK, (char *) 0, &length, (char *) wrk_rec);
	if (cc)
	{
		print_at (21, 0, "%R Cannot read record from file for remove! \007");
		sleep (sleepTime);
		print_at (21, 0, "%R This row has been locked by another user -- try again later \007");
		sleep (sleepTime);
		dsp_count ();
		return (EXIT_SUCCESS);
	}

	cc = dbdelete (wrk_name);
	if (cc)
		file_err (cc, wrk_name, "DBDELETE");
	cc = dbunlock (wrk_name);
	if (cc)
		file_err (cc, wrk_name, "DBUNLOCK");
	for (cc = cur_rec; cc < tot_recs; cc++)
		rec_nos[cc] = rec_nos[cc + 1];
	tot_recs--;
	if (cur_rec >= tot_recs)
		cur_rec--;
	if (tot_recs > 0L)
	{
		isrecnum = rec_nos[cur_rec];
		cc = dbfind (wrk_name, EQUAL, (char *) 0, &length, (char *) wrk_rec);
		dsp_count ();
	}
	else
	{
		init_rec (wrk_rec);
		dsp_rec ();
		cur_rec = -1L;
		dsp_count ();
		cur_rec++;
	}

	dsp_rec ();
	return (FN16);
}

int
rmv_no (void)
{
	return (FN16);
}

/*
Please see comments in the beginning of run_form().

omirp
*/

int
screen (void)
{
	int	i,
		j,
		k;

	int _w, ix,x=0,x1;
   
    /*	
	if (multiple_lines_used)
	{
		scn_offset += columns_used;
	}
	else
	{
		scn_offset += SCN_DEPTH;
    }
    */

    scn_offset += SCN_DEPTH;

	if (scn_offset >= wrk_cols)
	{
	   	 scn_offset = 0;
    }

	for (i = 0; i < SCN_DEPTH; i++)
	{
	    k = i + SCN_START+x;

		if (i + scn_offset < wrk_cols)
	    {
			j = i + scn_offset;

	        _w = _wide ? 132 : 80;
			_w -= (VAL_POS + 1);

			workOffset	= (qry_rec == wrk_rec) ? sel_meth[j] : SEL_NONE;
        	if ( wrk_list[j].vwlen <= _w || wrk_list[j].vwtype != CHARTYPE)
			{
				print_at (k, PRMT_POS, "%-18.18s%-2.2s[",
								wrk_list[j].vwname,
								slct_str[workOffset]);
			}
			 
			switch (wrk_list[j].vwtype)
			{
			case	CHARTYPE:
				_w = _wide ? 132 :80;
				_w -= (VAL_POS + 1);

				if (wrk_list[j].vwlen <= _w)
				{
					printf ("%-*.*s]",
			    	wrk_list[j].vwlen,
			    	wrk_list[j].vwlen,
				    " ");
				}
				else
 				{

	 			    int _wx;

					workOffset	= (qry_rec == wrk_rec) ? sel_meth[j] : SEL_NONE;
				    print_at(k,PRMT_POS,"%-18.18s%-2.2s[",
					  			wrk_list[j].vwname,
				    			slct_str[workOffset]);

				    printf("%-*.*s]",_w,_w," ");

				    x1 = wrk_list[j].vwlen/_w;
        	        _wx = wrk_list[j].vwlen - _w;

				    for (ix =0; ix <x1; ix++)
				    {
						x++;
            	        print_at(++k,PRMT_POS,"%18.18s%-2.2s[","","");

	 					printf ("%-*.*s]",
						   _wx,_wx," ");
                    	_wx -= _w;
			    	}

				}

				break;

			case	INTTYPE:
				printf ("%-6.6s]",
				    " ");
				break;

			case	LONGTYPE:
				printf ("%-11.11s]",
				    " ");
				break;

			case	DOUBLETYPE:
				printf ("%-17.17s]",
				    " ");
				break;

			case	FLOATTYPE:
				printf ("%-9.9s]",
				    " ");
				break;

			case	SERIALTYPE:
				printf ("%-11.11s]",
				    " ");
				break;

			case	DATETYPE:
				if (FullYear())
					printf ("%-10.10s]", " ");
				else
					printf ("%-8.8s]", " ");
				break;

			case	MONEYTYPE:
				printf ("%-17.17s] (Money)",
				    " ");
				break;
			}
			cl_line ();
	    }
	    else
	    {
			move (0, k);
			cl_line ();
	    }
	}

	if (cur_rec < tot_recs)
	    dsp_rec ();

	if (tot_recs < 1L)
	{
	    cur_rec = -1L;
	    dsp_count ();
	    cur_rec++;
	}
	else
	    dsp_count ();

    return (EXIT_SUCCESS);
}
 
/*
Code inserted here to accomodate multi-line display.  Please see comments
from run_form() and etc.

omirp
*/
void
dsp_rec (void)
{
	int	i,
		j,
		k;
	union	_type_var
	{
		char	*cptr;
		int	*iptr;
		long	*lptr;
		float	*fptr;
		double	*dptr;
	} rec_ptr;

	int _w,x=0,x1,ix;
    char temp_buf[132]; /* note the width of array */
    char *temp_bufx;

	for (i = 0; i < SCN_DEPTH; i++)
	{
	    k = i + SCN_START+x;
	    j = i + scn_offset;
	    if (j >= wrk_cols)
	    {
			move (0, k);
			cl_line ();
			continue;
	    }

	    rec_ptr.cptr = (char *) wrk_rec + wrk_list[j].vwstart;
	    switch (wrk_list[j].vwtype)
	    {
	    case	CHARTYPE:

        _w = _wide ? 132 : 80;
        _w -= (VAL_POS +1);

		if (wrk_list[j].vwlen <= _w)
		{
			print_at (k, VAL_POS, "%-*.*s",
		    	wrk_list[j].vwlen,
		    	wrk_list[j].vwlen,
		    	rec_ptr.cptr);
		}
		else
		{
			int _wx;

			x1 = (wrk_list[j].vwlen/_w)+1;
			temp_bufx= rec_ptr.cptr;
        	_wx = wrk_list[j].vwlen - _w ;
            
			memset(temp_buf,0,132);
			strncpy(temp_buf,temp_bufx,_w);
            print_at(k,VAL_POS,"%-*.*s", _w,_w,temp_buf)  ;
		
			temp_bufx += _w;

			for (ix =1; ix < x1; ix++)
			{
               memset(temp_buf,0,132);
               strncpy(temp_buf,temp_bufx,_wx > _w ? _w : _wx);
               print_at(++k,VAL_POS,"%-*.*s", _wx,_wx,temp_buf)  ;

			   temp_bufx += _wx > _w ? _w : _wx;
               _wx -= _w;
			   x++;

			}

		}

		break;

	    case	INTTYPE:
		print_at (k, VAL_POS, "%-6d", *(rec_ptr.iptr));
		break;

	    case	LONGTYPE:
		print_at (k, VAL_POS, "%-11ld", *(rec_ptr.lptr));
		break;

	    case	DOUBLETYPE:
		sprintf (inp_buf, "%-32.15f", *(rec_ptr.dptr));
		num_fmt (17);
		print_at (k, VAL_POS, "%-17.17s", inp_buf);
		break;

	    case	FLOATTYPE:
		sprintf (inp_buf, "%-16.7f", *(rec_ptr.fptr));
		num_fmt (9);
		print_at (k, VAL_POS, "%-9.9s", inp_buf);
		break;

	    case	SERIALTYPE:
		print_at (k, VAL_POS, "%-11ld", *(rec_ptr.lptr));
		break;

	    case	DATETYPE:
		print_at (k, VAL_POS, "%s", DateToString (*(rec_ptr.lptr)));
		break;

	    case	MONEYTYPE:
		print_at (k, VAL_POS, "%-17.*f", (wrk_list[j].vwlen & 0xff),
		    DOLLARS (*(rec_ptr.dptr)));
		break;
	    }
	}

}

void
num_fmt (
 int                len)
{
	char	junk[40];
	int	i;

	strcpy (junk, inp_buf);
	clip (junk);
	lclip (junk);
	i = strlen (junk);
	i--;
	while (junk[i] == '0' && junk[i - 1] != '.')
	{
		junk[i] = ' ';
		i--;
	}
	junk[len] = 0;
	strcpy (inp_buf, junk);
}

/*
Code inserted here to accomodate mulitple line entry for very wide
fields

omirp
*/

int
input_rec (
 int                mode)
{
	int	i,
		j=0,
		k,
		do_exit = FALSE;
    int _w,x=0,x1,ix;

	union	_type_var
	{
		char	*cptr;
		int	*iptr;
		long	*lptr;
		float	*fptr;
		double	*dptr;
	} res_ptr, rec_ptr;

	/*-------------------------------
	| Pre-initialise record for add	|
	-------------------------------*/
	if (mode != INP_UPD)
	{
	    init_rec ((mode == INP_QRY) ? qry_rec : wrk_rec);
	    dsp_rec ();
	}

	i = 0;
	while (!do_exit)
	{
	    k = i % SCN_DEPTH;
	    j = k + SCN_START+x;
	    res_ptr.cptr = (char *) qry_rec + wrk_list[i].vwstart;
	    if (mode == INP_QRY)
		rec_ptr.cptr = (char *) qry_rec + wrk_list[i].vwstart;
	    else
		rec_ptr.cptr = (char *) wrk_rec + wrk_list[i].vwstart;
	
	    switch (wrk_list[i].vwtype)
	    {
	    case	CHARTYPE:
        _w = _wide ? 132 : 80;
		_w -= (VAL_POS+1);
        
		if (wrk_list[i].vwlen < _w)
		{
			Getalpha (VAL_POS, j, (char *) rec_ptr.cptr, (char *) res_ptr.cptr, wrk_list[i].vwlen);
		}
		else
		{
		   int jx = j; 
		   char *temp_buf;    		
		   char *temp_bufs;
           char temp_bufxs[5][132];
		   char temp_bufx[5][132]; /* assume 132 * 5 the maximumu width */
           int	_wx = wrk_list[i].vwlen  ;
           x1 = (wrk_list[i].vwlen/_w)+1;

		   temp_buf = (char *)rec_ptr.cptr;
		   temp_bufs = (char *)res_ptr.cptr;
           
           for (ix =0; ix < x1; ix++)
		   {
			  x++;
			  memset(temp_bufx[ix],0,132);
			  strncpy(temp_bufx[ix],temp_buf,_wx > _w ? _w : _wx);

              memset(temp_bufxs[ix],0,132);
			  strncpy(temp_bufxs[ix],temp_bufs,_wx > _w ? _w : _wx);

              Getalpha (VAL_POS,jx++,temp_bufx[ix],temp_bufxs[ix],_wx > _w ? _w : _wx);
              temp_buf += _wx > _w ? _w : _wx;
			  temp_bufs += _wx > _w ? _w : _wx;

			  _wx -= _w;

		   }
           x--;  
		   
		   for (ix =0; ix < x1; ix++)
		   {
			   strcpy(rec_ptr.cptr,temp_bufx[ix]);
			   strcpy(res_ptr.cptr,temp_bufxs[ix]);
               
			   rec_ptr.cptr += _w ;
			   res_ptr.cptr += _w;

		   }

		}
        
	  	if (!dflt_used && mode == INP_QRY)
	   	    sel_meth[i] = qry_meth (j);

		break;

	    case	INTTYPE:
		Getint (VAL_POS, j, rec_ptr.iptr, res_ptr.iptr);
		if (!dflt_used && mode == INP_QRY)
		    sel_meth[i] = qry_meth (j);
		break;

	    case	LONGTYPE:
		Getlong (VAL_POS, j, rec_ptr.lptr, res_ptr.lptr);
		if (!dflt_used && mode == INP_QRY)
		    sel_meth[i] = qry_meth (j);
		break;

	    case	DOUBLETYPE:
		Getdouble (VAL_POS, j, rec_ptr.dptr, res_ptr.dptr);
		if (!dflt_used && mode == INP_QRY)
		    sel_meth[i] = qry_meth (j);
		break;

	    case	FLOATTYPE:
		Getfloat (VAL_POS, j, rec_ptr.fptr, res_ptr.fptr);
		if (!dflt_used && mode == INP_QRY)
		    sel_meth[i] = qry_meth (j);
		break;

	    case	SERIALTYPE:
		if (mode == INP_QRY)
		{
		    Getlong (VAL_POS, j, rec_ptr.lptr, res_ptr.lptr);
		    if (!dflt_used)
			sel_meth[i] = qry_meth (j);
		}
		else
		    if (last_char != UP_KEY)
			last_char = DOWN_KEY;
		break;

	    case	DATETYPE:
		GetDate (VAL_POS, j, rec_ptr.lptr, res_ptr.lptr);
		if (!dflt_used && mode == INP_QRY)
		    sel_meth[i] = qry_meth (j);
		break;

	    case	MONEYTYPE:
		Getmoney (VAL_POS, j, rec_ptr.dptr, res_ptr.dptr, i);
		if (!dflt_used && mode == INP_QRY)
		    sel_meth[i] = qry_meth (j);
		break;
	    }

	    switch (last_char)
	    {
	    case	FN1:
		return (EXIT_FAILURE);
		break;

	    case	FN16:
		return (EXIT_SUCCESS);

	    case	UP_KEY:
		j = i % SCN_DEPTH;
		i--;
		if (i < 0)
		{
		    i = wrk_cols - 1;
			x=0;
		}

		if (j == 0)
		{
		    j = i / SCN_DEPTH;
			x=0;
		    scn_offset = (j * SCN_DEPTH) - SCN_DEPTH;
		    screen ();
		    dsp_rec ();
		}
		break;

	    case	DOWN_KEY:
	    case	'\r':
		i++;
		if (i >= wrk_cols)
		{
		    i = 0;
			x=0;
		    scn_offset = -SCN_DEPTH;
		    screen ();
		    dsp_rec ();
		    break;
		}
		j = i % SCN_DEPTH;
		if (j == 0)
		{
		    screen ();
		    dsp_rec ();
			x=0;
		}
		break;

	    default:
		break;
	    }
	}

	return (EXIT_SUCCESS);
}

void
init_rec (
 char*              rec)
{
	int	i;
	union	_type_var
	{
		char	*cptr;
		int	*iptr;
		long	*lptr;
		float	*fptr;
		double	*dptr;
	} rec_ptr;


	for (i = 0; i < wrk_cols; i++)
	{
		rec_ptr.cptr = (char *) rec + wrk_list[i].vwstart;
		switch (wrk_list[i].vwtype)
		{
		case	CHARTYPE:
			sprintf (rec_ptr.cptr, "%-*.*s",
				wrk_list[i].vwlen,
				wrk_list[i].vwlen,
				" ");
			break;

		case	INTTYPE:
			*rec_ptr.iptr = 0;
			break;

		case	LONGTYPE:
		case	SERIALTYPE:
		case	DATETYPE:
			*rec_ptr.lptr = 0L;
			break;

		case	FLOATTYPE:
			*rec_ptr.fptr = (float) 0.00;
			break;

		case	DOUBLETYPE:
		case	MONEYTYPE:
			*rec_ptr.dptr = (float) 0.00;
			break;
		}
	}
}

void
Getint (
 int                x,
 int                y,
 int*               iptr,
 int*               ires)
{
	val_chrs = "-0123456789";
	sprintf (inp_buf, "%-6d", *iptr);
	sprintf (res_buf, "%-6d", *ires);
	cc = get_inp (x, y, 6);
	if (cc != FN1)
		*iptr = atoi (inp_buf);
}

void
Getlong (
 int                x,
 int                y,
 long*              lptr,
 long*              lres)
{
	val_chrs = "-0123456789";
	sprintf (inp_buf, "%-11ld", *lptr);
	sprintf (res_buf, "%-11ld", *lres);
	cc = get_inp (x, y, 11);
	if (cc != FN1)
		*lptr = atol (inp_buf);
}

void
Getfloat (
 int                x,
 int                y,
 float*             fptr,
 float*             fres)
{
	val_chrs = "-.0123456789";
	sprintf (inp_buf, "%16.7f", *fres);
	num_fmt (9);
	strcpy (res_buf, inp_buf);
	sprintf (inp_buf, "%16.7f", *fptr);
	num_fmt (9);
	cc = get_inp (x, y, 9);
	if (cc != FN1)
	{
		*fptr = (float) atof (inp_buf);
		sprintf (inp_buf, "%16.7f", *fptr);
		num_fmt (9);
		print_at (y, x, inp_buf);
	}
}

void
Getdouble (
 int                x,
 int                y,
 double*            dptr,
 double*            dres)
{
	val_chrs = "-.0123456789";
	sprintf (inp_buf, "%32.15f", *dres);
	num_fmt (17);
	strcpy (res_buf, inp_buf);
	sprintf (inp_buf, "%32.15f", *dptr);
	num_fmt (17);
	cc = get_inp (x, y, 17);
	if (cc != FN1)
	{
		*dptr = atof (inp_buf);
		sprintf (inp_buf, "%32.15f", *dptr);
		num_fmt (17);
		print_at (y, x, inp_buf);
	}
}

void
Getmoney (
 int                x,
 int                y,
 double*            dptr,
 double*            dres,
 int                i)
{
	val_chrs = "-.0123456789";
	sprintf (inp_buf, "%32.15f", (*dres) / (double) 100.00);
	num_fmt (17);
	strcpy (res_buf, inp_buf);
	sprintf (inp_buf, "%32.15f", (*dptr) / (double) 100.00);
	num_fmt (17);
	cc = get_inp (x, y, 17);
	if (cc != FN1)
	{
		*dptr = (atof (inp_buf)) * (double) 100.00;
		*dptr = no_dec (*dptr);
		print_at (y, x, "%-17.*f",
		    (wrk_list[i].vwlen & 0xff),
		    DOLLARS (*dptr));
	}
}

void
GetDate (
 int                x,
 int                y,
 long*              lptr,
 long*              lres)
{
	val_chrs = "0123456789";
	sprintf (inp_buf, DateToString (*lres));
	inp_buf[2] = inp_buf[3];
	inp_buf[3] = inp_buf[4];
	inp_buf[4] = inp_buf[6];
	inp_buf[5] = inp_buf[7];
	if (FullYear())
	{
		inp_buf[6] = inp_buf[8];
		inp_buf[7] = inp_buf[9];
	}
	else
	{
		inp_buf[6] = ' ';
		inp_buf[7] = ' ';
	}
	strcpy (res_buf, inp_buf);
	sprintf (inp_buf, DateToString (*lptr));
	inp_buf[2] = inp_buf[3];
	inp_buf[3] = inp_buf[4];
	inp_buf[4] = inp_buf[6];
	inp_buf[5] = inp_buf[7];
	if (FullYear())
	{
		inp_buf[6] = inp_buf[8];
		inp_buf[7] = inp_buf[9];
		inp_buf[8] = ' ';
		inp_buf[9] = ' ';
	}
	else
	{
		inp_buf[6] = ' ';
		inp_buf[7] = ' ';
	}
	if (FullYear())
		cc = get_inp (x, y, 8);
	else
		cc = get_inp (x, y, 6);
	if (cc != FN1)
	{
		if (FullYear())
		{
			inp_buf[9] = (inp_buf[7] == ' ') ? '0' : inp_buf[7];
			inp_buf[8] = (inp_buf[6] == ' ') ? '0' : inp_buf[6];
		}
		inp_buf[7] = (inp_buf[5] == ' ') ? '0' : inp_buf[5];
		inp_buf[6] = (inp_buf[4] == ' ') ? '0' : inp_buf[4];
		inp_buf[5] = '/';
		inp_buf[4] = (inp_buf[3] == ' ') ? '0' : inp_buf[3];
		inp_buf[3] = (inp_buf[2] == ' ') ? '0' : inp_buf[2];
		inp_buf[2] = '/';
		inp_buf[1] = (inp_buf[1] == ' ') ? '0' : inp_buf[1];
		inp_buf[0] = (inp_buf[0] == ' ') ? '0' : inp_buf[0];
		print_at (y, x, inp_buf);
		*lptr = StringToDate (inp_buf);
	}
}

void
Getalpha (
 int                x,
 int                y,
 char*              cptr,
 char*              cres,
 int                len)
{
	val_chrs = " !\"#$%&'()*+,-./0123456789;:<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
	strcpy (res_buf, cres);
	strcpy (inp_buf, cptr);
	cc = get_inp (x, y, len);
	if (cc != FN1)
		sprintf (cptr, "%-*.*s", len, len, inp_buf);
}

int
get_inp (
 int                x,
 int                y,
 int                len)
{
	int	j,
		init = TRUE,
		pos,
		chr,
		insert = FALSE,
		do_exit = FALSE;

	print_at (y, x, inp_buf);
	move (x, y);
	pos = 0;
	crsr_on ();
	while (!do_exit)
	{
	    chr = getkey ();
	    last_char = chr;
	    switch (chr)
	    {
	    case	INSCHAR:
	    case	INSLINE:
		init = FALSE;
		insert = TRUE;
		break;

	    case	DELCHAR:
	    case	DELLINE:
		init = FALSE;
		insert = FALSE;
		break;

	    case	FN2:
		strcpy (inp_buf, res_buf);
		print_at (y, x, inp_buf);
		last_char = DOWN_KEY;
	    case	FN1:
	    case	FN16:
	    case	UP_KEY:
	    case	DOWN_KEY:
	    case	'\r':
		do_exit = TRUE;
		break;

	    case	LEFT_KEY:
	    case	BS:
		init = FALSE;
		if (pos > 0)
		{
		    putchar (BS);
		    pos--;
		}
		else
			putchar (BELL);
		break;

	    case	RIGHT_KEY:
		init = FALSE;
		if (pos < len)
		{
		    pos++;
		    move (x + pos, y);
		}
		else
			putchar (BELL);
		break;

	    case	0x18:		/* Ctrl (X)	*/
		init = FALSE;
		for (j = pos; j < len; j++)
		    inp_buf[j] = inp_buf[j + 1];
		inp_buf[len] = ' ';
		print_at (y, x, "%-*.*s", len, len, inp_buf);
		move (x + pos, y);
		break;

	    default:
		if (strchr (val_chrs, chr) && pos < len)
		{
		    if (init)
		    {
			sprintf (inp_buf, "%-*.*s", len, len, " ");
			print_at (y, x, inp_buf);
			move (x, y);
			init = FALSE;
		    }

		    if (insert)
		    {
			for (j = len; j > pos; j--)
			    inp_buf[j] = inp_buf[j - 1];
			inp_buf[pos] = chr;
			print_at (y, x, "%-*.*s", len, len, inp_buf);
			pos++;
			move (x + pos, y);
		    }
		    else
		    {
			inp_buf[pos] = chr;
			pos++;
			putchar (chr);
		    }
		}
		else
		{
		    init = FALSE;
		    putchar (BELL);
		}
		break;
	    }
	}
	crsr_off ();
	dflt_used = init;
	return (chr);
}

int
qry_meth (
 int                line)
{
	run_menu (_query_menu, "", 22);
	print_at (line, QRY_POS, "%-2.2s", slct_str[cur_qry_meth]);
	return (cur_qry_meth);
}

int
qry_none (void)
{
	cur_qry_meth = SEL_NONE;
	return (FN16);
}

int
qry_equal (void)
{
	cur_qry_meth = SEL_EQUAL;
	return (FN16);
}

int
qry_nequal (void)
{
	cur_qry_meth = SEL_NEQUAL;
	return (FN16);
}

int
qry_gteq (void)
{
	cur_qry_meth = SEL_GTEQ;
	return (FN16);
}

int
qry_greater (void)
{
	cur_qry_meth = SEL_GREATER;
	return (FN16);
}

int
qry_lteq (void)
{
	cur_qry_meth = SEL_LTEQ;
	return (FN16);
}

int
qry_less (void)
{
	cur_qry_meth = SEL_LESS;
	return (FN16);
}

void
dsp_count (void)
{
	move (0, 21);
	if (_wide)
	{
		line (53);
		printf ("> ");
		rv_on ();
		printf (" Record %6ld of %6ld ", cur_rec + 1, tot_recs);
		rv_off ();
		printf (" <");
		line (52);
	}
	else
	{
		line (27);
		printf ("> ");
		rv_on ();
		printf (" Record %6ld of %6ld ", cur_rec + 1, tot_recs);
		rv_off ();
		printf (" <");
		line (26);
	}
}

int
re_draw (void)
{
	int		width = _wide ? 132 : 80;
	char	*title = super_user ?
				" L O G I S T I C   T A B L E   E D I T O R " :
				" L O G I S T I C   T A B L E   V I E W E R ";

	clear ();
	if (_wide)
	    swide ();
	else
	    snorm ();
	rv_pr (title, 17, 0, TRUE);
	line_at (1, 0, width);

	centre_at (1, _wide ? 132 : 80,
		" Total records: %-lu ",
		RecCount (wrk_name));

	scn_offset -= SCN_DEPTH;
	screen ();
    return (EXIT_SUCCESS);
}

static unsigned long
RecCount (
 char*              table)
{
	/*
	 *	Returns the number of rows found in "table" (according to systables)
	 */
	memset (&tables_rec, 0, sizeof (tables_rec));
	strcpy (tables_rec.tabname, table);

	if (find_rec (tables, &tables_rec, GTEQ, "r") ||	/* not there */
		strcmp (clip (tables_rec.tabname), table))		/* bad name */
	{
		return (EXIT_SUCCESS);
	}
	return (tables_rec.nrows);
}
