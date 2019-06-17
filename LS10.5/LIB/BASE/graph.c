/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( graph.c        )                                 |
|  Program Desc  : ( General purpose graphing function.           )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written  : 29/06/90      Author      : Trevor van Bremen      |
|---------------------------------------------------------------------|
|  Date Modified : (20/02/91)  | Modified by : Trevor van Bremen      |
|  Date Modified : (09/04/91)  | Modified by : Trevor van Bremen      |
|  Date Modified : (09/04/91)  | Modified by : Jonathan Chen          |
|                                                                     |
|  Comments      : 20/02/91 - Created 2 new methods...                |
|                :            GR_TYPE_3BAR and GR_TYPE_4BAR           |
|                : 09/04/91 - Created 1 new methods...                |
|                :            GR_TYPE_5BAR (Also, improved graph.h)   |
|     (18.01.94) : PSL 10305. MAXFLOAT resides in <values.h> in most  |
|                : X_OPEN compliant systems.                          |

	$Log: graph.c,v $
	Revision 5.1  2001/08/06 22:40:54  scott
	RELEASE 5.0
	
	Revision 5.0  2001/06/19 06:59:34  cha
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.0  2001/03/09 00:52:37  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 3.0  2000/10/12 13:34:21  gerry
	Revision No. 3 Start
	<after Rel-10102000>
	
	Revision 2.0  2000/07/15 07:17:14  gerry
	Forced revision no. to 2.0 - Rel-15072000
	
	Revision 1.5  1999/09/23 22:46:40  jonc
	Replaced the use of deprecated <values.h> with <float.h>
	
=====================================================================*/
#include	<std_decs.h>
#include	<float.h>

static	int	itm_ix1 = 1,
			itm_ix2 = 1,
			itm_ix3 = 1,
			itm_ix4 = 1,
			itm_ix5 = 1;

int		GraphFooter	=	1;

static	char	item_str[4][42] =
{
	"PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP",
	"MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM",
	"NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN",
	"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"
};

static	int graph_1bar (struct GR_WINDOW *window, int gr_item_cnt, struct GR_NAMES *names, double *values, struct GR_LIMITS *limits);
static	int graph_2bar (struct GR_WINDOW *window, int gr_item_cnt, struct GR_NAMES *names, double *values, struct GR_LIMITS *limits);
static	int graph_3bar (struct GR_WINDOW *window, int gr_item_cnt, struct GR_NAMES *names, double *values, struct GR_LIMITS *limits);
static	int graph_4bar (struct GR_WINDOW *window, int gr_item_cnt, struct GR_NAMES *names, double *values, struct GR_LIMITS *limits);
static	int graph_5bar (struct GR_WINDOW *window, int gr_item_cnt, struct GR_NAMES *names, double *values, struct GR_LIMITS *limits);
static	struct GR_LIMITS *GR_set_limits(struct GR_LIMITS *limits, struct GR_LIMITS *lcl_limits, int gr_item_cnt, int gr_ary_cnt, double *values);

int
GR_graph (struct GR_WINDOW *window, int gr_type, int gr_item_cnt, struct GR_NAMES *names, double *values, struct GR_LIMITS *limits)
{
int	rv = 0;

	switch (gr_type)
	{
	case GR_TYPE_SBAR:		/* Stacked bar			*/
		rv = EGR_NOT_IMP;
		break;

	case GR_TYPE_1BAR:		/* Standard single bar		*/
		rv = graph_1bar (window, gr_item_cnt, names, values, limits);
		break;

	case GR_TYPE_2BAR:		/* Standard double bar		*/
		rv = graph_2bar (window, gr_item_cnt, names, values, limits);
		break;

	case GR_TYPE_3BAR:		/* Standard triple bar		*/
		rv = graph_3bar (window, gr_item_cnt, names, values, limits);
		break;

	case GR_TYPE_4BAR:		/* Standard quadruple bar	*/
		rv = graph_4bar (window, gr_item_cnt, names, values, limits);
		break;

	case GR_TYPE_5BAR:		/* Standard pentuple bar	*/
		rv = graph_5bar (window, gr_item_cnt, names, values, limits);
		break;

	case GR_TYPE_SROW:		/* Stacked row			*/
		rv = EGR_NOT_IMP;
		break;

	case GR_TYPE_1ROW:		/* Standard single row		*/
		rv = EGR_NOT_IMP;
		break;

	case GR_TYPE_2ROW:		/* Standard double row		*/
		rv = EGR_NOT_IMP;
		break;

	case GR_TYPE_3ROW:		/* Standard triple row		*/
		rv = EGR_NOT_IMP;
		break;

	case GR_TYPE_4ROW:		/* Standard quadruple row	*/
		rv = EGR_NOT_IMP;
		break;

	case GR_TYPE_5ROW:		/* Standard pentuple row	*/
		rv = EGR_NOT_IMP;
		break;

	case GR_TYPE_LINE:		/* Standard line		*/
		rv = EGR_NOT_IMP;
		break;

	case GR_TYPE_PIE:		/* Standard pie chart		*/
		rv = EGR_NOT_IMP;
		break;

	default:
		rv = EGR_NOT_IMP;
		break;
	}
	return (rv);
}

static	struct GR_LIMITS *GR_set_limits(struct GR_LIMITS *limits, struct GR_LIMITS *lcl_limits, int gr_item_cnt, int gr_ary_cnt, double *values)
{
	struct	GR_LIMITS *lim_ptr;
	int	count;

	if (limits != NULL)
		lim_ptr = limits;
	else
	{
		lcl_limits->min_y = FLT_MAX;
		lcl_limits->max_y = -FLT_MAX;
		lim_ptr = lcl_limits;
		for (count = 0; count < gr_item_cnt * gr_ary_cnt; count++)
		{
			if (values[count] < lcl_limits->min_y)
				lcl_limits->min_y = values[count];
			if (values[count] > lcl_limits->max_y)
				lcl_limits->max_y = values[count];
		}
	}

	return (lim_ptr);
}

