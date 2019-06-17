#ifndef DSP_UTILS_H
#define DSP_UTILS_H

#define	PAGES	500
#define	PSIZE	20

long	_disp_page[PAGES];

struct	{
	char	*_dsp;
	char	*_key;
} _scn[PSIZE];

int	_left_col;	/* top left corner		*/
int	_top_row;	/* of diplay box		*/
int	_top_line;	/* start of _page_length lines	*/
int	_page_length;	/* number of lines to display	*/
int	_page_width;	/* internal width of display box*/
int	_extras;
int	_disp_on;

char	*_header[3];

FILE	*_work_file;

void	tdsp_open (int left_col, int top_row, int page_size);
void	tdsp_close (void);
int	tdsp_saverec (char *in_str);
int	dsp_save_fn (char * in_str, char * _extra);
int	_Null_fn (char * _null_str);
int	tdsp_srch (void);
int	dsp_srch_fn (int (*_run_fn) (char *));
int	_Line_down (int line_no, int page_size);
int	_Line_up (int line_no, int page_size);
void	_Show_display (int line_no);
void	_Show_line (int line_no, int rv);
int	t_load_display (int max_lines, int page_no);
int	t_load_d_line (int line_no);
int	t_print_display (int p_width);
void	t_parse_str (char * in_str, char * _extra, int page_width);

void
tdsp_open (
 int	left_col,
 int	top_row,
 int	page_size)
{
	register	int	i;
	int	fd;
	static	int	set;
	char	*sptr = getenv("PROG_PATH");
	char	work_name[101];

	if (set)
		_disp_on = 1;
	else
	{
		if (_disp_on)
			sys_err("dsp_close() has not been run for the previous display",-1,PNAME);
		_disp_on = 1;
	}

	set = 1;

	_extras = 0;

	sprintf(work_name,"%s/WORK/dsp%06d",(sptr != (char *)0) ? sptr : "/usr/DB",getpid());
	if ((_work_file = fopen(work_name,"w")) == 0)
	{
		sprintf(err_str,"Error in %s during (WKOPEN)",work_name);
		sys_err(err_str,errno,"Dsp_open");
	}

	fd = fileno(_work_file);

	_header[0] = (char *)0;
	_header[1] = (char *)0;
	_header[2] = (char *)0;

	_left_col = left_col;
	_top_row = top_row;
	_page_length = page_size;

	max_work = 0;
	_top_line = 0;
	_page_width = 0;

	for (i = 0;i < _page_length;i++)
	{
		_scn[i]._dsp = (char *)0;
		_scn[i]._key = (char *)0;
	}
}

void
tdsp_close (void)
{
	register	int	i;
	char	work_name[101];
	char	*sptr = getenv("PROG_PATH");

	fclose(_work_file);
	sprintf(work_name,"%s/WORK/dsp%06d",(sptr != (char *)0) ? sptr : "/usr/DB",getpid());
	unlink(work_name);

	_disp_on = 0;

	for (i = 0;i < 3;i++)
	{
		if (_header[i] != (char *)0)
		{
			free(_header[i]);
			_header[i] = (char *)0;
		}
	}
	max_work = 0;

	for (i = 0;i < _page_length;i++)
	{
		if (_scn[i]._dsp != (char *)0)
		{
			free(_scn[i]._dsp);
			_scn[i]._dsp = (char *)0;
		}

		if (_scn[i]._key != (char *)0)
		{
			free(_scn[i]._key);
			_scn[i]._key = (char *)0;
		}
	}
}

int
tdsp_saverec (
 char *in_str)
{
	return(dsp_save_fn(in_str,(char *)0));
}

int
dsp_save_fn (
 char *	in_str,
 char *	_extra)
{
	static	int	x;
	static	int	xl;
	static	int	y;
	char	*p_strsave(char *);

	if (max_work <= 2)
	{
		if (max_work == 0)
			_page_width = strlen(in_str);

		if (strlen(in_str))
			_header[max_work] = p_strsave(in_str);
		max_work++;
		return(0);
	}

	if (max_work == 3)
	{
		crsr_off();
		t_print_display(_page_width);
		x = (_page_width - 13) / 2;
		x += _left_col;
		xl = (_page_width - 27) / 2;
		xl += _left_col;
		y = (_page_length - 1) / 2;
		y += _top_line;
	}
	if (((max_work - 3) / _page_length) >= PAGES)
		return(1);

	if (((max_work - 3) % 20) == 0)
	{
		if (((max_work - 3) % 40) == 0)
			rv_pr(" Loading ... ",x,y,0);
		else
			rv_pr(" Loading ... ",x,y,1);
	}

	if (((max_work - 3) % _page_length) == 0)
		_disp_page[(max_work - 3) / _page_length] = ftell(_work_file);

	t_parse_str(in_str,_extra,_page_width);
	max_work++;
	return(0);
}

