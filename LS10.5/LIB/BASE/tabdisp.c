/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( tabdisp.c      )                                 |
|  Program Desc  : ( Table display and maintenance routines.      )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
| Date Written   : (01/07/89)      | Author      : Huon Butterworth   |
|---------------------------------------------------------------------|
|  Date Modified : (31/05/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (26/08/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (02/07/99)      | Modified by : Trevor van Bremen  |
|                                                                     |
|  Comments                                                           |
|     (31/05/93) : Added additional check in tab_save() to ensure     |
|                  fprintf() errors are return on write failures.     |
|     (26/08/93) : Putting the lib into SCCS introduced problem of    |
|                  1 being replaced with Release numbers!             |
|                : Also fixed braindead macro conversion of SCR_WIDTH |
|     (02/07/99) : Updated to use stdarg.h instead of varargs.h       |
-----------------------------------------------------------------------
	$Log: tabdisp.c,v $
	Revision 5.1  2001/08/06 22:40:57  scott
	RELEASE 5.0
	
	Revision 5.0  2001/06/19 06:59:40  cha
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.0  2001/03/09 00:52:39  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 3.0  2000/10/12 13:34:30  gerry
	Revision No. 3 Start
	<after Rel-10102000>
	
	Revision 2.1  2000/08/10 01:56:25  scott
	Updated to increase limit on number of lines.
	
	Revision 2.0  2000/07/15 07:17:19  gerry
	Forced revision no. to 2.0 - Rel-15072000
	
	Revision 1.7  1999/11/10 06:42:59  scott
	Updated for PauseKey X/Y, better fixed now than to confuse forever
	
	Revision 1.6  1999/11/10 03:56:11  jonc
	Removed conflicting internal decl. of strdup which stops compile on
	RS6000 boxes.
	
=====================================================================*/
#include	<std_decs.h>

int	tab_max_page = 2000;

char	*tab_name (char *nm);
TAB_PTR	find_tabname (char *f_name), add_tabname (char *f_name);
extern	char	temp_str [];

static	KEY_PTR	get_xtra (KEY_PTR xtra_keys);
static	int	_tab_save (TAB_PTR work_tab, int add_flag, char *tmp_str);
static	int	mem_get (TAB_PTR work_tab, char *l_buf, int get_flag, int line_no);
static	int	file_get (TAB_PTR work_tab, char *l_buf, int get_flag, int line_no);
static	void	_redraw_page (TAB_PTR work_tab);
static	void	_redraw_line (TAB_PTR work_tab, int r_flag);
static	void	_draw_keys (TAB_PTR work_tab);
static	void	_draw_heading (TAB_PTR work_tab, int r_flag);
static	void	_draw_table(TAB_PTR work_tab);
static	void	_clear_tab (TAB_PTR work_tab);
static	void	_blank_table (TAB_PTR work_tab);
static	int	_load_tabpage(TAB_PTR work_tab, int p_no);
static	int	_load_tabline(TAB_PTR work_tab, int line_no);
static	int	_tab_halt(TAB_PTR work_tab);
static	int	pre_func (int c, KEY_PTR unused);
static	int	post_func (int iUnused, KEY_PTR unused);
static	int	up_func (int c, KEY_PTR ununsed);
static	int	down_func (int c, KEY_PTR ununsed);
static	int	f3_func (int c, KEY_PTR ununsed);
static	int	f14_func (int c, KEY_PTR ununsed);
static	int	f15_func (int c, KEY_PTR ununsed);
static	int	f16_func (int c, KEY_PTR ununsed);
static	void	tab_seek (TAB_PTR work_tab, long int offset, int type);
static	void	tab_end (TAB_PTR work_tab);

static	KEY_TAB	disp_tab [] =
	{
	   { "[NXT]",	FN14,      f14_func, "Display next page.",	"P" },
	   { "[PRV]",	FN15,      f15_func, "Display previous page.",	"P" },
	   { "[END]",	FN16,      f16_func, "Exit table."		    },

	   { NULL,	FN1,	   f16_func	 },
	   { NULL,	FN3, 	   f3_func	 },

	   { NULL,	'\b',      up_func	 },
	   { NULL,	UP_KEY,    up_func	 },
	   { NULL,	11,        up_func 	 },

	   { NULL,	' ',  	   down_func	 },
	   { NULL,	DOWN_KEY,  down_func	 },
	   { NULL,	10,        down_func	 },

	   { NULL,	LEFT_KEY,  f15_func	 },
	   { NULL,	8,         f15_func	 },
	   { NULL,	9,         f15_func	 },

	   { NULL,	RIGHT_KEY, f14_func	 },
	   { NULL,	12,        f14_func	 },
	   END_KEYS
	};

/*
.loc_func
	Function	:	get_xtra ()

	Description	:	Get extra user-defined hot-keys for table.

	Notes		:	This function allows the programmer to specify
				his own hot-keys to be used in addition to or 
				instead off the table default keys.
			
	Parameters	:	xtra_keys - Pointer to a table of hot-keys.

	Returns		:	new_keys  - New table of hot-keys.
.end
*/
static KEY_PTR	get_xtra (KEY_PTR xtra_keys)
{
	register	KEY_PTR	key_ptr, n_ptr;
	register	int	cnt;
	KEY_PTR		new_keys;

	if (!xtra_keys)
		return (disp_tab);

	for (key_ptr = disp_tab, cnt = 0; KEY_VAL; key_ptr++, cnt++)
		;
	for (key_ptr = xtra_keys; KEY_VAL; key_ptr++, cnt++)
		;
		
	if (!(new_keys = (KEY_PTR) malloc ((cnt + 1) * sizeof (KEY_TAB))))
		file_err (-1, "get_xtra", "MALLOC");
	memset (new_keys, 0, (cnt + 1) * sizeof (KEY_TAB));

	for (key_ptr = xtra_keys, n_ptr = new_keys; KEY_VAL; key_ptr++, n_ptr++)
		pin_bcopy ((char *) n_ptr, (char *) key_ptr, sizeof (KEY_TAB));
		
	for (key_ptr = disp_tab; KEY_VAL; key_ptr++, n_ptr++)
		pin_bcopy ((char *) n_ptr, (char *) key_ptr, sizeof (KEY_TAB));

	return (new_keys);
}

/*
.function
	Function	:	tab_open ()

	Description	:	Open Table Work File For Writing.

	Notes		:	Tab_open initialises an entry in the list of
				active tables.
			
	Parameters	:	f_name    - Pointer to table name.
				xtra_keys - Pointer to table of user-defined
					    hot-keys.
				row       - Table start row.
				col       - Table start column.
				depth     - Table depth.
				mem_flag  - TRUE - Table memory resident.
.end
*/
void
tab_open (char *f_name, KEY_PTR xtra_keys, int row, int col, int depth, int mem_flag)
{
	TAB_PTR	work_tab;

	if (!(work_tab = add_tabname (f_name)))
		file_err (-1, "tab_open", "ADD_TABNAME");

	if (TAB_ID)
		fclose (TAB_ID);

	if (!mem_flag && !(TAB_ID = fopen(tab_name (f_name),"w+"))) 
		file_err (-1, "tab_open", "FOPEN");

	TAB_ROW = row;
	TAB_COL = col;
	TAB_SRCH = FALSE;
	TAB_DEPTH = depth;
	TAB_MEMORY = mem_flag;
	TAB_LNENO = TAB_LINES = 0;
	TAB_KEYS = get_xtra (xtra_keys);
}

/*
.function
	Function	:	tab_srch ()

	Description	:	change srch flag on table info

	Parameters	:	f_name    - Pointer to table name.
				srch      - new value for srch flag
.end
*/
void
tab_srch(char *f_name, int srch)
{
	TAB_PTR	work_tab;

	if (!(work_tab = find_tabname (f_name)))
		file_err (-1, "tab_keyset", "FIND_TABNAME");

	TAB_SRCH = srch;
}
/*
.function
	Function	:	tab_keyset ()

	Description	:	Select new set of hot-keys for active table.

	Parameters	:	f_name	  - Pointer to string containing name
					    of table.
				xtra_keys - Pointer to table of new hot-keys.
.end
*/
void
tab_keyset (char *f_name, KEY_PTR xtra_keys)
{
	TAB_PTR	work_tab;

	if (!(work_tab = find_tabname (f_name)))
		file_err (-1, "tab_keyset", "FIND_TABNAME");

	TAB_KEYS = get_xtra (xtra_keys);
}

/*
.function
	Function	:	tab_close ()

	Description	:	Close active table.

	Notes		:	Tab_close initialises the relevant table entry
				to NULLs and unlinks any work file created.
			
	Parameters	:	f_name - Pointer to string containing table
					 name.
				ul_flg - TRUE  - unlink and initialise.
					 FALSE - close only.
.end
*/
void
tab_close(char *f_name, int init_flag)
{
	TAB_PTR	work_tab;

	if (!(work_tab = find_tabname (f_name)))
		file_err (-1, "tab_close", "FIND_TABNAME");

	if (!TAB_MEMORY)
		fclose(TAB_ID);

	if (init_flag)
	{
		if (HDR_LINE)
			free (HDR_LINE);
		if (TAB_NAME)
			free (TAB_NAME);
		HDR_LINE = TAB_NAME = (char *) NULL;
		if (TAB_KEYS != disp_tab)
			free (TAB_KEYS);

		if (TAB_PAGE)
		{
			_clear_tab (work_tab);
			TAB_FWIDTH = TAB_SWIDTH = 0;
			free (TAB_PAGE);
		}

		if (TAB_BUF)
			free (TAB_BUF);

		if (!TAB_MEMORY)
			unlink(tab_name (f_name));
	}
}

/*
.function
	Function	:	tab_update ()

	Description	:	Update current record in table.

	Notes		:	This function is used to write the values 
				passed into the current record in the
				specified table using the format mask given.
			
	Parameters	:	f_name - Pointer to string containing table
					 name.
				mask   - Mask used to format the values to be
					 written into the table (see printf).
				ARGS   - Arguments to be written into table.

	Returns		:	0      - If update is successful.
				1      - If update fails.
.end
*/
int
tab_update (char *f_name, char *mask, ...)
{
	va_list	args;
	char	tsv_str[BUFSIZ];
	TAB_PTR	work_tab,
		find_tabname (char *f_name);

	va_start (args, mask);

	vsprintf (tsv_str, mask, args);
	va_end (args);

	if (!(work_tab = find_tabname (f_name)))
		file_err (-1, "tab_update", "FIND_TABNAME");

	if (_tab_save (work_tab, FALSE, tsv_str))
		return (EXIT_FAILURE);

	tab_seek (work_tab, (long) -(TAB_FWIDTH + 1), 1);
	return (EXIT_SUCCESS);
}

/*
.function
	Function	:	tab_add ()

	Description	:	Add a record to the table.

	Notes		:	This function is used to write the values 
				passed into the current record in the
				specified table using the format mask given.
			
	Parameters	:	f_name - Pointer to string containing table
					 name.
				mask   - Mask used to format the values to be
					 written into the table (see printf).
				ARGS   - Arguments to be written into table.

	Returns		:	0      - If update is successful.
				1      - If update fails.
.end
*/
int
tab_add (char *f_name, char *mask, ...)
{
	va_list	args;
	char	tsv_str[BUFSIZ];
	TAB_PTR	work_tab,
		find_tabname (char *f_name);

	va_start (args, mask);

	vsprintf (tsv_str, mask, args);
	va_end (args);

	if (!(work_tab = find_tabname (f_name)))
		file_err (-1, "tab_add", "FIND_TABNAME");

	tab_end (work_tab);

	if (_tab_save (work_tab, TRUE, tsv_str))
		return (EXIT_FAILURE);

	if (TAB_LINES++ && TAB_LINES <= TAB_DEPTH)
		TAB_LNENO++;

	return (EXIT_SUCCESS);
}

/*
.function
	Function	:	tab_overide ()

	Description	:	Override default table name generation.

	Parameters	:	nm    - Pointer to user-defined location for
					table.
.end
*/

static	char	*PV_name = (char *) NULL;

void
tab_overide (char *nm)
{
	if (PV_name)
		free (PV_name);

	PV_name = strdup (nm);
}

/*
.function
	Function	:	tab_name ()

	Description	:	Generate work file name for table.

	Parameters	:	nm     - Pointer to user-defined portion of
					 table name.

	Returns		:	tmp_nm - Pointer to static string containing 
					 generated name.
.end
*/
char	*tab_name (char *nm)
{
	char	*ep;
	static	char	tmp_nm [256];
	char	*getenv(const char *);

	if (!PV_name)
	{
		ep = getenv("PROG_PATH");
		sprintf(tmp_nm,"%s/WORK/%s%06d", (ep) ? ep : "/usr/LS10.5",
								nm, getpid());
	}
	else
		sprintf (tmp_nm, "%s", PV_name);

	return (tmp_nm);
}

/*
.loc_func
	Function	:	_tab_save ()

	Description	:	Routine used to write data into an active table.

	Notes		:	This function is used by the routines tab_add 
				and tab_update to write information into an
				active table.
			
	Parameters	:	work_tab - Pointer to TAB_STRUCT entry for table
					   to be written to.
				add_flag - TRUE if data is to be written to end
					   of table.
				string   - Arguments to be written into table.


	Returns		:	0	 - If write succeeds.
				1	 - If write fails.
.end
*/
static	int
_tab_save (TAB_PTR work_tab, int add_flag, char *tmp_str)
{
	extern	int		MlHide;
	char	*nptr;
	int	tmp_width = 0, 
		sr_err = 0;

	if ((nptr = strchr (tmp_str, '\n')))
		*nptr = (char) NULL;

	MlHide	=	1;

	/*-----------------------
	| Column Headings	|
	-----------------------*/
	if (tmp_str [0] == '#')
	{
		blank_at (23, 0, 79);

		if ((tmp_width = strlen (tmp_str) - 1) < 17)
			tmp_width = 17;

		if (!HDR_LINE)
		{
			if (!(HDR_LINE = malloc (tmp_width + 1)))
				file_err (-1, "tab_save", "MALLOC");
			memset (HDR_LINE, 0, sizeof (tmp_width + 1));

			if (!(TAB_PAGE = (long *) malloc (tab_max_page * sizeof (long))))
				file_err (-1, "tab_save", "MALLOC");
			memset (TAB_PAGE, 0, tab_max_page * sizeof (long));
		}
		tab_seek(work_tab, 0L, 0);

		TAB_PGENO = TAB_PSIZE = TAB_LNENO = TAB_LINES = 0;

		if (!TAB_MEMORY && clear_file (work_tab, 0L))
			file_err (errno, "tab_save", "CHSIZE");

		if (!TAB_SWIDTH || TAB_SWIDTH != tmp_width)
		{
			if (tmp_str[1] == '.')
				tmp_width--;

			TAB_SWIDTH = tmp_width;
			if (tmp_str[1] == '.')
				sprintf(HDR_LINE, "%-.*s", tmp_width + 1, tmp_str + 2);
			else
				sprintf(HDR_LINE, "%-.*s", tmp_width + 1, ML(tmp_str + 1));

			_blank_table (work_tab);
			_draw_table (work_tab);
		}
		else
		{
			if (tmp_str[1] == '.')
				sprintf(HDR_LINE, "%-.*s", tmp_width + 1, tmp_str + 2);
			else
				sprintf(HDR_LINE, "%-.*s", tmp_width + 1, ML(tmp_str + 1));
			_draw_heading (work_tab, TRUE);
		}

		return(0);
	}
	else if (!TAB_BUF)
	{
		tmp_width = strlen (tmp_str);
		TAB_FWIDTH = (tmp_width < TAB_SWIDTH) ? TAB_SWIDTH : tmp_width;
		if (!(TAB_BUF = malloc ((TAB_DEPTH + 2) * (TAB_FWIDTH + 2))))
			file_err (-1, "tab_save", "MALLOC");
		memset (TAB_BUF, 0, (TAB_DEPTH + 2) * (TAB_FWIDTH + 2));
	}

	if (TAB_LINES / TAB_DEPTH >= tab_max_page)
		return(1);

	if (TAB_SRCH && TAB_LINES && !(TAB_LINES % tab_max_page))
		sr_err = _tab_halt (work_tab);

	if (TAB_LINES && add_flag && !(TAB_LINES % 20))
		rv_pr(" Busy  ... ", 0, 23, TAB_LINES % 40 ? 0 : 1);

	sprintf(DET_LINE, "%-*.*s", FILE_WIDTH, tmp_str);

	if (!TAB_MEMORY)
	{
		if (add_flag)
		{
			tab_end (work_tab);
			if (!(MAX_LINES % TAB_DEPTH))
				TAB_PAGE[TAB_LINES / TAB_DEPTH] = ftell(TAB_ID);
		}

		if (fprintf (TAB_ID, "%-*.*s\n", FILE_WIDTH, tmp_str) < 0)
			return (EXIT_FAILURE);
		fflush (TAB_ID);
	}

	return(sr_err);
}

/*
.function
	Function	:	tab_get ()

	Description	:	Read information from an active table.

	Notes		:	Tab_get retrieves information from an active
				table and places it into the receiving buffer
				pointed to by l_buf.
			
	Parameters	:	f_name   - Pointer to string containing name of
					   table from which information is to
					   be read.
				l_buf    - Pointer to string that will receive
					   retrieved information.
				get_flag - Type of read being performed :

					     FIRST    - First entry in table.
					     CURRENT  - Current entry in table.
					     EQUAL    - Entry at specified
						        line in table.
					     NEXT     - Next entry in table.
					     PREVIOUS - Previous entry in table.
					     LAST     - Previous entry in table.
						
				line_no  -   Line in table to retreive.
					     (EQUAL only)

	Returns		:	0  	 - If read succeeds.
				1	 - If read fails.
.end
*/
int
tab_get (char *f_name, char *l_buf, int get_flag, int line_no)
{
	TAB_PTR	work_tab, find_tabname (char *f_name);

	if (!(work_tab = find_tabname (f_name)))
		file_err (-1, "tab_get", "FIND_TABNAME");

	if (TAB_MEMORY)
		return (mem_get (work_tab, l_buf, get_flag, line_no));
	else
		return (file_get (work_tab, l_buf, get_flag, line_no));
}

/*
.function
	Function	:	tab_tline ()

	Description	:	Returns current line in table.

	Parameters	:	f_name - Pointer to string containing table
					 name.

	Returns		:	Current line in table.
.end
*/
int
tab_tline (char *f_name)
{
	TAB_PTR	work_tab, find_tabname (char *f_name);

	if (!(work_tab = find_tabname (f_name)))
		file_err (-1, "tab_tline", "FIND_TABNAME");

	return ((TAB_PGENO * TAB_DEPTH) + TAB_LNENO);
}

/*
.function
	Function	:	tab_sline ()

	Description	:	Returns current line on screen.

	Parameters	:	f_name - Pointer to string containing table
					 name.

	Returns		:	Current line on screen.
.end
*/
int
tab_sline (char *f_name)
{
	TAB_PTR	work_tab, find_tabname (char *f_name);

	if (!(work_tab = find_tabname (f_name)))
		file_err (-1, "tab_sline", "FIND_TABNAME");

	return (SCR_ROW);
}

/*
.loc_func
	Function	:	mem_get ()

	Description	:	Read information from a memory resident table.

	Notes		:	Mem_get retrieves information from an active
				memory resident table and places it into the
				receiving buffer pointed to by l_buf.
			
	Parameters	:	f_name   - Pointer to string containing name of
					   table from which information is to
					   be read.
				l_buf    - Pointer to string that will receive
					   retrieved information.
				get_flag - Type of read being performed :

					     FIRST    - First entry in table.
					     CURRENT  - Current entry in table.
					     EQUAL    - Entry at specified
						        line in table.
					     NEXT     - Next entry in table.
					     PREVIOUS - Previous entry in table.
					     LAST     - Previous entry in table.
						
				line_no  -   Line in table to retreive.
					     (EQUAL only)

	Returns		:	0  	 - If read succeeds.
				1	 - If read fails.
.end
*/
static	int
mem_get (TAB_PTR work_tab, char *l_buf, int get_flag, int line_no)
{
	int	tmp_line	=	0;

	switch (get_flag)
	{
		case FIRST :
			tmp_line = TAB_LNENO = 0;
			break;

		case EQUAL :
			if (line_no > TAB_DEPTH - 1 || line_no < 0)
				return (EXIT_FAILURE);
			tmp_line = TAB_LNENO = line_no;
			break;

		case CURRENT :
			tmp_line = TAB_LNENO;
			break;

		case NEXT :
			if (TAB_LNENO + 1 >= TAB_DEPTH)
				return (EXIT_FAILURE);
			tmp_line = ++TAB_LNENO;
			break;

		case PREVIOUS :
			if (TAB_LNENO - 1 < 0)
				return (EXIT_FAILURE);
			tmp_line = --TAB_LNENO;
			break;

		case LAST :
			tmp_line = TAB_LNENO = TAB_DEPTH - 1;
			break;
	}

	if (l_buf)
		strcpy (l_buf, TAB_DETAIL (tmp_line));

	return (EXIT_SUCCESS);
}

/*
.loc_func
	Function	:	file_get ()

	Description	:	Read information from a disk resident table.

	Notes		:	File_get retrieves information from an active
				disk resident table and places it into the
				receiving buffer pointed to by l_buf.
			
	Parameters	:	f_name   - Pointer to string containing name of
					   table from which information is to
					   be read.
				l_buf    - Pointer to string that will receive
					   retrieved information.
				get_flag - Type of read being performed :

					     FIRST    - First entry in table.
					     CURRENT  - Current entry in table.
					     EQUAL    - Entry at specified
						        line in table.
					     NEXT     - Next entry in table.
					     PREVIOUS - Previous entry in table.
					     LAST     - Previous entry in table.
						
				line_no  -   Line in table to retreive.
					     (EQUAL only)

	Returns		:	0  	 - If read succeeds.
				1	 - If read fails.
.end
*/
static	int
file_get (TAB_PTR work_tab, char *l_buf, int get_flag, int line_no)
{
	long	off_set;
	char	*n_ptr, *fgets (char *, int, FILE *);

	switch (get_flag)
	{
		case FIRST :
			fseek(TAB_ID, 0L, 0);
			break;

		case EQUAL :
			if (fseek(TAB_ID, (long) line_no * (TAB_FWIDTH + 1), 0))
				return (EXIT_FAILURE);
			break;

		case CURRENT :
			off_set = TAB_PAGE[TAB_PGENO] +
					(TAB_LNENO * (TAB_FWIDTH + 1));
			if (fseek(TAB_ID, off_set, 0))
				return (EXIT_FAILURE);
			break;

		case NEXT :
			if (fseek(TAB_ID, (long) TAB_FWIDTH + 1, 1))
				return (EXIT_FAILURE);
			break;

		case LAST :
			tab_end (work_tab);

		case PREVIOUS :
			if (fseek(TAB_ID, (long) -(TAB_FWIDTH + 1), 1))
				return (EXIT_FAILURE);
			break;
	}

	if (l_buf)
	{
		if (!fgets (l_buf, TAB_FWIDTH + 2, TAB_ID) ||
					fseek (TAB_ID, (long) -(TAB_FWIDTH + 1), 1))
			return (EXIT_FAILURE);
	}

	if ((line_no = (ftell (TAB_ID) / (TAB_FWIDTH + 1))) >= MAX_LINES)
		return (EXIT_FAILURE);

	TAB_PGENO = line_no / TAB_DEPTH;
	TAB_LNENO = line_no - (TAB_PGENO * TAB_DEPTH);
	if (l_buf)
		if ((n_ptr = strchr (l_buf, '\n')))
			*n_ptr = (char) NULL;

	return (EXIT_SUCCESS);
}

/*
.function
	Function	:	redraw_form ()

	Description	:	Redraw box and headings for table.

	Parameters	:	f_name - Pointer to string containing table
					 name.
.end
*/
void
redraw_form (char *f_name)
{
	TAB_PTR	work_tab;

	if ((work_tab = find_tabname (f_name)))
		_draw_table (work_tab);
}

/*
.function
	Function	:	redraw_heading ()

	Description	:	Redraw headings for table.

	Parameters	:	f_name - Pointer to string containing table
					 name.
				r_flag - If true print in reverse video.
.end
*/
void
redraw_heading (char *f_name, int r_flag)
{
	TAB_PTR	work_tab;

	if ((work_tab = find_tabname (f_name)))
		_draw_heading (work_tab, r_flag);
}

/*
.function
	Function	:	redraw_page ()

	Description	:	Redraw current page for table.

	Parameters	:	f_name - Pointer to string containing table
					 name.
				r_flag - If true print current line in reverse
					 video.
.end
*/
void
redraw_page (char *f_name, int r_flag)
{
	TAB_PTR	work_tab;
	char	str [256];

	if (!(work_tab = find_tabname (f_name)))
		return ;

	_redraw_page (work_tab);
	tab_seek (work_tab,
		(long) TAB_PAGE [TAB_PGENO] + (TAB_LNENO * (TAB_FWIDTH + 1)), 0);
	TAB_LPAGENO = TAB_PGENO;

	sprintf (str, "%-*.*s", TAB_SWIDTH, TAB_SWIDTH, DET_LINE);
	rv_pr (str, TAB_COL + 1, SCR_ROW, r_flag);
}

static	void
_redraw_page (TAB_PTR work_tab)
{
	int	i;

	for (i= 0; i < TAB_PSIZE; i++)
		print_at (TAB_ROW + i + 3, TAB_COL + 1, "%-*.*s",
					TAB_SWIDTH, TAB_SWIDTH, TAB_DETAIL(i));

	/*-------------------------------
	| Clear the rest of the screen	|
	-------------------------------*/
	for (; i < TAB_DEPTH; i++)
		print_at (TAB_ROW + i + 3, TAB_COL + 1, "%-*.*s",
						TAB_SWIDTH, TAB_SWIDTH, " ");
}

/*
.function
	Function	:	redraw_line ()

	Description	:	Redraw current line for table.

	Parameters	:	f_name - Pointer to string containing table
					 name.
				r_flag - If true print current line in reverse
					 video.
.end
*/
void
redraw_line (char *f_name, int r_flag)
{
	TAB_PTR	work_tab;

	if ((work_tab = find_tabname (f_name)))
		_redraw_line (work_tab, r_flag);
}

/*
.loc_func
	Function	:	_redraw_line ()

	Description	:	Redraw current line for table.

	Parameters	:	work_tab - Pointer to TAB_STRUCT entry for
					   table.
				r_flag   - If true print current line in reverse
					   video.
.end
*/
static	void
_redraw_line (TAB_PTR work_tab, int r_flag)
{
	char	str [256];

	if (TAB_PGENO != TAB_LPAGENO)
	{
		TAB_PSIZE = _load_tabpage (work_tab, TAB_PGENO);
		tab_seek (work_tab, (long) TAB_PAGE [TAB_PGENO], 0);
		TAB_LPAGENO = TAB_PGENO;
	}
	sprintf (str, "%-*.*s", TAB_SWIDTH, TAB_SWIDTH, DET_LINE);
	rv_pr (str, TAB_COL + 1, SCR_ROW, r_flag);
}

/*
.function
	Function	:	redraw_keys ()

	Description	:	Redraw hot-keys for table.

	Parameters	:	f_name - Pointer to string containing table
					 name.
.end
*/
void
redraw_keys (char *f_name)
{
	TAB_PTR	work_tab;

	if ((work_tab = find_tabname (f_name)))
		_draw_keys (work_tab);
}

/*
.function
	Function	:	redraw_table ()

	Description	:	Redraw table.

	Parameters	:	f_name - Pointer to string containing table
					 name.
.end
*/
void
redraw_table (char *f_name)
{
	TAB_PTR	work_tab;

	if (!(work_tab = find_tabname (f_name)))
		return;
	
	_draw_table (work_tab);
	_redraw_page (work_tab);

	if (TAB_ACTIVE)
		_draw_keys (work_tab);
}

/*
.function
	Function	:	_draw_keys ()

	Description	:	Draw hot-keys for table.

	Parameters	:	work_tab - Pointer to TAB_STRUCT entry for
					   table.
.end
*/
static	void
_draw_keys (TAB_PTR work_tab)
{
	if (MAX_LINES <= TAB_DEPTH)
		set_keys (TAB_KEYS, "P", KEY_PASSIVE);
	else
		set_keys (TAB_KEYS, "P", KEY_ACTIVE);

	disp_hotkeys(TAB_ROW + TAB_DEPTH + 3, TAB_COL+1, TAB_SWIDTH, TAB_KEYS);
}

/*
.function
	Function	:	tab_display ()

	Description	:	Display Table Data.

	Notes		:	Tab_display prints the form and first page of
				an active table if the table contains data.
				Otherwise the form and a message are printed.
			
	Parameters	:	f_name    - Pointer to string containing table
					    name.
				disp_flag - If TRUE display form.

	Returns		:
.end
*/
int
tab_display (char *f_name, int disp_flag)
{
	TAB_PTR	work_tab;
	int		crsr_stat;

	if (!(work_tab = find_tabname (f_name)))
		file_err (-1, "tab_display", "FIND_TABNAME");

	TAB_PGENO = TAB_LNENO = 0;
	TAB_LPAGENO = -1;

	tab_seek(work_tab, 0L, 0);

	/*----------------------------------------------------------------
	| Only Heading Lines found return to program and reset temp_str. |
	----------------------------------------------------------------*/
	if (TAB_LINES == 1)
	{
		crsr_stat = crsr_toggle (FALSE);
		_draw_table(work_tab);
		_load_tabpage(work_tab, TAB_PGENO);
		temp_str [0] = (char) NULL;
		rv_pr(" No match found. ",
				TAB_COL + 1, TAB_ROW + 3 + (TAB_DEPTH / 2), 1);
		blank_at (23, 0, 14);
		fflush(stdout);
		sleep(2);
		crsr_toggle (crsr_stat);
		return(1);
	}

	if (disp_flag)
	{
		blank_at (23, 0, 14);
		_draw_table (work_tab);
	}
	TAB_PSIZE = _load_tabpage(work_tab, TAB_PGENO);

	return(0);
}

/*
.loc_func
	Function	:	_draw_heading ()

	Description	:	Draw table column headings.

	Parameters	:	work_tab - Pointer to TAB_STRUCT entry for
					   table.
.end
*/
static	void
_draw_heading (TAB_PTR work_tab, int r_flag)
{
	int	crsr_stat;

	crsr_stat = crsr_toggle (FALSE);
	rv_pr(HDR_LINE, TAB_COL + 1, TAB_ROW + 1, r_flag);
	crsr_toggle (crsr_stat);
}

/*
.loc_func
	Function	:	_draw_table ()

	Description	:	Draw table.

	Parameters	:	work_tab - Pointer to TAB_STRUCT entry for
					   table.
.end
*/
static	void
_draw_table(TAB_PTR work_tab)
{
	int	crsr_stat;

	crsr_stat = crsr_toggle (FALSE);
	box(TAB_COL, TAB_ROW, TAB_SWIDTH + 2, TAB_DEPTH + 2);

	/*-----------------------
	| Print Column Headings	|
	-----------------------*/
	_draw_heading (work_tab, TRUE);

	move(TAB_COL,TAB_ROW + 2);
	PGCHAR(10);
	move(TAB_COL + 1, TAB_ROW + 2);
	line(TAB_SWIDTH + 1);
	PGCHAR(11);

	fflush(stdout);
	crsr_toggle (crsr_stat);
}

/*
.function
	Function	:	tab_clear ()

	Description	:	Clear space on screen occupied by table.

	Parameters	:	f_name - Pointer to string containing table
					 name.
.end
*/

void
tab_clear (char *f_name)
{
	TAB_PTR	work_tab, find_tabname (char *f_name);

	if (!(work_tab = find_tabname (f_name)))
		file_err (-1, "tab_clear", "FIND_TABNAME");

	_clear_tab (work_tab);
}

/*
.loc_func
	Function	:	_clear_tab ()

	Description	:	Clear space on screen occupied by table.

	Parameters	:	work_tab - Pointer to TAB_STRUCT entry for
					   table.
.end
*/
static	void
_clear_tab (TAB_PTR work_tab)
{
	register	int	i;
	int	crsr_stat;

	crsr_stat = crsr_toggle (FALSE);
	for (i = 0;i < TAB_DEPTH + 4;i++)
		print_at (TAB_ROW + i, TAB_COL,"%-*.*s", TAB_SWIDTH + 2,
							 TAB_SWIDTH + 2, " ");

	crsr_toggle (crsr_stat);
}

/*
.loc_func
	Function	:	blank_table ()

	Description	:	Blank space inside table form.

	Parameters	:	f_name - Pointer to string containing table
					 name.
.end
*/
void
blank_table (char *f_name)
{
	TAB_PTR	work_tab;

	if (!(work_tab = find_tabname (f_name)))
		file_err (-1, "blank_table", "FIND_TABNAME");

	_blank_table (work_tab);
}

/*
.loc_func
	Function	:	_blank_table ()

	Description	:	Blank space inside table form.

	Parameters	:	work_tab - Pointer to TAB_STRUCT entry for
					   table.
.end
*/
static	void
_blank_table (TAB_PTR work_tab)
{
	register	int	i;
	int	crsr_stat;

	crsr_stat = crsr_toggle (FALSE);
	for (i = 1;i < TAB_DEPTH + 3;i++)
		print_at (TAB_ROW + i, TAB_COL + 1, "%-*.*s",
						TAB_SWIDTH, TAB_SWIDTH, " ");

	crsr_toggle (crsr_stat);
}

/*
.loc_func
	Function	:	load_page ()

	Description	:	Load current page from work file.

	Parameters	:	f_name - Pointer to string containing table
					 name.

				r_flag - Re-display current line in reverse
					 video.
.end
*/
void
load_page (char *f_name, int r_flag)
{
	TAB_PTR	work_tab;
	char	str [256];

	if (!(work_tab = find_tabname (f_name)))
		file_err (-1, "load_page", "FIND_TABNAME");

	TAB_PSIZE = _load_tabpage (work_tab, TAB_PGENO);
	sprintf (str, "%-*.*s", TAB_SWIDTH, TAB_SWIDTH, DET_LINE);
	rv_pr (str, TAB_COL + 1, SCR_ROW, r_flag);
}

/*
.loc_func
	Function	:	_load_tabpage ()

	Description	:	Load one page from work file.

	Parameters	:	work_tab - Pointer to TAB_STRUCT entry for
					   table.
				p_no     - Number of page to load.

	Returns		:	psize	 - Number of lines in page.
.end
*/
static	int
_load_tabpage(TAB_PTR work_tab, int p_no)
{
	register	int	i;
	int	crsr_stat, p_size;

	tab_seek(work_tab, (long) TAB_PAGE[p_no], 0);
	crsr_stat = crsr_toggle (FALSE);
	for (i = 0; i < TAB_DEPTH && _load_tabline(work_tab, i); i++)
		print_at (TAB_ROW + i + 3, TAB_COL + 1, "%-*.*s",
						TAB_SWIDTH, TAB_SWIDTH, TAB_DETAIL(i));
	p_size = i;

	/*-------------------------------
	| Clear the rest of the screen	|
	-------------------------------*/
	for (; i < TAB_DEPTH; i++)
		print_at(TAB_ROW + i+3, TAB_COL + 1, "%-*.*s", TAB_SWIDTH, TAB_SWIDTH, " ");

	crsr_toggle (crsr_stat);

	return(p_size < 0 ? 0 : p_size);
}

/*
.loc_func
	Function	:	_load_tabline ()

	Description	:	Load one line from work file.

	Parameters	:	work_tab - Pointer to TAB_STRUCT entry for
					   table.
				line_no  - Number of line to load.

	Returns		:	0	 - If read succeeds.
				1	 - If read fails.
.end
*/
static	int
_load_tabline(TAB_PTR work_tab, int line_no)
{
	char	*lptr;

	if (line_no >= MAX_LINES)
		return (FALSE);

	if (TAB_MEMORY)
		return (TRUE);

	if (!(lptr = fgets(TAB_DETAIL(line_no), TAB_FWIDTH + 2, TAB_ID)))
		return(FALSE);

	if ((lptr = strchr (TAB_DETAIL (line_no), '\n')))
		*lptr = (char) NULL;

	return(TRUE);
}

/*
.function
	Function	:	_tab_halt ()

	Description	:	Controled dropout from search routine.

	Notes		:	Routine to allow a user to abort lengthy table
				loads.
			
	Parameters	:	work_tab - Pointer to TAB_STRUCT entry for 
					   table.

	Returns		:	TRUE 	 - If FN16 is pressed.
				FALSE	 - If any key other than FN16 is
					   pressed.
.end
*/
static	int
_tab_halt(TAB_PTR work_tab)
{
	int	stop_key; 

	print_at(23, 0, "%RPress F16 to stop search, any other key to continue.\007");
	stop_key = getkey();
	blank_at(23, 0, 79);
	rv_pr(" Busy  ... ", 0, 23, TAB_LINES % 40 ? 0 : 1);
	return((stop_key == FN16));
}

static	TAB_STRUCT	tab_names [10];

/*
.function
	Function	:	find_tabname ()

	Description	:	Return pointer to TAB_STRUCT entry for table.

	Notes		:	Find_tabname is used to get the TAB_STRUCT entry
				for the specified table.
			
	Parameters	:	f_name         - Pointer to string containing
						 table name.

	Returns		:	work_tab       - Pointer to TAB_STRUCT entry if
					 	 found.
				(TAB_PTR) NULL - NULL pointer if table is not
						 found.
.end
*/
TAB_PTR	find_tabname (char *f_name)
{
	int	cnt;
	TAB_PTR	work_tab;

	for (cnt = 0, work_tab = tab_names; cnt < 10; cnt++, work_tab++)
		if (TAB_NAME && !strcmp (f_name, TAB_NAME))
			return (work_tab);

	return ((TAB_PTR) NULL);
}

/*
.function
	Function	:	add_tabname ()

	Description	:	Return pointer to empty TAB_STRUCT entry.

	Notes		:	Add_tabname is used to get an empty TAB_STRUCT
				entry to add a new table.
			
	Parameters	:	f_name         - Pointer to string containing
						 table name.

	Returns		:	work_tab       - Pointer to empty TAB_STRUCT
						 entry if found.
				(TAB_PTR) NULL - NULL pointer if empty slot is
						 not found.
.end
*/
TAB_PTR	add_tabname (char *f_name)
{
	int	cnt;
	TAB_PTR	work_tab;

	if ((work_tab = find_tabname (f_name)))
		return ((TAB_PTR) NULL);

	for (cnt = 0, work_tab = tab_names; cnt < 10; cnt++, work_tab++)
		if (!TAB_NAME)
		{
			pin_bfill ((char *) work_tab, '\0', sizeof (TAB_STRUCT));
			if (!(TAB_NAME = strdup (f_name)))
				return ((TAB_PTR) NULL);
			else
				return (work_tab);
		}

	return ((TAB_PTR) NULL);
}

/*
	All routines from here down use the one copy of work_tab.
	This work_tab is set by scan_table.
*/

static	TAB_PTR	work_tab = (TAB_PTR) NULL;

/*
.function
	Function	:	tab_scan ()

	Description	:	Interactively scan table.

	Notes		:	Tab_scan allows a user to interactively move
				up and down a table and execute various hot-key
				options set by the programmer.
			
	Parameters	:	f_name - Pointer to string containing name of
					 table to scan.

	Returns		:	c      - Result of call to run_hotkeys.
.end
*/
int
tab_scan (char *f_name)
{
	TAB_PTR	temp_tab;
	int	c, crsr_stat;

	temp_tab = work_tab;

	if (!(work_tab = find_tabname (f_name)))
		file_err (-1, "tab_scan", "FIND_TABNAME");

	TAB_PGENO = TAB_LNENO = 0;
	TAB_LPAGENO = -1;
	tab_seek(work_tab, 0L, 0);

	crsr_stat = crsr_toggle (FALSE);
	_draw_keys (work_tab);

	_redraw_line (work_tab, TRUE);
	ptab_scan ();
	TAB_ACTIVE = TRUE;
	c = run_hotkeys (TAB_KEYS, pre_func, post_func);

	TAB_ACTIVE = FALSE;
	work_tab = temp_tab;
	crsr_toggle (crsr_stat);

	return(c);
}

/*
.loc_func
	Function	:	pre_func ()

	Description	:	Called before any table cursor movement.

	Notes		:	Pre_func is used to initialise temp_str before
				any hot-key function is used.
			
	Parameters	:	c - Key pressed.
*/
static	int
pre_func (int c, KEY_PTR unused)
{
	if (c != FN1)
		sprintf(temp_str,"%-*.*s", TAB_SWIDTH, TAB_SWIDTH, DET_LINE);
	else
		sprintf(temp_str,"%-*.*s", TAB_SWIDTH, TAB_SWIDTH, " ");

	print_at (SCR_ROW, TAB_COL + 1, "%-*.*s", TAB_SWIDTH, TAB_SWIDTH, DET_LINE);
	TAB_LPAGENO = TAB_PGENO;
	return (EXIT_SUCCESS);
}

/*
.loc_func
	Function	:	post_func ()

	Description	:	Called after any table cursor movement.

	Notes		:	Post_func is used to load a new page if needed
					after a hot-key function is used and to 
					high-light the new line in table.
*/
static	int
post_func (int iUnused, KEY_PTR unused)
{
	char	str [256];

	if (TAB_PGENO != TAB_LPAGENO)
		TAB_PSIZE = _load_tabpage (work_tab, TAB_PGENO);
	sprintf (str, "%-*.*s", TAB_SWIDTH, TAB_SWIDTH, DET_LINE);
	rv_pr (str, TAB_COL + 1, SCR_ROW, TRUE);
	tag_other ();
	return (EXIT_SUCCESS);
}

/*
.loc_func
	Function	:	up_func ()

	Description	:	Called if upward movement hot-key is pressed.

	Notes		:	Up_func moves up the table either a page or
				a line depending on where the high-light bar is
				on the current page.

	Parameters	:	c - Integer holding ascii value of key pressed.
	
	Returns		:	c - Integer holding ascii value of key pressed.
*/
static	int
up_func (int c, KEY_PTR unused)
{
	if (TAB_LNENO) 
		TAB_LNENO--;
	else
	{
		TAB_PGENO = (TAB_PGENO) ? TAB_PGENO - 1 : (MAX_LINES - 1) / TAB_DEPTH;
		TAB_LNENO = (TAB_PGENO == TAB_LPAGENO) ? TAB_PSIZE - 1 : 0;
	}

	return (c);
}

/*
.loc_func
	Function	:	down_func ()

	Description	:	Called if downward movement hot-key is pressed.

	Notes		:	Down_func moves down the table either a page or
				a line depending on where the high-light bar is
				on the current page.

	Parameters	:	c - Integer holding ascii value of key pressed.
	
	Returns		:	c - Integer holding ascii value of key pressed.
*/
static	int
down_func (int c, KEY_PTR unused)
{
	if (TAB_LNENO == TAB_PSIZE - 1) 
	{
		TAB_PGENO = ((TAB_PGENO + 1) * TAB_DEPTH > (MAX_LINES - 1)) ? 0 : TAB_PGENO + 1;
		TAB_LNENO = 0;
	}
	else
		TAB_LNENO++;

	return (c);
}

/*
.loc_func
	Function	:	f3_func ()

	Description	:	Called if FN3 hot-key is pressed.

	Notes		:	F3_func redraws the currently active table.

	Parameters	:	c - Integer holding ascii value of key pressed.
	
	Returns		:	c - Integer holding ascii value of key pressed.
.end
*/
static	int
f3_func (int c, KEY_PTR unused)
{
	redraw_table (TAB_NAME);

	return (c);
}

/*
.loc_func
	Function	:	f14_func ()

	Description	:	Called if FN14 hot-key is pressed.

	Notes		:	F14_func displays the next page in the current
				table or the first page if the current page is
				the last one in the table.

	Parameters	:	c - Integer holding ascii value of key pressed.
	
	Returns		:	c - Integer holding ascii value of key pressed.
.end
*/
static	int
f14_func (int c, KEY_PTR unused)
{
	TAB_PGENO = ((TAB_PGENO + 1) * TAB_DEPTH > (MAX_LINES - 1)) ? 0 : TAB_PGENO + 1;
	TAB_LNENO = 0;

	return (c);
}

/*
.loc_func
	Function	:	f15_func ()

	Description	:	Called if FN15 hot-key is pressed.

	Notes		:	F14_func displays the previous page in the
				current table or the last page if the current
				page is the first one in the table.

	Parameters	:	c - Integer holding ascii value of key pressed.
	
	Returns		:	c - Integer holding ascii value of key pressed.
.end
*/
static	int
f15_func (int c, KEY_PTR unused)
{
	TAB_PGENO = (TAB_PGENO) ? TAB_PGENO - 1 : (MAX_LINES - 1) / TAB_DEPTH;
	TAB_LNENO = 0;

	return (c);
}

/*
.loc_func
	Function	:	f16_func ()

	Description	:	Called if FN16 hot-key is pressed.

	Notes		:	F16_func terminates interactive scanning of the 
				current table.

	Parameters	:	c - Integer holding ascii value of key pressed.
	
	Returns		:	c - Integer holding ascii value of key pressed.
.end
*/
static	int
f16_func (int c, KEY_PTR unused)
{
	crsr_toggle (TRUE);
	return(c == '\r' ? FN16 : c);
}

/*
.loc_func
	Function	:	tab_seek ()

	Description	:	Positions file pointer for disk resident table.

	Parameters	:	work_tab - Pointer to TAB_STRUCT entry for
					   table.
				offset	 - Offset in file to re-position
					   pointer to.
				type	 - Point from which to re-position.
						0 - Start of file.
						1 - Current position in file.
						2 - End of file.
.end
*/
static	void
tab_seek (TAB_PTR work_tab, long int offset, int type)
{
	if (!TAB_MEMORY)
		fseek (TAB_ID, offset, type);
}
static	void
tab_end (TAB_PTR work_tab)
{
	tab_seek (work_tab, (long) (TAB_LINES - 1) * (TAB_FWIDTH + 1), 0);
}

int
clear_file(TAB_PTR work_tab, long int size)
{
	long	fsize = 0L;
	FILE	*wk_file;

	if ((wk_file = fopen(tab_name(TAB_NAME),"w")) == NULL)
		return(-1);

	/*---------------------------------------
	| output from original file		|
	---------------------------------------*/
	while ( fsize < size )
	{
		putc('\0',wk_file);
		fsize++;
	}
	fclose(wk_file);

	fseek(TAB_ID,0L,0);
	return(0);
}