static	void
GR_setup_hdg(struct GR_WINDOW *window, struct GR_NAMES *names, int cnt)
{
	int	left_ofs;
	int	right_ofs;
	int	head_width;
	char	hdg_string[133];

	Dsp_prn_open (window->x_posn, window->y_posn, window->y_size, names->pr_head, (char *) 0, (char *) 0, (char *) 0, (char *) 0, (char *) 0, (char *) 0);
	head_width = (names->heading == NULL) ? 0 : strlen (names->heading);
	head_width = (head_width > window->x_size) ? window->x_size : head_width;
	right_ofs = (window->x_size - head_width) / 2;
	left_ofs = window->x_size - (right_ofs + head_width);
	sprintf (hdg_string, "%-*.*s%-*.*s%-*.*s",
		left_ofs,
		left_ofs,
		" ",
		head_width,
		head_width,
		(names->heading == NULL) ? "" : names->heading,
		right_ofs,
		right_ofs,
		" ");
	Dsp_saverec (hdg_string);
	Dsp_saverec ("");

	left_ofs = (window->x_size - 16) / 2;
	right_ofs = window->x_size - (left_ofs + 16);
	sprintf (hdg_string, "%-*.*s%s%-*.*s",
		left_ofs - 2,
		left_ofs - 2,
		" ",
		" [ INPUT/END ]  ",
		right_ofs - 2,
		right_ofs - 2,
		" ");
	if (GraphFooter)
		Dsp_saverec (hdg_string);
	else
		Dsp_saverec ("");

	if (names->gpx_ch_indx == (char *) 0 ||
	    strlen (names->gpx_ch_indx) < cnt)
	{
		itm_ix1 = itm_ix2 = itm_ix3 = itm_ix4 = itm_ix5 = 1;
		if (cnt == 2)
			itm_ix2 = 2;
		return;
	}

	switch (cnt)
	{
	case	5:
		if (names->gpx_ch_indx[4] > '3' || names->gpx_ch_indx[4] < '0')
			itm_ix5 = 1;
		else
			itm_ix5 = names->gpx_ch_indx[4] - '0';

	case	4:
		if (names->gpx_ch_indx[3] > '3' || names->gpx_ch_indx[3] < '0')
			itm_ix4 = 1;
		else
			itm_ix4 = names->gpx_ch_indx[3] - '0';

	case	3:
		if (names->gpx_ch_indx[2] > '3' || names->gpx_ch_indx[2] < '0')
			itm_ix3 = 1;
		else
			itm_ix3 = names->gpx_ch_indx[2] - '0';

	case	2:
		if (names->gpx_ch_indx[1] > '3' || names->gpx_ch_indx[1] < '0')
			itm_ix2 = 1;
		else
			itm_ix2 = names->gpx_ch_indx[1] - '0';

	case	1:
		if (names->gpx_ch_indx[0] > '3' || names->gpx_ch_indx[0] < '0')
			itm_ix1 = 1;
		else
			itm_ix1 = names->gpx_ch_indx[0] - '0';
		break;
	}
}

static	int graph_1bar (struct GR_WINDOW *window, int gr_item_cnt, struct GR_NAMES *names, double *values, struct GR_LIMITS *limits)
{
	struct	GR_LIMITS wrk_limits;
	struct	GR_LIMITS lcl_limits;
	struct	GR_LIMITS *lim_ptr;
	int	gph_rc;
	int	lim_min_lin;
	int	lim_max_lin;
	int	count;
	int	itemno;
	int	item_width;
	int	side_gap;
	double	step_value;
	double	tmp_value;
	char	fmt_str[256];
	char	tmp_str[256];
	char	lcl_string[256];

	if (window->x_posn + window->x_size > ((_wide) ? 130 : 78))
	    return (EGR_WIDTH);
	if (window->x_posn < 0 || window->x_size < (gr_item_cnt + 8))
	    return (EGR_WIDTH);
	if (window->y_posn + window->y_size > 23 || window->y_posn < 0 || window->y_size < 5)
	    return (EGR_HEIGHT);

	lim_ptr = GR_set_limits (limits, &lcl_limits, gr_item_cnt, 1, values);

	step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
	item_width = (window->x_size - 8) / gr_item_cnt;
	if (item_width == 1)
	    sprintf (fmt_str, "%%-1.1s");
	else
	{
	    side_gap = (item_width - 1) / 3;
	    count = item_width - ((side_gap * 2) + 1);
	    sprintf (fmt_str, "E%-*.*s%%-%d.%ds%-*.*s",
		side_gap, side_gap, item_str[0],
		count, count,
		side_gap, side_gap, item_str[0]);
	}

	while (1)
	{
	    GR_setup_hdg (window, names, 1);

	    for (count = 0; count < window->y_size - 2; count++)
	    {
		tmp_value = lim_ptr->max_y - (count * step_value);
		if (lim_ptr->max_y > (double) 9999999 || lim_ptr->min_y < (double) -999999)
		    sprintf (lcl_string, "%8.0f", tmp_value);
		else
		    if (lim_ptr->max_y > (double) 99 || lim_ptr->min_y < (double) -9)
			sprintf (lcl_string, "%7.0f ", tmp_value);
		    else
			sprintf (lcl_string, "%7.2f ", tmp_value);
		strcat (lcl_string, "^^");
		for (itemno = 0; itemno < gr_item_cnt; itemno++)
		{
		    if (values[itemno] >= tmp_value)
			sprintf (tmp_str, fmt_str, item_str[itm_ix1]);
		    else
			sprintf (tmp_str, fmt_str, item_str[0]);
		    strcat (lcl_string, tmp_str);
		}
		if (item_width > 1)
		    strcat (lcl_string, "E");
		strcat (lcl_string, "^^");
		Dsp_save_fn (lcl_string, " ");
	    }

	    strcpy (lcl_string, "        ^^");
	    if (item_width == 1)
	    {
		for (count = 0; count < gr_item_cnt; count++)
		    strcat (lcl_string, "G");
	    }
	    else
	    {
		strcat (lcl_string, "K");
		for (itemno = 1; itemno <= gr_item_cnt; itemno++)
		{
		    for (count = 1; count < item_width; count++)
			strcat (lcl_string, "G");
		    if (itemno != gr_item_cnt)
			strcat (lcl_string, "H");
		    else
			strcat (lcl_string, "L");
		}
	    }
	    strcat (lcl_string, "^^");
	    Dsp_saverec (lcl_string);

	    strcpy (lcl_string, "        ");
	    for (count = 0; count < gr_item_cnt; count++)
	    {
		if (item_width > 1)
		    sprintf (tmp_str, "^E%-*.*s",
			item_width - 1,
			item_width - 1,
			*(names->legends + count));
		else
		    sprintf (tmp_str, "%-*.*s",
			item_width,
			item_width,
			*(names->legends + count));
		strcat (lcl_string, tmp_str);
	    }
	    if (item_width > 1)
		strcat (lcl_string, "^E");
	    Dsp_saverec (lcl_string);

	    gph_rc = Dsp_srch_grph (&lim_min_lin, &lim_max_lin);
	    Dsp_close ();

	    if (gph_rc == -1)
	    {
		lim_ptr = GR_set_limits (limits, &lcl_limits, gr_item_cnt, 1, values);
		step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
		continue;
	    }

	    if (gph_rc == 1)
	    {
		step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
		wrk_limits.min_y = lim_ptr->max_y - (step_value * lim_min_lin);
		wrk_limits.max_y = lim_ptr->max_y - (step_value * lim_max_lin);
		lim_ptr = GR_set_limits (&wrk_limits, &lcl_limits, gr_item_cnt, 1, values);
		step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
		continue;
	    }

	    break;
	}
	return (EXIT_SUCCESS);
}

