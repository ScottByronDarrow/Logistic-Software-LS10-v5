/*=====================================================================
|  Copyright (C) 1988 - 1994 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( ts_lett_mnt.c  )                                 |
|  Program Desc  : ( Telesales Letter Maintenance.                )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, tslh, tsln,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  tslh, tsln,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 04/12/91         |
|---------------------------------------------------------------------|
|  Date Modified : (09/12/91)      | Modified  by  : Campbell Mander  |
|  Date Modified : (21/12/91)      | Modified  by  : Campbell Mander  |
|  Date Modified : (19/06/92)      | Modified  by  : Campbell Mander  |
|  Date Modified : (18/08/92)      | Modified  by  : Campbell Mander  |
|  Date Modified : (12/10/92)      | Modified  by  : Campbell Mander  |
|  Date Modified : (03/11/94)      | Modified  by  : Dirk Heinsius.   |
|  Date Modified : (04/09/97)      | Modified  by  : Ana Marie Tario. |
|                                                                     |
|  Comments      : (09/12/91) - Added new dot commands                |
|                :                                                    |
|  (21/12/91)    : Added follow_up flag.                              |
|                :                                                    |
|  (19/06/92)    : Change follow_up to lett_type.                     |
|                :                                                    |
|  (18/08/92)    : Fix copying of mailers. DPL SC 7632.               |
|                :                                                    |
|  (12/10/92)    : Add UPDATE/IGNORE/DELETE option when updating.     |
|                : SC 7924 DPL.                                       |
|                :                                                    |
|  (03/11/94)    : INF 11364 Fixed to check for correct mailer label  |
|                : format before allowing a label to be saved.        |
|  (04/09/97)    : Incorporated multilingual conversion and DMY4 date.|
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lett_mnt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TS/ts_lett_mnt/lett_mnt.c,v 5.4 2001/11/06 08:28:35 scott Exp $";

#define MAXSCNS 	2
#define MAXLINES	500

#define	X_OFF		0
#define	Y_OFF		0

#define	TXT_REQD
#include <ml_ts_mess.h>
#include <ml_std_mess.h>
#include <pslscr.h>
#include <hot_keys.h>
#include <minimenu.h>
#include <tabdisp.h>

#define	MNU_UPDATE	0
#define	MNU_IGNORE	1
#define	MNU_DELETE	2

/*
extern	int	X_EALL;
extern	int	Y_EALL;
extern	int	SR_X_POS;
extern	int	SR_Y_POS;
*/

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	int	new_mailer = FALSE;

	/*=========================
	| Common Record Structure |
	=========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
	};

	int comm_no_fields = 5;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tes_name[41];
	} comm_rec;

	/*===============================
	| Tele-Sales Letter Header file |
	===============================*/
	struct dbview tslh_list[] ={
		{"tslh_co_no"},
		{"tslh_let_code"},
		{"tslh_let_desc"},
		{"tslh_hhlh_hash"},
		{"tslh_lett_type"},
	};

	int	tslh_no_fields = 5;

	struct	{
		char	lh_co_no[3];
		char	lh_lett_code[11];
		char	lh_lett_desc[41];
		long	lh_hhlh_hash;
		char	lh_lett_type[2];
	} tslh_rec;

	/*===============================
	| Tele-Sales Letter Detail File |
	===============================*/
	struct dbview tsln_list[] ={
		{"tsln_hhlh_hash"},
		{"tsln_line_no"},
		{"tsln_desc"},
	};

	int	tsln_no_fields = 3;

	struct	{
		long	ln_hhlh_hash;
		int	ln_line_no;
		char	ln_desc[79];
	} tsln_rec;

	char	*data = "data",
		*comm = "comm",
	    	*tslh = "tslh",
	    	*tsln = "tsln";