int
_Null_fn (
 char *	_null_str)
{
	return (0);
}

int
tdsp_srch (void)
{
	return (dsp_srch_fn (_Null_fn));
}

int
dsp_srch_fn (
 int (*_run_fn) (char *))
{
	int	fd;
	int	c;
	int	max_lines = max_work - 4;
	int	line_no = 0;
	int	old_line_no = -1;
	int	page_no = 0;
	int	old_page_no = -1;
	int	page_size = 0;
	int	max_page;
	char	work_name[101];
	char	*sptr = getenv("PROG_PATH");

	max_page = max_lines / _page_length;
	
	if (max_lines < 0)
	{
		tdsp_saverec(" ");
		tdsp_saverec(" ** NO DATA ** ");
		max_lines = 1;
	}

	fclose(_work_file);
	sprintf(work_name,"%s/WORK/dsp%06d",(sptr != (char *)0) ? sptr : "/usr/DB",getpid());
	if ((_work_file = fopen(work_name,"r")) == NULL)
	{
		fclose(_work_file);
		return(1);
	}

	fd = fileno(_work_file);

	if (_header[2] == (char *)0)
	{
		crsr_on();
		return(0);
	}

	while (1)
	{
		if (page_no != old_page_no)
		{
			page_size = t_load_display(max_lines,page_no);
			if (page_size == -1)
			{
				crsr_on();
				return(1);
			}

			if (_extras && line_no != old_line_no)
				line_no = _Line_down(-1,page_size);
			_Show_display(line_no);
		}
		else
		{
			if (_extras && line_no != old_line_no)
			{
				_Show_line(old_line_no,0);
				_Show_line(line_no,1);
			}
		}

		old_page_no = page_no;
		old_line_no = line_no;

		c = getkey();

		switch (c)
		{
		/*---------------
		| Redraw	|
		---------------*/
		case	FN3:
			t_print_display(_page_width);
			old_page_no = -1;
			break;

		/*---------------
		| Page Down	|
		---------------*/
		case	FN13:
		case	FN14:
			if (++page_no > max_page)
				page_no = 0;
			line_no = -1;
			old_line_no = -2;
			break;

		/*---------------
		| Page Up	|
		---------------*/
		case	FN15:
			if (--page_no < 0)
				page_no = max_page;
			line_no = -1;
			old_line_no = -2;
			break;

		/*---------------
		| Exit Display	|
		---------------*/
		case	FN16:
			crsr_on();
			return(0);
			break;

		case	UP_KEY:
		case	11:
			if (_extras)
				line_no = _Line_up(line_no,page_size);
			else
				putchar('\007');
			break;

		case	DOWN_KEY:
		case	10:
			if (_extras)
				line_no = _Line_down(line_no,page_size);
			else
				putchar('\007');
			break;

		case	'\r':
			if (_extras)
			{
				(*_run_fn)(_scn[line_no]._key);
				break;
			}

		default:
			putchar('\007');
			break;
		}
	}
/*
Statements never reached!! TvB 16/11/90
	crsr_on();
	return(0);
*/
}

int
_Line_down (
 int	line_no,
 int	page_size)
{
	int	i;

	for (i = line_no;++i < page_size;)
	{
		if (_scn[i]._dsp != (char *)0 && _scn[i]._key != (char *)0)
			return(i);
	}

	for (i = -1;++i <= line_no;)
	{
		if (_scn[i]._dsp != (char *)0 && _scn[i]._key != (char *)0)
			return(i);
	}

	return(line_no);
}

int
_Line_up (
 int	line_no,
 int	page_size)
{
	int	i;

	for (i = line_no;--i >= 0;)
	{
		if (_scn[i]._dsp != (char *)0 && _scn[i]._key != (char *)0)
			return(i);
	}

	for (i = page_size;--i > line_no;)
	{
		if (_scn[i]._dsp != (char *)0 && _scn[i]._key != (char *)0)
			return(i);
	}

	return(line_no);
}

void
_Show_display (
 int	line_no)
{
	register	int	i;

	for (i = 0;i < _page_length;i++)
		_Show_line(i,(_extras && i == line_no));
}

void
_Show_line (
 int	line_no,
 int	rv)
{
	if (line_no >= 0 && line_no < _page_length && _scn[line_no]._dsp != (char *)0)
		rv_pr(_scn[line_no]._dsp,_left_col + 1,_top_line + line_no,rv);
}

int
t_load_display (
 int	max_lines,
 int	page_no)
{
	int	page_size = 0;
	register	int	i;

	if (fseek(_work_file,_disp_page[page_no],0) != 0)
	{
		fclose(_work_file);
		return(-1);
	}

	for (i = 0,page_size = 0;i < _page_length;i++)
	{
		if (t_load_d_line(i))
			page_size++;
	}

	return(page_size);
}

