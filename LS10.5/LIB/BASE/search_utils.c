/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( search_utils.c )                                 |
|  Program Desc  : ( Logistic Search Utilities.                     ) |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (10/05/96)      | Modified  by  : Scott B Darrow.  |
|                                                                     |
|  Comments      :                                                    |
|    (10/05/96)  : Updated to speed up Search.                        |
|                :                                                    |
|  Date Modified : (07/07/1996)    | Modified  by  : Scott Darrow.    |
|   Comments     :  Updated for fast searching.                       |
|                :  Updated to fix big in fast searching.             |
|                :                                                    |
|  Date Modified : (DD/MM/YYYY)    | Modified  by  : xxxxxxxxxxxxxxxx |
|   Comments     :                                                    |
|                :                                                    |
|  Date Modified : (DD/MM/YYYY)    | Modified  by  : xxxxxxxxxxxxxxxx |
|   Comments     :                                                    |
|                :                                                    |
|                :                                                    |
| $Log: search_utils.c,v $
| Revision 5.1  2001/08/06 22:40:56  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 06:59:38  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:52:39  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/12 13:34:26  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/08/15 08:44:14  scott
| Updated as length of search description being forces to 40.
|
| Revision 2.0  2000/07/15 07:17:18  gerry
| Forced revision no. to 2.0 - Rel-15072000
|
| Revision 1.4  2000/07/14 11:02:24  scott
| Updated to fix problem when _work_open is used. Regardless of what length is set to search used length of first heading line.
|
| Revision 1.3  1999/09/13 09:36:33  scott
| Updated for Copyright
|
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+*/
#include	<std_decs.h>
#define	MAX_SRCH	12
#define	P_SIZE	(NO_SRCH_LINES - 1)
#define	MAX_PAGES	200

	int	SR_X_POS = 0,
		SR_Y_POS = 0;
	int	NO_SRCH_LINES	=	MAX_SRCH;

char	*SrchStore	[ MAX_PAGES * MAX_SRCH ];

struct	line_type {
	char	key[41];
	char	desc[81];
} _line_rec[MAX_SRCH + 1];

long	_page_no[MAX_PAGES];

extern	char	temp_str[];

int		CalcLen;
int		_CodeLen;
int		_DescLen;
int		_DspLen;
int		_DataComplete	=	0;
int		_MaxRecords		=	0;
int		_DisplayedData	=	0;
int		_PageNo			=	0;
int		_PageSize		=	0;
int		_OldPageNo		=	0;
int		_DisplayExit	=	0;
int		_FN1Pressed	=	0;
char	SrWorkStr [ 200 ];
char	DispKey[41];

/*====================================
| Open Search Work File For Writing. |
====================================*/
int
work_open(void)
{
	return (_work_open (15,0,40));
}

int
_work_open
(
	int	CodeLength,
	int	DispLength,
	int	DescLength
)
{
	_CodeLen = CodeLength;
	_DspLen = DispLength;
	_DescLen = DescLength;
	_DataComplete	=	0;
	_PageNo			=	0;
	_OldPageNo		=	0;
	_MaxRecords		=	0;
	_DisplayedData	=	0;
	_DisplayExit	=	0;
	_FN1Pressed	=	0;

	sprintf(_line_rec[NO_SRCH_LINES].key,"%-.40s"," ");
	sprintf(_line_rec[NO_SRCH_LINES].desc,"%-.80s"," ");
	_MaxRecords = 0;

	crsr_off();
	fflush(stdout);

	return(0);
}

/*====================================
| Close Search Work File and remove. |
====================================*/
void
work_close(void)
{
	return;
}

/*============================
| Save Records From Program. |
=============================*/
int
save_rec
(
	char	*field1,
	char	*field2
)
{
	extern	int		MlHide;
	extern	char	*p_strsave	(char *);
	int	sr_err = 0;
	int	box_width;
	int	CurrPage;

	MlHide	=	1;
 	CalcLen = (_DspLen) ? _DspLen : _CodeLen;
	box_width = CalcLen + _DescLen + 5;

	if (_MaxRecords / NO_SRCH_LINES >= MAX_PAGES)
		return(1);

	/*-------------------
	| Column Headings	|
	-------------------*/
	if (field1[0] == '#' && field2[0] == '#')
	{
		if (_MaxRecords == 0)
		{
			sprintf(_line_rec[NO_SRCH_LINES].key,"%-.40s",ML(field1 + 1));
			sprintf(_line_rec[NO_SRCH_LINES].desc,"%-.80s",ML(field2 + 1));
		}
		return(0);
	}

	_page_no[_MaxRecords / NO_SRCH_LINES] = (_MaxRecords / NO_SRCH_LINES) * NO_SRCH_LINES;


	if (_MaxRecords == 0)
	{
		int		newLength;

		_line_rec[NO_SRCH_LINES].key[strlen(field1)] = '\0';
		if (strlen (field1) > _CodeLen)
			_CodeLen	= strlen(field1);
		newLength = strlen(field2);
		if (newLength > _DescLen)
			_DescLen = newLength;
		_draw_page(0);
	}

	sprintf(SrWorkStr,"%-.*s|%-.*s\n",_CodeLen,field1,_DescLen,field2);
		
	if ( SrchStore [_MaxRecords] != (char *) NULL )	
	{
		free ( SrchStore [_MaxRecords] );
		SrchStore [_MaxRecords] = (char *) NULL;
	}
	SrchStore [_MaxRecords] = p_strsave (SrWorkStr);
	if ( SrchStore [_MaxRecords] == (char *) NULL )	
	{
		printf ("\r\nSystem Error : malloc failed in search routines\007.");
		fflush (stdout);
		sleep (2);
		return (-1);
	}
	_MaxRecords++;

	CurrPage = (_MaxRecords / NO_SRCH_LINES) * NO_SRCH_LINES;
		if (_MaxRecords % 20 == 0)

	if ( _MaxRecords != 0 && (_MaxRecords % NO_SRCH_LINES == 0) && !_DisplayExit)
	{
		int	rtnVal;

		rtnVal = _disp_srch ();
		return (rtnVal);
	}

	return(sr_err);
}

