/*=====================================================================
|  Copyright (C) 1988 - 1996 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( ps_ring_sec.c  )                                 |
|  Program Desc  : ( Ring menu Security.                          )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  : opts ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Updates Files : opts ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 27/03/91         |
|---------------------------------------------------------------------|
|  Date Modified : (16/07/92)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (12/09/97)      | Modified  by  : Marnie Organo.   |
|                                                                     |
|  Comments      : (16/07/92) -  Updated to remove usage if optx.     |
|                : (12/09/97) -  Updated for Multilingual Conversion. |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ring_sec.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/ps_ring_sec/ring_sec.c,v 5.3 2002/07/25 11:17:39 scott Exp $";

#define	MAXSCNS		2
#define	MAXLINES	100
#define	MAXWIDTH	150
#include	<pslscr.h>
#include	<minimenu.h>
#include	<ml_std_mess.h>
#include	<ml_utils_mess.h>

#define	SLEEP_TIME	2

#define	SEL_UPDATE	0
#define	SEL_IGNORE	1
#define	SEL_DELETE	2

#define		HEADER_SCN		1
#define		DETAIL_SCN		2

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"}
	};

	int comm_no_fields = 6;
	
	struct {
		int  termno;
		char tco_no[3];
		char tco_name[41];
		char tes_no[3];
		char tes_name[41];
		long t_dbt_date;
	} comm_rec;

	/*============================================+
	 | System options menu ( used sk_alldisp etc. |
	 +============================================*/
#define	OPTS_NO_FIELDS	6

	struct dbview	opts_list [OPTS_NO_FIELDS] =
	{
		{"opts_access_code"},
		{"opts_prog_name"},
		{"opts_option_no"},
		{"opts_key"},
		{"opts_allowed"},
		{"opts_key_desc"}
	};

	struct tag_optsRecord
	{
		char	access_code [9];
		char	prog_name [15];
		int		option_no;
		char	key [31];
		char	allowed [2];
		char	key_desc [61];
	}	opts_rec;

	/*======================================+
	 | Menu System Access Description File. |
	 +======================================*/
#define	MNAC_NO_FIELDS	3

	struct dbview	mnac_list [MNAC_NO_FIELDS] =
	{
		{"mnac_hhac_hash"},
		{"mnac_code"},
		{"mnac_description"}
	};

	struct tag_mnacRecord
	{
		long	hhac_hash;
		char	code [9];
		char	desc[31];
	}	mnac_rec;


	char	access_code[9],
			curr_access[9],
			prog_name[15],
			curr_prog[15],
			allowed[2],
			key[31],
			key_desc[61];

	int	option_no;

struct	{
	char	dummy[11];
} local_rec;