char	*dot_desc[] = {
	"Current Date, Format dd/mm/yy",
	"Current Date, Format 1st January 1990 ",
	"Company Name.",
	"Company Address part one.",
	"Company Address part two.",
	"Company Address part three.",
	"Customer Number",
	"Customer Acronym",
	"Customer Name",
	"Customer Address Part 1",
	"Customer Address Part 2",
	"Customer Address Part 3",
	"Customer Contact Name.",
	"Customer Contact Position.",
	"Customer Alternate Contact Name.",
	"Customer Alternate Contact Position.",
	"Salesman Number.",
	"Salesman Name.",
	"Area Number.",
	"Area Name.",
	"Customer Contract Type.",
	"Customer Price Type.",
	"Customer Bank Code.",
	"Customer Bank Branch.",
	"Customer Discount Code.",
	"Customer Tax Code.",
	"Customer Tax Number.",
	"Date Of Last Invoice.",
	"Date Of Last Payment.",
	"Amount Of Last Payment.",
	"Month To Date Sales.",
	"Year To Date Sales.",
	"Value Of Current Orders.",
	"Customer Post Code.",
	"Customer Business Sector.",
	"Customer Phone Number.",
	"Customer Fax Number.",
	"Phone Frequency.",
	"Next Phone Date.",
	"Next Phone Time.",
	"Visit Frequency.",
	"Next Visit Date.",
	"Next Visit Time.",
	"Current Operator.",
	"Last Operator.",
	"Last Phone Date.",
	"Best Phone Time.",
	"Date Created.",
	"Number Of Labels Printed Horizontally.",
	"Number Of Labels Printed Vertically.",
	"Number Of Blank Lines Before 1st Label.",
	"Number Of Blank Lines After Last Label.",
	"Horizontal Spacing Between Labels.",
	"Vertical Spacing Between Labels.",
	""
};
#include	<ts_commands.h>

int	clear_ok;
int	first_time = TRUE;

MENUTAB upd_menu [] =
	{
		{ " 1. UPDATE RECORD  ",
		  "" },
		{ " 2. IGNORE CHANGES ",
		  "" },
		{ " 3. DELETE RECORD  ",
		  "" },
		{ ENDMENU }
};

/*===========================
| Local & Screen Structures |
===========================*/
struct {					 	/*---------------------------------------*/
	char	dummy[11];	 	 	/*| Dummy Used In Screen Generator.     |*/
	char	comments[79];    	/*| Holds Comments for each line.       |*/
	char	lett_code[11];   	/*| Holds Letter Number                 |*/
	char	lett_desc[41];   	/*| Holds Letter Description            |*/
	char	code_from[11];   	/*| Holds Letter Number to be copied    |*/
	char	lett_type[17];    	/*| N-Normal F-Follow Up L-Label Def.   |*/
	char	lett_type_desc[17]; /*| Holds the letter type description   |*/
				 				/*|_____________________________________|*/
} local_rec;            