static	int graph_2bar (struct GR_WINDOW *window, int gr_item_cnt, struct GR_NAMES *names, double *values, struct GR_LIMITS *limits)
{
	struct	GR_LIMITS lcl_limits;
	struct	GR_LIMITS *lim_ptr;
	struct	GR_LIMITS wrk_limits;
	int	gph_rc;
	int	lim_min_lin;
	int	lim_max_lin;
	int	count;
	int	itemno;
	int	item_width;
	int	bar_gap;
	int	side_gap;
	int	cntr_gap;
	double	step_value;
	double	tmp_value;
	char	fmt_str[256];
	char	tmp_str[256];
	char	lcl_string[256];

	if (window->x_posn + window->x_size > ((_wide) ? 130 : 78))
	    return (EGR_WIDTH);
	if (window->x_posn < 0 || window->x_size < ((gr_item_cnt * 2) + 8))
	    return (EGR_WIDTH);
	if (window->y_posn + window->y_size > 23 || window->y_posn < 0 || window->y_size < 5)
	    return (EGR_HEIGHT);

	lim_ptr = GR_set_limits (limits, &lcl_limits, gr_item_cnt, 2, values);

	step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
	item_width = (window->x_size - 8) / gr_item_cnt;
	if (item_width == 2)
	    sprintf (fmt_str, "%%-1.1s%%-1.1s");
	else
	{
	    bar_gap = (item_width + 2) / 4;
	    side_gap = ((item_width - 1) - (bar_gap * 2)) / 3;
	    cntr_gap = (item_width - 1) - (2 * (bar_gap + side_gap));
	    sprintf (fmt_str, "E%-*.*s%%-%d.%ds%-*.*s%%-%d.%ds%-*.*s",
		side_gap, side_gap, item_str[0],
		bar_gap, bar_gap,
		cntr_gap, cntr_gap, item_str[0],
		bar_gap, bar_gap,
		side_gap, side_gap, item_str[0]);
	}

	while (1)
	{
	    GR_setup_hdg (window, names, 2);

	    for (count = 0; count < window->y_size - 2; count++)
	    {
		tmp_value = lim_ptr->max_y - (count * step_value);
		if (lim_ptr->max_y > (double) 9999999 || lim_ptr->min_y < (double) -999999)
		    sprintf (lcl_string, "%8.0f", tmp_value);
		else
		    if (lim_ptr->max_y > (double) 99 || lim_ptr->min_y < (double) -9)
			sprintf (lcl_string, "%7.0f ", tmp_value);
		    else
			sprintf (lcl_string, "%7.2f ", tmp_value);
		strcat (lcl_string, "^^");
		for (itemno = 0; itemno < gr_item_cnt; itemno++)
		{
		    if (values[itemno] >= tmp_value)
			if (values[itemno + gr_item_cnt] >= tmp_value)
			    sprintf (tmp_str, fmt_str,
				item_str[itm_ix1],
				item_str[itm_ix2]);
			else
			    sprintf (tmp_str, fmt_str,
				item_str[itm_ix1],
				item_str[0]);
		    else
			if (values[itemno + gr_item_cnt] >= tmp_value)
			    sprintf (tmp_str, fmt_str,
				item_str[0],
				item_str[itm_ix2]);
			else
			    sprintf (tmp_str, fmt_str,
				item_str[0],
				item_str[0]);
		    strcat (lcl_string, tmp_str);
		}
		if (item_width > 2)
		    strcat (lcl_string, "E");
		strcat (lcl_string, "^^");
		Dsp_save_fn (lcl_string, " ");
	    }

	    strcpy (lcl_string, "        ^^");
	    if (item_width == 2)
	    {
		for (count = 0; count < window->x_size - 9; count++)
		    strcat (lcl_string, "G");
	    }
	    else
	    {
		strcat (lcl_string, "K");
		for (itemno = 1; itemno <= gr_item_cnt; itemno++)
		{
		    for (count = 1; count < item_width; count++)
			strcat (lcl_string, "G");
		    if (itemno != gr_item_cnt)
			strcat (lcl_string, "H");
		    else
			strcat (lcl_string, "L");
		}
	    }
	    strcat (lcl_string, "^^");
	    Dsp_saverec (lcl_string);

	    strcpy (lcl_string, "        ");
	    for (count = 0; count < gr_item_cnt; count++)
	    {
		if (item_width > 2)
		    sprintf (tmp_str, "^E%-*.*s",
			item_width - 1,
			item_width - 1,
			*(names->legends + count));
		else
		    sprintf (tmp_str, "%-*.*s",
			item_width,
			item_width,
			*(names->legends + count));
		strcat (lcl_string, tmp_str);
	    }
	    if (item_width > 2)
		strcat (lcl_string, "^E");
	    Dsp_saverec (lcl_string);

	    gph_rc = Dsp_srch_grph (&lim_min_lin, &lim_max_lin);
	    Dsp_close ();

	    if (gph_rc == -1)
	    {
		lim_ptr = GR_set_limits (limits, &lcl_limits, gr_item_cnt, 2, values);
		step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
		continue;
	    }

	    if (gph_rc == 1)
	    {
		step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
		wrk_limits.min_y = lim_ptr->max_y - (step_value * lim_min_lin);
		wrk_limits.max_y = lim_ptr->max_y - (step_value * lim_max_lin);
		lim_ptr = GR_set_limits (&wrk_limits, &lcl_limits, gr_item_cnt, 2, values);
		step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
		continue;
	    }

	    break;
	}
	return (EXIT_SUCCESS);
}