int
disp_srch (void)
{
	int	rtnVal;

	_DataComplete	=	1;
	/*------------------------------------------------------------
	| No Lines where found return to program and reset temp_str. |
	------------------------------------------------------------*/
	if (_MaxRecords == 0)
	{
		_draw_page(0);
		strcpy(temp_str," ");
		rv_pr(" No Match found. ",2 + SR_X_POS,SR_Y_POS + ((NO_SRCH_LINES + 6)/2) ,1);
		fflush(stdout);
		sleep(1);
		crsr_on();
		return(1);
	}
	if (_DisplayExit)
	{
		if (_FN1Pressed)
			return (EXIT_FAILURE);

		return (EXIT_SUCCESS);
	}

	rtnVal = _disp_srch ();
	return (rtnVal);
/*	return (_disp_srch()); */
}
/*============================================
| Display Search Data , 15 lines per screen. |
============================================*/
int
_disp_srch (void)
{
	int	c;
	int	lines = 0;
	int	max_lines = _MaxRecords - 1;
	int	_RedrawScreen 	=	0;
	int	rtnVal;

	crsr_off();

	_draw_page(1);
	_PageSize = _load_page(max_lines,0);

	_DisplayedData = 1;

	/*--------------------
	| Control movement . |
	--------------------*/
	while ((c = getkey()))
	{
 		CalcLen = (_DspLen) ? _DspLen : _CodeLen;
		sprintf (DispKey, "%-.*s", CalcLen,_line_rec[lines].key);
		rv_pr(DispKey,2 + SR_X_POS,lines + 3 + SR_Y_POS,0);

		_OldPageNo = _PageNo;

		switch (c)
		{
		case	FN3:
			_draw_page(2);
			_RedrawScreen 	=	1;
			break;

		case FN15:
		case LEFT_KEY:
		case 8:
		case 9:
			_PageNo = (_PageNo == 0) ? max_lines / NO_SRCH_LINES : _PageNo - 1;
			lines = 0;
			break;

		case UP_KEY:
		case 11:
			if (lines == 0) 
			{
				_PageNo = (_PageNo == 0) ? max_lines / NO_SRCH_LINES : _PageNo - 1;
				lines = (_PageNo == _OldPageNo) ? _PageSize - 1 : 0;
			}
			else
				lines--;
			break;

		case DOWN_KEY:
		case 10:
			if (lines == _PageSize - 1) 
			{
				if ( !_DataComplete)
				{
					_PageNo++;
					return (EXIT_SUCCESS);
				}
				if (max_lines != P_SIZE)
					_PageNo = (_PageNo == max_lines / NO_SRCH_LINES) ? 0 : _PageNo + 1;
				lines = 0;
			}
			else
				lines++;
			break;

		case FN14:
		case RIGHT_KEY:
		case 12:
			if ( !_DataComplete)
			{
				_PageNo++;
				return(0);
			}

			if (max_lines != P_SIZE)
				_PageNo = (_PageNo == max_lines / NO_SRCH_LINES) ? 0 : _PageNo + 1;
			lines = 0;
			break;

		case FN1:
		case FN16:
		case '\r':
			sprintf(temp_str,"%-*.*s",
				_CodeLen,
				_CodeLen,
				(c == FN1) ? " " : _line_rec[lines].key);
			crsr_on();
			_DisplayExit	=	1;

			rtnVal = 0;
			if (c == FN1)
			{
				rtnVal = 1;
				_FN1Pressed = 1;
			}

			return (rtnVal);

		default:
			putchar(BELL);
			break;
		}

		if (_PageNo != _OldPageNo)
			_PageSize = _load_page(max_lines,lines);

 		CalcLen = (_DspLen) ? _DspLen : _CodeLen;
		sprintf (DispKey, "%-.*s", CalcLen,_line_rec[lines].key);
		rv_pr(DispKey,2 + SR_X_POS,lines + 3 + SR_Y_POS,1);
	}
	return(0);
}

