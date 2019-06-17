/*=====================================================================
|  Copyright (C) 1986 - 1997 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( ml_input.c   )                                   |
|  Program Desc  : ( Multi-Lingual entry program.                   ) |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (09/09/1997)    | Author      :  Scott B Darrow.   |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ml_input.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/ml_input/ml_input.c,v 5.2 2001/08/09 09:27:03 scott Exp $";

#define	MAXSCNS		2
#define	MAXLINES	20000
#define	MAXWIDTH	136
#define	TABLINES	14
#define	SR			store[line_cnt]
#include <pslscr.h>
#include <dsp_process.h>

	char	*LangSpace	=	"                                                                                                                                  ";

	int		IgnorePipeFields = FALSE;

	/*====================================+
	 | Screen generator field label text. |
	 +====================================*/
#define	MLDB_NO_FIELDS	9

	struct dbview	mldb_list [MLDB_NO_FIELDS] =
	{
		{"mldb_lu_prmpt"},
		{"mldb_text1"},
		{"mldb_text2"},
		{"mldb_text3"},
		{"mldb_text4"},
		{"mldb_text5"},
		{"mldb_pname"},
		{"mldb_org_len"},
		{"mldb_hide"}
	};

	struct tag_mldbRecord
	{
		char	lu_prmpt [121];
		char	text[5] [131];
		char	pname[21];
		int		org_len;
		int		hide;
	}	mldb_rec;

	/*====================================+
	 | Screen generator field label text. |
	 +====================================*/
#define	MLCF_NO_FIELDS	2

	struct dbview	mlcf_list [MLCF_NO_FIELDS] =
	{
		{"mlcf_lang_no"},
		{"mlcf_lang_desc"}
	};

	struct tag_mlcfRecord
	{
		int		lang_no;
		char	lang_desc [41];
	}	mlcf_rec;

char	*PercentToThilde (char *);
char	*ThildeToPercent (char *);
int 	CountThilde (char	*);

char    *data   =  "data",
        *mldb   =  "mldb",
        *mlcf   =  "mlcf";

	struct	{
		char	label[121];
		char	english[131];
		char	pname[21];
		int		org_len;
		int		hide;
	} store[MAXLINES];

struct {
	char	text[131];
	int		lang_no;
	char	start_no[2],
			end_no[2];
	char	dummy[11];
} local_rec;

