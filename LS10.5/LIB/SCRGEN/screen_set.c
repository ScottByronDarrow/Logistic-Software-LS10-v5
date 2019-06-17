#ident	"$Id: screen_set.c,v 5.1 2001/08/06 22:47:19 scott Exp $"
/*=====================================================================
|  Copyright (C) 1988 - 1993 Pinnacle Software Limited.               |
|=====================================================================|
|  Date Written  : (  .  .  )      | Author       : Unknown           |
|---------------------------------------------------------------------|
|  Date Modified : (26/05/1993)    | Modified by : Trevor van Bremen  |
|                : (13/04/1994)    | Modified by : Jonathan Chen      |
|                : (25/07/1994)    | Modified by : Jonathan Chen      |
|                : (03/08/1999)    | Modified by : Eumir Que Camara   |
|                                                                     |
|  Comments      :                                                    |
|  (26/05/1993)  : AMB 8738. Allow for MASK/LOWVAL/HIGHVAL in SCN/    |
|  (13/04/1994)  : Removed stupid strchr decls to compile on AIX      |
|  (25/07/1994)  : Checks for screen file in $PROG_PATH as well as    |
|                  $PSL_MENU_PATH                                     |
|  (03/08/1999)  : used ltrim() instead of lclip()  to avoid problems |
|                  associated with writing to constant memory.        |
-----------------------------------------------------------------------
	$Log: screen_set.c,v $
	Revision 5.1  2001/08/06 22:47:19  scott
	RELEASE 5.0
	
	Revision 5.0  2001/06/19 07:11:19  cha
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.0  2001/03/09 00:54:07  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 2.0  2000/07/15 07:34:18  gerry
	Forced Revision No. Start to 2.0 - Rel-15072000
	
	Revision 1.3  1999/10/07 00:26:22  jonc
	First pass at ANSI-fying source.
	Local functions are now tied tighter to the file itself.
	
=====================================================================*/
#include	<pslscr.h>

#ifdef	HAS_UNISTD_H
#include	<unistd.h>
#else
#define		F_OK	0				/* used by access (2) */
#endif	/*HAS_UNISTD_H*/

#define	MAX_SCN_LENGTH	256
#define	MAX_BOX_LINES	25

char	*scn_fprompt;
char	*scn_fmask;
char	*scn_flowval;
char	*scn_fhighval;
int	scn_fno;
int	scn_frow;
int	scn_fcol;
int	scn_ftype;

struct
{
	char	*_cmd;	/* Commend type.      		*/
	int	_value;	/* Screen field type.		*/
} commands[] =
{
	{"REQUIRED",	YES},	/* Required field			*/
	{"INPUT",		NO},	/* Input but not required.		*/
	{"DISPLAY",		NA},	/* Display only field.			*/
	{"NOEDIT",		NE},	/* No Edit but input.			*/
	{"NOINPUT",		NI},	/* No Input but edit.			*/
	{"HIDE",		ND},	/* No Display field, hidden field.	*/
	{"",			ND}		/* Null entry. 				*/
};

struct
{
	int	_scn;
	int	_row;
	int	_col;
	int	_width;
	int	_depth;
	int	_ebox;
} boxes[MAX_BOX_LINES];

struct
{
	int	_scn;
	int	_row;
	int	_col;
	int	_length;
} lines[MAX_BOX_LINES];


int	max_box = 0,
	max_line = 0;

/*
 *	Local functions
 */
static void	UpdateVars (struct var * vars);
static int	splat (char * line),
			get_ftype (const char * fnd_type);
static void	_load_boxes (char *sptr, int _ebox),
			_load_lines (char * sptr);

/*
 *	External interface
 */

/*========================================
| Process file name supplier by program. |
========================================*/
void
screen_set (
 const char * scn_rname,
 struct var	*vars)
{
	FILE	*fin;
	char	*wkptr,
			input_line[MAX_SCN_LENGTH + 1],
			file_name[61];
	char	*subdir = "SCN/";
	int		f_type;

	/*	Check for a screen file in the following places :

			1. Current directory
			2. PSL_MENU_PATH (user-specific directory)
			3. PROG_PATH (generic directory)

	*/
	if (access (strcat (strcpy (file_name, subdir), scn_rname), F_OK))
	{
		int		gotit = FALSE;
		char	*envdir = getenv ("PSL_MENU_PATH");

		if (envdir)
		{
			sprintf (file_name, "%s/%s%s", envdir, subdir, scn_rname);
			gotit = !access (file_name, F_OK);
		}

		/*
		 *	Check PROG_PATH
		 */
		if (!gotit)
		{
			envdir = getenv ("PROG_PATH");
			sprintf (file_name, "%s/BIN/%s%s", envdir, subdir, scn_rname);
			gotit = !access (file_name, F_OK);
		}

		if (!gotit)
		{
			UpdateVars (vars);
			return;
		}
	}

