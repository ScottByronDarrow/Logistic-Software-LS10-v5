/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: bm_uom_mnt.c,v 5.7 2002/06/28 07:41:46 kaarlo Exp $
|  Program Name  : (bm_uom_mnt.c)
|  Program Desc  : (Maintain Unit Of Measure)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 14/08/91         |
|---------------------------------------------------------------------|
| $Log: bm_uom_mnt.c,v $
| Revision 5.7  2002/06/28 07:41:46  kaarlo
| S/C 3984. Initialize prog_exit in AddUFuct to fix problem.
|
| Revision 5.6  2002/05/09 04:16:56  scott
| Updated to convert to app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: bm_uom_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/BM/bm_uom_mnt/bm_uom_mnt.c,v 5.7 2002/06/28 07:41:46 kaarlo Exp $";

#include <pslscr.h>
#include <tabdisp.h>
#include <hot_keys.h>
#include <ml_std_mess.h>
#include <ml_bm_mess.h>
#include <arralloc.h>

	char	inp_type [2];

	/*
	 * Special fields and flags
	 */
   	int	new_code = 0;

#include	"schema"

struct commRecord	comm_rec;
struct inumRecord	inum_rec;
struct inumRecord	wkum1_rec;
struct inumRecord	wkum2_rec;
//struct inumRecord	inum_rec;

char	*wkum	=	"wkum";

int		exit_head;
int		exit_mod;
int		AddGroup = FALSE;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	curr_grp [21];
	char	grp_name [21];
	int		curr_num;
	char	uom [5];
	char	udesc [41];
	float	cnv_fct;
} local_rec;

static int AddFunc 	 	(int, KEY_TAB *);
static int ModFunc 	 	(int, KEY_TAB *);
static int ExitFunc 	(int, KEY_TAB *);
static int ExitUFunc 	(int, KEY_TAB *);
static int AddUFuct 	(int, KEY_TAB *);
static int ModUFuct 	(int, KEY_TAB *);

#ifdef	GVISION
static KEY_TAB head_keys [] =
{
    { " ADD NEW GROUP ",	'A', AddFunc,
	"Add A New Group Classification",				"E" },
    { " MODIFY GROUP ",		'M', ModFunc,
	"Modify A Group Of Units",					"E" },
    { "",			FN16, ExitFunc,
	"",								"E" },
    END_KEYS
};

static KEY_TAB uom_keys [] =
{
    { " ADD NEW UNIT ",		'A', AddUFuct,
	"Add Units To A Group ",					"E" },
    { " MODIFY UNIT ",		'M', ModUFuct,
	"Modify The Units In A Group",					"E" },
    { "",			FN1, ExitUFunc,
	"",								"E" },
    { "",			FN16, ExitUFunc,
	"",								"E" },
    END_KEYS
};
#else
static KEY_TAB head_keys [] =
{
    { " [A]DD NEW GROUP",	'A', AddFunc,
	"Add A New Group Classification",				"E" },
    { " [M]ODIFY GROUP",		'M', ModFunc,
	"Modify A Group Of Units",					"E" },
    { "",			FN16, ExitFunc,
	"",								"E" },
    END_KEYS
};

static KEY_TAB uom_keys [] =
{
    { " [A]DD UNIT",		'A', AddUFuct,
	"Add Units To A Group ",					"E" },
    { " [M]ODIFY UNIT",		'M', ModUFuct,
	"Modify The Units In A Group",					"E" },
    { "",			FN1, ExitUFunc,
	"",								"E" },
    { "",			FN16, ExitUFunc,
	"",								"E" },
    END_KEYS
};
#endif