static  struct  var vars[] =
{
    {1, LIN, "lang_no", 2, 23, INTTYPE,
        "N", "          ",
        " ", " ", "Enter Language Number", " ",
        NE, NO, JUSTLEFT, "", "", (char *)&local_rec.lang_no},
    {1, LIN, "lang_desc", 2, 40, CHARTYPE,
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
        " ", " ", "", " ",
        NA, NO, JUSTLEFT, "", "", mlcf_rec.lang_desc},
    {1, LIN, "start", 3, 23, CHARTYPE,
        "U", "          ",
        " ", " ", "Start Range (A-Z)", " ",
        YES, NO, JUSTLEFT, "", "", local_rec.start_no},
    {1, LIN, "end", 3, 80, CHARTYPE,
        "U", "          ",
        " ", "~", "End Range (A-Z)", " ",
        YES, NO, JUSTLEFT, "", "", local_rec.end_no},
    {2, TAB, "text1", MAXLINES, 0, CHARTYPE,
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
        " ", " ", "                                                                                                                        ", 
		"Enter the text for this label.",
        YES, NO,  JUSTLEFT, "" , "", local_rec.text},
    {0, LIN, "",  0, 0, CHARTYPE,
        "A", "          ",
        " ", "", "dummy", " ",
        YES, NO, JUSTRIGHT, "" , "", local_rec.dummy},
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void tab_other (int line_no);
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
int heading (int scn);
void LoadRecords (void);
void update (void);
void srch_mlcf (char *key_val);

/*==========================
| Main processing routine. |
==========================*/
int
main (
 int                argc,
 char*              argv[])
{
	char	*sptr;
	
	tab_row	= 6;
	tab_col = 0;

	SETUP_SCR(vars);
	init_scr();
	set_tty();
	set_masks();

	sptr = strrchr (argv[0], '/');
	if (sptr == (char *) 0)
		sptr = argv[0];
	else
		sptr++;
	if (!strncmp(sptr, "ml_input",8))
		IgnorePipeFields = TRUE;

	SYS_LANG = lang_select();

	OpenDB();
	swide();

	prog_exit = 0;
	while (prog_exit == 0)
	{
		search_ok = 1;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		lcount[2] = 0;
		init_vars(1);	
		init_vars(2);	

		heading(1);
		scn_display(1);
		entry(1);
		if (prog_exit || restart)
			continue;

		scn_write(1);
		scn_display(1);

		if (!restart)
			LoadRecords ();

		scn_write(2);
		scn_display(2);

		edit(2);

		if (restart)
			continue;

		update ();

	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
tab_other (
 int                line_no)
{
	char	OrgLenText[21];

	strcpy (OrgLenText, ML ("Original Length"));
	
	centre_at (5, 130, "%R %s (%s) %s (%3d)", ML("ENGLISH TEXT TO CONVERT FROM"),store [line_no].pname, OrgLenText,store [line_no].org_len);
	centre_at (6, 130, "%R %100.100s ", store [line_no].english);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB (void)
{
    abc_dbopen (data);
    open_rec (mldb, mldb_list, MLDB_NO_FIELDS, "mldb_lu_prmpt");
    open_rec (mlcf, mlcf_list, MLCF_NO_FIELDS, "mlcf_lang_no");
}

void
CloseDB (void)
{
    abc_fclose (mldb);
    abc_fclose (mlcf);
    abc_dbclose(data);
}

int
spec_valid (
 int                field)
{
	if (LCHECK("lang_no"))
	{
		if (SRCH_KEY)
		{
			srch_mlcf (temp_str);
			return (EXIT_SUCCESS);
		}
			
		mlcf_rec.lang_no	=	local_rec.lang_no;
		cc = find_rec ("mlcf", &mlcf_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML ("Language is not on file"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("lang_desc");


		return(0);
	}
	if (LCHECK("text1"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.text, SR.english);
			DSP_FLD ("text1");
		}
		if (CountThilde (local_rec.text) != CountThilde (SR.english))
		{
			errmess (ML ("Number of ~ needs to match."));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
heading (
 int                scn)
{
	if (restart)
		return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set(scn);

	clear();
	switch(scn)
	{
	case	1:
		scn_set(2);
		scn_display(2);

		box(0,1,132,2);
		box(0,4,132,17);
		break;

	case	2:
		scn_set(1);
		scn_write(1);
		scn_display(1);
		box(0,1,132,2);
		box(0,4,132,17);
	}
	print_at (0,40, "%R %s", ML ("Multi-Lingual conversion entry screen"));
	print_at (22,40, "%R %s", ML ("NOTE THAT % has been replaced by ~ for entry"));
	scn_write(scn);
    return (EXIT_SUCCESS);
}

void
LoadRecords (void)
{
	int		i	=	0;

	scn_set(2);

	lcount[2] = 0;

	sprintf (mldb_rec.lu_prmpt, "%-120.120s", local_rec.start_no);

	cc = find_rec(mldb, &mldb_rec, GTEQ, "r");
	while (!cc)
	{
		if ((IgnorePipeFields && strchr (mldb_rec.text[0], '|')) || mldb_rec.hide)
		{
			cc = find_rec(mldb, &mldb_rec, NEXT, "r");
			continue;
		}
		if (toupper (mldb_rec.lu_prmpt[0]) > toupper (local_rec.end_no[0]))
		{
			cc = find_rec(mldb, &mldb_rec, NEXT, "r");
			continue;
		}
		if (toupper (mldb_rec.lu_prmpt[0]) < toupper (local_rec.start_no[0]))
		{
			cc = find_rec(mldb, &mldb_rec, NEXT, "r");
			continue;
		}
		sprintf (err_str, "%06d", i++);
		dsp_process ("Reading Lines", err_str);

		if (!strcmp (mldb_rec.text [ local_rec.lang_no - 1], LangSpace))
			strcpy (local_rec.text, mldb_rec.text [0]);
		else
			strcpy (local_rec.text, mldb_rec.text [local_rec.lang_no - 1]);

		if (!strcmp (local_rec.text, LangSpace))
		{
			cc = find_rec(mldb, &mldb_rec, NEXT, "r");
			continue;
		}
		strcpy (local_rec.text, PercentToThilde (local_rec.text));
		strcpy (store[ lcount[2] ].english, PercentToThilde (mldb_rec.text[0]));
		strcpy (store[ lcount[2] ].pname, mldb_rec.pname);
		store[ lcount[2] ].org_len	=	mldb_rec.org_len;
		store[ lcount[2] ].hide		=	mldb_rec.hide;
		strcpy (store[ lcount[2] ].label, mldb_rec.lu_prmpt);
		
		putval(lcount[2]++);

		if (lcount[2] > MAXLINES)
			break;

		cc = find_rec(mldb, &mldb_rec, NEXT, "r");
	}
	vars [scn_start].row = lcount [2];
	scn_set(1);
}

void
update (void)
{
	clear();
	print_at(0,0, ML ("Please Wait, Updating records"));

	scn_set (2);

	for (line_cnt = 0; line_cnt < lcount[2]; line_cnt++)
	{
		getval(line_cnt);

		strcpy (mldb_rec.lu_prmpt, SR.label);
		cc = find_rec(mldb, &mldb_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, "mldb", "DBFIND");

		if (strncmp (mldb_rec.text [local_rec.lang_no -1], "NOTE THAT % has been replaced by ~ for entry",44))
			strcpy (mldb_rec.text [local_rec.lang_no - 1], ThildeToPercent( local_rec.text));
		
		cc = abc_update(mldb, &mldb_rec);
		if (cc)
			file_err(cc, mldb, "DBUPDATE");
	}
}

char *
PercentToThilde (
 char*              in_mesg)
{
	int 	j,
			len;

	len = strlen(in_mesg);
	for (j = 0; j < len; j++)
	{
		if (in_mesg[j] == '%')
			in_mesg[j] = '~';
	}
	return (in_mesg);
}

char *
ThildeToPercent (
 char*              in_mesg)
{
	int 	j,
			len;

	len = strlen(in_mesg);
	for (j = 0; j < len; j++)
	{
		if (in_mesg[j] == '~')
			in_mesg[j] = '%';
	}
	return (in_mesg);
}

int
CountThilde (
 char*              in_mesg)
{
	int		i,
			len,
			cnt = 0;

	len = strlen(in_mesg);
	for (i = 0; i < len; i++)
	{
		if (in_mesg[i] == '~')
			cnt++;
	}
	return (cnt);
}

/*==========================
| Search on UOM (inum)     |
==========================*/
void
srch_mlcf (
 char*              key_val)
{
	work_open();
	sprintf (err_str, "#%s", ML ("Language Description"));
	save_rec("#N",err_str);

	mlcf_rec.lang_no	=	atoi (key_val);
	cc = find_rec ("mlcf", &mlcf_rec, GTEQ, "r");
	while (!cc)
	{
		sprintf (err_str, "%1d", mlcf_rec.lang_no);
		cc = save_rec(err_str, mlcf_rec.lang_desc);
		if (cc)
			break;

		cc = find_rec(mlcf, &mlcf_rec, NEXT, "r");
	}

	cc = disp_srch();
	work_close();
	if (cc)
		return;

	mlcf_rec.lang_no	=	atoi (temp_str);
	cc = find_rec ("mlcf", &mlcf_rec, COMPARISON, "r");
	if (cc)
		file_err(cc, mlcf, "DBFIND");
}