static	struct	var	vars[] =
{
	{1, LIN, "code",	 3, 12, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", " Letter Code:", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.lett_code},
	{1, LIN, "code_from",	 3, 55, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "Copy from Letter:", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.code_from},
	{1, LIN, "desc",	 4, 12, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", tslh_rec.lh_lett_desc, " Description:", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.lett_desc},
	{1, LIN, "lett_type",	 5, 12, CHARTYPE,
		"U", "          ",
		" ", "N", " Mailer Type:", " N(ormal) F(ollow up) L(abel Definition) ",
		 NO, NO,  JUSTLEFT, "NFL", "", local_rec.lett_type},
	{1, LIN, "lett_type_desc",	 5, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.lett_type_desc},

	{2, TXT, "comm",	 8, 0, 0,
		"", "          ",
		" ", " ", ".....T.........T.........T.........T.........T.........T.........T........", "",
		 10, 78,  MAXLINES, "", "", local_rec.comments},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<std_decs.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
int load_dots (void);
int show_dots (void);
int read_tsln (long shash);
void show_tslh (char *key_val);
int update_menu (void);
int update (void);
void prn_co (void);
int heading (int scn);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int        argc,
 char*      argv[])
{
	SETUP_SCR (vars);

	init_scr();
	set_tty();
	set_masks();

	OpenDB();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	prog_exit = 0;
	while (prog_exit == 0)
	{
		clear_ok = TRUE;
		search_ok = 1;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_vars(1);	
		init_vars(2);	

		heading(1);
		entry(1);
		if (prog_exit || restart)
			continue;

		if (new_mailer)
		{
			cc = FALSE;
			while (cc == FALSE)
			{
				scn_display(2);
				edit(2);
				if (restart)
					cc = TRUE;
				else
					cc = update();
			}
		}
		else
		{
			cc = FALSE;
			while (cc == FALSE)
			{
				scn_display(2);
				edit_all();
				if (restart)
					cc = TRUE;
				else
					cc = update_menu();
			}
		}
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence . |
=========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open Database Files . |
=======================*/
void
OpenDB (void)
{
	abc_dbopen(data);

	open_rec(tslh, tslh_list, tslh_no_fields, "tslh_id_no");
	open_rec(tsln, tsln_list, tsln_no_fields, "tsln_id_no");
}

/*========================
| Close Database Files . |
========================*/
void
CloseDB (void)
{
	abc_fclose(tslh);
	abc_fclose(tsln);
	abc_dbclose(data);
}

int
spec_valid (
 int    field)
{
	/*-----------------------
	| Validate Letter Code  |
	-----------------------*/
	if ( LCHECK("code") )
	{
	    	if (SRCH_KEY)
	    	{
			show_tslh(temp_str);
			return(0);
	    	}

	    	strcpy(tslh_rec.lh_co_no, comm_rec.tco_no);
	    	sprintf(tslh_rec.lh_lett_code, "%-10.10s", local_rec.lett_code);
	    	if ( !find_rec(tslh, &tslh_rec, COMPARISON, "r") )
	    	{
	    		sprintf(local_rec.lett_desc, 
				"%-40.40s", 
				tslh_rec.lh_lett_desc);

			switch (tslh_rec.lh_lett_type[0])
			{
			case 'N':
				strcpy (local_rec.lett_type,"N");
				strcpy (local_rec.lett_type_desc,"Normal          ");
				break;

			case 'F':
				strcpy (local_rec.lett_type,"F");
				strcpy (local_rec.lett_type_desc,"Follow Up       ");
				break;

			case 'L':
				strcpy (local_rec.lett_type,"L");
				strcpy (local_rec.lett_type_desc,"Label Definition");
				break;
			}

			DSP_FLD("desc");
			DSP_FLD("lett_type");
			DSP_FLD("lett_type_desc");
			FLD("code_from") = NA;

			if ( read_tsln( tslh_rec.lh_hhlh_hash ) )
			{
				print_mess(ML(mlTsMess093));
				sleep (sleepTime);
				clear_mess();
				return(1);
			}

			entry_exit = TRUE;
			new_mailer = FALSE;
	    	}
		else
		{
			FLD("code_from") = YES;
			new_mailer = TRUE;
		}

	    	return(0);
	}

	/*--------------------------
	| Validate Para Code_From. |
	--------------------------*/
	if (LCHECK("code_from"))
	{
	    	if (SRCH_KEY)
	    	{
			show_tslh(temp_str);
			return(0);
	    	}
	
	    	if (!strcmp(local_rec.code_from,"          "))
			return(0);
	
	    	strcpy(tslh_rec.lh_co_no, comm_rec.tco_no);
	    	strcpy(tslh_rec.lh_lett_code, local_rec.code_from);
	    	if ( !find_rec(tslh, &tslh_rec, COMPARISON, "r"))
	    	{
	    		strcpy(local_rec.lett_desc, tslh_rec.lh_lett_desc);
	    		strcpy(local_rec.lett_type, tslh_rec.lh_lett_type);
			switch (tslh_rec.lh_lett_type[0])
			{
			case 'N':
				strcpy (local_rec.lett_type_desc,"Normal          ");
				break;

			case 'F':
				strcpy (local_rec.lett_type_desc,"Follow Up       ");
				break;

			case 'L':
				strcpy (local_rec.lett_type_desc,"Label Definition");
				break;
			}
			DSP_FLD("desc");
			DSP_FLD("lett_type");
			DSP_FLD("lett_type_desc");
			entry_exit = 1;
			FLD("code_from") = NE;
			if ( read_tsln(tslh_rec.lh_hhlh_hash) )
				return(1);
	    	}
	    	else
	    	{
				errmess(ML(mlStdMess109));
				sleep (sleepTime);
				clear_mess ();
				return(1);
	    	}

	    	return(0);
	}

	if (LCHECK("lett_type"))
	{
		switch (local_rec.lett_type[0])
		{
		case 'N':
			strcpy (local_rec.lett_type_desc,"Normal          ");
			break;

		case 'F':
			strcpy (local_rec.lett_type_desc,"Follow Up       ");
			break;

		case 'L':
			strcpy (local_rec.lett_type_desc,"Label Definition");
			break;
		}

		DSP_FLD("lett_type_desc");
	
		return(0);
	}

	/*------------------------
	| Validate Comment Line. |
	------------------------*/
	if (LCHECK("comm"))
        {
		if (SRCH_KEY)
		{
			show_dots();

			clear_ok = FALSE;
			heading(2);
			scn_display(2);
			clear_ok = TRUE;
			crsr_on();

			return(0);
		}
	}
	return(0);
}

/*-------------------------------------
| Load dot commands and descriptions  |
-------------------------------------*/
int
load_dots (void)
{
	int	i;
	char	disp_str[200];
	
	tab_open("dot_tab", (KEY_TAB *)0, 3, 0, 4, FALSE);
	tab_add("dot_tab", 
		"#                    Dot Commands                    ");

	for ( i = 0; i < N_CMDS; i++ )
	{
		if (!strchr(dot_cmds[i].format, local_rec.lett_type[0]))
			continue;

		sprintf(disp_str," .%-7.7s  %-40.40s ", 
					dot_cmds[i].command, dot_desc[ i ]);

		tab_add("dot_tab", disp_str);
	}

	return(0);
}

/*-------------------------------------
| Show dot commands and descriptions  |
-------------------------------------*/
int
show_dots (void)
{
	load_dots();

	tab_display("dot_tab", TRUE);
	tab_scan("dot_tab");

	tab_close("dot_tab", TRUE);

	/*-----------------
	| Redraw Screen 1 |
	-----------------*/
	box(0,2,80,3);
	scn_write(1);
	scn_display(1);

	return(0);
}

/*==========================
| Read Letter Detail Lines |
==========================*/
int
read_tsln (
 long   shash)
{
	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set(2);
	lcount[2] = 0;

	tsln_rec.ln_hhlh_hash = shash;
	tsln_rec.ln_line_no = 0; 

	cc = find_rec("tsln", &tsln_rec, GTEQ, "r");
	while (!cc && tsln_rec.ln_hhlh_hash == shash)
	{
		strcpy(local_rec.comments, tsln_rec.ln_desc);
		putval(lcount[2]++);
		cc = find_rec("tsln", &tsln_rec, NEXT, "r");
	}
	scn_set(1);

	/*---------------------
	| No entries to edit. |
	---------------------*/
	if (lcount[2] == 0)
		return(1);

	return(0);
}

/*=============================================
| Search routine for Letter master file.      |
=============================================*/
void
show_tslh (
 char*  key_val)
{
	work_open();
	save_rec("#Code","#Description");
	strcpy(tslh_rec.lh_co_no,comm_rec.tco_no);
	sprintf(tslh_rec.lh_lett_code, "%-10.10s", key_val);
	cc = find_rec("tslh", &tslh_rec, GTEQ, "r");
	while (!cc && 
		  !strcmp(tslh_rec.lh_co_no,comm_rec.tco_no) &&
	          !strncmp(tslh_rec.lh_lett_code,key_val,strlen(key_val)))
	{
		cc = save_rec(tslh_rec.lh_lett_code, tslh_rec.lh_lett_desc);
		if (cc)
			break;

		cc = find_rec("tslh", &tslh_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(tslh_rec.lh_co_no, comm_rec.tco_no);
	sprintf(tslh_rec.lh_lett_code, "%-10.10s", key_val);
	cc = find_rec("tslh", &tslh_rec, COMPARISON, "r");
	if (cc)
		sys_err("Error in tslh During (DBFIND)", cc, PNAME);

	sprintf(local_rec.lett_desc,"%-40.40s",tslh_rec.lh_lett_desc);
}

/*===================
| Update mini menu. |
===================*/
int
update_menu (void)
{
	for (;;)
	{
	    mmenu_print (" UPDATE SELECTION. ", upd_menu, 0);
	    switch (mmenu_select (upd_menu))
	    {
		case MNU_UPDATE :
		case 99 :
			cc = update();
			return (cc);

		case MNU_IGNORE :
		case -1 :
			abc_unlock(tslh);
			return (TRUE);

		case MNU_DELETE :
			/*---------------------
			| Delete lines first. |
			---------------------*/
			tsln_rec.ln_hhlh_hash = tslh_rec.lh_hhlh_hash;
			tsln_rec.ln_line_no = 0;
			cc =  find_rec(tsln, &tsln_rec, GTEQ, "u");
			while (!cc && 
			       tsln_rec.ln_hhlh_hash == tslh_rec.lh_hhlh_hash)
			{
    				putchar('D');
    				fflush(stdout);
				cc = abc_delete(tsln);
				if (cc)
					file_err(cc, tsln, "DBDELETE");
		
				tsln_rec.ln_hhlh_hash = tslh_rec.lh_hhlh_hash;
				tsln_rec.ln_line_no = 0;
				cc =  find_rec("tsln", &tsln_rec, GTEQ, "u");
			}

			/*----------------
			| Delete header. |
			----------------*/
			cc = abc_delete(tslh);
			if (cc)
				file_err( cc, tslh, "DBDELETE" );
			return (TRUE);
	
		default :
			break;
	    }
	}
}

/*-------------------
| Update all files. |
-------------------*/
int
update (void)
{
	int 	add_item = FALSE;
	int		wk_line;
	char 	*cptr;
	char 	*dptr;
	int		labels_hor = 0;
	int		labels_vrt = 0;

	if (local_rec.lett_type[0] == 'L')
	{
		/*-----------------------
		| look for dot command	|
		-----------------------*/
		scn_set(2);
	   	getval(0);
		cptr = strchr (local_rec.comments, '.');
		dptr = cptr + 1;
		while (cptr)
		{
			if (!strncmp(cptr + 1, "LBL_", 4))
			{
				if (!strncmp(cptr + 5, "HZ", 2))
					labels_hor = atoi(cptr + 7);

				if (!strncmp(cptr + 5, "VT", 2))
					labels_vrt = atoi(cptr + 7);
			}
			cptr = strchr (dptr, '.');
			dptr = cptr + 1;
		}
		if (labels_hor == 0 || labels_vrt == 0)
		{
			print_mess(ML(mlTsMess094));
			sleep (sleepTime);
			clear_mess ();
			return (FALSE);
		}
	}

	clear();
		
	if (new_mailer) 
	{
		/*rv_pr("Now Creating New Letter Record.", 2, 0, 1);*/
		rv_pr(ML(mlTsMess057), 2, 0, 1);

		/*-------------------
		| Add header record |
		-------------------*/
		strcpy(tslh_rec.lh_co_no, comm_rec.tco_no);
		strcpy(tslh_rec.lh_lett_code, local_rec.lett_code);
		strcpy(tslh_rec.lh_lett_desc, local_rec.lett_desc);
		sprintf(tslh_rec.lh_lett_type, "%-1.1s", local_rec.lett_type);
		tslh_rec.lh_hhlh_hash = 0L;
		cc = abc_add(tslh, &tslh_rec);
		if (cc) 
			file_err(cc, tslh, "DBADD");

		/*-------------------------
		| Read back header record |
		| to get hhlh hash.       |
		-------------------------*/
		strcpy(tslh_rec.lh_co_no, comm_rec.tco_no);
		strcpy(tslh_rec.lh_lett_code, local_rec.lett_code);
		cc = find_rec(tslh, &tslh_rec, COMPARISON, "r");
		if (cc)
			return (FALSE);
	}
	else
	{
		/*rv_pr("Now Updating Letter Record.", 2,2,1);*/
		rv_pr(ML(mlTsMess058), 2,2,1);

		/*----------------------
		| Update header record |
		----------------------*/
		strcpy(tslh_rec.lh_lett_desc, local_rec.lett_desc);
		sprintf(tslh_rec.lh_lett_type, "%-1.1s", local_rec.lett_type);
		cc = abc_update(tslh, &tslh_rec);
		if (cc) 
			file_err(cc, tslh, "DBUPDATE");
	}

	scn_set(2);
	for (wk_line = 0;wk_line < lcount[2];wk_line++) 
	{
	    	getval(wk_line);

	    	tsln_rec.ln_hhlh_hash = tslh_rec.lh_hhlh_hash;
	    	tsln_rec.ln_line_no = wk_line;
	    	add_item = find_rec("tsln", &tsln_rec, COMPARISON, "u");
	    	strcpy(tsln_rec.ln_desc, local_rec.comments);
	    	if (add_item)
	    	{
	    		putchar('A');
	    		fflush(stdout);
			cc = abc_add(tsln, &tsln_rec);
			if (cc) 
				file_err(cc, tsln, "DBADD");
	    	}
	    	else
	    	{
		    	/*------------------------
		    	| Update existing order. |
		    	------------------------*/
		    	cc = abc_update(tsln, &tsln_rec);
		    	if (cc) 
				file_err(cc, tsln, "DBUPDATE");
	    	}
		abc_unlock(tsln);
	}

	/*--------------------------
	| Delete extraneous lines. |
	--------------------------*/
	tsln_rec.ln_hhlh_hash = tslh_rec.lh_hhlh_hash;
	tsln_rec.ln_line_no = lcount[2];
	cc =  find_rec(tsln, &tsln_rec, GTEQ, "u");
	while (!cc && tsln_rec.ln_hhlh_hash == tslh_rec.lh_hhlh_hash)
	{
    		putchar('D');
    		fflush(stdout);
		cc = abc_delete(tsln);
		if (cc)
			file_err(cc, tsln, "DBDELETE");

		tsln_rec.ln_hhlh_hash = tslh_rec.lh_hhlh_hash;
		tsln_rec.ln_line_no = lcount[2];
		cc =  find_rec("tsln", &tsln_rec, GTEQ, "u");
	}

	/*-----------------------------
	| Check that there are still  |
	| some lines for this header. |
	-----------------------------*/
	if (!new_mailer) 
	{	
		tsln_rec.ln_hhlh_hash = tslh_rec.lh_hhlh_hash;
		tsln_rec.ln_line_no = 0;
		cc =  find_rec(tsln, &tsln_rec, GTEQ, "r");
		if (cc || tsln_rec.ln_hhlh_hash != tslh_rec.lh_hhlh_hash)
	    	{
	    		/*---------------
	    		| Delete letter |
	    		---------------*/
			/*rv_pr("Now Deleting Letter Record.",2,2,1);*/
			rv_pr(ML(mlTsMess059),2,2,1);

			cc = abc_delete(tslh);
			if (cc)
				file_err(cc, tslh, "DBDELETE");
	    	}
	    	abc_unlock(tslh);
	}

	return (TRUE);
}

/*========================
| Print Company Details. |
========================*/
void
prn_co (void)
{
	move(0,20);
	line(80);
	strcpy(err_str,ML(mlStdMess038));
	print_at(21,0,err_str,comm_rec.tco_no,comm_rec.tco_name);
	move(0,22);
	line(80);
}

/*================
| Print Heading. |
================*/
int
heading (
 int    scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		if (clear_ok)
			clear();
		
		rv_pr(ML(mlTsMess060),30,0,1);
		
		move(0,1);
		line(80);

		box(0,2,80,3);
		if (scn == 1)
		{
			if (clear_ok)
			{
				scn_set(2);
				scn_display(2);
			}
		}

		if (scn == 2)
		{
			scn_set(1);
			scn_write(1);
			scn_display(1);
		}

		scn_set( scn );

		prn_co();
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
        return (EXIT_SUCCESS);
	}
    return (EXIT_FAILURE);
}