static	int graph_3bar (struct GR_WINDOW *window, int gr_item_cnt, struct GR_NAMES *names, double *values, struct GR_LIMITS *limits)
{
	struct	GR_LIMITS lcl_limits;
	struct	GR_LIMITS *lim_ptr;
	struct	GR_LIMITS wrk_limits;
	int	gph_rc;
	int	lim_min_lin;
	int	lim_max_lin;
	int	count;
	int	itemno;
	int	item_width;
	int	pipe_on;
	int	bar_gap;
	int	remainder;
	int	side_gap;
	int	cntr_gap;
	double	step_value;
	double	tmp_value;
	char	fmt_str[256];
	char	tmp_str[256];
	char	lcl_string[256];

	if (window->x_posn + window->x_size > ((_wide) ? 130 : 78))
	    return (EGR_WIDTH);
	if (window->x_posn < 0 || window->x_size < ((gr_item_cnt * 3) + 8))
	    return (EGR_WIDTH);
	if (window->y_posn + window->y_size > 23 || window->y_posn < 0 || window->y_size < 5)
	    return (EGR_HEIGHT);

	lim_ptr = GR_set_limits (limits, &lcl_limits, gr_item_cnt, 3, values);

	step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
	item_width = (window->x_size - 8) / gr_item_cnt;
	if (item_width == 3)
	    sprintf (fmt_str, "%%-1.1s%%-1.1s%%-1.1s");
	else
	{
	    bar_gap = (item_width + 3) / 7;
	    remainder = (item_width - 1) - (bar_gap * 3);
	    cntr_gap = remainder / 4;
	    if (cntr_gap == 0 && remainder > 1)
		cntr_gap = 1;
	    side_gap = (remainder - (cntr_gap * 2));
	    pipe_on = side_gap / 2;
	    pipe_on = side_gap - (pipe_on * 2);
	    side_gap = side_gap / 2;
	    if (pipe_on)
	    {
		sprintf (fmt_str, "E%-*.*s%%-%d.%ds%-*.*s%%-%d.%ds%-*.*s%%-%d.%ds%-*.*s",
		    side_gap, side_gap, item_str[0],
		    bar_gap, bar_gap,
		    cntr_gap, cntr_gap, item_str[0],
		    bar_gap, bar_gap,
		    cntr_gap, cntr_gap, item_str[0],
		    bar_gap, bar_gap,
		    side_gap, side_gap, item_str[0]);
	    }
	    else
	    {
		sprintf (fmt_str, "E%-*.*s%%-%d.%ds%-*.*s%%-%d.%ds%-*.*s%%-%d.%ds%-*.*s",
		    side_gap - 1, side_gap - 1, item_str[0],
		    bar_gap, bar_gap,
		    cntr_gap, cntr_gap, item_str[0],
		    bar_gap, bar_gap,
		    cntr_gap, cntr_gap, item_str[0],
		    bar_gap, bar_gap,
		    side_gap, side_gap, item_str[0]);
	    }
	}

	while (1)
	{
	    GR_setup_hdg (window, names, 3);

	    for (count = 0; count < window->y_size - 2; count++)
	    {
		tmp_value = lim_ptr->max_y - (count * step_value);
		if (lim_ptr->max_y > (double) 9999999 || lim_ptr->min_y < (double) -999999)
		    sprintf (lcl_string, "%8.0f", tmp_value);
		else
		    if (lim_ptr->max_y > (double) 99 || lim_ptr->min_y < (double) -9)
			sprintf (lcl_string, "%7.0f ", tmp_value);
		    else
			sprintf (lcl_string, "%7.2f ", tmp_value);
		strcat (lcl_string, "^^");
		for (itemno = 0; itemno < gr_item_cnt; itemno++)
		{
		  if (values[itemno] >= tmp_value)
		    if (values[itemno + gr_item_cnt] >= tmp_value)
		      if (values[itemno + (gr_item_cnt * 2)] >= tmp_value)
			sprintf (tmp_str, fmt_str,
			    item_str[itm_ix1],
			    item_str[itm_ix2],
			    item_str[itm_ix3]);
		      else
			sprintf (tmp_str, fmt_str,
			    item_str[itm_ix1],
			    item_str[itm_ix2],
			    item_str[0]);
		    else
		      if (values[itemno + (gr_item_cnt * 2)] >= tmp_value)
			sprintf (tmp_str, fmt_str,
			    item_str[itm_ix1],
			    item_str[0],
			    item_str[itm_ix3]);
		      else
			sprintf (tmp_str, fmt_str,
			    item_str[itm_ix1],
			    item_str[0],
			    item_str[0]);
		  else
		    if (values[itemno + gr_item_cnt] >= tmp_value)
		      if (values[itemno + (gr_item_cnt * 2)] >= tmp_value)
			sprintf (tmp_str, fmt_str,
			    item_str[0],
			    item_str[itm_ix2],
			    item_str[itm_ix3]);
		      else
			sprintf (tmp_str, fmt_str,
			    item_str[0],
			    item_str[itm_ix2],
			    item_str[0]);
		    else
		      if (values[itemno + (gr_item_cnt * 2)] >= tmp_value)
			sprintf (tmp_str, fmt_str,
			    item_str[0],
			    item_str[0],
			    item_str[itm_ix3]);
		      else
			sprintf (tmp_str, fmt_str,
			    item_str[0],
			    item_str[0],
			    item_str[0]);
		    strcat (lcl_string, tmp_str);
		}
		if (item_width != 3)
		    strcat (lcl_string, "E");
		strcat (lcl_string, "^^");
		Dsp_save_fn (lcl_string, " ");
	    }

	    strcpy (lcl_string, "        ^^");
	    if (item_width != 3)
		strcat (lcl_string, "K");
	    for (itemno = 1; itemno <= gr_item_cnt; itemno++)
	    {
		for (count = 1; count < item_width; count++)
		    strcat (lcl_string, "G");
		if (item_width == 3)
		    strcat (lcl_string, "G");
		else
		    if (itemno != gr_item_cnt)
			strcat (lcl_string, "H");
		    else
			strcat (lcl_string, "L");
	    }
	    strcat (lcl_string, "^^");
	    Dsp_saverec (lcl_string);

	    strcpy (lcl_string, "        ");
	    for (count = 0; count < gr_item_cnt; count++)
	    {
		if (item_width == 3)
		    sprintf (tmp_str, "%-*.*s",
			item_width,
			item_width,
			*(names->legends + count));
		else
		    sprintf (tmp_str, "^E%-*.*s",
			item_width - 1,
			item_width - 1,
			*(names->legends + count));
		strcat (lcl_string, tmp_str);
	    }
	    if (item_width > 3)
		strcat (lcl_string, "^E");
	    Dsp_saverec (lcl_string);

	    gph_rc = Dsp_srch_grph (&lim_min_lin, &lim_max_lin);
	    Dsp_close ();

	    if (gph_rc == -1)
	    {
		lim_ptr = GR_set_limits (limits, &lcl_limits, gr_item_cnt, 3, values);
		step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
		continue;
	    }

	    if (gph_rc == 1)
	    {
		step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
		wrk_limits.min_y = lim_ptr->max_y - (step_value * lim_min_lin);
		wrk_limits.max_y = lim_ptr->max_y - (step_value * lim_max_lin);
		lim_ptr = GR_set_limits (&wrk_limits, &lcl_limits, gr_item_cnt, 3, values);
		step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
		continue;
	    }

	    break;
	}
	return (EXIT_SUCCESS);
}