/*===============================
| disp = 0 iff display box only	|
| disp = 1 iff display search	|
| disp = 2 iff display all		|
===============================*/
void
_draw_page
(
	int		disp
)
{
	register	int	i;
	int		box_width;

	box_width = (_DspLen) ? _DspLen : _CodeLen;
	box_width += _DescLen + 5;

	for (i = 0;(disp == 0 || disp == 2) && i < NO_SRCH_LINES + 6;i++)
	{
		move(SR_X_POS,i + SR_Y_POS);
		printf("%-*.*s",box_width + 2,box_width + 2," ");
	}

	if (disp == 0 || disp == 2)
	{
		box(0 + SR_X_POS,0 + SR_Y_POS,box_width + 2,NO_SRCH_LINES + 4);

		/*-----------------------
		| Print Column Headings	|
		-----------------------*/
		CalcLen = (_DspLen) ? _DspLen : _CodeLen;
		move(2 + SR_X_POS,1 + SR_Y_POS);
		printf("%-*.*s",
			CalcLen,
			CalcLen,
			_line_rec[NO_SRCH_LINES].key);
		move(CalcLen + 5 + SR_X_POS ,1 + SR_Y_POS);
		printf("%-*.*s",
			_DescLen,
			_DescLen,
			_line_rec[NO_SRCH_LINES].desc);

		move(SR_X_POS,2 + SR_Y_POS);
		PGCHAR(10);
		move(1 + SR_X_POS,2 + SR_Y_POS);
		line(box_width + 1);
		PGCHAR(11);

		move(SR_X_POS,NO_SRCH_LINES + 3 + SR_Y_POS);
		PGCHAR(10);
		move(1 + SR_X_POS,NO_SRCH_LINES + 3 + SR_Y_POS);
		line(box_width + 1);
		PGCHAR(11);

		move(((box_width - 20) / 2) + SR_X_POS,NO_SRCH_LINES + 4 + SR_Y_POS);
		printf("[NEXT] [PREV] [END]");
	}

	if (disp == 1 || disp == 2)
	{
		CalcLen = (_DspLen) ? _DspLen : _CodeLen;
		box(CalcLen + 3 + SR_X_POS,3 + SR_Y_POS,1,NO_SRCH_LINES - 2);
		move(CalcLen + 3 + SR_X_POS,2 + SR_Y_POS);
		PGCHAR(8);
		move(CalcLen + 3 + SR_X_POS,NO_SRCH_LINES + 3 + SR_Y_POS);
		PGCHAR(9);
	}
	fflush(stdout);
}

/*===============================
| Load one page from work file. |
===============================*/
int
_load_page
(
	int		max_lines,
	int		lines
)
{
	int	i = 0;

	if (max_lines < 0)
		return(0);

	_PageSize = (_PageNo == (max_lines / NO_SRCH_LINES)) 
									? (max_lines % NO_SRCH_LINES) + 1: NO_SRCH_LINES;

	if (_PageSize <= 0)
		return(0);

	for (i = 0;i < _PageSize && _load_line( _page_no[_PageNo] + i, i);i++);

	for (i = 0;i < _PageSize;i++)
	{
 		CalcLen = (_DspLen) ? _DspLen : _CodeLen;
		sprintf (DispKey, "%-.*s", CalcLen,_line_rec[i].key);
		rv_pr(DispKey,2 + SR_X_POS,i + 3 + SR_Y_POS,(i == lines) ? 1 : 0);

		CalcLen = (_DspLen) ? _DspLen : _CodeLen;
		rv_pr(_line_rec[i].desc,CalcLen + 5 + SR_X_POS,i + 3 + SR_Y_POS,0);
	}

	/*-------------------------------
	| Clear the rest of the screen	|
	-------------------------------*/
	for (i = _PageSize; i < NO_SRCH_LINES; i++)
	{
		CalcLen = (_DspLen) ? _DspLen : _CodeLen;
		move(2 + SR_X_POS,i + 3 + SR_Y_POS);
		printf("%*.*s",CalcLen,CalcLen," ");
		move(CalcLen + 5 + SR_X_POS,i + 3 + SR_Y_POS);
		printf("%*.*s",_DescLen,_DescLen," ");
	}
	return(_PageSize);
}

/*===============================
| Load one line from work file. |
===============================*/
int
_load_line
(
	int		Offset,
	int		line_no
)
{
	char	*lptr;
	char	*sptr;

	strcpy (SrWorkStr, SrchStore[ Offset ]);

	lptr = SrWorkStr;

	if (lptr == (char *) NULL)
		return(0);

	sptr	=	lptr;
	lptr += strlen(sptr) - 1;
		
	if (*lptr == '\n')
		*lptr = '\0';

	lptr = strchr(sptr,'|');
	*lptr++ = '\0';

	sprintf(_line_rec[line_no ].key,"%-.40s",sptr);
	sprintf(_line_rec[line_no].desc,"%-.80s",lptr);
	return(1);
}
