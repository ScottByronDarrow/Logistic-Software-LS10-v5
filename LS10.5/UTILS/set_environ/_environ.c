/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: _environ.c,v 5.5 2002/07/24 08:39:35 scott Exp $
|  Program Name  : (set_environ.c)
|  Program Desc  : (Environment Variable Maintenance)
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 12/05/88         |
|---------------------------------------------------------------------|
| $Log: _environ.c,v $
| Revision 5.5  2002/07/24 08:39:35  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.4  2002/07/03 04:30:13  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.3  2001/08/09 09:27:37  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:58:59  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:20:07  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _environ.c,v $",
		*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/set_environ/_environ.c,v 5.5 2002/07/24 08:39:35 scott Exp $";

#define	MAXLINES	1000
#define	MAXWIDTH	140
#include 	<pslscr.h>
#include	<license2.h>
#include	<pinn_env.h>
#include	<ml_std_mess.h>
#include	<ml_utils_mess.h>
#include 	<arralloc.h>

struct	DES_REC	des_rec;
struct	LIC_REC	lic_rec;

PinnEnv	envRec;

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode 	[sizeof envRec.env_name];
	char	envName		[sizeof envRec.env_name];
	char	envValue	[sizeof envRec.env_value];
	char	envDesc		[sizeof envRec.env_desc];
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

int		SortFunc			(const	void *,	const void *);

extern	int		envMaintOption;

struct	storeRec {
	char	vars [sizeof envRec.env_name];
} store [MAXLINES];

struct	{
	char	dummy [11];
} local_rec;


char	filename [2] [BUFSIZ];

static	struct	var	vars []	={	

	{1, TAB, "env_name", MAXLINES, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUU", "          ", 
		" ", " ", " Variable Name ", " ", 
		YES, NO, JUSTLEFT, "", "", envRec.env_name}, 
	{1, TAB, "env_value", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "        Variable Value        ", " ", 
		YES, NO, JUSTLEFT, "", "", envRec.env_value},
	{1, TAB, "env_desc", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "               V a r i a b l e    D e s c r i p t i o n               ", " ", 
		YES, NO, JUSTLEFT, "", "", envRec.env_desc},
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*
 * Function Declarations 
 */
int  	spec_valid 		(int);
int  	heading 		(int);
void 	LoadEnviron 	(void);
void 	Update 			(void);

/*
 * Main Processing Routine. 
 */
int
main (
	int 	argc,
	char 	*argv [])
{
	char	*sptr = getenv ("PROG_PATH");
	char	*vptr = getenv ("PSL_ENV_NAME");

	envMaintOption	=	TRUE;
	SETUP_SCR (vars);


	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();

	ser_msg (lc_check (&des_rec, &lic_rec), &lic_rec, TRUE);

	set_masks ();			/*  setup print using masks	*/
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (1, store, sizeof (struct storeRec));
#endif
	init_vars (1);			/*  set default values		*/
	swide ();

	if (vptr)
	{
		strcpy (filename [0],vptr);
		sprintf (filename [1],"%s.o",vptr);
	}
	else
	{
		if (sptr == (char *) 0)
		{
			strcpy (filename [0],"/usr/ver9.10/BIN/LOGISTIC");
			strcpy (filename [1],"/usr/ver9.10/BIN/LOGISTIC.o");
		}
		else
		{
			sprintf (filename [0],"%s/BIN/LOGISTIC",sptr);
			sprintf (filename [1],"%s/BIN/LOGISTIC.o",sptr);
		}
	}

	/*
	 * Beginning of input control loop . 
	 */
	while (prog_exit == 0)
	{
		/*
		 * Reset Control Flags
		 */
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_vars (1);

		LoadEnviron ();
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		prog_exit = TRUE;

		Update ();
	}
	clear ();
	rset_tty ();
	snorm ();
	crsr_on ();
	return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	/*
	 * environment variable name
	 */
	if (LCHECK ("env_name"))
	{
		if (dflt_used)
		{
			if (lcount [1] < 1)
				return (EXIT_SUCCESS);

			lcount [1]--;
			for (i = line_cnt;line_cnt < lcount [1];line_cnt++)
			{
				getval (line_cnt + 1);
				putval (line_cnt);
				strcpy (store [line_cnt].vars,store [line_cnt + 1].vars);
				if (line_cnt / TABLINES == this_page)
					line_display ();
			}
			if (line_cnt / TABLINES == this_page)
				blank_display ();

			line_cnt = i;
			getval (line_cnt);
		}

		for (i = 0;i < lcount [1];i++)
		{
			if (!strcmp (store [i].vars,envRec.env_name) && i != line_cnt)
			{
				print_mess (ML (mlUtilsMess062));
				return (EXIT_FAILURE);
			}
		}
		strcpy (store [line_cnt].vars, envRec.env_name);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
LoadEnviron (void)
{
	int		env_fd = open_env ();
	int		i;

	RF_REWIND (env_fd);

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	lcount [1] = 0;

	cc = RF_READ (env_fd, (char *) &envRec);
	while (!cc)
	{
		/*
		 * Check the array size before adding new element.
		 */
		if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
			sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

		if (envRec.env_desc [strlen (envRec.env_desc) - 1] == '\n')
			envRec.env_desc [strlen (envRec.env_desc) - 1] = ' ';

		strcpy (sortRec [sortCnt].sortCode, envRec.env_name);
		strcpy (sortRec [sortCnt].envName, envRec.env_name);
		strcpy (sortRec [sortCnt].envValue, envRec.env_value);
		strcpy (sortRec [sortCnt].envDesc, envRec.env_desc);
		/*
		 * Increment array counter.
		 */
		sortCnt++;
		cc = RF_READ (env_fd, (char *) &envRec);
	}
	close_env (env_fd);

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);

	for (i = 0; i < sortCnt; i++)
	{
		sprintf (envRec.env_name,	"%-15.15s", sortRec [i].envName);
		sprintf (envRec.env_value,	"%-30.30s",	sortRec [i].envValue);
		sprintf (envRec.env_desc,	"%-70.70s",	sortRec [i].envDesc);
		strcpy (store [lcount [1]].vars, envRec.env_name);
		putval (lcount [1]++);
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
}

void
Update (void)
{
	int	c;

	crsr_on ();
	c = prmptmsg (ML (mlUtilsMess063),"YyNn",0,2);

	clear ();
	
	if (c == 'N' || c == 'n')
		return;

	if (fork () == 0)
	{
		if (rename (filename [0], filename [1]))
		{
			sprintf (err_str,"Error in mv %s %s",filename [0],filename [1]);
			sys_err (err_str,errno,PNAME);
		}
		prog_exit = 1;
		return;
	}
	else
		wait ((int *) 0);

	umask (0);
	for (line_cnt = 0;line_cnt < lcount [1];line_cnt++)
	{
		getval (line_cnt);
		put_env (envRec.env_name,envRec.env_value,envRec.env_desc);
	}
}


int 
SortFunc (
	const void *a1, 
	const void *b1)
{
	int	result;
	const struct SortStruct a = * (const struct SortStruct *) a1;
	const struct SortStruct b = * (const struct SortStruct *) b1;

	result = strcmp (a.sortCode, b.sortCode);

	return (result);
}
/*
 * Display Screen Heading  
 */
int
heading (
	int		scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	swide ();
	clear ();
	rv_pr (ML (mlUtilsMess027),49,0,1);
	line_at (1,0,132);
	print_at (4,10,ML (mlUtilsMess004), filename [0]);

	line_at (20,0,132);
	line_at (22,0,132);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}