static	int graph_4bar (struct GR_WINDOW *window, int gr_item_cnt, struct GR_NAMES *names, double *values, struct GR_LIMITS *limits)
{
	struct	GR_LIMITS lcl_limits;
	struct	GR_LIMITS *lim_ptr;
	struct	GR_LIMITS wrk_limits;
	int	gph_rc;
	int	lim_min_lin;
	int	lim_max_lin;
	int	count;
	int	itemno;
	int	item_width;
	int	pipe_on;
	int	bar_gap;
	int	side_gap;
	int	cntr_gap;
	int	remainder;
	double	step_value;
	double	tmp_value;
	char	fmt_str[256];
	char	tmp_str[256];
	char	lcl_string[256];

	if (window->x_posn + window->x_size > ((_wide) ? 130 : 78))
	    return (EGR_WIDTH);
	if (window->x_posn < 0 || window->x_size < ((gr_item_cnt * 4) + 8))
	    return (EGR_WIDTH);
	if (window->y_posn + window->y_size > 23 || window->y_posn < 0 || window->y_size < 5)
	    return (EGR_HEIGHT);

	lim_ptr = GR_set_limits (limits, &lcl_limits, gr_item_cnt, 4, values);

	step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
	item_width = (window->x_size - 8) / gr_item_cnt;
	if (item_width == 4)
	    sprintf (fmt_str, "%%-1.1s%%-1.1s%%-1.1s%%-1.1s");
	else
	{
	    bar_gap = (item_width + 4) / 9;
	    remainder = (item_width - 1) - (bar_gap * 4);
	    cntr_gap = remainder / 5;
	    if (cntr_gap == 0 && remainder > 2)
		cntr_gap = 1;
	    side_gap = (remainder - (cntr_gap * 3));
	    pipe_on = side_gap / 2;
	    pipe_on = side_gap - (pipe_on * 2);
	    side_gap = side_gap / 2;
	    sprintf (fmt_str, "E%-*.*s%%-%d.%ds%-*.*s%%-%d.%ds%-*.*s%%-%d.%ds%-*.*s%%-%d.%ds%-*.*s",
		side_gap, side_gap, item_str[0],
		bar_gap, bar_gap,
		cntr_gap, cntr_gap, item_str[0],
		bar_gap, bar_gap,
		cntr_gap, cntr_gap, item_str[0],
		bar_gap, bar_gap,
		cntr_gap, cntr_gap, item_str[0],
		bar_gap, bar_gap,
		side_gap + pipe_on, side_gap + pipe_on, item_str[0]);
	}

	while (1)
	{
	    GR_setup_hdg (window, names, 4);

	    for (count = 0; count < window->y_size - 2; count++)
	    {
		tmp_value = lim_ptr->max_y - (count * step_value);
		if (lim_ptr->max_y > (double) 9999999 || lim_ptr->min_y < (double) -999999)
		    sprintf (lcl_string, "%8.0f", tmp_value);
		else
		    if (lim_ptr->max_y > (double) 99 || lim_ptr->min_y < (double) -9)
			sprintf (lcl_string, "%7.0f ", tmp_value);
		    else
			sprintf (lcl_string, "%7.2f ", tmp_value);
		strcat (lcl_string, "^^");
		for (itemno = 0; itemno < gr_item_cnt; itemno++)
		{
		  if (values[itemno] >= tmp_value)
		    if (values[itemno + gr_item_cnt] >= tmp_value)
		      if (values[itemno + (gr_item_cnt * 2)] >= tmp_value)
			if (values[itemno + (gr_item_cnt * 3)] >= tmp_value)
			  sprintf (tmp_str, fmt_str,
			    item_str[itm_ix1],
			    item_str[itm_ix2],
			    item_str[itm_ix3],
			    item_str[itm_ix4]);
			else
			  sprintf (tmp_str, fmt_str,
			    item_str[itm_ix1],
			    item_str[itm_ix2],
			    item_str[itm_ix3],
			    item_str[0]);
		      else
			if (values[itemno + (gr_item_cnt * 3)] >= tmp_value)
			  sprintf (tmp_str, fmt_str,
			    item_str[itm_ix1],
			    item_str[itm_ix2],
			    item_str[0],
			    item_str[itm_ix4]);
			else
			  sprintf (tmp_str, fmt_str,
			    item_str[itm_ix1],
			    item_str[itm_ix2],
			    item_str[0],
			    item_str[0]);
		    else
		      if (values[itemno + (gr_item_cnt * 2)] >= tmp_value)
			if (values[itemno + (gr_item_cnt * 3)] >= tmp_value)
			  sprintf (tmp_str, fmt_str,
			    item_str[itm_ix1],
			    item_str[0],
			    item_str[itm_ix3],
			    item_str[itm_ix4]);
			else
			  sprintf (tmp_str, fmt_str,
			    item_str[itm_ix1],
			    item_str[0],
			    item_str[itm_ix3],
			    item_str[0]);
		      else
			if (values[itemno + (gr_item_cnt * 3)] >= tmp_value)
			  sprintf (tmp_str, fmt_str,
			    item_str[itm_ix1],
			    item_str[0],
			    item_str[0],
			    item_str[itm_ix4]);
			else
			  sprintf (tmp_str, fmt_str,
			    item_str[itm_ix1],
			    item_str[0],
			    item_str[0],
			    item_str[0]);
		  else
		    if (values[itemno + gr_item_cnt] >= tmp_value)
		      if (values[itemno + (gr_item_cnt * 2)] >= tmp_value)
			if (values[itemno + (gr_item_cnt * 3)] >= tmp_value)
			  sprintf (tmp_str, fmt_str,
			    item_str[0],
			    item_str[itm_ix2],
			    item_str[itm_ix3],
			    item_str[itm_ix4]);
			else
			  sprintf (tmp_str, fmt_str,
			    item_str[0],
			    item_str[itm_ix2],
			    item_str[itm_ix3],
			    item_str[0]);
		      else
			if (values[itemno + (gr_item_cnt * 3)] >= tmp_value)
			  sprintf (tmp_str, fmt_str,
			    item_str[0],
			    item_str[itm_ix2],
			    item_str[0],
			    item_str[itm_ix4]);
			else
			  sprintf (tmp_str, fmt_str,
			    item_str[0],
			    item_str[itm_ix2],
			    item_str[0],
			    item_str[0]);
		    else
		      if (values[itemno + (gr_item_cnt * 2)] >= tmp_value)
			if (values[itemno + (gr_item_cnt * 3)] >= tmp_value)
			  sprintf (tmp_str, fmt_str,
			    item_str[0],
			    item_str[0],
			    item_str[itm_ix3],
			    item_str[itm_ix4]);
			else
			  sprintf (tmp_str, fmt_str,
			    item_str[0],
			    item_str[0],
			    item_str[itm_ix3],
			    item_str[0]);
		      else
			if (values[itemno + (gr_item_cnt * 3)] >= tmp_value)
			  sprintf (tmp_str, fmt_str,
			    item_str[0],
			    item_str[0],
			    item_str[0],
			    item_str[itm_ix4]);
			else
			  sprintf (tmp_str, fmt_str,
			    item_str[0],
			    item_str[0],
			    item_str[0],
			    item_str[0]);
		    strcat (lcl_string, tmp_str);
		}
		if (item_width != 4)
		    strcat (lcl_string, "E");
		strcat (lcl_string, "^^");
		Dsp_save_fn (lcl_string, " ");
	    }

	    strcpy (lcl_string, "        ^^");
	    if (item_width != 4)
		strcat (lcl_string, "K");
	    for (itemno = 1; itemno <= gr_item_cnt; itemno++)
	    {
		for (count = 1; count < item_width; count++)
		    strcat (lcl_string, "G");
		if (item_width == 4)
		    strcat (lcl_string, "G");
		else
		    if (itemno != gr_item_cnt)
			strcat (lcl_string, "H");
		    else
			strcat (lcl_string, "L");
	    }
	    strcat (lcl_string, "^^");
	    Dsp_saverec (lcl_string);

	    strcpy (lcl_string, "        ");
	    for (count = 0; count < gr_item_cnt; count++)
	    {
		if (item_width == 4)
		    sprintf (tmp_str, "%-*.*s",
			item_width,
			item_width,
			*(names->legends + count));
		else
		    sprintf (tmp_str, "^E%-*.*s",
			item_width - 1,
			item_width - 1,
			*(names->legends + count));
		strcat (lcl_string, tmp_str);
	    }
	    if (item_width > 4)
		strcat (lcl_string, "^E");
	    Dsp_saverec (lcl_string);

	    gph_rc = Dsp_srch_grph (&lim_min_lin, &lim_max_lin);
	    Dsp_close ();

	    if (gph_rc == -1)
	    {
		lim_ptr = GR_set_limits (limits, &lcl_limits, gr_item_cnt, 4, values);
		step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
		continue;
	    }

	    if (gph_rc == 1)
	    {
		step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
		wrk_limits.min_y = lim_ptr->max_y - (step_value * lim_min_lin);
		wrk_limits.max_y = lim_ptr->max_y - (step_value * lim_max_lin);
		lim_ptr = GR_set_limits (&wrk_limits, &lcl_limits, gr_item_cnt, 4, values);
		step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
		continue;
	    }

	    break;
	}
	return (EXIT_SUCCESS);
}

