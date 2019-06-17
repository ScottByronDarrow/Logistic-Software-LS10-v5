/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: _trinput.c,v 5.3 2002/07/23 05:15:43 scott Exp $
|  Program Name  : (lrp_trinput.c) 
|  Program Desc  : (Input Purchase Orders from Reorder Review)
|                  (Report. (ie Qty's & Select Warehouse))
|---------------------------------------------------------------------|
|  Date Written  : (18/08/1998)    | Author      : Scott B Darrow     |
|---------------------------------------------------------------------|
| $Log: _trinput.c,v $
| Revision 5.3  2002/07/23 05:15:43  scott
| .
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _trinput.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_trinput/_trinput.c,v 5.3 2002/07/23 05:15:43 scott Exp $";

#include	<pslscr.h>
#include	<getnum.h>
#include	<hot_keys.h>
#include	<get_lpno.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<ml_lrp_mess.h>
#include    <tabdisp.h>
extern	int		tab_max_page;

#define		WK_DEPTH	14
#define		MAX_WH		100

#include	"schema"

struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct ffwkRecord	ffwk_rec;
struct ffwkRecord	ffwk2_rec;
struct inccRecord	incc_rec;
struct inmrRecord	inmr_rec;

	float	*ffwk_cons	=	&ffwk_rec.cons_1;

	char 	*ffwk2		= "ffwk2",
			*inmr2		= "inmr2",
			*scanFfwk	= "scanFfwk";

	int		firstLine 		= 0,
			NoWHTabLines	= 0,
			FirstScan		= TRUE;	

	long	hhccHash;

	char	LRP_FileName [15];
	int		LRP_LinesProcessed	=	0;
	int		LRP_NoRecords		=	0;

static	int	KeyConfirm 			(int, KEY_TAB *);
static	int	KeyConfirmAll 		(int, KEY_TAB *);
static	int	KeyQuantity 		(int, KEY_TAB *);
static	int	KeyReject  			(int, KEY_TAB *);
static	int	KeyRejectAll		(int, KEY_TAB *);
static	int	KeyStockDisplay		(int, KEY_TAB *);
static	int	KeyLrpDisplay		(int, KEY_TAB *);
static	int	KeyMove    			(int, KEY_TAB *);

#ifdef GVISION
static	KEY_TAB	Header_keys [] =
{
	{ " Quantity ",	'Q',		KeyQuantity,
		"Enter a new quantity"						},
	{ " Accept ",	'A',		KeyConfirm,
		"Accept the current Purchase-Order"			},
	{ " Accept All ",('a'),		KeyConfirmAll,
		"Accept ALL remaining Purchase-Orders"		},
	{ " Cancel ",	'C',		KeyReject,
		"Cancel the current Purchase-Order"			},
	{ " Cancel All ",'c',	KeyRejectAll,
		"Cancel ALL remaining Purchase-Orders"		},
	{ " Display stock ",	'S',	KeyStockDisplay,
		"Stock Display"								},
	{ " LRP Display ",	'L',	KeyLrpDisplay,
		"LRP Display"								},
	{ " Move ",	'M',	KeyMove,
		"Move to previous position in file."	    },
	END_KEYS
};
#else
static	KEY_TAB	Header_keys [] =
{
	{ "[Q]uantity",	'Q',		KeyQuantity,
		"Enter a new quantity"						},
	{ "[A]ccept ",	'A',		KeyConfirm,
		"Accept the current Purchase-Order"			},
	{ "[^A]ccept All",	CTRL ('A'),	KeyConfirmAll,
		"Accept ALL remaining Purchase-Orders"		},
	{ "[C]ancel",	'C',		KeyReject,
		"Cancel the current Purchase-Order"			},
	{ "[^C]ancel All",	CTRL ('C'),	KeyRejectAll,
		"Cancel ALL remaining Purchase-Orders"		},
	{ "S(tock Display",	'S',	KeyStockDisplay,
		"Stock Display"								},
	{ "L(RP Display",	'L',	KeyLrpDisplay,
		"LRP Display"								},
	{ "M(ove",	'M',	KeyMove,
		"Move to previous position in file."	    },
	END_KEYS
};
#endif

/*
 * Local Function Prototypes.
 */
void 	shutdown_prog 	 (void);
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
void 	ReadMisc 		 (void);
int 	GetFileName 	 (void);
int 	heading 		 (int);
void 	Update 			 (void);
void 	TagOther 		 (void);


/*
 * Main Processing Routine 
 */
int
main (
 int    argc,
 char*  argv [])
{
	/*
	 * Set the maximum number of items displayable. Each page can hold 16 items.
	 */
	tab_max_page = 1000;

	init_scr ();
	set_tty ();


	OpenDB ();

	ReadMisc ();

	swide ();

	heading (1);

	if (!GetFileName ())
	{
		shutdown_prog ();
		rset_tty ();
		return (EXIT_FAILURE);
	}

	FirstScan		=	TRUE;
	TagOther ();
	if (!tab_scan (scanFfwk))
		Update ();

	tab_close (scanFfwk, TRUE);

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence. 
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files 
 */
void
OpenDB (void)
{
	abc_dbopen ("data");

	abc_alias (inmr2, inmr);
	abc_alias (ffwk2, ffwk);

	open_rec (ffwk,  ffwk_list, FFWK_NO_FIELDS, "ffwk_id_no_2");
	open_rec (ffwk2, ffwk_list, FFWK_NO_FIELDS, "ffwk_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (ccmr);
	abc_fclose (ffwk);
	abc_fclose (ffwk2);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (ccmr);
	abc_dbclose ("data");
}

/*
 * Get common info from commom database file. 
 */
void
ReadMisc (void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	hhccHash = ccmr_rec.hhcc_hash;

	abc_selfield (ccmr, "ccmr_hhcc_hash");

	open_rec (comr, comr_list,COMR_NO_FIELDS,"comr_co_no");
	strcpy (comr_rec.co_no,comm_rec.co_no);
	cc = find_rec (comr, &comr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	abc_fclose (comr);
}

/*
 * Read "filename" that data was filed under on database	
 */
int
GetFileName (void)
{
	int		NoData	=	TRUE;
	
	LRP_LinesProcessed	=	0;
	LRP_NoRecords		=	0;

	sprintf (LRP_FileName, "TRN FROM %s/%s", comm_rec.est_no, comm_rec.cc_no);
	ffwk_rec.hhcc_hash = hhccHash;
	sprintf (ffwk_rec.filename,"%-14.14s",LRP_FileName);
	cc = find_rec (ffwk2,&ffwk_rec,COMPARISON,"r");
	if (cc)
		return (EXIT_SUCCESS);

	tab_open (scanFfwk, Header_keys, 4, 0, WK_DEPTH, FALSE);
	tab_add (scanFfwk, "#St |   Item Number   |       Item Description       |Available Sk|On Order Stk|Weeks Cover.| Qty Cover. |Min Require.| Sugg. Qty. ");


	ffwk_rec.hhcc_hash = hhccHash;
	sprintf (ffwk_rec.filename, "%-14.14s", LRP_FileName);
	sprintf (ffwk_rec.sort, "%-34.34s", " ");
	sprintf (ffwk_rec.crd_no, "      ");
	cc = find_rec (ffwk, &ffwk_rec, GTEQ, "r");
	while (!cc && ffwk_rec.hhcc_hash == hhccHash &&
	    		   !strcmp (ffwk_rec.filename,LRP_FileName))
	{
	    /*
	     * If we come across an ffwk	record which refers to an	
	     * inmr which has since been	deleted, IGNORE IT!!	    
	     */
		inmr_rec.hhbr_hash	=	ffwk_rec.hhbr_hash;
	    cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	    if (cc)
	    {
			cc = find_rec (ffwk, &ffwk_rec, NEXT,"r");
			continue;
	    }
		ccmr_rec.hhcc_hash	=	ffwk_rec.r_hhcc_hash;
		cc = find_rec ("ccmr", &ccmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (ffwk, &ffwk_rec, NEXT,"r");
			continue;
	    }
	    if (ffwk_rec.sugg_qty > 0.00 && ffwk_rec.hhit_hash == 0L)
		{
			tab_add 
			(
				scanFfwk, 
				" %-1.1s |%-16.16s |%-29.29s |%11.2f |%11.2f |%11.2f |%11.2f |%11.2f |%11.2f   %s%s %010ld",
				 (ffwk_rec.stat_flag [0] == 'U') ? "A" : "C",
				inmr_rec.item_no,
				inmr_rec.description,
				ffwk_cons [0],
				ffwk_cons [1],
				ffwk_cons [2],
				ffwk_cons [3],
				ffwk_cons [4],
				ffwk_cons [5],
				ffwk_rec.sort,
				ffwk_rec.crd_no,
				ccmr_rec.hhcc_hash
			);

			if (ffwk_rec.stat_flag [0] == 'U')
				LRP_LinesProcessed++;

			LRP_NoRecords++;
			NoData	=	FALSE;
		}
	    cc = find_rec (ffwk,&ffwk_rec,NEXT,"r");
	}
	if (NoData)
		tab_add (scanFfwk, "%s", ML ("Sorry but no data exists based on input selection"));

	return (EXIT_FAILURE);
}

/*
 * Heading concerns itself with clearing the screen,painting the  
 * screen overlay in preparation for input                       
 */
int
heading (
 int    scn)
{
	clear ();
	line_at (1,0,132);

	rv_pr (ML (mlLrpMess076),50,0,1);

	if (scn == 1)
	{
		print_at (22,0, ML (mlStdMess038), comm_rec.co_no,  comm_rec.co_short);
		print_at (22,40,ML (mlStdMess039), comm_rec.est_no, comm_rec.est_short);
		print_at (22,90,ML (mlStdMess099), comm_rec.cc_no,  comm_rec.cc_short);
	}
    return (EXIT_SUCCESS);
}

static	int
KeyConfirm (
 int        iUnused,
 KEY_TAB*   psUnused)
{
	char	rec_buffer [256];
	int		old_line;

	old_line = tab_tline (scanFfwk);
	cc = tab_get (scanFfwk, rec_buffer, EQUAL, old_line);
	if (!cc)
	{
		rec_buffer [1] = 'A';
		tab_update (scanFfwk, "%s", rec_buffer);
		cc = tab_get (scanFfwk, rec_buffer, NEXT, 0);
		if (cc)
			cc = tab_get (scanFfwk, rec_buffer, EQUAL, old_line);
	}
	old_line = tab_tline (scanFfwk);
	if ((old_line % WK_DEPTH) == 0)
		load_page (scanFfwk, FALSE);
	redraw_page (scanFfwk, TRUE);
    return (iUnused);
}

static int
KeyConfirmAll (
	int        iUnused,
	KEY_TAB*   psUnused)
{
	char	rec_buffer [256];
	int		old_line;

	old_line = tab_tline (scanFfwk);
	cc = tab_get (scanFfwk, rec_buffer, EQUAL, firstLine);
	while (!cc)
	{
		rec_buffer [1] = 'A';
		tab_update (scanFfwk, "%s", rec_buffer);
		cc = tab_get (scanFfwk, rec_buffer, NEXT, 0);
	}
	cc = tab_get (scanFfwk, rec_buffer, EQUAL, old_line);
	load_page (scanFfwk, FALSE);
	redraw_page (scanFfwk, TRUE);
    return (iUnused);
}

static int
KeyQuantity (
	int        iUnused,
	KEY_TAB*   psUnused)
{
	char	rec_buffer [256];
	float	new_val;
	int		old_line,
			scn_line;

	old_line = tab_tline (scanFfwk);
	cc = tab_get (scanFfwk, rec_buffer, EQUAL, old_line);
	if (!cc)
	{
		rec_buffer [1] = 'C';
		scn_line = tab_sline (scanFfwk);
		crsr_on ();
		new_val = getfloat (119, scn_line, "NNNNNNNN.NN");
		crsr_off ();
		if (dflt_used && last_char == '\r')
		{
			new_val = atof (rec_buffer + 105);
			print_at (scn_line,119,"%11.2f", new_val);
		}
		sprintf ((rec_buffer + 118), "%11.2f", new_val);

		tab_update (scanFfwk, "%s", rec_buffer);
		cc = tab_get (scanFfwk, rec_buffer, NEXT, 0);
		if (cc)
			cc = tab_get (scanFfwk, rec_buffer, EQUAL, old_line);
	}
	old_line = tab_tline (scanFfwk);
	if ((old_line % WK_DEPTH) == 0)
		load_page (scanFfwk, FALSE);
	redraw_page (scanFfwk, TRUE);
    return (iUnused);
}

static int
KeyReject (
	int        iUnused,
	KEY_TAB*   psUnused)
{
	char	rec_buffer [256];
	int		old_line;

	old_line = tab_tline (scanFfwk);
	cc = tab_get (scanFfwk, rec_buffer, EQUAL, old_line);
	if (!cc)
	{
		rec_buffer [1] = 'C';
		tab_update (scanFfwk, "%s", rec_buffer);
		cc = tab_get (scanFfwk, rec_buffer, NEXT, 0);
		if (cc)
			cc = tab_get (scanFfwk, rec_buffer, EQUAL, old_line);
	}
	old_line = tab_tline (scanFfwk);
	if ((old_line % WK_DEPTH) == 0)
		load_page (scanFfwk, FALSE);
	redraw_page (scanFfwk, TRUE);
    return (iUnused);
}

static int
KeyRejectAll (
	int        iUnused,
	KEY_TAB*   psUnused)
{
	char	rec_buffer [256];
	int		old_line;

	old_line = tab_tline (scanFfwk);
	cc = tab_get (scanFfwk, rec_buffer, EQUAL, firstLine);
	while (!cc)
	{
		rec_buffer [1] = 'C';
		tab_update (scanFfwk, "%s", rec_buffer);
		cc = tab_get (scanFfwk, rec_buffer, NEXT, 0);
	}
	cc = tab_get (scanFfwk, rec_buffer, EQUAL, old_line);
	load_page (scanFfwk, FALSE);
	redraw_page (scanFfwk, TRUE);
    return (iUnused);
}

/*
 * Update ffwk records 
 */
void
Update (void)
{
	char	rec_buffer [256];
	int		i	=	0;
	int		j	=	14;
	int		NoBars;

	sprintf (err_str, "%101.101s", " ");
	us_pr (err_str, 14,3,0);

	NoBars	=	LRP_NoRecords / 100;
	if (!NoBars)
		NoBars = 1;

	if (tab_get (scanFfwk, rec_buffer, FIRST, 0))
		return;

	ffwk_rec.hhcc_hash = hhccHash;
	sprintf (ffwk_rec.filename, "%-14.14s", LRP_FileName);
	sprintf (ffwk_rec.sort, "%-34.34s", rec_buffer + 132);
	sprintf (ffwk_rec.crd_no, "%-6.6s", rec_buffer + 166);
	cc = find_rec (ffwk, &ffwk_rec, EQUAL, "u");
	while
	 (
		!cc &&
		ffwk_rec.hhcc_hash == hhccHash &&
		!strcmp (ffwk_rec.filename,LRP_FileName)
	)
	{
	    /*
	     * If we come across an ffwk	record which refers to an	
	     * inmr which has since been	deleted, IGNORE IT!!	
	     */
		inmr_rec.hhbr_hash	=	ffwk_rec.hhbr_hash;
	    cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	    if (cc)
	    {
			if (tab_get (scanFfwk, rec_buffer, NEXT, 0))
				break;

			ffwk_rec.hhcc_hash = hhccHash;
			sprintf (ffwk_rec.filename, "%-14.14s", LRP_FileName);
			sprintf (ffwk_rec.sort, "%-34.34s", rec_buffer + 132);
			sprintf (ffwk_rec.crd_no, "%-6.6s", rec_buffer + 166);
			cc = find_rec (ffwk, &ffwk_rec, EQUAL, "u");
			if (cc)
				continue;
	    }
	    if (ffwk_rec.sugg_qty > 0.00 && ffwk_rec.hhit_hash == 0)
	    { 
			i++;
			if ((i % NoBars) == 0)
				us_pr (" ", j++,3,1);
				
			switch (rec_buffer [1])
			{
			case	'A':
				ffwk_rec.order_qty = atof (rec_buffer + 118);
				strcpy (ffwk_rec.stat_flag, "U");
				cc = abc_update (ffwk, &ffwk_rec);
				if (cc)
					file_err (cc, ffwk, "DBUPDATE");

		    break;

			default:
				strcpy (ffwk_rec.stat_flag, " ");
				cc = abc_update (ffwk, &ffwk_rec);
				if (cc)
					file_err (cc, ffwk, "DBUPDATE");
		    	break;
			}
			abc_unlock (ffwk);
	    }
	    if (tab_get (scanFfwk, rec_buffer, NEXT, 0))
			break;

	    ffwk_rec.hhcc_hash = hhccHash;
	    sprintf (ffwk_rec.filename, "%-14.14s", LRP_FileName);
	    sprintf (ffwk_rec.sort, "%-34.34s", rec_buffer + 132);
		sprintf (ffwk_rec.crd_no, "%-6.6s", rec_buffer + 166);
	    cc = find_rec (ffwk, &ffwk_rec, EQUAL, "u");
	}
	abc_unlock (ffwk);
}

static	int
KeyStockDisplay (
	int        iUnused,
	KEY_TAB*   psUnused)
{
	char	rec_buffer [256];
	int		old_line;

	old_line = tab_tline (scanFfwk);
	cc = tab_get (scanFfwk, rec_buffer, EQUAL, old_line);
	if (!cc)
	{
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		sprintf (inmr_rec.item_no, "%-16.16s", rec_buffer + 4);
		cc = find_rec (inmr2, &inmr_rec, EQUAL, "r");
		if (!cc)
		{
			sprintf (err_str, "sk_alldisp %s %010ld",
						ffwk_rec.source,inmr_rec.hhbr_hash); 
			sys_exec (err_str);
		}
		swide ();
		cc = tab_get (scanFfwk, rec_buffer, EQUAL, old_line);
	}
	heading (1);
	load_page (scanFfwk, FALSE);
	redraw_table (scanFfwk);
    return (iUnused);
}

static int
KeyLrpDisplay (
	int        iUnused,
	KEY_TAB*   psUnused)
{
	char	rec_buffer [256];
	int		old_line;

	old_line = tab_tline (scanFfwk);
	cc = tab_get (scanFfwk, rec_buffer, EQUAL, old_line);
	if (!cc)
	{
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		sprintf (inmr_rec.item_no, "%-16.16s", rec_buffer + 4);
		cc = find_rec (inmr2, &inmr_rec, EQUAL, "r");
		if (!cc)
		{
			sprintf (err_str, "lrp_display %010ld 0", inmr_rec.hhbr_hash); 
			sys_exec (err_str);
		}
		swide ();
		cc = tab_get (scanFfwk, rec_buffer, EQUAL, old_line);
	}
	heading (1);
	load_page (scanFfwk, FALSE);
	redraw_table (scanFfwk);
    return (iUnused);
}

/*
 * OK selection function. 
 */
void
TagOther (void)
{
	char	rec_buffer [300];
	int		scn_line;

	if (FirstScan)
		cc = tab_get (scanFfwk, rec_buffer, EQUAL, 0);
	else
	{
		scn_line	=	tab_tline (scanFfwk);
		cc = tab_get (scanFfwk, rec_buffer, EQUAL, scn_line);
		LRP_LinesProcessed		=	scn_line;
	}
	FirstScan	=	FALSE;

	if (!cc)
	{
		scn_line = tab_sline (scanFfwk);
		ccmr_rec.hhcc_hash	=	atol (rec_buffer + 173);
		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (!cc)
		{
			file_err (cc, ccmr, "DBFIND");

			box (12,2,103,1);
			sprintf (err_str," Supply to Branch : %-2.2s / Warehouse : %2.2s / Name : %-40.40s (%-9.9s)",
					ccmr_rec.est_no,
					ccmr_rec.cc_no,
					ccmr_rec.name,
					ccmr_rec.acronym);
			us_pr (err_str, 14,3,1);
		}
	}
}

static int
KeyMove (
	int        iUnused,
	KEY_TAB*   psUnused)
{
	char	rec_buffer [256];
	cc = tab_get (scanFfwk, rec_buffer, EQUAL, LRP_LinesProcessed);
    return (iUnused);
}