static	struct	var	vars[]	=	
{
	{HEADER_SCN, LIN, "code", 3, 16, CHARTYPE, 
		"AAAAAAAA", "          ", 
		" ", " ", "Access Code ", "", 
		NE, NO, JUSTLEFT, "0123456789abcdefghijklmnopqrstuvwxyz*", "", mnac_rec.code}, 
	{HEADER_SCN, LIN, "desc", 4, 16, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description ", " ", 
		NA, NO, JUSTLEFT, "", "", mnac_rec.desc}, 
	{DETAIL_SCN, TAB, "access_code", MAXLINES, 2, CHARTYPE, 
		"AAAAAAAA", "          ", 
		" ", " ", " Access Code ", " ", 
		NA, NO, JUSTLEFT, "", "", access_code}, 
	{DETAIL_SCN, TAB, "option_no", 0, 0, INTTYPE, 
		"NNNNN", "          ", 
		" ", " ", "", " ", 
		ND, NO, JUSTLEFT, "", "", (char *)&option_no}, 
	{DETAIL_SCN, TAB, "prog_name", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", " ", " Program Name ", " ", 
		ND, NO, JUSTLEFT, "", "", prog_name}, 
	{DETAIL_SCN, TAB, "key", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "    H o t   K e y   N a m e   ", " ", 
		NA, NO, JUSTLEFT, "", "", key}, 
	{DETAIL_SCN, TAB, "allowed", 0, 3, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Allowed", "Enter Y(es) N(o) ", 
		YES, NO, JUSTLEFT, "YyNn", "", allowed}, 
	{DETAIL_SCN, TAB, "key_desc", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "          H o t    K e y     D e s c r i p t i o n          ", " ", 
		NA, NO, JUSTLEFT, "", "", key_desc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy}, 

};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void OpenDB (void);
void CloseDB (void);
void shutdown_prog (void);
int spec_valid (int field);
void LoadAccess (char *ProgName, char *Access);
void update_menu (void);
void srch_mnac (char *key_val);
int heading (int scn);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int                argc,
 char*              argv[])
{
	if ( argc != 2 )
	{
		print_at(0,0,mlUtilsMess701,argv[0]);
        return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	OpenDB ();

	/*---------------------------------------
	| initialise lists			|
	---------------------------------------*/
	init_scr ();
	set_tty ();

	sprintf ( curr_prog, "%-14.14s", argv[1] );

	set_masks ();			/*  setup print using masks	*/
	swide ();
	
	tab_row = 6;

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (prog_exit == 0)
	{
		/*----------------------------------
		| Reset Control Flags              |
		----------------------------------*/
		entry_exit 	= 0;
		edit_exit 	= 0;
		prog_exit 	= 0;
		restart 	= 0;
		search_ok 	= 1;
		lcount [DETAIL_SCN] 	= 0;
		init_vars (HEADER_SCN);
		init_vars (DETAIL_SCN);

		heading (HEADER_SCN);
		scn_display (HEADER_SCN);
		entry (HEADER_SCN);

		if (prog_exit || restart)
			continue;

		scn_write (HEADER_SCN);
		scn_display (HEADER_SCN);
		scn_write (DETAIL_SCN);
		scn_display (DETAIL_SCN);
		

		edit (DETAIL_SCN);

		if (prog_exit || restart)
			continue;

		edit_all();

		if (restart)
			continue;

		update_menu ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*===============================
| Open data base files.			|
===============================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm ( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec ("opts", opts_list, OPTS_NO_FIELDS, "opts_id_no");
	open_rec ("mnac", mnac_list, MNAC_NO_FIELDS, "mnac_code");
}

void
CloseDB(void)
{
	abc_fclose ("opts");
	abc_fclose ("mnac");
	abc_dbclose ("data");
}
/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
 int                field)
{
	if (LCHECK("code"))
	{
		if (SRCH_KEY)
		{
		   srch_mnac (temp_str);
		   return (EXIT_SUCCESS);
		}

		if (strchr(temp_str, '*'))
		{
			sprintf(mnac_rec.code, "%-8.8s", "*");
			sprintf(temp_str, "%-8.8s", "*");
		}

		cc = find_rec("mnac",&mnac_rec,COMPARISON,"r");
		if (cc)
		{
			/*sprintf (err_str, "Access code %s is not on file.",mnac_rec.code);*/
			sprintf (err_str, ML(mlUtilsMess067),mnac_rec.code);
			errmess (err_str);
		}
		entry_exit	=	1;
		LoadAccess (curr_prog, mnac_rec.code);

		DSP_FLD("desc");
        return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
LoadAccess ( 
 char*              ProgName,
 char*              Access)
{
	char	wk_access[9];

	scn_set (DETAIL_SCN);
	lcount[DETAIL_SCN] = 0;

	sprintf( wk_access, "%-8.8s", Access );

	sprintf (opts_rec.access_code, "%-8.8s", wk_access);
	sprintf (opts_rec.prog_name, "%-14.14s", ProgName);
	opts_rec.option_no = 0;
	sprintf (opts_rec.key, "%30.30s", " ");
	cc = find_rec("opts", &opts_rec, GTEQ, "r");
	if ( cc ||
		strncmp (opts_rec.access_code, Access,   strlen (Access)) ||
		strncmp (opts_rec.prog_name, ProgName, strlen (ProgName)))
	{
		sprintf ( wk_access, "%-8.8s", "*" );
		sprintf ( opts_rec.access_code, "%-8.8s",  wk_access);
		sprintf ( opts_rec.prog_name, "%-14.14s", ProgName);
		opts_rec.option_no = 0;
		sprintf ( opts_rec.key, "%30.30s", " ");
		cc = find_rec ("opts", &opts_rec, GTEQ, "r");
	}
	while ( !cc && 
		!strncmp (opts_rec.access_code,wk_access,strlen(wk_access)) &&
		!strncmp (opts_rec.prog_name,ProgName,strlen(ProgName)))
	{
		sprintf (access_code, "%-8.8s", Access);
		sprintf (prog_name, "%-14.14s", opts_rec.prog_name);
		sprintf (allowed, "%-1.1s",     opts_rec.allowed);
		option_no = opts_rec.option_no;
		sprintf (key, "%-30.30s",       opts_rec.key);
		sprintf (key_desc, "%-60.60s",  opts_rec.key_desc);

		putval (lcount[DETAIL_SCN]++);
		cc = find_rec ("opts", &opts_rec, NEXT, "r");
	}
	vars[ scn_start ].row = lcount[ DETAIL_SCN ];
	scn_set (HEADER_SCN);
}

MENUTAB upd_menu [] =
	{
		{ " 1. UPDATE RECORDS  ",
		  " UPDATE USER SPECIFIC HOT KEYS WITH CHANGES MADE. " },
		{ " 2. IGNORE CHANGES ",
		  " IGNORE CHANGES JUST MADE TO USER SPECIFIC HOT KEYS. " },
		{ " 3. DELETE ALL ",
		  " DELETE ALL USER SPECIFIC HOT KEYS. " },
		{ ENDMENU }
	};
/*===================
| Update mini menu. |
===================*/
void
update_menu (void)
{
	char	opt_str[61];

	opt_str[0] = '\0';

	for (;;)
	{
	    mmenu_print (" UPDATE SELECTION. ", upd_menu, 0);
	    switch (mmenu_select (upd_menu))
	    {
		case SEL_UPDATE :
		clear ();
				
		/*rv_pr ("Saving user Specific hot key selection.",0,0,1);*/
		rv_pr (ML(mlUtilsMess024),0,0,1);

		fflush (stdout);

		for ( line_cnt = 0; line_cnt < lcount[DETAIL_SCN]; line_cnt++ )
		{
			getval (line_cnt);
	
			strcat (opt_str, allowed);

			sprintf (opts_rec.access_code, "%-8.8s",   access_code);
			sprintf (opts_rec.prog_name,   "%-14.14s", prog_name);
			opts_rec.option_no = option_no;
			sprintf (opts_rec.key,       "%-30.30s", key);
			cc = find_rec ("opts", &opts_rec, COMPARISON, "r" );
			if (cc)
			{
				putchar ('A');
				fflush (stdout);
	
				strcpy (opts_rec.allowed, allowed );
				strcpy (opts_rec.key_desc, key_desc );
				cc = abc_add ("opts", &opts_rec);
				if ( cc )
					file_err (cc, "opts", "DBADD" );
			}
			else
			{
				putchar ('U');
				fflush (stdout);
				strcpy (opts_rec.allowed, allowed );

				cc = abc_update ("opts", &opts_rec);
				if (cc)
					file_err( cc, "opts", "DBUPDATE" );
			}
		}
		return;
		break;

		case SEL_IGNORE :
			return;

		case SEL_DELETE :

			clear();
			/*rv_pr ("Deleting user Specific hot key selection. ",0,0,1);*/
			rv_pr (ML(mlUtilsMess025),0,0,1);
			fflush(stdout);
		
			for (line_cnt = 0;line_cnt < lcount [DETAIL_SCN];line_cnt++)
			{
				putchar ('D');
				fflush (stdout);
		
				getval (line_cnt);
		
				sprintf (opts_rec.access_code, "%-8.8s", access_code);
				sprintf (opts_rec.prog_name, "%-14.14s", prog_name);
				sprintf (opts_rec.key,       "%-30.30s", key);
				opts_rec.option_no = option_no;
				cc = find_rec ("opts", &opts_rec, COMPARISON, "r" );
				if ( cc || !strcmp (access_code, "*       " ))
					continue;

				cc = abc_delete ("opts");
				if ( cc )
					file_err (cc, "opts", "DBDELETE" );
			}
			return;
		break;
	
		default :
			break;
	    }
	}
}

/*=======================
| Search for accs_code  |
=======================*/
void
srch_mnac (
 char*              key_val)
{
	work_open ();
	save_rec ("#  Code  ","#Description                             ");
	sprintf (mnac_rec.code,"%-8.8s",key_val);
	cc = find_rec ("mnac", &mnac_rec, GTEQ, "r");
	while (!cc && !strncmp (mnac_rec.code,key_val,strlen(key_val)))
	{                        
		if (!strncmp (mnac_rec.code, "ERROR", 5))
		{
			cc = find_rec ("mnac",&mnac_rec,NEXT,"r");
			continue;
		}

		cc = save_rec (mnac_rec.code,mnac_rec.desc); 
		if (cc)
		        break;
		cc = find_rec ("mnac",&mnac_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	        return;

	sprintf (mnac_rec.code,"%-8.8s",temp_str);
	cc = find_rec ("mnac", &mnac_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "mnac", "DBFIND" );
}
	
/*=========================
| Display Screen Heading  |
=========================*/
int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		swide ();
		clear ();
		rv_pr (ML(mlUtilsMess026),20,0,1);
		move (0,1);
		line (132);

		move (1,input_row);
		switch (scn)
		{
		case	HEADER_SCN:

			scn_set (DETAIL_SCN);
			scn_write (DETAIL_SCN);
			scn_display (DETAIL_SCN);

			box (0,2,132,2);
			break;

		case	DETAIL_SCN:
			scn_set (HEADER_SCN);
			scn_write (HEADER_SCN);
			scn_display (HEADER_SCN);
			box (0,2,132,2);
			break;
		}
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	move (0,20);
	line (132);
	/*print_at (21,0," Company no. : %s   %s",comm_rec.tco_no,comm_rec.tco_name);*/
	print_at (21,0,ML(mlStdMess035),comm_rec.tco_no,comm_rec.tco_name);
	move (0,22);
	line (132);
    return (EXIT_SUCCESS);
}