static	int graph_5bar (struct GR_WINDOW *window, int gr_item_cnt, struct GR_NAMES *names, double *values, struct GR_LIMITS *limits)
{
	struct	GR_LIMITS lcl_limits;
	struct	GR_LIMITS *lim_ptr;
	struct	GR_LIMITS wrk_limits;
	int	gph_rc;
	int	lim_min_lin;
	int	lim_max_lin;
	int	count;
	int	itemno;
	int	item_width;
	int	pipe_on;
	int	bar_gap;
	int	side_gap;
	int	cntr_gap;
	int	remainder;
	double	step_value;
	double	tmp_value;
	char	fmt_str[256];
	char	tmp_str[256];
	char	lcl_string[256];

	if (window->x_posn + window->x_size > ((_wide) ? 130 : 78))
	    return (EGR_WIDTH);
	if (window->x_posn < 0 || window->x_size < ((gr_item_cnt * 5) + 8))
	    return (EGR_WIDTH);
	if (window->y_posn + window->y_size > 23 || window->y_posn < 0 || window->y_size < 5)
	    return (EGR_HEIGHT);

	lim_ptr = GR_set_limits (limits, &lcl_limits, gr_item_cnt, 5, values);

	step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
	item_width = (window->x_size - 8) / gr_item_cnt;
	if (item_width == 5)
	    sprintf (fmt_str, "%%-1.1s%%-1.1s%%-1.1s%%-1.1s%%-1.1s");
	else
	{
	    bar_gap = (item_width + 5) / 11;
	    remainder = (item_width - 1) - (bar_gap * 5);
	    cntr_gap = remainder / 6;
	    if (cntr_gap == 0 && remainder > 3)
		cntr_gap = 1;
	    side_gap = (remainder - (cntr_gap * 4));
	    pipe_on = side_gap / 2;
	    pipe_on = side_gap - (pipe_on * 2);
	    side_gap = side_gap / 2;
	    sprintf (fmt_str, "E%-*.*s%%-%d.%ds%-*.*s%%-%d.%ds%-*.*s%%-%d.%ds%-*.*s%%-%d.%ds%-*.*s%%-%d.%ds%-*.*s",
		side_gap, side_gap, item_str[0],
		bar_gap, bar_gap,
		cntr_gap, cntr_gap, item_str[0],
		bar_gap, bar_gap,
		cntr_gap, cntr_gap, item_str[0],
		bar_gap, bar_gap,
		cntr_gap, cntr_gap, item_str[0],
		bar_gap, bar_gap,
		cntr_gap, cntr_gap, item_str[0],
		bar_gap, bar_gap,
		side_gap + pipe_on, side_gap + pipe_on, item_str[0]);
	}

	while (1)
	{
	    GR_setup_hdg (window, names, 5);

	    for (count = 0; count < window->y_size - 2; count++)
	    {
		tmp_value = lim_ptr->max_y - (count * step_value);
		if (lim_ptr->max_y > (double) 9999999 || lim_ptr->min_y < (double) -999999)
		    sprintf (lcl_string, "%8.0f", tmp_value);
		else
		    if (lim_ptr->max_y > (double) 99 || lim_ptr->min_y < (double) -9)
			sprintf (lcl_string, "%7.0f ", tmp_value);
		    else
			sprintf (lcl_string, "%7.2f ", tmp_value);
		strcat (lcl_string, "^^");
		for (itemno = 0; itemno < gr_item_cnt; itemno++)
		{
		  if (values[itemno] >= tmp_value)
		  {
		    if (values[itemno + gr_item_cnt] >= tmp_value)
		    {
		      if (values[itemno + (gr_item_cnt * 2)] >= tmp_value)
		      {
			if (values[itemno + (gr_item_cnt * 3)] >= tmp_value)
			{
			  if (values[itemno + (gr_item_cnt * 4)] >= tmp_value)
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[itm_ix1],
			      item_str[itm_ix2],
			      item_str[itm_ix3],
			      item_str[itm_ix4],
			      item_str[itm_ix5]);
			  }
			  else
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[itm_ix1],
			      item_str[itm_ix2],
			      item_str[itm_ix3],
			      item_str[itm_ix4],
			      item_str[0]);
			  }
			}
			else
			{
			  if (values[itemno + (gr_item_cnt * 4)] >= tmp_value)
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[itm_ix1],
			      item_str[itm_ix2],
			      item_str[itm_ix3],
			      item_str[0],
			      item_str[itm_ix5]);
			  }
			  else
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[itm_ix1],
			      item_str[itm_ix2],
			      item_str[itm_ix3],
			      item_str[0],
			      item_str[0]);
			  }
			}
		      }
		      else
		      {
			if (values[itemno + (gr_item_cnt * 3)] >= tmp_value)
			{
			  if (values[itemno + (gr_item_cnt * 4)] >= tmp_value)
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[itm_ix1],
			      item_str[itm_ix2],
			      item_str[0],
			      item_str[itm_ix4],
			      item_str[itm_ix5]);
			  }
			  else
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[itm_ix1],
			      item_str[itm_ix2],
			      item_str[0],
			      item_str[itm_ix4],
			      item_str[0]);
			  }
			}
			else
			{
			  if (values[itemno + (gr_item_cnt * 4)] >= tmp_value)
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[itm_ix1],
			      item_str[itm_ix2],
			      item_str[0],
			      item_str[0],
			      item_str[itm_ix5]);
			  }
			  else
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[itm_ix1],
			      item_str[itm_ix2],
			      item_str[0],
			      item_str[0],
			      item_str[0]);
			  }
			}
		      }
		    }
		    else
		    {
		      if (values[itemno + (gr_item_cnt * 2)] >= tmp_value)
		      {
			if (values[itemno + (gr_item_cnt * 3)] >= tmp_value)
			{
			  if (values[itemno + (gr_item_cnt * 4)] >= tmp_value)
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[itm_ix1],
			      item_str[0],
			      item_str[itm_ix3],
			      item_str[itm_ix4],
			      item_str[itm_ix5]);
			  }
			  else
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[itm_ix1],
			      item_str[0],
			      item_str[itm_ix3],
			      item_str[itm_ix4],
			      item_str[0]);
			  }
			}
			else
			{
			  if (values[itemno + (gr_item_cnt * 4)] >= tmp_value)
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[itm_ix1],
			      item_str[0],
			      item_str[itm_ix3],
			      item_str[0],
			      item_str[itm_ix5]);
			  }
			  else
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[itm_ix1],
			      item_str[0],
			      item_str[itm_ix3],
			      item_str[0],
			      item_str[0]);
			  }
			}
		      }
		      else
		      {
			if (values[itemno + (gr_item_cnt * 3)] >= tmp_value)
			{
			  if (values[itemno + (gr_item_cnt * 4)] >= tmp_value)
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[itm_ix1],
			      item_str[0],
			      item_str[0],
			      item_str[itm_ix4],
			      item_str[itm_ix5]);
			  }
			  else
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[itm_ix1],
			      item_str[0],
			      item_str[0],
			      item_str[itm_ix4],
			      item_str[0]);
			  }
			}
			else
			{
			  if (values[itemno + (gr_item_cnt * 4)] >= tmp_value)
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[itm_ix1],
			      item_str[0],
			      item_str[0],
			      item_str[0],
			      item_str[itm_ix5]);
			  }
			  else
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[itm_ix1],
			      item_str[0],
			      item_str[0],
			      item_str[0],
			      item_str[0]);
			  }
			}
		      }
		    }
		  }
		  else
		  {
		    if (values[itemno + gr_item_cnt] >= tmp_value)
		    {
		      if (values[itemno + (gr_item_cnt * 2)] >= tmp_value)
		      {
			if (values[itemno + (gr_item_cnt * 3)] >= tmp_value)
			{
			  if (values[itemno + (gr_item_cnt * 4)] >= tmp_value)
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[0],
			      item_str[itm_ix2],
			      item_str[itm_ix3],
			      item_str[itm_ix4],
			      item_str[itm_ix5]);
			  }
			  else
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[0],
			      item_str[itm_ix2],
			      item_str[itm_ix3],
			      item_str[itm_ix4],
			      item_str[0]);
			  }
			}
			else
			{
			  if (values[itemno + (gr_item_cnt * 4)] >= tmp_value)
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[0],
			      item_str[itm_ix2],
			      item_str[itm_ix3],
			      item_str[0],
			      item_str[itm_ix5]);
			  }
			  else
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[0],
			      item_str[itm_ix2],
			      item_str[itm_ix3],
			      item_str[0],
			      item_str[0]);
			  }
			}
		      }
		      else
		      {
			if (values[itemno + (gr_item_cnt * 3)] >= tmp_value)
			{
			  if (values[itemno + (gr_item_cnt * 4)] >= tmp_value)
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[0],
			      item_str[itm_ix2],
			      item_str[0],
			      item_str[itm_ix4],
			      item_str[itm_ix5]);
			  }
			  else
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[0],
			      item_str[itm_ix2],
			      item_str[0],
			      item_str[itm_ix4],
			      item_str[0]);
			  }
			}
			else
			{
			  if (values[itemno + (gr_item_cnt * 4)] >= tmp_value)
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[0],
			      item_str[itm_ix2],
			      item_str[0],
			      item_str[0],
			      item_str[itm_ix5]);
			  }
			  else
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[0],
			      item_str[itm_ix2],
			      item_str[0],
			      item_str[0],
			      item_str[0]);
			  }
			}
		      }
		    }
		    else
		    {
		      if (values[itemno + (gr_item_cnt * 2)] >= tmp_value)
		      {
			if (values[itemno + (gr_item_cnt * 3)] >= tmp_value)
			{
			  if (values[itemno + (gr_item_cnt * 4)] >= tmp_value)
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[0],
			      item_str[0],
			      item_str[itm_ix3],
			      item_str[itm_ix4],
			      item_str[itm_ix5]);
			  }
			  else
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[0],
			      item_str[0],
			      item_str[itm_ix3],
			      item_str[itm_ix4],
			      item_str[0]);
			  }
			}
			else
			{
			  if (values[itemno + (gr_item_cnt * 4)] >= tmp_value)
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[0],
			      item_str[0],
			      item_str[itm_ix3],
			      item_str[0],
			      item_str[itm_ix5]);
			  }
			  else
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[0],
			      item_str[0],
			      item_str[itm_ix3],
			      item_str[0],
			      item_str[0]);
			  }
			}
		      }
		      else
		      {
			if (values[itemno + (gr_item_cnt * 3)] >= tmp_value)
			{
			  if (values[itemno + (gr_item_cnt * 4)] >= tmp_value)
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[0],
			      item_str[0],
			      item_str[0],
			      item_str[itm_ix4],
			      item_str[itm_ix5]);
			  }
			  else
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[0],
			      item_str[0],
			      item_str[0],
			      item_str[itm_ix4],
			      item_str[0]);
			  }
			}
			else
			{
			  if (values[itemno + (gr_item_cnt * 4)] >= tmp_value)
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[0],
			      item_str[0],
			      item_str[0],
			      item_str[0],
			      item_str[itm_ix5]);
			  }
			  else
			  {
			    sprintf (tmp_str, fmt_str,
			      item_str[0],
			      item_str[0],
			      item_str[0],
			      item_str[0],
			      item_str[0]);
			  }
			}
		      }
		    }
		  }
		  strcat (lcl_string, tmp_str);
		}
		if (item_width != 5)
		    strcat (lcl_string, "E");
		strcat (lcl_string, "^^");
		Dsp_save_fn (lcl_string, " ");
	    }

	    strcpy (lcl_string, "        ^^");
	    if (item_width != 5)
		strcat (lcl_string, "K");
	    for (itemno = 1; itemno <= gr_item_cnt; itemno++)
	    {
		for (count = 1; count < item_width; count++)
		    strcat (lcl_string, "G");
		if (item_width == 5)
		    strcat (lcl_string, "G");
		else
		    if (itemno != gr_item_cnt)
			strcat (lcl_string, "H");
		    else
			strcat (lcl_string, "L");
	    }
	    strcat (lcl_string, "^^");
	    Dsp_saverec (lcl_string);

	    strcpy (lcl_string, "        ");
	    for (count = 0; count < gr_item_cnt; count++)
	    {
		if (item_width == 5)
		    sprintf (tmp_str, "%-*.*s",
			item_width,
			item_width,
			*(names->legends + count));
		else
		    sprintf (tmp_str, "^E%-*.*s",
			item_width - 1,
			item_width - 1,
			*(names->legends + count));
		strcat (lcl_string, tmp_str);
	    }
	    if (item_width > 5)
		strcat (lcl_string, "^E");
	    Dsp_saverec (lcl_string);

	    gph_rc = Dsp_srch_grph (&lim_min_lin, &lim_max_lin);
	    Dsp_close ();

	    if (gph_rc == -1)
	    {
		lim_ptr = GR_set_limits (limits, &lcl_limits, gr_item_cnt, 5, values);
		step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
		continue;
	    }

	    if (gph_rc == 1)
	    {
		step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
		wrk_limits.min_y = lim_ptr->max_y - (step_value * lim_min_lin);
		wrk_limits.max_y = lim_ptr->max_y - (step_value * lim_max_lin);
		lim_ptr = GR_set_limits (&wrk_limits, &lcl_limits, gr_item_cnt, 5, values);
		step_value = (lim_ptr->max_y - lim_ptr->min_y) / (window->y_size - 3);
		continue;
	    }

	    break;
	}
	return (EXIT_SUCCESS);
}
