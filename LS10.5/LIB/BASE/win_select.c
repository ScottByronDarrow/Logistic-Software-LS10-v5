#include	<std_decs.h>
#include	<win_select.h>

#define	BS	'\b'

static	struct	SEL_STR	*sel_head = WIN_SEL_NULL;

static	int	sel_col_wid,
		sel_no_cols,
		sel_no_rows,
		sel_x_wid,
		sel_y_wid,
		sel_x_pos,
		sel_y_pos;

static	struct	SEL_STR	*sel_win(struct SEL_STR *scn_head);
static	void	dsp_win(struct SEL_STR *scn_head);
static	struct	SEL_STR	*win_search(struct SEL_STR *curr, int *row, int *col, int chr);

extern	int	last_char;
extern	char	err_str[];

void
win_display (char *title, int width, struct SEL_STR *head, int x, int y, int dx, int dy)
{
	int	i;

	sel_head = head;
	sel_x_pos = x + 2;
	sel_y_pos = y + 1;
	sel_x_wid = dx;
	sel_y_wid = dy;
	sel_no_rows = dy;
	sel_no_cols = (dx - 3) / (width + 2);
	if (sel_no_cols < 1)
		return;
	sel_col_wid = width + 2;
	crsr_off ();
	box (x, y, dx, dy);
	i = 0;
	if (title != (char *) NULL)
		i = strlen (title);
	if (i > 0)
	{
		i += 4;
		i = dx - i;
		i /= 2;
		move (x + i, y);
		printf (" ");
		print_at (y, x + i + 1, "%R %s ", title);
		printf (" ");
	}

	dsp_win (sel_head);

	return;
}

struct	SEL_STR
*win_select (char *title, int width, struct SEL_STR *head, int x, int y, int dx, int dy)
{
	int	i;

	sel_head = head;
	sel_x_pos = x + 2;
	sel_y_pos = y + 1;
	sel_x_wid = dx;
	sel_y_wid = dy;
	sel_no_rows = dy;
	sel_no_cols = (dx - 3) / (width + 2);
	if (sel_no_cols < 1)
		return ((struct SEL_STR *) NULL);
	sel_col_wid = width + 2;
	crsr_off ();
	box (x, y, dx, dy);
	i = 0;
	if (title != (char *) NULL)
		i = strlen (title);
	if (i > 0)
	{
		i += 4;
		i = dx - i;
		i /= 2;
		move (x + i, y);
		printf (" ");
		print_at (y, x + i + 1, "%R %s ", title);
		printf (" ");
	}

	return (sel_win (sel_head));
}

static	struct	SEL_STR	*sel_win(struct SEL_STR *scn_head)
{
	int	row = 0,
		col = 0,
		ok = TRUE,
		chr;
	struct	SEL_STR
		*tmp_head;