	if (!(fin = fopen (file_name, "r")))
	{
		UpdateVars (vars);
		return;
	}

	wkptr = fgets (input_line, MAX_SCN_LENGTH, fin);

	while (wkptr != (char *) 0)
	{
		/*------------------------------------------
		| Process boxed and lines and ignore rest. |
		------------------------------------------*/
		if (!strncmp (wkptr, "((", 2))
		{
			wkptr = fgets (input_line, MAX_SCN_LENGTH, fin);

			while (wkptr != (char *) 0 && strncmp (wkptr, "))", 2))
			{
				if (!strncmp (wkptr, "box(", 4))
					_load_boxes (wkptr, 0);

				if (!strncmp (wkptr, "ebox(", 5))
					_load_boxes (wkptr + 1, 1);

				if (!strncmp (wkptr, "line(", 5))
					_load_lines (wkptr);

				wkptr = fgets (input_line, MAX_SCN_LENGTH, fin);
			}
			break;
		}
		else
		{
			f_type = splat (wkptr);
			if (f_type)
			{
				if (strncmp (scn_fprompt, "LEAVE", 5))
					vars[scn_fno].prmpt = p_strsave (scn_fprompt);

				if (scn_ftype != 999)
					vars[scn_fno].required = scn_ftype;

				if (vars[scn_fno].stype == LIN)
				{
					if (scn_frow > 0)
						vars[scn_fno].row = scn_frow;

					if (scn_fcol > 0)
						vars[scn_fno].col = scn_fcol;
				}

				if (scn_fmask != (char *) 0)
					vars[scn_fno].mask = p_strsave (scn_fmask);

				if (scn_flowval != (char *) 0)
					vars[scn_fno].lowval = p_strsave (scn_flowval);

				if (scn_fhighval != (char *) 0)
					vars[scn_fno].highval = p_strsave (scn_fhighval);
			}
		}
		wkptr = fgets (input_line, MAX_SCN_LENGTH, fin);
	}
	fclose (fin);

	UpdateVars (vars);
}

static void
UpdateVars (
 struct var * vars)
{
	int		i			=	0,
			ml_p_Len	=	0,
			ml_c_Len	=	0;

	char	*ml_c_str,
			*ml_p_str;

	char	WrkComm	[256],
			WrkPrmpt[256];

	if (SYS_LANG < 0)
		return;

	for (i = 0; vars[i].scn != 0; i++)
	{
		ml_p_Len 	=	strlen (vars[i].prmpt);
		ml_c_Len	=	strlen (vars[i].comment);

		ml_c_str 	= 	p_strsave (ML (ltrim (vars[i].comment, WrkComm)));
		ml_p_str 	= 	p_strsave (ML (ltrim (vars[i].prmpt, WrkPrmpt)));

		if (ml_p_str !=	(char *)0 && strlen (ml_p_str))
		{
			sprintf (WrkPrmpt,  "%-*.*s", ml_p_Len, ml_p_Len, ml_p_str);
			vars [i].prmpt		=	p_strsave (WrkPrmpt);
		}
		if (ml_c_str !=	(char *)0 && strlen (ml_c_str))
		{
			sprintf (WrkComm,   "%-*.*s", ml_c_Len, ml_c_Len, ml_c_str);
			vars [i].comment	=	p_strsave (WrkComm);
		}
	}
}