static	struct	var	vars [] =
{
	{1, LIN, "grp_name",	16, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Group Classification: ", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.grp_name},
	{1, LIN, "uom",	17, 10, CHARTYPE,
		"AAAA", "          ",
		" ", "", " UOM: ", "",
		YES, NO,  JUSTLEFT, "", "", local_rec.uom},
	{1, LIN, "cnv_fct",	17, 55, FLOATTYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", "", " Conversion Factor:", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.cnv_fct},
	{1, LIN, "udesc",	18, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Description:", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.udesc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

int	InumSort	(const void * , const void *);

/*---------------------------------------------------------------------------
|   Structure for dynamic array,  for the sorting of invoices using qsort   |
---------------------------------------------------------------------------*/
struct InumStructure
{
    char    sortField [5];
    char    uom [5];
    char    description [41];
    float   cnv_fct;
	long    hhum_hash;
}   *uom;

int inumCnt = 0;
DArray uom_d;


/*
 * Local Function Prototypes.
 */
int 	Process 		(void);
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ClearFlds 		(void);
int 	spec_valid 		(int);
int 	heading 		(int);

int
main (
	int		argc,
	char	*argv [])
{
	/*
	 * Setup required parameters. 
	 */
	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();


	set_help (FN6, "FN6 - HELP");
	Process ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

int
Process (void)
{
	int	data_fnd;
	int	first_time;
	char	curr_grp [21];

	heading (0);

	exit_head = FALSE;
	while (!exit_head)
	{
		first_time = TRUE;
		data_fnd = FALSE;
		tab_open ("uom_grp", head_keys, 2, 14, 6, FALSE);
		tab_add ("uom_grp", "# %-30.30s %15.15s    ", " Group Classification ", 
			"No. In Group");
	
		sprintf (inum_rec.uom_group, "%-20.20s", " ");
		local_rec.curr_num = 0;
		inum_rec.hhum_hash = 0L;
	
		cc = find_rec (inum,&inum_rec,GTEQ,"r");
		while (!cc)
		{
			if (first_time)
			{
				sprintf (curr_grp, "%-20.20s", inum_rec.uom_group);
				first_time = FALSE;
			}
	
			data_fnd = TRUE;
			if (strcmp (inum_rec.uom_group, curr_grp))
			{
				tab_add ("uom_grp", 
					" %-30.30s      %4d ", 
					curr_grp,
					local_rec.curr_num);
				local_rec.curr_num = 0;
			}
	
			sprintf (curr_grp, "%-20.20s", inum_rec.uom_group);
			local_rec.curr_num++;
	
			cc = find_rec (inum,&inum_rec,NEXT,"r");
		}
	
		if (data_fnd)
		{
			tab_add ("uom_grp", " %-30.30s      %4d ",curr_grp,local_rec.curr_num);
		}

		tab_scan ("uom_grp");

		tab_close ("uom_grp", TRUE);
	}

	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

    read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_id_no");
	abc_alias (wkum, inum);
	open_rec (wkum, inum_list, INUM_NO_FIELDS, "inum_uom");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (inum);
	abc_fclose (wkum);
	abc_dbclose ("data");
}

int
spec_valid (
 int                field)
{
	if (LCHECK ("grp_name"))
	{
		if (FLD ("grp_name") == ND)
			return (EXIT_SUCCESS);

		if (strlen (clip (local_rec.grp_name)) == 0)
		{
			print_mess (ML (mlStdMess149));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (inum_rec.uom_group, "%-20.20s", local_rec.grp_name);
		inum_rec.hhum_hash = 0L;
		cc = find_rec (inum, &inum_rec, GTEQ, "r");
		if (!cc && !strcmp (inum_rec.uom_group, local_rec.grp_name))
		{
			print_mess (ML (mlStdMess150));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("uom"))
	{
		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		sprintf (wkum2_rec.uom, "%-4.4s", local_rec.uom);
		cc = find_rec (wkum, &wkum2_rec, COMPARISON, "r");
		if (!cc)
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

int
heading (
	int	scn)
{
	if (scn != cur_screen && scn != 0)
		scn_set (scn);
	if (scn == 0)
	{
		clear ();
		rv_pr (ML (mlBmMess046), 25,0,1);
		line_at (1,0,80);
		line_at (20,0,80);
		line_at (22,0,80);
		strcpy (err_str, ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.co_no,comm_rec.co_name);
	}

	if (scn == 1)
	{
		if (AddGroup)
			box (0,15,80,3);
		else
			box (0,16,80,2);
	}

	line_cnt = 0;

	if (scn != 0)
		scn_write (scn);

    return (EXIT_SUCCESS);
}

static int
AddFunc (
 int                c,
 KEY_TAB*           psUnused)
{
	int	i;
	ClearFlds ();
	AddGroup = TRUE;
	FLD ("grp_name") = YES;
	scn_set (1);
	restart = FALSE;
	prog_exit = FALSE;
	heading (1);
	rv_pr (ML (mlBmMess047), 25, 15, 1);
	entry (1);

	if (restart || prog_exit)
	{
		for (i = 15; i < 20; i++)
		{
			move (0,i);
			cl_line ();
		}
	
		return (c);
	}

	heading (1);
	rv_pr (ML (mlBmMess047), 25, 15, 1);
	scn_display (1);
	edit (1);

	for (i = 15; i < 20; i++)
	{
		move (0,i);
		cl_line ();
	}

	if (restart)
		return (c);

	tab_add ("uom_grp", 
             " %-30.30s      %4d ", 
             local_rec.grp_name,
             1);

	sprintf (inum_rec.uom_group, "%-20.20s", local_rec.grp_name);
	sprintf (inum_rec.uom, "%-4.4s", local_rec.uom);
	sprintf (inum_rec.desc, "%-40.40s", local_rec.udesc);
	inum_rec.cnv_fct = local_rec.cnv_fct;
	cc = abc_add (inum, &inum_rec);
	if (cc)
		file_err (cc, inum, "DBADD");

	return (FN16);
}

static int
ModFunc (
 int                c,
 KEY_TAB*           psUnused)
{
	char	get_line [100];
	char	tmp_line [100];
	int	rtn_val = c;
	int	i;

	tab_get ("uom_grp", get_line, CURRENT, 0);
	
	sprintf (local_rec.curr_grp, "%-20.20s", &get_line [1]);
	local_rec.curr_num = atoi (&get_line [38]);

	exit_mod = FALSE;
	while (!exit_mod)
	{
		tab_open ("uom", uom_keys, 7, 4, 4, FALSE);
		tab_add ("uom", 
			"# %-5.5s %-40.40s %-20.20s ", 
			" UOM ", 
			" UOM Description ", 
			" Conversion Factor");
	
   		ArrAlloc (&uom_d, &uom, sizeof (struct InumStructure), 1000);

	   	inumCnt = 0;

		sprintf (inum_rec.uom_group, "%-20.20s", local_rec.curr_grp);
		inum_rec.hhum_hash = 0L;
	
		cc = find_rec (inum,&inum_rec,GTEQ,"r");
		while (!cc && !strcmp (inum_rec.uom_group,local_rec.curr_grp))
		{
			sprintf (uom [inumCnt].sortField, "%-4.4s", inum_rec.uom);
			uom [inumCnt].hhum_hash = inum_rec.hhum_hash;
			strcpy (uom [inumCnt].uom , inum_rec.uom);
			strcpy (uom [inumCnt].description , inum_rec.desc);
			uom [inumCnt].cnv_fct = inum_rec.cnv_fct;
			inumCnt++;		
			cc = find_rec (inum,&inum_rec,NEXT,"r");
		}

		qsort (	uom,
				inumCnt, 
				sizeof (struct InumStructure),
				InumSort);

		for (i = 0; i < inumCnt; i++)
		{
			tab_add ("uom", 
				"  %-4.4s  %-40.40s    %14.6f ", 
				uom [i].uom,
				uom [i].description,
				uom [i].cnv_fct);
		}

		tab_scan ("uom");
		if (exit_mod)
			tab_clear ("uom");

		if (!exit_mod)
			tab_close ("uom", TRUE);
	}

	for (i = 0; i <= local_rec.curr_num; i++)
	{
		tab_get ("uom", tmp_line, EQUAL, i);
		sprintf (wkum2_rec.uom, "%-4.4s", &tmp_line [2]);
		cc = find_rec (wkum, &wkum2_rec, COMPARISON, "u");
		if (cc)
			continue;
		
		if (tagged (tmp_line))
		{
			local_rec.curr_num--;
			cc = abc_delete (wkum);
			if (cc)
				file_err (cc, wkum, "DBADD");
		}
	}

	tab_close ("uom", TRUE);

	if (local_rec.curr_num <= 0)
		rtn_val = FN16;
	else
	{
		tab_update
		(
			"uom_grp",
			" %-30.30s      %4d ", 
			local_rec.curr_grp,
			local_rec.curr_num
		);
	}

	tab_display ("uom_grp", TRUE);
	redraw_keys ("uom_grp");

	return (rtn_val);
}

int
InumSort (
	const	void *a1,
	const	void *b1)
{
    int result;
	const struct InumStructure a = * (const struct InumStructure *) a1;
	const struct InumStructure b = * (const struct InumStructure *) b1;
								
	result = strcmp (a.sortField, b.sortField);

	return (result);   
}         
														 
static int
ExitFunc (
 int                c,
 KEY_TAB*           psUnused)
{
	exit_head = TRUE;
	return (c);
}

static int
ExitUFunc (
 int                c,
 KEY_TAB*           psUnused)
{
	exit_mod = TRUE;
	return (c);
}

static int
AddUFuct (
 int                c,
 KEY_TAB*           psUnused)
{
	int	i;
	ClearFlds ();
	AddGroup = FALSE;
	FLD ("grp_name") = ND;
	restart = FALSE;
	prog_exit = FALSE;
	scn_set (1);
	heading (1);
	rv_pr (ML (mlBmMess048), 30, 16, 1);
	entry (1);
	heading (1);
	rv_pr (ML (mlBmMess048), 30, 16, 1);
	scn_display (1);
	edit (1);

	for (i = 16; i < 20; i++)
	{
		move (0,i);
		cl_line ();
	}

	if (restart || prog_exit)
		return (c);

	tab_add ("uom", 
		"  %-4.4s  %-40.40s    %14.6f ", 
		local_rec.uom,
		local_rec.udesc,
		local_rec.cnv_fct);

	sprintf (inum_rec.uom_group, "%-20.20s", local_rec.curr_grp);
	sprintf (inum_rec.uom, "%-4.4s", local_rec.uom);
	sprintf (inum_rec.desc, "%-40.40s", local_rec.udesc);
	inum_rec.cnv_fct = local_rec.cnv_fct;
	cc = abc_add (inum, &inum_rec);
	if (cc)
		file_err (cc, inum, "DBADD");

	local_rec.curr_num++;

	return (FN16);
}

static int
ModUFuct (
 int                c,
 KEY_TAB*           psUnused)
{
	int	i;
	char	get_line [100];
	AddGroup = FALSE;
	
	tab_get ("uom", get_line, CURRENT, 0);
	ClearFlds ();
	if (tagged (get_line))
	{
		print_mess (ML (mlBmMess045));
		sleep (sleepTime);
		clear_mess ();
		return (c);
	}

	FLD ("grp_name") = ND;	
	sprintf (local_rec.uom, "%-4.4s", &get_line [2]);
	sprintf (local_rec.udesc, "%-40.40s", &get_line [8]);
	local_rec.cnv_fct = (float) atof (&get_line [52]);

	sprintf (wkum1_rec.uom, "%-4.4s", local_rec.uom);
	cc = find_rec (wkum, &wkum1_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, wkum, "DBADD");

	FLD ("uom") = NA;

	restart = FALSE;
	heading (1);
	rv_pr (ML (mlBmMess049), 35, 16, 1);
	scn_display (1);
	edit (1);

	FLD ("uom") = YES;

	for (i = 16; i < 20; i++)
	{
		move (0,i);
		cl_line ();
	}

	if (restart)
		return (c);

	tab_update ("uom",
		"  %-4.4s  %-40.40s    %14.6f ", 
		local_rec.uom,
		local_rec.udesc,
		local_rec.cnv_fct);

	sprintf (wkum1_rec.uom, "%-4.4s", local_rec.uom);
	sprintf (wkum1_rec.desc, "%-40.40s", local_rec.udesc);
	wkum1_rec.cnv_fct = local_rec.cnv_fct;
	cc = abc_update (wkum, &wkum1_rec);
	if (cc)
		file_err (cc, wkum, "DBUPDATE");

	return (c);
}


void
ClearFlds (void)
{
	sprintf (local_rec.grp_name, "%20s", " ");
	sprintf (local_rec.uom, "%5s", " ");
	sprintf (local_rec.udesc, "%40s"," ");
}