	dsp_win (sel_head);
	if (sel_head == (WIN_SEL_NULL))
		return ((struct SEL_STR *) NULL);
	while (ok)
	{
	    print_at
	    (
		sel_y_pos + row,
		sel_x_pos + (col * sel_col_wid),
		"%R %-*.*s ",
		sel_col_wid - 2,
		sel_col_wid - 2,
		scn_head->sel_name
	    );
	    chr = getkey ();
	    last_char = chr;
	    switch (chr)
	    {
	    case	27:
	    case	FN1:
	    case	FN2:
	    case	FN3:
	    case	FN4:
	    case	FN5:
	    case	FN6:
	    case	FN7:
	    case	FN8:
	    case	FN9:
	    case	FN10:
	    case	FN11:
	    case	FN12:
	    case	FN13:
	    case	FN14:
	    case	FN15:
	    case	FN16:
		return ((struct SEL_STR *) NULL);
		break;

	    case	'\r':
		return (scn_head);
		break;

	    case	LEFT_KEY:
	    case	BS:
		if (scn_head->prev == WIN_SEL_NULL)
			putchar (BELL);
		else
		{
		    print_at
		    (
			sel_y_pos + row,
			sel_x_pos + (col * sel_col_wid),
			" %-*.*s ",
			sel_col_wid - 2,
			sel_col_wid - 2,
			scn_head->sel_name
		    );
		    for (chr = 0; chr < sel_no_rows; chr++)
		    {
			if (scn_head->prev == WIN_SEL_NULL)
			{
			    putchar (BELL);
			    break;
			}
			scn_head = scn_head->prev;
			row--;
			if (row < 0)
			{
			    row = sel_no_rows - 1;
			    col--;
			    if (col < 0)
			    {
				tmp_head = scn_head;
				for (col = sel_no_cols * sel_no_rows; col > 1; col--)
				    tmp_head = tmp_head->prev;
				col = sel_no_cols - 1;
				dsp_win (tmp_head);
			    }
			}
		    }
		    print_at
		    (
			sel_y_pos + row,
			sel_x_pos + (col * sel_col_wid),
			"%R %-*.*s ",
			sel_col_wid - 2,
			sel_col_wid - 2,
			scn_head->sel_name
		    );
		}
		break;

	    case	RIGHT_KEY:
		if (scn_head->next == WIN_SEL_NULL)
			putchar (BELL);
		else
		{
		    print_at
		    (
			sel_y_pos + row,
			sel_x_pos + (col * sel_col_wid),
			" %-*.*s ",
			sel_col_wid - 2,
			sel_col_wid - 2,
			scn_head->sel_name
		    );
		    for (chr = 0; chr < sel_no_rows; chr++)
		    {
			if (scn_head->next == WIN_SEL_NULL)
			{
			    putchar (BELL);
			    break;
			}
			scn_head = scn_head->next;
			row++;
			if (row >= sel_no_rows)
			{
			    row = 0;
			    col++;
			    if (col >= sel_no_cols)
			    {
				col = 0;
				dsp_win (scn_head);
			    }
			}
		    }
		    print_at
		    (
			sel_y_pos + row,
			sel_x_pos + (col * sel_col_wid),
			"%R %-*.*s ",
			sel_col_wid - 2,
			sel_col_wid - 2,
			scn_head->sel_name
		    );
		}
		break;

	    case	UP_KEY:
		if (scn_head->prev == WIN_SEL_NULL)
			putchar (BELL);
		else
		{
		    print_at
		    (
			sel_y_pos + row,
			sel_x_pos + (col * sel_col_wid),
			" %-*.*s ",
			sel_col_wid - 2,
			sel_col_wid - 2,
			scn_head->sel_name
		    );
		    scn_head = scn_head->prev;
		    row--;
		    if (row < 0)
		    {
			row = sel_no_rows - 1;
			col--;
			if (col < 0)
			{
			    tmp_head = scn_head;
			    for (col = sel_no_cols * sel_no_rows; col > 1; col--)
				tmp_head = tmp_head->prev;
			    col = sel_no_cols - 1;
			    dsp_win (tmp_head);
			}
		    }
		    print_at
		    (
			sel_y_pos + row,
			sel_x_pos + (col * sel_col_wid),
			"%R %-*.*s ",
			sel_col_wid - 2,
			sel_col_wid - 2,
			scn_head->sel_name
		    );
		}
		break;

	    case	DOWN_KEY:
	    case	' ':
		if (scn_head->next == WIN_SEL_NULL)
			putchar (BELL);
		else
		{
		    print_at
		    (
			sel_y_pos + row,
			sel_x_pos + (col * sel_col_wid),
			" %-*.*s ",
			sel_col_wid - 2,
			sel_col_wid - 2,
			scn_head->sel_name
		    );
		    scn_head = scn_head->next;
		    row++;
		    if (row >= sel_no_rows)
		    {
			row = 0;
			col++;
			if (col >= sel_no_cols)
			{
			    col = 0;
			    dsp_win (scn_head);
			}
		    }
		    print_at
		    (
			sel_y_pos + row,
			sel_x_pos + (col * sel_col_wid),
			"%R %-*.*s ",
			sel_col_wid - 2,
			sel_col_wid - 2,
			scn_head->sel_name
		    );
		}
		break;

	    default:
		print_at
		(
		    sel_y_pos + row,
		    sel_x_pos + (col * sel_col_wid),
		    " %-*.*s ",
		    sel_col_wid - 2,
		    sel_col_wid - 2,
		    scn_head->sel_name
		);
		tmp_head = win_search (scn_head, &row, &col, chr);
		if (tmp_head == WIN_SEL_NULL)
		    putchar (BELL);
		else
		    scn_head = tmp_head;
		print_at
		(
		    sel_y_pos + row,
		    sel_x_pos + (col * sel_col_wid),
		    "%R %-*.*s ",
		    sel_col_wid - 2,
		    sel_col_wid - 2,
		    scn_head->sel_name
		);
	    }
	}
	/* NOTREACHED */
	return ((struct SEL_STR *) NULL);
}