/*===================================
| Split out every line and convert. |
===================================*/
static int
splat (
 char * line)
{
	char	*sptr = line,
		*tptr;
	int	indx = 0;

	*(sptr + strlen(line) - 1) = '\0';

	scn_fmask = (char *) 0;
	scn_flowval = (char *) 0;
	scn_fhighval = (char *) 0;

	/*-------------------------------
	| Not end of current line	|
	-------------------------------*/
	while (*sptr != '\n' && *sptr)
	{
		while (*sptr == '\t')
			sptr++;

		tptr = sptr;

		while (*tptr != '\n' && *tptr != '\t' && *tptr)
			tptr++;

		*tptr = '\0';

		/*-------------------------------
		| Decide what to do with data	|
		-------------------------------*/
		switch (indx)
		{
		/*-------------------------------
		| Field 1 is prompt.		|
		-------------------------------*/
		case	0:
			scn_fprompt = sptr;
			break;

		/*-------------------------------
		| Field 2 is label for compare	|
		-------------------------------*/
		case	1:
			scn_fno = label (sptr);
			if (scn_fno == -1)
				return (EXIT_SUCCESS);
			break;

		/*-------------------------------
		| Field 3 is new row.		|
		-------------------------------*/
		case	2:
			if (!strncmp (sptr, "LEAVE", 5))
				scn_frow = -1;
			else
				scn_frow = atoi (sptr);
			break;

		/*-------------------------------
		| Field 4 is new column.	|
		-------------------------------*/
		case	3:
			if (!strncmp (sptr, "LEAVE", 5))
				scn_fcol = -1;
			else
				scn_fcol = atoi (sptr);
			break;

		/*-------------------------------
		| Field 5 is new field_type.	|
		-------------------------------*/
		case	4:
			if (strncmp (sptr, "LEAVE", 5))
			{
				scn_ftype = get_ftype (sptr);
				if (scn_ftype == -999)
					return (EXIT_SUCCESS);
			}
			else
				scn_ftype = 999;
			break;

		/*-------------------------------
		| Field 6 is new mask.		|
		-------------------------------*/
		case	5:
			if (strncmp (sptr, "LEAVE", 5))
				scn_fmask = sptr;
			else
				scn_fmask = (char *) 0;
			break;

		/*-------------------------------
		| Field 7 is new lowval.	|
		-------------------------------*/
		case	6:
			if (strncmp (sptr, "LEAVE", 5))
				scn_flowval = sptr;
			else
				scn_flowval = (char *) 0;
			break;

		/*-------------------------------
		| Field 8 is new highval.	|
		-------------------------------*/
		case	7:
			if (strncmp (sptr, "LEAVE", 5))
				scn_fhighval = sptr;
			else
				scn_fhighval = (char *) 0;
			break;

		/*-------------------------------
		| Fields 9 onward are illegal.	|
		-------------------------------*/
		default:
			return (EXIT_SUCCESS);
		}

		sptr = tptr + 1;
		indx++;
	}
	return (EXIT_FAILURE);
}
/*====================================
| Get SCRGEN compatable field types. |
====================================*/
static int
get_ftype (
 const char * fnd_type)
{
	int	cmd;

	for (cmd = 0; strlen (commands [cmd]._cmd); cmd++)
	{
		if (!strncmp (commands [cmd]._cmd, fnd_type,
				strlen (commands [cmd]._cmd)))
			return (commands[cmd]._value);
	}
	return (-999);
}

/*==========================================
| Routine to print stored boxes and lines. |
==========================================*/
void
pr_box_lines (
 int scn)
{
	int	i;

	for (i = 0; i < max_box; i++)
	{
		if (boxes[i]._scn == scn)
		{
			if (boxes[i]._ebox )
				cl_box (boxes[i]._col, boxes[i]._row, boxes[i]._width, boxes[i]._depth);
			else
				box (boxes[i]._col, boxes[i]._row, boxes[i]._width, boxes[i]._depth);
		}
	}

	for (i = 0; i < max_line; i++)
	{
		if (lines[i]._scn == scn)
		{
			move (lines[i]._col, lines[i]._row);
			line (lines[i]._length);
		}
	}
}

/*============================
| Load information re boxes. |
============================*/
static void
_load_boxes (
 char *sptr,
 int _ebox)
{
	char	*tptr;

	if (max_box == MAX_BOX_LINES)
		return;

	boxes[max_box]._scn = atoi(sptr + 4);
	boxes[max_box]._ebox = _ebox;

	tptr = strchr(sptr + 4,',');
	if (tptr != (char *)0)
	{
		boxes[max_box]._col = atoi(tptr + 1);
		sptr = tptr + 1;

		tptr = strchr(sptr,',');
		if (tptr != (char *)0)
		{
			boxes[max_box]._row = atoi(tptr + 1);
			sptr = tptr + 1;

			tptr = strchr(sptr,',');
			if (tptr != (char *)0)
			{
				boxes[max_box]._width = atoi(tptr + 1);
				sptr = tptr + 1;

				tptr = strchr(sptr,',');
				if (tptr != (char *)0)
				{
					boxes[max_box]._depth = atoi(tptr + 1);
					max_box++;
				}
			}
		}
	}
}

/*============================
| Load information re lines. |
============================*/
static void
_load_lines (
 char * sptr)
{
	char	*tptr;

	if ( max_line == MAX_BOX_LINES )
		return;

	lines[max_line]._scn = atoi(sptr + 5);

	tptr = strchr(sptr + 5,',');
	if (tptr != (char *)0)
	{
		lines[max_line]._col = atoi(tptr + 1);
		sptr = tptr + 1;

		tptr = strchr(sptr,',');
		if (tptr != (char *)0)
		{
			lines[max_line]._row = atoi(tptr + 1);
			sptr = tptr + 1;

			tptr = strchr(sptr,',');
			if (tptr != (char *)0)
			{
				lines[max_line]._length = atoi(tptr + 1);
				max_line++;
			}
		}
	}
}