int
t_load_d_line (
 int	line_no)
{
	char	*sptr;
	char	*tptr;
	char	data[200];

	sptr = fgets(data,200,_work_file);

	if (_scn[line_no]._dsp != (char *)0)
	{
		free(_scn[line_no]._dsp);
		_scn[line_no]._dsp = (char *)0;
	}

	if (_scn[line_no]._key != (char *)0)
	{
		free(_scn[line_no]._key);
		_scn[line_no]._key = (char *)0;
	}

	if (sptr != (char *)0)
	{
		tptr = strchr (sptr,'\t');
		if (tptr != (char *)0)
		{
			*tptr = '\0';
			tptr++;
			_scn[line_no]._key = p_strsave(tptr);
		}

		_scn[line_no]._dsp = p_strsave(sptr);

		return(1);
	}
	return(0);
}

int
t_print_display (
 int	p_width)
{
	register	int	i = 0;
	int	page_size = _page_length + 2;
	static	int	page_width;

	_top_line = _top_row + 3;

	if (p_width != 0)
		page_width = p_width;

	/*-----------------------
	| Second header line	|
	-----------------------*/
	if (_header[1] != (char * )0)
	{
		page_size++;
		_top_line++;
	}

	/*---------------
	| Footer line	|
	---------------*/
	if (_header[2] != (char * )0)
		page_size++;

	box(_left_col,_top_row,page_width + 2,page_size);

	/*-------------------------------
	| Print First Heading Line	|
	-------------------------------*/
	rv_pr(_header[0],_left_col + 1,_top_row + 1,1);

	/*-------------------------------
	| Print Second Heading Line	|
	-------------------------------*/
	if (_header[1] != (char *)0)
		rv_pr(_header[1],_left_col + 1,_top_row + 2,1);

	move(_left_col + 1,_top_line - 1);
	line(page_width + 1);

	for (i = 0;i < _page_length;i++)
	{
		move(_left_col + 1,_top_line + i);
		printf("%-*.*s",page_width,page_width," ");
	}

	if (_header[2] != (char *)0)
	{
		move(_left_col + 1,_top_line + _page_length);
		printf("%-*.*s",page_width,page_width," ");
		rv_pr(_header[2],_left_col + ((page_width + 2) - strlen(_header[2]))/2,_top_line + _page_length,1);
	}
	fflush(stdout);
	return(0);
}

void
t_parse_str (
 char *	in_str,
 char *	_extra,
 int	page_width)
{
	int	len = page_width;
	int	indx;
	int	g_on = FALSE;
	char	*sptr = strchr (in_str,'^');
	char	*tptr = in_str;

	if (_extra != (char *)0)
		_extras = 1;

	while (g_on || sptr != (char *)0)
	{
		while (g_on && len > 0 && *sptr != '^')
		{
			indx = *sptr - 'A';
			if (indx >= 0 && indx < 12)
			{
				fprintf(_work_file,"%c",ta[12][indx]);
				len--;
			}
			sptr++;
		}

		*sptr = '\0';
		if (len > strlen(tptr))
			fprintf(_work_file,"%s",tptr);
		else
			break;

		len -= strlen(tptr);
		if (len <= 0)
			break;

		sptr++;
		switch (*sptr)
		{
		case	'^':
			g_on = !g_on;
			sptr++;
			fprintf(_work_file,"%s",(g_on) ? ta[16] : ta[17]);

			break;

		case	'1':
			fprintf(_work_file,"%s %s",ta[8],ta[9]);
			sptr++;
			len--;
			break;

		case	'2':
			fprintf(_work_file,"%s %s",ta[10],ta[11]);
			sptr++;
			len--;
			break;

		case	'3':
			fprintf(_work_file,"%s %s",ta[13],ta[14]);
			sptr++;
			len--;
			break;

		case	'4':
			fprintf(_work_file,"%s %s",ta[18],ta[19]);
			sptr++;
			len--;
			break;

		default:
			indx = *sptr - 'A';
			if (indx >= 0 && indx < 12)
			{
				if (g_on)
					fprintf(_work_file,"%c",ta[12][indx]);
				else
					fprintf(_work_file,"%s%c%s",ta[16],ta[12][indx],ta[17]);
				len--;
				sptr++;
			}
		}

		if (!g_on)
		{
			tptr = sptr;
			sptr = strchr (in_str,'^');
		}
	}

	fprintf(_work_file,"%-*.*s",len,len,tptr);

	if (g_on)
		fprintf(_work_file,"%s",ta[17]);

	if (_extra != (char *)0)
		fprintf(_work_file,"%c%s\n",'\t',_extra);
	else
		fprintf(_work_file,"\n");
}
#endif /*DSP_UTILS_H*/