static	void
dsp_win(struct SEL_STR *scn_head)
{
	int	i,
		row,
		col;
	struct	SEL_STR
		*col_head[40],
		*tmp_head = scn_head;

	i = 0;
	for (col = 0; col < sel_no_cols; col++)
	{
	    col_head[i] = scn_head;
	    i++;
	    for (row = 0; row < sel_no_rows; row++)
	    {
		if (scn_head != WIN_SEL_NULL)
		    scn_head = scn_head->next;
	    }
	}

	for (row = 0; row < sel_no_rows; row++)
	{
	    move (sel_x_pos, sel_y_pos + row);
	    for (col = 0; col < sel_no_cols; col++)
	    {
		if (col_head[col] != WIN_SEL_NULL)
		{
		    printf
		    (
			" %-*.*s ",
			sel_col_wid - 2,
			sel_col_wid - 2,
			col_head[col]->sel_name
		    );
		    col_head[col] = col_head[col]->next;
		}
		else
		{
		    printf
		    (
			" %-*.*s ",
			sel_col_wid - 2,
			sel_col_wid - 2,
			" "
		    );
		}
	    }
	}
	col = sel_x_wid / 2;
	if (tmp_head != WIN_SEL_NULL && tmp_head->prev != WIN_SEL_NULL)
	{
	    if (scn_head != WIN_SEL_NULL && scn_head->next != WIN_SEL_NULL)
			strcpy (err_str, "%R << MORE >> ");
	    else
			strcpy (err_str, "%R << MORE ");
	}
	else
	{
	    if (scn_head != WIN_SEL_NULL && scn_head->next != WIN_SEL_NULL)
			strcpy (err_str, "%R MORE >> ");
	    else
			strcpy (err_str, "");
	}
	col = ((sel_x_wid - 14) / 2) + sel_x_pos;
	move (col, sel_y_pos + sel_no_rows);
	line (15);
	row = strlen (err_str);
	if (row > 0)
	{
	    col = (sel_x_wid - (row + 2)) / 2;
	    move (sel_x_pos + col, sel_y_pos + sel_no_rows);
	    printf (" ");
	    print_at (sel_y_pos + sel_no_rows, sel_x_pos + col + 1, err_str);
	    printf (" ");
	}
}

static	struct	SEL_STR	*win_search(struct SEL_STR *curr, int *row, int *col, int chr)
{
	struct	SEL_STR
		*tmp_head = WIN_SEL_NULL,
		*tmp_curr;
	int	trow,
		tcol;

	if (chr < ' ' || chr > '~')
	    return (WIN_SEL_NULL);

	tmp_curr = curr;
	trow = *row;
	tcol = *col;

	while (TRUE)
	{
	    if (tmp_curr->next == WIN_SEL_NULL)
	    {
		trow = 0;
		tcol = 0;
		tmp_curr = sel_head;
		tmp_head = sel_head;
	    }
	    else
	    {
		tmp_curr = tmp_curr->next;
		trow++;
		if (trow >= sel_no_rows)
		{
		    trow = 0;
		    tcol++;
		    if (tcol >= sel_no_cols)
		    {
			tcol = 0;
			tmp_head = tmp_curr;
		    }
		}
	    }
	    if (tmp_curr == curr)
		return (WIN_SEL_NULL);
	    if (tmp_curr->sel_name[0] == chr)
	    {
		if (tmp_head != WIN_SEL_NULL)
		    dsp_win (tmp_head);
		*row = trow;
		*col = tcol;
		return (tmp_curr);
	    }
	}
}
