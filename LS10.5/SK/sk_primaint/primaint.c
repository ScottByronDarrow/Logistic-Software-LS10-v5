/*====================================================================|
|  Program Name  : (sk_primaint.c)                                    |
|  Program Desc  : (Inventory Price Book Maintenance.          )      |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow  | Date Written  : 17/03/88         |
|---------------------------------------------------------------------|
|  Date Modified : (17/03/88)      | Modified  by  : Roger Gibbison.  |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: primaint.c,v $
| Revision 5.9  2002/08/14 04:36:22  scott
| Updated for warning
|
| Revision 5.8  2002/07/24 08:39:16  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.7  2002/06/20 07:11:07  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.6  2001/12/12 10:01:29  robert
| LS10.5-GUI update
|
| Revision 5.5  2001/11/28 02:55:24  scott
| Updated for field "length" on inpl
|
| Revision 5.4  2001/11/26 07:15:33  cha
| Updated to correct some mistype struct member.
|
| Revision 5.3  2001/11/15 01:11:51  scott
| Updated for length define.
|
| Revision 5.2  2001/08/09 09:19:34  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:34  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:17:04  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:38:25  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:55  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:36  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  2000/07/10 01:53:41  scott
| Updated to replace "@ (" with "@(" to ensure psl_what works correctly
|
| Revision 1.8  2000/06/13 05:03:15  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.7  1999/11/03 07:32:22  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.6  1999/10/26 03:37:27  scott
| Updated for missing language translations
|
| Revision 1.5  1999/10/12 21:20:36  scott
| Updated by Gerry from ansi project.
|
| Revision 1.4  1999/10/08 05:32:45  scott
| First Pass checkin by Scott.
|
| Revision 1.3  1999/06/20 05:20:28  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
char	*PNAME = "$RCSfile: primaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_primaint/primaint.c,v 5.9 2002/08/14 04:36:22 scott Exp $";
#define	CCMAIN
#define	MAXLINES	50
#define	MAXWIDTH	140
#define	MAXSTR		100
#define	MAXSCNS		11
#define	MAX_COL		7
#define	MAX_COLUMNS	20
#define MX_WDTH		132
#include 	<pslscr.h>
#include	<ring_menu.h>
#include	<ml_std_mess.h>

int		max_columns;
int		curr_col;
int		type_page;
int		new_page;
int		new_sub;

struct	storeRec {
	long	hhbrHash [MAX_COL];
} store [MAXLINES];

#define	BLANK_PAGE	 (local_rec.blank_page [0] == 'Y')
#define	DEFAULT_PAGE	 (local_rec.this_page == 0)

#define	PAGE_SCN	1
#define	INDEX_SCN	2
#define	COL_SCN		3
#define	TITLE_SCN	4
#define	A_TYPE		5
#define	B_TYPE		6
#define	C_TYPE		7
#define	D_TYPE		8
#define	COPY_SCN	9

#define	min(a,b)	 (((a) < (b)) ? (a) : (b))
#define	valid_inpi() (!cc && inpi_rec.hhph_hash == inph_rec.hhph_hash)
#define	valid_inpl() (!cc && inpl_rec.hhpr_hash == inps_rec.hhpr_hash)
#define	valid_inpc() (!cc && inpc_rec.hhpr_hash == inps_rec.hhpr_hash)
#define	valid_inpt() (!cc && inpt_rec.hhpr_hash == inps_rec.hhpr_hash)

extern	int		PV_tlines;

struct	{
	int		c_start;
	char	c_type [2];
	int		c_width;
	char	c_head [2] [46];
	char	c_format [2];
} col_store [MAX_COLUMNS], tmp_col_store;

struct	{
	char	t_type [2];
	char	t_title [97];
} title_store [4];

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inphRecord	inph_rec;
struct inphRecord	sinph_rec;
struct inpiRecord	inpi_rec;
struct inpsRecord	inps_rec;
struct inpsRecord	sinps_rec;
struct inplRecord	inpl_rec;
struct inptRecord	inpt_rec;
struct inpcRecord	inpc_rec;

long	*inpl_hhbr_hash	=	&inpl_rec.hhbr_hash_1;

/*===========================
| Column Formats			|
===========================*/
struct	{
	char	*_type;
	char	*_desc;
} colformats [] = {
	{"C",	"Centre"},
	{"L",	"Left Justify"},
	{"R",	"Right Justify"},
	{"",	""},
};

#define	CFTYPE	colformats [i]._type
#define	CFDESC	colformats [i]._desc
/*===========================
| Column Types				|
===========================*/
struct	{
	char	*_type;
	char	*_desc;
} coltypes [] = {
	{"I",	"Item Number"},
	{"P",	"Pack Size"},
	{"L",	"Length"},
	{"D",	"Description"},
	{"O",	"Other (padding)"},
	{"$",	"Price"},
	{"C",	"Cost"},
	{"Q",	"Qty Brk"},
	{"",	""},
};

#define	CTYPE	coltypes [i]._type
#define	CDESC	coltypes [i]._desc
/*===========================
| Page Formats				|
===========================*/
struct	{
	char	*_format;
	int		_type;
	char	*_desc;
} formats [] = {
	{"A",	A_TYPE,	"A - Type Page"},
	{"B",	B_TYPE,	"B - Type Page"},
	{"C",	C_TYPE,	"C - Type Page"},
	{"D",	D_TYPE,	"D - Type Page"},
	{"",	0,	""},
};

#define	FFORMAT	formats [i]._format
#define	FTYPE	formats [i]._type
#define	FDESC	formats [i]._desc

/*=======================
| Callback Declarations |
=======================*/
static	int	modify_page (void);
static	int	copy_page (void);
static	int	delete_page (void);
static	int	redr_page (void);
static	int	save_page (void);
/* ========================== */
int		show_column (void);
int		insert_column (void);
int		append_column (void);
int		modify_column (void);
int		delete_column (void);
/* ========================== */
int		process_cols (void);

#ifndef GVISION
menu_type	_main_menu [] = {
	{ " Modify Sub Page ",	"Modify Price Book Sub Page",	modify_page,
		"Mm",	0,	ALL,	},
	{ " Copy Sub Page ",	"Copy Price Book Sub Page",	copy_page,
		"Cc",	0,	ALL,	},
	{ " Delete Sub Page ",	"Delete Price Book Sub Page",	delete_page,
		"Dd",	0,	ALL,	},
	{ " Redraw Screen ",	"Redraw Screen",		redr_page,
		"Rr",	FN3,		},
	{ " Save Sub Page ",	"Save Current Sub Page",	save_page,
		"Ss",	FN16,	ALL,	},
	{ " Quit ",		"Quit",				_no_option,
		"Qq",	FN1,	ALL,	},
	{ "",
					},
};


menu_type	_col_menu [] = {
	{ " Show ",	"Redisplay Columns",			show_column,
		"Ss",	FN3,		},
	{ " Insert ",	"Insert Column Before Selected Column",	insert_column,
		"Ii",			},
	{ " Append ",	"Append Column After Selected Column",	append_column,
		"Aa",			},
	{ " Modify ",	"Modify Selected Column",		modify_column,
		"Mm",			},
	{ " Delete ",	"Delete Selected Column",		delete_column,
		"Dd",			},
	{ " Quit ",	"Return",				_no_option,
		"Qq",	FN16,	ALL,	},
	{ "",
					},
};
#else
menu_type	_main_menu [] = {
	{0, " Modify Sub Page ",	"Modify Price Book Sub Page",	modify_page },
	{0, " Copy Sub Page ",	"Copy Price Book Sub Page",	copy_page },
	{0, " Delete Sub Page ",	"Delete Price Book Sub Page",	delete_page },
	{0, " Redraw Screen ",	"Redraw Screen",		redr_page },
	{0, " Save Sub Page ",	"Save Current Sub Page",	save_page },
	{0, " Quit ",		"Quit",				_no_option },
	{0, ""}
};


menu_type	_col_menu [] = {
	{0, " Show ",	"Redisplay Columns",			show_column },
	{0, " Insert ",	"Insert Column Before Selected Column",	insert_column },
	{0, " Append ",	"Append Column After Selected Column",	append_column },
	{0, " Modify ",	"Modify Selected Column",		modify_column },
	{0, " Delete ",	"Delete Selected Column",		delete_column },
	{0, " Quit ",	"Return",				_no_option },
	{0, ""}
};
#endif


struct	{
	char	*_desc;			/* edit_all () description	*/
	char	*_heading;		/* screen heading		*/
	int		_edit_all;	/* TRUE iff using edit ()not fn	*/
	int		 (* _funct) (void);/* function for edit		*/
} screens [] = {
	{ "Page Screen",	" Page Selection ",
		TRUE,		     },
	{ "Index Screen",	" Page Index Reference ",
		TRUE,		     },
	{ "Column Screen",	" Sub Page Column Format ",
		FALSE,	process_cols },
	{ "Title Screen",	" Sub Page Title ",
		TRUE,		     },
	{ "Price Book Screen",	" Price Book (A) Sub Page ",
		TRUE,		     },
	{ "Price Book Screen",	" Price Book (B) Sub Page ",
		TRUE,		     },
	{ "Price Book Screen",	" Price Book (C) Sub Page ",
		TRUE,		     },
	{ "Price Book Screen",	" Price Book (D) Sub Page ",
		TRUE,		     },
	{ "New Page Screen",	" Destination Sub Page ",
		TRUE,		     },
	{ "",			"",
		TRUE		     },
};

#define	DESCR		screens [i]._desc
#define	HEADING		screens [i]._heading
#define	EDIT_ALL	screens [i]._edit_all
#define	FUNCT		screens [i]._funct

/*=========================== 
| Local & Screen Structures.|
===========================*/
int		MODIFY;

struct {
	char	dummy [11];
	char	format [21];
	int		this_page;
	int		copy_page;
	char	blank_page [4];
	char	sub_page [2];
	char	copy_sub [2];
	char	title_type [2];
	char	title_text [97];
	char	action [2];
	char	col_type [21];
	int		col_width;
	char	col_head [2] [46];
	char	col_format [21];
	char	bolt_length [9];
	char	item_desc [41];
	char	pack_size [5];
	double	price_per;
	double	prc [3];
	char	item_no [MAX_COL] [17];
} local_rec;

static	struct	var	vars []	={	

	{PAGE_SCN, LIN, "this_page", 4, 16, INTTYPE, 
		"NNNNN", "          ", 
		" ", "", "Page Number ", " ", 
		NE, NO, JUSTRIGHT, "", "", (char *)&local_rec.this_page}, 
	{PAGE_SCN, LIN, "blank_page", 4, 40, CHARTYPE, 
		"U", "          ", 
		" ", "No ", "Blank Page ", " Whether Page is Place Holder Only ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.blank_page}, 
	{PAGE_SCN, LIN, "sub_page", 6, 16, CHARTYPE, 
		"U", "          ", 
		" ", "", "Sub Page", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.sub_page}, 
	{PAGE_SCN, LIN, "page_format", 7, 16, CHARTYPE, 
		"U", "          ", 
		" ", "D", "Page Format ", " A / B / C / D ", 
		YES, NO, JUSTRIGHT, "ABCDSFI", "", local_rec.format}, 
	{INDEX_SCN, TAB, "reference", MAXLINES, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "           I n d e x   R e f e r e n c e          ", " ", 
		YES, NO, JUSTLEFT, "", "", inpi_rec.reference}, 
	{COL_SCN, LIN, "col_type", 15, 20, CHARTYPE, 
		"U", "          ", 
		" ", "O", "Column Type", " Item Number,  Pack Size,  Length,  Description,  Other,  $ Price, Cost, Qty Brk", 
		YES, NO, JUSTLEFT, "IPLOD$CQ", "", local_rec.col_type}, 
	{COL_SCN, LIN, "col_width", 16, 20, INTTYPE, 
		"NN", "          ", 
		" ", "10", "Column Width", " ", 
		YES, NO, JUSTRIGHT, "2", "45", (char *)&local_rec.col_width}, 
	{COL_SCN, LIN, "col_head_1", 17, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Column Heading ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.col_head [0]}, 
	{COL_SCN, LIN, "col_head_2", 18, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "               ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.col_head [1]}, 
	{COL_SCN, LIN, "col_format", 19, 20, CHARTYPE, 
		"U", "          ", 
		" ", "C", "Column Format", " Centre, Left Justify, Right Justify ", 
		YES, NO, JUSTLEFT, "CLR", "", local_rec.col_format}, 
	{TITLE_SCN, TAB, "title_type", 4, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", " ", "C)entred   E)xpanded   B (oth - (Centred & Expanded)   N (either  D)elete Line  I (nsert Line ", 
		YES, NO, JUSTLEFT, "CEBNDI", "", local_rec.title_type}, 
	{TITLE_SCN, TAB, "title_text", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "                                       T i t l e   L i n e s                                    ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.title_text}, 
	{A_TYPE, TAB, "length", MAXLINES, 0, CHARTYPE, 
		"NUNNNNNN", "          ", 
		" ", " ", " Length ", " \\I - Inserts  \\D - Deletes ", 
		YES, NO, JUSTLEFT, " ID0123456789-./\\", "", local_rec.bolt_length}, 
	{A_TYPE, TAB, "column_1", 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number.  ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no [0]}, 
	{A_TYPE, TAB, "column_2", 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number.  ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no [1]}, 
	{A_TYPE, TAB, "column_3", 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number.  ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no [2]}, 
	{A_TYPE, TAB, "column_4", 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number.  ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no [3]}, 
	{A_TYPE, TAB, "column_5", 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number.  ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no [4]}, 
	{A_TYPE, TAB, "column_6", 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number.  ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no [5]}, 
	{C_TYPE, TAB, "column_1", MAXLINES, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number.  ", " \\I - Inserts  \\D - Deletes ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no [0]}, 
	{C_TYPE, TAB, "item_desc", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "         D e s c r i p t i o n.         ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.item_desc}, 
	{C_TYPE, TAB, "pack_size", 0, 1, CHARTYPE, 
		"UUUU", "          ", 
		" ", " ", " Pack ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.pack_size}, 
	{C_TYPE, TAB, "price_per", 0, 4, MONEYTYPE, 
		"NNNNNNN.NN", "          ", 
		" ", "0", " Price / Per. ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.price_per}, 
	{B_TYPE, TAB, "length", MAXLINES, 0, CHARTYPE, 
		"NUNNNNNN", "          ", 
		" ", " ", " Length ", " \\I - Inserts  \\D - Deletes ", 
		YES, NO, JUSTLEFT, " ID0123456789-./\\", "", local_rec.bolt_length}, 
	{B_TYPE, TAB, "column_1", 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number.  ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no [0]}, 
	{B_TYPE, TAB, "column_2", 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number.  ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no [1]}, 
	{B_TYPE, TAB, "column_3", 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number.  ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no [2]}, 
	{B_TYPE, TAB, "column_4", 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number.  ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no [3]}, 
	{B_TYPE, TAB, "column_5", 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number.  ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no [4]}, 
	{B_TYPE, TAB, "column_6", 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number.  ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no [5]}, 
	{B_TYPE, TAB, "column_7", 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number.  ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no [6]}, 
	{D_TYPE, TAB, "D_code", MAXLINES, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number.  ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no [0]}, 
	{D_TYPE, TAB, "D_desc", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "  Item Description.          ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.item_desc}, 
	{COPY_SCN, LIN, "this_page", 4, 16, INTTYPE, 
		"NNNNN", "          ", 
		" ", "", "Page Number ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.this_page}, 
	{COPY_SCN, LIN, "blank_page", 5, 40, CHARTYPE, 
		"U", "          ", 
		" ", "No ", "Blank Page ", " ", 
		NA, NO, JUSTRIGHT, "", "", local_rec.blank_page}, 
	{COPY_SCN, LIN, "sub_page", 6, 16, CHARTYPE, 
		"U", "          ", 
		" ", "", "Sub Page", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.sub_page}, 
	{COPY_SCN, LIN, "page_format", 7, 16, CHARTYPE, 
		"U", "          ", 
		" ", "", "Page Format ", " ", 
		NA, NO, JUSTRIGHT, "", "", local_rec.format}, 
	{COPY_SCN, LIN, "copy_page", 9, 16, INTTYPE, 
		"NNNNN", "          ", 
		" ", "", "Page Number ", " New Page Number ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.copy_page}, 
	{COPY_SCN, LIN, "copy_sub", 10, 16, CHARTYPE, 
		"U", "          ", 
		" ", "", "Sub Page", " New Sub Page ", 
		YES, NO, JUSTLEFT, "", "", local_rec.copy_sub}, 
	{0, LIN, "dummy", 0, 0, CHARTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

char	CURR_CODE [4];

#include	<FindBasePrice.h>
/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void delete_inpl (int start_line);
void delete_inpc (int start_line);
void delete_inpt (int start_line);
void delete_inps (void);
void delete_inpi (void);
void delete_inph (void);
int  get_ctype (char *form);
int  get_cformat (char *form);
int  get_format (char *form);
int  spec_valid (int field);
void setup_edit (void);
int  delete_line (void);
int  insert_line (void);
int  delete_title (void);
int  insert_title (void);
void load_title (void);
void load_columns (void);
void dflt_columns (void);
void std_dflt (void);
void save_col (int ccol, int cc_start, char *cc_type, int cc_width, char *cc_head0, char *cc_head1, char *cc_format);
void load_index (void);
void load_book (void);
void title_display (int offset);
void cols_display (int max_col, int offset);
int  choose_col (int max_col);
void prnt_col (int ccol, int rv_flag, int offset);
void col_load (int ccol);
void disp_col (void);
void insert_col (int ccol, int max_col);
void append_col (int ccol, int max_col);
int  delete_col (int ccol, int max_col);
void update (int page, char *sub_page);
void update_inph (int page);
void update_inpi (void);
void update_inps (char *sub_page);
void update_inpc (void);
void update_inpt (void);
void update_inpl (void);
void show_inps (void);
void show_inph (char *key_val);
int  srch_price (char *key_val);
int  save_inph (void);
int  heading (int scn);


/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc,
 char * argv [])
{
	int		i;
	SETUP_SCR (vars);


	sprintf (CURR_CODE, "%-3.3s", get_env ("CURR_CODE"));

	/*-------------------------------
	| initialise terminal etc		|
	-------------------------------*/
	init_scr ();
	set_tty (); 
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (A_TYPE, store, sizeof (struct storeRec));
	SetSortArray (B_TYPE, store, sizeof (struct storeRec));
	SetSortArray (C_TYPE, store, sizeof (struct storeRec));
	SetSortArray (D_TYPE, store, sizeof (struct storeRec));
#endif
	/*-------------------------------
	| initialise edit structure		|
	-------------------------------*/
	for (i = 0;strlen (DESCR);i++)
	{
		tab_data [i]._desc = DESCR;
		tab_data [i]._actn = FUNCT;
		tab_data [i]._win = EDIT_ALL;
	}
	/*---------------------------
	| open database etc			|
	---------------------------*/
	OpenDB ();
	swide ();
	while (prog_exit == 0) 
	{
		abc_unlock ("inph");
		abc_unlock ("inps");
		new_page 	= FALSE;
		new_sub 	= FALSE;
		entry_exit 	= FALSE;
		restart 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		lcount [INDEX_SCN] 	= 0;
		lcount [TITLE_SCN] 	= 0;
		lcount [A_TYPE] 		= 0;
		lcount [C_TYPE] 		= 0;
		lcount [D_TYPE] 		= 0;

		max_columns = 0;
		init_vars (PAGE_SCN);	
		/*-------------------------------
		| enter price book page			|
		-------------------------------*/
		heading (PAGE_SCN);
		entry (PAGE_SCN);
		if (prog_exit || restart)
			continue;
		
		/*---------------------------------------
		| allow modify, copy, deletion of page	|
		---------------------------------------*/
		if (!new_sub)
		{
			scn_display (PAGE_SCN);
#ifndef GVISION
			run_menu (_main_menu,"",19);
#else
			run_menu (NULL, _main_menu);
#endif
		}
		else
		{
			/*---------------------------
			| index screen				|
			---------------------------*/
			if (!DEFAULT_PAGE && lcount [INDEX_SCN] == 0)
			{
				heading (INDEX_SCN);
				entry (INDEX_SCN);
				if (restart)
					continue;
			}
			/*---------------------------
			| columns screen			|
			---------------------------*/
			if (!BLANK_PAGE && max_columns == 0)
			{
				process_cols ();
				if (restart)
					continue;
			}
			/*---------------------------
			| title screen				|
			---------------------------*/
			if (!BLANK_PAGE && lcount [TITLE_SCN] == 0)
			{
				heading (TITLE_SCN);
				entry (TITLE_SCN);
				if (prog_exit || restart)
					continue;
			}
			/*---------------------------
			| format screen				|
			---------------------------*/
			if (!BLANK_PAGE && lcount [type_page] == 0)
			{
				heading (type_page);
				entry (type_page);
				if (prog_exit || restart)
					continue;
			}
			/*---------------------------
			| edit screen				|
			---------------------------*/
			setup_edit ();
			edit_all ();
			if (restart)
				continue;
			update (local_rec.this_page,local_rec.sub_page);
		}
	}
	shutdown_prog ();	
    return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB (
 void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	abc_alias ("sinph","inph");
	abc_alias ("sinps","inps");
	open_rec ("ccmr",ccmr_list,CCMR_NO_FIELDS,"ccmr_id_no");
	open_rec ("inmr",inmr_list,INMR_NO_FIELDS,"inmr_id_no");
	open_rec ("inph",inph_list,INPH_NO_FIELDS,"inph_id_no");
	open_rec ("sinph",inph_list,INPH_NO_FIELDS,"inph_id_no");
	open_rec ("sinps",inps_list,INPS_NO_FIELDS,"inps_id_no");
	open_rec ("inps",inps_list,INPS_NO_FIELDS,"inps_id_no");
	open_rec ("inpl",inpl_list,INPL_NO_FIELDS,"inpl_id_no");
	open_rec ("inpt",inpt_list,INPT_NO_FIELDS,"inpt_id_no");
	open_rec ("inpc",inpc_list,INPC_NO_FIELDS,"inpc_id_no");
	open_rec ("inpi",inpi_list,INPI_NO_FIELDS,"inpi_hhph_hash");
	OpenBasePrice ();
}

void
CloseDB (
 void)
{
	abc_fclose ("inmr");
	abc_fclose ("ccmr");
	abc_fclose ("sinph");
	abc_fclose ("inph");
	abc_fclose ("sinps");
	abc_fclose ("inps");
	abc_fclose ("inpl");
	abc_fclose ("inpt");
	abc_fclose ("inpc");
	abc_fclose ("inpi");
	CloseBasePrice ();
	SearchFindClose ();
	abc_dbclose ("data");
}

static	int
modify_page (
 void)
{
	/*---------------------------
	| edit screen				|
	---------------------------*/
	setup_edit ();
	edit_all ();
	if (restart)
		return (EXIT_SUCCESS);
	update (local_rec.this_page,local_rec.sub_page);
    return (EXIT_SUCCESS);
}

static	int
copy_page (
 void)
{
	/*-------------------------------
	| input new page / sub page		|
	-------------------------------*/
	if (BLANK_PAGE)
	{
		print_mess (ML (" Cannot Copy Blank Page "));
		sleep (sleepTime);
		return (EXIT_SUCCESS);
	}
	local_rec.copy_page = 0;
	strcpy (local_rec.copy_sub," ");
	entry_exit 	= FALSE;
	restart 	= FALSE;
	edit_exit 	= FALSE;
	prog_exit 	= FALSE;
	init_ok 	= FALSE;

	/*-----------------------------------
	| enter destination page & sub page	|
	-----------------------------------*/
	heading (COPY_SCN);
	scn_display (COPY_SCN);
	entry (COPY_SCN);
	if (restart)
		return (EXIT_SUCCESS);
	/*-----------------------------------
	| edit destination page & sub page	|
	-----------------------------------*/
	setup_edit ();
	edit_all ();
	if (restart)
		return (EXIT_SUCCESS);
	update (local_rec.copy_page,local_rec.copy_sub);

    return (EXIT_SUCCESS);
}

static	int
delete_page (
 void)
{
	int		i;
	/*-------------------------------
	| prompt for confirmation		|
	-------------------------------*/
	i = prmptmsg (" Confirm Deletion Of Sub Page ? ","YyNn",2,2);
	move (0,2);
	cl_line ();
	/*---------------------------
	| abort deletion			|
	---------------------------*/
	if (i == 'N' || i == 'n')
		return (EXIT_SUCCESS);
	clear ();
	printf ("\n\r\n\rDeleting Price Book Lines ... ");
	delete_inpl (0);
	printf ("\n\r\n\rDeleting Price Book Columns ... ");
	delete_inpc (0);
	printf ("\n\r\n\rDeleting Price Book Titles ... ");
	delete_inpt (0);
	printf ("\n\r\n\rDeleting Price Book Sub Page ... ");
	delete_inps ();
	if (BLANK_PAGE)
	{
		printf ("\n\r\n\rDeleting Price Index Page ... ");
		delete_inpi ();
		printf ("\n\r\n\rDeleting Price Book Page ... ");
		delete_inph ();
	}
    return (EXIT_SUCCESS);
}

void
delete_inpl (
 int start_line)
{
	scn_set (type_page);
	/*---------------------------------------
	| delete extra lines from price book	|
	---------------------------------------*/
	inpl_rec.hhpr_hash = inps_rec.hhpr_hash;
	inpl_rec.line_no = start_line;
	cc = find_rec ("inpl",&inpl_rec,GTEQ,"w");
	while (valid_inpl ())
	{
		putchar ('D');
		fflush (stdout);
		abc_unlock ("inpl");
		cc = abc_delete ("inpl");
		if (cc)
			file_err (cc, "inpl", "DBDELETE");

		/*---------------------------------------
		| find next line to delete		|
		---------------------------------------*/
		inpl_rec.hhpr_hash = inps_rec.hhpr_hash;
		inpl_rec.line_no = start_line;
		cc = find_rec ("inpl",&inpl_rec,GTEQ,"w");
	}
}

void
delete_inpc (
 int start_line)
{
	/*-------------------------------
	| delete extra columns			|
	-------------------------------*/
	inpc_rec.hhpr_hash = inps_rec.hhpr_hash;
	inpc_rec.col_no = start_line;
	cc = find_rec ("inpc",&inpc_rec,GTEQ,"w");
	while (valid_inpc ())
	{
		abc_unlock ("inpc");
		putchar ('D');
		fflush (stdout);
		cc = abc_delete ("inpc");
		if (cc)
			file_err (cc, "inpc",  "DBDELETE)");

		/*---------------------------
		| find next column			|
		---------------------------*/
		inpc_rec.hhpr_hash = inps_rec.hhpr_hash;
		inpc_rec.col_no = start_line;
		cc = find_rec ("inpc",&inpc_rec,GTEQ,"w");
	}
}

void
delete_inpt (
 int start_line)
{
	/*-------------------------------
	| delete extra title lines		|
	-------------------------------*/
	inpt_rec.hhpr_hash = inps_rec.hhpr_hash;
	inpt_rec.line_no = start_line;
	cc = find_rec ("inpt",&inpt_rec,GTEQ,"w");
	while (valid_inpt ())
	{
		abc_unlock ("inpt");
		putchar ('D');
		fflush (stdout);
		cc = abc_delete ("inpt");
		if (cc)
			file_err (cc, "inpt", "DBDELETE");
		inpt_rec.hhpr_hash = inps_rec.hhpr_hash;
		inpt_rec.line_no = start_line;
		cc = find_rec ("inpt",&inpt_rec,GTEQ,"w");
	}
}

void
delete_inps (
 void)
{
	abc_unlock ("inps");
	putchar ('D');
	fflush (stdout);
	cc = abc_delete ("inps");
	if (cc)
		file_err (cc, "inps", "DBDELETE");
}

void
delete_inpi (
 void)
{
	cc = find_hash ("inpi",&inpi_rec,GTEQ,"u",inph_rec.hhph_hash);
	while (valid_inpi ())
	{
		/*-----------------------
		| delete				|
		-----------------------*/
		putchar ('D');
		fflush (stdout);
		abc_unlock ("inpi");
		cc = abc_delete ("inpi");
		if (cc)
			file_err (cc, "inpi", "DBDELETE");

		cc = find_hash ("inpi",&inpi_rec,GTEQ,"u",inph_rec.hhph_hash);
	}
}

void
delete_inph (
 void)
{
	abc_unlock ("inph");
	putchar ('D');
	fflush (stdout);
	cc = abc_delete ("inph");
	if (cc)
		file_err (cc, "inph", "DBDELETE");
}

static	int
redr_page (
 void)
{
	heading (PAGE_SCN);
	scn_display (PAGE_SCN);
    return (EXIT_SUCCESS);
}

static	int
save_page (
 void)
{
	update (local_rec.this_page,local_rec.sub_page);
    return (EXIT_SUCCESS);
}

int
process_cols (
 void)
{
	heading (COL_SCN);
	curr_col = 0;
	cols_display (max_columns,0);
	disp_col ();
#ifndef GVISION
	run_menu (_col_menu,"",6);
#else
	run_menu (NULL, _col_menu);
#endif
    return (EXIT_SUCCESS);
}

int
show_column (
 void)
{
	cols_display (max_columns,0);
    return (EXIT_SUCCESS);
}

int
insert_column (
 void)
{
	/*-----------------------------------
	| check for number of columns		|
	-----------------------------------*/
	if (max_columns == MAX_COLUMNS - 1)
	{
		print_mess (ML (" Cannot Insert More Columns "));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	/*---------------------------
	| not first column			|
	---------------------------*/
	if (max_columns != 0)
		curr_col = choose_col (max_columns);
	else
		curr_col = 0;

	/*-------------------------------
	| entry on current column		|
	-------------------------------*/
	restart = FALSE;
	scn_write (COL_SCN);
	entry (COL_SCN);
	if (!restart)
	{
		/*---------------
		| add column	|
		---------------*/
		max_columns++;
		insert_col (curr_col,max_columns);

	}
	cols_display (max_columns,0);
	return (EXIT_SUCCESS);
}

int
append_column (
 void)
{
	/*-----------------------------------
	| check for number of columns		|
	-----------------------------------*/
	if (max_columns == MAX_COLUMNS - 1)
	{
		print_mess (ML (" Cannot Append More Columns "));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	/*---------------------------
	| not first column			|
	---------------------------*/
	if (max_columns != 0)
	{
		curr_col = choose_col (max_columns);
		curr_col++;
	}
	else
		curr_col = 0;

	/*-------------------------------
	| entry on current column		|
	-------------------------------*/
	restart = FALSE;
	scn_write (COL_SCN);
	entry (COL_SCN);
	if (!restart)
	{
		/*---------------
		| add column	|
		---------------*/
		append_col (curr_col,max_columns);
		max_columns++;
	}
	cols_display (max_columns,0);
	return (EXIT_SUCCESS);
}

int
modify_column (
 void)
{

	/*-----------------------------------
	| check there are columns to modify	|
	-----------------------------------*/
	if (max_columns != 0)
		curr_col = choose_col (max_columns);
	else
	{
		print_mess (ML (" No Columns To Modify "));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	/*-------------------------------
	| load & edit column			|
	-------------------------------*/
	restart = FALSE;
	col_load (curr_col);
	scn_write (COL_SCN);
	scn_display (COL_SCN);
	MODIFY = TRUE;
	edit (COL_SCN);
	
	scn_write (COL_SCN);
	entry_exit = TRUE;
	cols_display (max_columns,0);

	return (EXIT_SUCCESS);
}

int
delete_column (
 void)
{
	/*-----------------------------------
	| check there are columns to delete	|
	-----------------------------------*/
	if (max_columns != 0)
		curr_col = choose_col (max_columns);
	else
	{
		print_mess (ML (" No Columns To Delete "));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	/*-------------------------------
	| load & delete column			|
	-------------------------------*/
	col_load (curr_col);
	if (delete_col (curr_col,max_columns))
		max_columns--;
	entry_exit = TRUE;
	cols_display (max_columns,0);
	return (EXIT_SUCCESS);
}

int
get_ctype (
 char *form)
{
	int		i;
	/*---------------------------
	| find col_type				|
	---------------------------*/
	for (i = 0;strlen (CTYPE);i++)
	{
		if (CTYPE [0] == *form)
			return (i);
	}
	return (-1);
}

int
get_cformat (
 char	*form)
{
	int		i;
	/*---------------------------
	| find col_format			|
	---------------------------*/
	for (i = 0;strlen (CFTYPE);i++)
	{
		if (CFTYPE [0] == *form)
			return (i);
	}
	return (-1);
}

int
get_format (
 char	*form)
{
	int		i;
	/*---------------------------
	| find page_format			|
	---------------------------*/
	for (i = 0;strlen (FFORMAT);i++)
	{
		if (FFORMAT [0] == *form)
			return (i);
	}
	return (-1);
}

int
spec_valid (
 int field)
{
	int		i;
	int		indx;
	int		tmp_wdth;

	/*---------------------------
	| price book page			|
	---------------------------*/
	if (LCHECK ("this_page"))
	{
		if (FIELD.required == NA)
			return (EXIT_SUCCESS);
		/*-----------------------
		| search				|
		-----------------------*/
		if (SRCH_KEY)
		{
			show_inph (temp_str);
			return (EXIT_SUCCESS);
		}
		/*-------------------------------
		| lookup price book page		|
		-------------------------------*/
		strcpy (inph_rec.co_no,comm_rec.co_no);
		inph_rec.this_page = local_rec.this_page;
		cc = find_rec ("inph",&inph_rec,COMPARISON,"w");
		if (cc)
		{
			/*-----------------------------------
			| configuration page must be blank	|
			-----------------------------------*/
			if (DEFAULT_PAGE)
			{
				strcpy (local_rec.blank_page,"Yes");
				DSP_FLD ("blank_page");
				skip_entry = 1;
			}
			new_page = TRUE;
			new_sub = TRUE;
		}
		else
		{
			strcpy (local_rec.blank_page,inph_rec.blank_page);
			strcpy (local_rec.blank_page, (BLANK_PAGE) ? "Yes" : "No ");
			DSP_FLD ("blank_page");
			skip_entry = 1;
			new_page = FALSE;
		}
		if (BLANK_PAGE && !DEFAULT_PAGE)
		{
			strcpy (local_rec.sub_page," ");
			strcpy (local_rec.format," ");
			entry_exit = TRUE;
			if (new_page)
				lcount [INDEX_SCN] = 0;
			else
				load_index ();
			scn_set (PAGE_SCN);
		}
		return (EXIT_SUCCESS);
	}
	/*-------------------------------
	| destination page for copy		|
	-------------------------------*/
	if (LCHECK ("copy_page"))
	{
		if (SRCH_KEY)
		{
			show_inph (temp_str);
			return (EXIT_SUCCESS);
		}
		/*-------------------------------
		| lookup price book page		|
		-------------------------------*/
		strcpy (inph_rec.co_no,comm_rec.co_no);
		inph_rec.this_page = local_rec.copy_page;
		cc = find_rec ("inph",&inph_rec,COMPARISON,"w");
		if (cc)
		{
			new_page = TRUE;
			new_sub = TRUE;
		}
		else
		{
			strcpy (local_rec.blank_page,inph_rec.blank_page);
			if (BLANK_PAGE)
			{
				print_mess (ML (" Cannot Copy to Blank Page "));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			new_page = FALSE;
		}
		return (EXIT_SUCCESS);
	}
	/*---------------------------
	| blank page				|
	---------------------------*/
	if (LCHECK ("blank_page"))
	{
		if (FIELD.required == NA)
			return (EXIT_SUCCESS);

		/*-------------------------------
		| expand blank page field		|
		-------------------------------*/
		strcpy (local_rec.blank_page, (BLANK_PAGE) ? "Yes" : "No ");
		DSP_FLD ("blank_page");
		if (BLANK_PAGE && !DEFAULT_PAGE)
			entry_exit = TRUE;
		return (EXIT_SUCCESS);
	}
	/*-------------------------------
	| sub page in price book		|
	-------------------------------*/
	if (LCHECK ("sub_page"))
	{
		if (FIELD.required == NA || (new_page && !DEFAULT_PAGE && !BLANK_PAGE))
			return (EXIT_SUCCESS);
		/*-----------------------
		| search				|
		-----------------------*/
		if (SRCH_KEY)
		{
			show_inps ();
			return (EXIT_SUCCESS);
		}
		/*-------------------------------
		| configuration page			|
		-------------------------------*/
		if (DEFAULT_PAGE)
		{
			/*---------------------------------------
			| page format = sub page for default	|
			---------------------------------------*/
			i = get_format (local_rec.sub_page);
			if (i < 0)
			{
				print_mess (ML (" Default Sub Page must be A, B, C, or D only "));
				return (EXIT_FAILURE);
			}
			strcpy (local_rec.format,FDESC);
			DSP_FLD ("page_format");
			type_page = FTYPE;
			entry_exit = TRUE;
		}
		/*-----------------------------------
		| lookup sub page in price book		|
		-----------------------------------*/
		inps_rec.hhph_hash = inph_rec.hhph_hash;
		strcpy (inps_rec.sub_page,local_rec.sub_page);
		cc = find_rec ("inps",&inps_rec,COMPARISON,"w");
		if (!cc)
		{
			new_sub = FALSE;
			strcpy (local_rec.format,inps_rec.page_style);
			i = get_format (local_rec.format);
			if (i < 0)
				return (EXIT_SUCCESS);
			strcpy (local_rec.format,FDESC);
			DSP_FLD ("page_format");
			type_page = FTYPE;
			load_title ();
			load_columns ();
			load_book ();
			dflt_columns ();
			scn_set (PAGE_SCN);
			entry_exit = TRUE;
		}
		else
		{
			lcount [TITLE_SCN] = 0;
			lcount [A_TYPE] = 0;
			lcount [C_TYPE] = 0;
			lcount [D_TYPE] = 0;

			max_columns = 0;
			new_sub = TRUE;
			/*-------------------------------
			| configuration page			|
			-------------------------------*/
			if (DEFAULT_PAGE)
			{
				strcpy (local_rec.format,local_rec.sub_page);
				i = get_format (local_rec.format);
				if (i < 0)
					return (EXIT_SUCCESS);
				strcpy (local_rec.format,FDESC);
				DSP_FLD ("page_format");
				type_page = FTYPE;
				dflt_columns ();
			}
		}
		if (new_page)
			lcount [INDEX_SCN] = 0;
		else
			load_index ();
		scn_set (PAGE_SCN);
		return (EXIT_SUCCESS);
	}
	/*-----------------------------------
	| destination sub page in copy		|
	-----------------------------------*/
	if (LCHECK ("copy_sub"))
	{
		if (new_page)
			return (EXIT_SUCCESS);
		/*-----------------------------------
		| lookup sub page in price book		|
		-----------------------------------*/
		inps_rec.hhph_hash = inph_rec.hhph_hash;
		strcpy (inps_rec.sub_page,local_rec.copy_sub);
		cc = find_rec ("inps",&inps_rec,COMPARISON,"r");
		if (!cc)
		{
			print_mess (ML (" Destination Sub Page Already Exists "));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		new_sub = TRUE;
		return (EXIT_SUCCESS);
	}
	/*-------------------------------
	| format for sub page			|
	-------------------------------*/
	if (LCHECK ("page_format"))
	{
		if (FIELD.required == NA)
			return (EXIT_SUCCESS);
		i = get_format (local_rec.format);
		if (i < 0)
			return (EXIT_SUCCESS);
		strcpy (local_rec.format,FDESC);
		type_page = FTYPE;
		DSP_FLD ("page_format");
		if (!BLANK_PAGE)
			dflt_columns ();
		return (EXIT_SUCCESS);
	}
	/*---------------------------
	| index reference			|
	---------------------------*/
	if (LCHECK ("reference"))
	{
		if (dflt_used)
			return (delete_line ());
		return (EXIT_SUCCESS);
	}
	/*-------------------------------
	| formatting for title			|
	-------------------------------*/
	if (LCHECK ("title_type"))
	{
		switch (local_rec.title_type [0])
		{
		case	'D':
			return (delete_title ());

		case	'I':
			return (insert_title ());

		default:
			sprintf (title_store [line_cnt].t_type,"%-1.1s",local_rec.title_type);
			break;
		}
		title_display (tab_col + 4);
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| text of  title			|
	---------------------------*/
	if (LCHECK ("title_text"))
	{
		strcpy (title_store [line_cnt].t_title,local_rec.title_text);
		title_display (tab_col + 4);
		return (EXIT_SUCCESS);
	}
	/*-----------------------------------
	| type of column - from table		|
	-----------------------------------*/
	if (LCHECK ("col_type"))
	{
		sprintf (tmp_col_store.c_type,"%-1.1s",local_rec.col_type);
		if (MODIFY)
			sprintf (col_store [curr_col].c_type,"%-1.1s",local_rec.col_type);
	
		i = get_ctype (local_rec.col_type);
		if (i >= 0)
		{
			sprintf (local_rec.col_type,"%-20.20s",CDESC);
			DSP_FLD ("col_type");
		}
		return (EXIT_SUCCESS);
	}
	/*-------------------------------
	| output width of column		|
	-------------------------------*/
	if (LCHECK ("col_width"))
	{
		/*------------------------------------------------
		| Validate that column will fit on screen.  	 |
		| Check width of columns against width of screen | 
		------------------------------------------------*/
		tmp_wdth = 1;
		for (i = 0; i < MAX_COLUMNS; i++)
		{
			tmp_wdth += col_store [i].c_width;
			if (col_store [i].c_width > 0)
				tmp_wdth++;
		}

		if ((tmp_wdth + atoi (temp_str) + 1) >= MX_WDTH)
		{
			print_mess (ML (" Column is too wide for screen "));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		tmp_col_store.c_width = local_rec.col_width;
		if (MODIFY)
			col_store [curr_col].c_width = local_rec.col_width;

		return (EXIT_SUCCESS);
	}
	/*---------------------------------------
	| first line of column heading		|
	---------------------------------------*/
	if (LCHECK ("col_head_1"))
	{
		strcpy (tmp_col_store.c_head [0],local_rec.col_head [0]);
		if (MODIFY)
			strcpy (col_store [curr_col].c_head [0],local_rec.col_head [0]);

		return (EXIT_SUCCESS);
	}
	/*---------------------------------------
	| second line of column heading		|
	---------------------------------------*/
	if (LCHECK ("col_head_2"))
	{
		strcpy (tmp_col_store.c_head [1],local_rec.col_head [1]);
		if (MODIFY)
			strcpy (col_store [curr_col].c_head [1],local_rec.col_head [1]);

		return (EXIT_SUCCESS);
	}
	/*---------------------------------------
	| format (justification) for column	|
	---------------------------------------*/
	if (LCHECK ("col_format"))
	{
		sprintf (tmp_col_store.c_format,"%-1.1s",local_rec.col_format);
		if (MODIFY)
			sprintf (col_store [curr_col].c_format,"%-1.1s",local_rec.col_format);

		i = get_cformat (local_rec.col_format);
		if (i >= 0)
		{
			sprintf (local_rec.col_format,"%-20.20s",CFDESC);
			DSP_FLD ("col_format");
		}
		return (EXIT_SUCCESS);
	}
	/*---------------------------
	| length for item			|
	---------------------------*/
	if (LCHECK ("length"))
	{
		if (local_rec.bolt_length [0] == 92)
		{
			/*---------------------------------------
			| Must Be In Edit Mode to Use Delete	|
			---------------------------------------*/
			if (prog_status == ENTRY)
			{
				print_mess (ML (mlStdMess005));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			/*-------------------------------
			| deletion / insertion			|
			-------------------------------*/
			switch (local_rec.bolt_length [1])
			{
			case	'D':
				return (delete_line ());

			case	'I':
				return (insert_line ());

			default:
				break;
			}
		}
		return (EXIT_SUCCESS);
	}
	/*-------------------------------
	| item in price book			|
	-------------------------------*/
	if (strncmp (FIELD.label,"column_",7) == 0) 
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		indx = atoi (FIELD.label + 7);
		indx--;
		/*---------------------------------------
		| delete line from I - Type format page	|
		---------------------------------------*/
		if (local_rec.item_no [0] [0] == 92 && type_page == C_TYPE)
		{
			if (prog_status == ENTRY)
			{
				print_mess (ML (mlStdMess005));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			/*-------------------------------
			| deletion / insertion			|
			-------------------------------*/
			switch (local_rec.item_no [0] [1])
			{
			case	'D':
				return (delete_line ());

			case	'I':
				return (insert_line ());

			default:
				return (EXIT_SUCCESS);
			}
		}
		/*---------------------------
		| place holder only			|
		---------------------------*/
		if (!strcmp (local_rec.item_no [indx],"                "))
		{
			store [line_cnt].hhbrHash [indx] = 0L;
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.item_no [indx], 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no [indx]);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		display_field (field);

		/*---------------------------
		| store hash				|
		---------------------------*/
		store [line_cnt].hhbrHash [indx] = inmr_rec.hhbr_hash;
		if (type_page == C_TYPE)
		{
			sprintf (local_rec.item_desc,inmr_rec.description);
			sprintf (local_rec.pack_size,inmr_rec.sale_unit);
			local_rec.price_per = 	FindBasePrice
									 (	
										comm_rec.est_no,
										comm_rec.cc_no,
										inmr_rec.hhbr_hash,
										1,
										CURR_CODE
									);
			DSP_FLD ("item_desc");
			DSP_FLD ("pack_size");
			DSP_FLD ("price_per");
		}
		SuperSynonymError ();
		return (EXIT_SUCCESS);
	}

	if (strcmp (FIELD.label,"D_code") == 0) 
	{
		sprintf (inmr_rec.item_no, "%-16.16s", temp_str);
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		cc = FindInmr (comm_rec.co_no, inmr_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, inmr_rec.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		else
		{
			sprintf (local_rec.item_no [0], "%-16.16s", inmr_rec.item_no);
			sprintf (local_rec.item_desc, "%-40.40s", inmr_rec.description);
			DSP_FLD ("D_desc");

			/*---------------------------
			| store hash				|
			---------------------------*/
			store [line_cnt].hhbrHash [0] = inmr_rec.hhbr_hash;

		}
		SuperSynonymError ();
		display_field (field);
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
setup_edit (
 void)
{
	/*---------------------------
	| default column			|
	---------------------------*/
	if (DEFAULT_PAGE)
	{
		no_edit (PAGE_SCN);
		no_edit (INDEX_SCN);
		edit_ok (COL_SCN);
		no_edit (TITLE_SCN);
		no_edit (A_TYPE);
		no_edit (B_TYPE);
		no_edit (C_TYPE);
		no_edit (D_TYPE);
		no_edit (COPY_SCN);
		return;
	}
	/*---------------------------
	| blank page				|
	---------------------------*/
	if (BLANK_PAGE)
	{
		no_edit (PAGE_SCN);
		edit_ok (INDEX_SCN);
		no_edit (COL_SCN);
		no_edit (TITLE_SCN);
		no_edit (A_TYPE);
		no_edit (B_TYPE);
		no_edit (C_TYPE);
		no_edit (D_TYPE);
		no_edit (COPY_SCN);
		return;
	}
	/*---------------------------
	| normal situation			|
	---------------------------*/
	no_edit (PAGE_SCN);
	edit_ok (INDEX_SCN);
	edit_ok (COL_SCN);
	edit_ok (TITLE_SCN);
	no_edit (A_TYPE);
	no_edit (B_TYPE);
	no_edit (C_TYPE);
	no_edit (D_TYPE);
	no_edit (COPY_SCN);
	edit_ok (type_page);
}

int
delete_line (
 void)
{
	int		i;
	int		ccol;
	int		max_col = (cur_screen == A_TYPE) ? 6 : 7;
	int		this_page = (line_cnt / PV_tlines);
	/*-------------------------------
	| no lines to delete			|
	-------------------------------*/
	if (lcount [cur_screen] < 1)
	{
		print_mess (ML (mlStdMess032));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	/*---------------------------------------
	| must be in edit mode to use delete	|
	---------------------------------------*/
	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	lcount [cur_screen]--;
	/*---------------------------
	| delete line				|
	---------------------------*/
	for (i = line_cnt;line_cnt < lcount [cur_screen];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);

		if (cur_screen == C_TYPE || cur_screen == D_TYPE)
			store [line_cnt].hhbrHash [0] = store [line_cnt + 1].hhbrHash [0];
		else
			for (ccol = 0;cur_screen != INDEX_SCN && ccol < max_col;ccol++)
				store [line_cnt].hhbrHash [ccol] = store [line_cnt + 1].hhbrHash [ccol];

		if (this_page == (line_cnt / PV_tlines))
			line_display ();
	}
	/*-----------------------------------
	| blank last line - if required		|
	-----------------------------------*/
	if (this_page == (line_cnt / PV_tlines))
		blank_display ();
	/*---------------------------------------
	| init tabular if last line on screen	|
	---------------------------------------*/
	if (lcount [cur_screen] <= 0)
	{
		init_vars (cur_screen);
		putval (line_cnt);
	}
	move (0,2);
	cl_line ();
	fflush (stdout);
	line_cnt = min (i,lcount [cur_screen]);
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

int
insert_line (
 void)
{
	int		i;
	int		ccol;
	int		max_col = (cur_screen == A_TYPE) ? 6 : 7;
	int		this_page = (line_cnt / PV_tlines);

	MODIFY = FALSE;
	/*-------------------------------
	| check for max lines			|
	-------------------------------*/
	if (lcount [cur_screen] == vars [scn_start].row)
	{
		print_mess (ML (mlStdMess076));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	/*---------------------------
	| insert line				|
	---------------------------*/
	for (i = line_cnt,line_cnt = lcount [cur_screen];line_cnt > i;line_cnt--)
	{
		getval (line_cnt - 1);
		putval (line_cnt);
		/*---------------------------
		| shuffle hashes			|
		---------------------------*/
		if (cur_screen == C_TYPE || cur_screen == D_TYPE)
			store [line_cnt].hhbrHash [0] = store [line_cnt -1].hhbrHash [0];
		else
			for (ccol = 0;ccol < max_col;ccol++)
				store [line_cnt].hhbrHash [ccol] = store [line_cnt -1].hhbrHash [ccol];
		/*-----------------------------------
		| display line if on current page	|
		-----------------------------------*/
		if (this_page == (line_cnt / PV_tlines))
			line_display ();
	}
	lcount [cur_screen]++;
	/*---------------------------------------
	| display blank line if on current page	|
	---------------------------------------*/
	if (this_page == (line_cnt / PV_tlines))
		blank_display ();
	/*-----------------------------------
	| clean up insertion message		|
	-----------------------------------*/
	move (0,2);
	cl_line ();
	fflush (stdout);
	/*---------------------------
	| insert new line			|
	---------------------------*/
	init_ok = 0;
	prog_status = ENTRY;
	scn_entry (cur_screen);
	prog_status = !ENTRY;
	init_ok = 1;
	/*---------------------------
	| read current line			|
	---------------------------*/
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

int
delete_title (
 void)
{
	int		i;
	/*-----------------------------------
	| check there are lines to delete	|
	-----------------------------------*/
	if (lcount [TITLE_SCN] < 1)
	{
		print_mess (ML (mlStdMess032)); 
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	/*---------------------------------------
	| must be in edit mode to use delete	|
	---------------------------------------*/
	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005)); 
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	/*-------------------------------
	| delete line from title		|
	-------------------------------*/
	lcount [TITLE_SCN]--;
	for (i = line_cnt;line_cnt < lcount [TITLE_SCN];line_cnt++)
	{
		strcpy (title_store [line_cnt].t_type,title_store [line_cnt + 1].t_type);
		strcpy (title_store [line_cnt].t_title,title_store [line_cnt + 1].t_title);
		getval (line_cnt + 1);
		putval (line_cnt);
		line_display ();
	}
	/*-----------------------------------
	| initialise title structure		|
	-----------------------------------*/
	strcpy (title_store [line_cnt].t_type," ");
	sprintf (title_store [line_cnt].t_title,"%-96.96s"," ");
	strcpy (local_rec.title_type," ");
	sprintf (local_rec.title_text,"%-96.96s"," ");
	/*---------------------------------------
	| init tabular if last line on screen	|
	---------------------------------------*/
	if (lcount [cur_screen] <= 0)
	{
		init_vars (cur_screen);
		putval (line_cnt);
	}
	line_display ();
	line_cnt = min (i,lcount [TITLE_SCN]);
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

int
insert_title (
 void)
{
	int		i;
	/*-----------------------------------
	| check lines can be inserted		|
	-----------------------------------*/
	if (lcount [TITLE_SCN] == vars [scn_start].row)
	{
		print_mess (ML (mlStdMess076)); 
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	/*-----------------------------------
	| creat space for inserted line		|
	-----------------------------------*/
	for (i = line_cnt,line_cnt = lcount [TITLE_SCN];line_cnt > i;line_cnt--)
	{
		strcpy (title_store [line_cnt].t_type,title_store [line_cnt - 1].t_type);
		strcpy (title_store [line_cnt].t_title,title_store [line_cnt - 1].t_title);
		getval (line_cnt - 1);
		putval (line_cnt);
		line_display ();
	}
	lcount [TITLE_SCN]++;
	/*-----------------------------------
	| initialise screen & table data	|
	-----------------------------------*/
	strcpy (local_rec.title_type," ");
	sprintf (local_rec.title_text,"%-96.96s"," ");
	strcpy (title_store [line_cnt].t_type," ");
	sprintf (title_store [line_cnt].t_title,"%-96.96s"," ");
	putval (line_cnt);
	line_display ();
	/*---------------------------
	| insert line				|
	---------------------------*/
	init_ok = 0;
	prog_status = ENTRY;
	scn_entry (cur_screen);
	prog_status = !ENTRY;
	init_ok = 1;
	/*-----------------------
	| get line				|
	-----------------------*/
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

void
load_title (
 void)
{
	/*---------------------------
	| initialise screen			|
	---------------------------*/
	scn_set (TITLE_SCN);
	lcount [TITLE_SCN] = 0;
	if (DEFAULT_PAGE)
		return;
	/*---------------------------
	| print action				|
	---------------------------*/
	move (2,2);
	printf ("Loading Title ...");
	fflush (stdout);
	/*---------------------------
	| load title lines			|
	---------------------------*/
	inpt_rec.hhpr_hash = inps_rec.hhpr_hash;
	inpt_rec.line_no = 0;
	cc = find_rec ("inpt",&inpt_rec,GTEQ,"r");
	while (!cc && inpt_rec.hhpr_hash == inps_rec.hhpr_hash)
	{
		strcpy (local_rec.title_type,inpt_rec.format);
		strcpy (local_rec.title_text,inpt_rec.title);
		strcpy (title_store [lcount [TITLE_SCN]].t_type,inpt_rec.format);
		strcpy (title_store [lcount [TITLE_SCN]].t_title,inpt_rec.title);
		putval (lcount [TITLE_SCN]++);
		if (lcount [TITLE_SCN] >= vars [scn_start].row)
			break;
		cc = find_rec ("inpt",&inpt_rec,NEXT,"r");
	}
	move (2,2);
	cl_line ();
	fflush (stdout);
}

void
load_columns (
 void)
{
	/*---------------------------
	| print action				|
	---------------------------*/
	move (2,2);
	printf ("Loading Column Layout ...");
	fflush (stdout);
	/*-------------------------------
	| load column layout			|
	-------------------------------*/
	max_columns = 0;
	inpc_rec.hhpr_hash = inps_rec.hhpr_hash;
	inpc_rec.col_no = 0;
	cc = find_rec ("inpc",&inpc_rec,GTEQ,"r");
	while (valid_inpc ())
	{
		strcpy (col_store [max_columns].c_type,inpc_rec.col_type);
		col_store [max_columns].c_width = inpc_rec.width;
		strcpy (col_store [max_columns].c_head [0],inpc_rec.heading_1);
		strcpy (col_store [max_columns].c_head [1],inpc_rec.heading_2);
		strcpy (col_store [max_columns++].c_format,inpc_rec.format);
		/*-------------------------------
		| check for table over flow		|
		-------------------------------*/
		if (max_columns == MAX_COLUMNS)
			break;

		cc = find_rec ("inpc",&inpc_rec,NEXT,"r");
	}
	move (2,2);
	cl_line ();
	fflush (stdout);
}

void
dflt_columns (
 void)
{
	/*-----------------------------------
	| no columns loaded - use default	|
	-----------------------------------*/
	if (max_columns == 0)
	{
		/*-------------------------------
		| page 0 holds defaults			|
		-------------------------------*/
		strcpy (sinph_rec.co_no,comm_rec.co_no);
		sinph_rec.this_page = 0;
		cc = find_rec ("sinph",&sinph_rec,COMPARISON,"r");
		if (cc)
		{
			std_dflt ();
			return;
		}
		/*-----------------------------------
		| sub page of type hold defaults	|
		-----------------------------------*/
		sinps_rec.hhph_hash = sinph_rec.hhph_hash;
		sprintf (sinps_rec.sub_page,"%-1.1s",local_rec.format);
		cc = find_rec ("sinps",&sinps_rec,COMPARISON,"r");
		if (cc)
		{
			std_dflt ();
			return;
		}
		/*---------------------------
		| read columns				|
		---------------------------*/
		inpc_rec.hhpr_hash = sinps_rec.hhpr_hash;
		inpc_rec.col_no = 0;
		cc = find_rec ("inpc",&inpc_rec,GTEQ,"r");
		while (!cc && inpc_rec.hhpr_hash == sinps_rec.hhpr_hash)
		{
			save_col
			 (
				max_columns++,
				0,
				inpc_rec.col_type,
				inpc_rec.width,
				inpc_rec.heading_1,
				inpc_rec.heading_2,
				inpc_rec.format
			);
			cc = find_rec ("inpc",&inpc_rec,NEXT,"r");
		}
		/*---------------------------------------
		| no columns read			|
		---------------------------------------*/
		if (max_columns == 0)
		{
			std_dflt ();
			return;
		}
	}
}

void
std_dflt (
 void)
{
	if (max_columns == 0)
	{
		switch (local_rec.format [0])
		{
		/*-------------------------------
		| A - Type default columns		|
		-------------------------------*/
		case	'A':
			save_col (max_columns++,0,"L",8," "," ","L");
			save_col (max_columns++,0,"P",4,"PACK"," ","L");
			save_col (max_columns++,0,"$",10,"BLACK"," ","C");
			save_col (max_columns++,0,"$",10,"ZINC."," ","C");
			save_col (max_columns++,0,"$",10,"GALV."," ","C");
			save_col (max_columns++,0,"P",4,"PACK"," ","L");
			save_col (max_columns++,0,"$",10,"BLACK"," ","C");
			save_col (max_columns++,0,"$",10,"ZINC."," ","C");
			save_col (max_columns++,0,"$",10,"GALV."," ","C");
			break;

		/*-------------------------------
		| C - Type default columns		|
		-------------------------------*/
		case	'C':
			save_col (max_columns++,0,"I",16,"Item Number "," ","L");
			save_col (max_columns++,0,"O",10," "," ","R");
			save_col (max_columns++,0,"D",40,"Item Description"," ","L");
			save_col (max_columns++,0,"O",10," "," ","R");
			save_col (max_columns++,0,"P",4,"Pack"," ","L");
			save_col (max_columns++,0,"$",10,"Price/Per "," ","C");
			break;

		/*-------------------------------
		| B - Type default columns		|
		-------------------------------*/
		case	'B':
			save_col (max_columns++,0,"L",8," "," ","L");
			save_col (max_columns++,0,"P",4,"Pack"," ","L");
			save_col (max_columns++,0,"$",10," "," ","C");
			save_col (max_columns++,0,"$",10," "," ","C");
			save_col (max_columns++,0,"$",10," "," ","C");
			save_col (max_columns++,0,"$",10," "," ","C");
			save_col (max_columns++,0,"$",10," "," ","C");
			save_col (max_columns++,0,"$",10," "," ","C");
			save_col (max_columns++,0,"$",10," "," ","C");
			break;

		/*-------------------------------
		| D - Type default columns		|
		-------------------------------*/
		case	'D':
			save_col (max_columns++,0,"I",16,"Item Number"," ","L");
			save_col (max_columns++,0,"D",42,"Item Description"," ","L");
			save_col (max_columns++,0,"$",10,"Price1"," ","R");
			save_col (max_columns++,0,"$",10,"Price2"," ","R");
			save_col (max_columns++,0,"$",10,"Price3"," ","R");
			save_col (max_columns++,0,"C",10,"Cost"," ","R");
			save_col (max_columns++,0,"Q",10,"Qty Brk"," ","R");
			break;
		}
	}
}

void
save_col (
 int	ccol,			/* column in table					*/
 int	cc_start,		/* start column (on screen)		*/
 char	*cc_type,		/* column type						*/
 int	cc_width,		/* column width						*/
 char	*cc_head0,		/* column heading line 1			*/
 char	*cc_head1,		/* column heading line 2			*/
 char	*cc_format)		/* column format (justification)	*/
{
	col_store [ccol].c_start 	= cc_start;
	strcpy (col_store [ccol].c_type,cc_type);
	col_store [ccol].c_width 	= cc_width;
	sprintf (col_store [ccol].c_head [0],"%-45.45s",cc_head0);
	sprintf (col_store [ccol].c_head [1],"%-45.45s",cc_head1);
	strcpy (col_store [ccol].c_format,cc_format);
}

void
load_index (
 void)
{
	/*---------------------------
	| initialise screen			|
	---------------------------*/
	scn_set (INDEX_SCN);
	lcount [INDEX_SCN] = 0;
	if (DEFAULT_PAGE)
		return;
	/*---------------------------
	| print action				|
	---------------------------*/
	move (2,2);
	printf ("Loading Index References ...");
	fflush (stdout);
	/*-------------------------------
	| load index references			|
	-------------------------------*/
	cc = find_hash ("inpi",&inpi_rec,GTEQ,"r",inph_rec.hhph_hash);
	while (valid_inpi ())
	{
		putval (lcount [INDEX_SCN]++);
		if (lcount [INDEX_SCN] >= vars [scn_start].row)
			break;
		cc = find_hash ("inpi",&inpi_rec,NEXT,"r",inph_rec.hhph_hash);
	}
	scn_set (PAGE_SCN);
	move (2,2);
	cl_line ();
	fflush (stdout);
}

/*===========================
| A_TYPE	- 6 columns		|
| B_TYPE	- 1 column		|
| C_TYPE	- 7 columns		|
| D_TYPE	- 1 column		|
===========================*/
void
load_book (
 void)
{
	int		i;
	int		max_col = (type_page == A_TYPE) ? 6 : 7;
	/*---------------------------
	| initialise screen			|
	---------------------------*/
	scn_set (type_page);
	lcount [type_page] = 0;
	if (DEFAULT_PAGE)
		return;
	/*---------------------------
	| print action				|
	---------------------------*/
	move (2,2);
	printf ("Loading Price Book ...");
	fflush (stdout);
	/*-------------------------------
	| read price book lines			|
	-------------------------------*/
	abc_selfield ("inmr","inmr_hhbr_hash");
	inpl_rec.hhpr_hash = inps_rec.hhpr_hash;
	inpl_rec.line_no = 0;
	cc = find_rec ("inpl",&inpl_rec,GTEQ,"r");
	while (valid_inpl ())
	{
		if (type_page == D_TYPE)
		{
			inmr_rec.hhbr_hash	=	inpl_hhbr_hash [0];
			cc = find_rec ("inmr",&inmr_rec,COMPARISON,"r");
			if (cc)
			{
				sprintf (local_rec.item_no [0],	"%16.16s"," ");
				sprintf (local_rec.item_desc,	"%40.40s"," ");
				store [lcount [type_page]].hhbrHash [0] = 0L;
			}
			else
			{
				store [lcount [type_page]].hhbrHash [0] = inpl_hhbr_hash [0];
				strcpy (local_rec.item_no [0],	inmr_rec.item_no);
				strcpy (local_rec.item_desc, 	inmr_rec.description);
			}
		}
		else
		{
			if (type_page == C_TYPE)
			{
				inmr_rec.hhbr_hash	=	inpl_hhbr_hash [0];
				cc = find_rec ("inmr",&inmr_rec,COMPARISON,"r");
				if (cc)
				{
					sprintf (local_rec.item_no [0],	"%16.16s"," ");
					sprintf (local_rec.item_desc,	"%40.40s"," ");
					store [lcount [type_page]].hhbrHash [0] = 0L;
					sprintf (local_rec.pack_size,	"%4.4s"," ");
				}
				else
				{
					store [lcount [type_page]].hhbrHash [0] = inpl_hhbr_hash[0];
					strcpy (local_rec.item_no [0],	inmr_rec.item_no);
					strcpy (local_rec.item_desc, 	inmr_rec.description);
					strcpy (local_rec.pack_size,	inmr_rec.sale_unit);
				}
				local_rec.price_per = 	FindBasePrice
										 (	
											comm_rec.est_no,
											comm_rec.cc_no,
											inmr_rec.hhbr_hash,
											1,
											CURR_CODE
										);
				store [lcount [type_page]].hhbrHash [0] = (!cc) ? inpl_hhbr_hash [0] : 0L;
			}
			else
			{
				strcpy (local_rec.bolt_length,inpl_rec.inpl_length);
	
				for (i = 0;i < max_col;i++)
				{
					inmr_rec.hhbr_hash	=	inpl_hhbr_hash [i];
					cc = find_rec ("inmr",&inmr_rec,COMPARISON,"r");
					sprintf (local_rec.item_no [i],"%-16.16s", (cc) ? " " : inmr_rec.item_no);
					store [lcount [type_page]].hhbrHash [i] = (!cc) ? inpl_hhbr_hash [i] : 0L;
				}
			}
		}
		putval (lcount [type_page]++);
		if (lcount [type_page] >= vars [scn_start].row)
			break;
		cc = find_rec ("inpl",&inpl_rec,NEXT,"r");
	}
	scn_set (PAGE_SCN);
	abc_selfield ("inmr","inmr_id_no");
	move (2,2);
	cl_line ();
	fflush (stdout);
}

void
title_display (
 int offset)
{
	int		i;
	int		start_row = 4;
	int		box_width;
	int		high_val;
	char	*sptr;
	char	buffer [200];
	char	print_line [100];
	/*---------------------------
	| calc high val				|
	---------------------------*/
	if (prog_status == ENTRY)
	{
		if (line_cnt <= lcount [TITLE_SCN])
			high_val = lcount [TITLE_SCN];
		else
			high_val = line_cnt + 1;
	}
	else
		high_val = lcount [TITLE_SCN];
	/*---------------------------
	| setup screen				|
	---------------------------*/
	/*---------------------------
	| no columns loaded			|
	---------------------------*/
	if (max_columns == 0)
		box_width = 96;
	else
	{
		box_width = col_store [max_columns - 1].c_start;
		box_width += col_store [max_columns - 1].c_width;
	}
	/*-----------------------
	| draw box				|
	-----------------------*/
	box (offset,start_row - 1,box_width,4);
	/*-----------------------------------
	| display title info  in box		|
	-----------------------------------*/
	for (i = 0;i < 4;i++)
	{
		if (i >= high_val)
		{
			print_at (start_row + i, offset + 1,"%*.*s",box_width - 2,box_width - 2," ");
			continue;
		}
		/*---------------------------------------
		| format title info			|
		---------------------------------------*/
		switch (title_store [i].t_type [0])
		{
		case	'C':
			strcpy (buffer,title_store [i].t_title);
			sptr = clip (buffer);
			sprintf (print_line,"%*s%s", (int) (box_width - strlen (buffer)) / 2, " ",
				buffer);
			break;

		case	'E':
			sptr = expand (buffer,title_store [i].t_title);
			sprintf (print_line,"%-*.*s",box_width,box_width,buffer);
			break;

		case	'B':
			sptr = expand (buffer,title_store [i].t_title);
			sptr = clip (buffer);
			sprintf (print_line,"%*s%s",
				 (int) (box_width - strlen (buffer)) / 2,
				" ",
				buffer);
			break;

		case	'N':
			sprintf (print_line,"%-*.*s",box_width,box_width,title_store [i].t_title);
			break;

		default:
			sprintf (print_line,"%-*.*s",box_width,box_width," ");
			break;
		}
		print_at (start_row + i, offset + 1,"%-*.*s",box_width - 2,box_width - 2,print_line);
	}
	fflush (stdout);
	/*-------------------------------
	| cleanup a bit of the mess		|
	-------------------------------*/
	print_at (2,0, "                     ");
	crsr_on ();
}

void
cols_display (
 int max_col, 
 int offset)
{
	int		i;
	/*-----------------------------------
	| initialise for column display		|
	-----------------------------------*/
	curr_col = 2;
	move (0,0);
	crsr_off ();
	/*-----------------------------------
	| clear display section of screen	|
	-----------------------------------*/
	for (i = 9;i < 13;i++)
	{
		move (0,i);
		cl_line ();
	}
	/*-------------------------------
	| no columns to display			|
	-------------------------------*/
	if (max_col == 0)
	{
		crsr_on ();
		return;
	}
	/*---------------------------
	| display columns			|
	---------------------------*/
	for (i = 0;i < max_col;i++)
	{
		col_store [i].c_start = curr_col;

		prnt_col (i,0,offset);

		curr_col += col_store [i].c_width;
		curr_col++;
	}
	/*-----------------------------------
	| draw box & up & down ticks		|
	-----------------------------------*/
	box (offset + 1,9,curr_col - 1,2);
	for (i = 1;i < max_col;i++)
	{
		move (col_store [i].c_start - 1 + offset,9);
		PGCHAR (8);
		move (col_store [i].c_start - 1 + offset,12);
		PGCHAR (9);
	}
	crsr_on ();
}

int
choose_col (
 int max_col)
{
	int		ccol = 0;
	int		lcol = -1;
	int		c;
	/*-------------------------------
	| no columns to select from		|
	-------------------------------*/
	if (max_col == 0)
		return (EXIT_SUCCESS);
	/*-----------------------------------
	| display first column in reverse	|
	-----------------------------------*/
	prnt_col (ccol,1,0);
	crsr_off ();
	while (TRUE)
	{
		c = getkey ();
		lcol = ccol;
		/*-------------------------------
		| process key stroke			|
		-------------------------------*/
		switch (c)
		{
		case	REDRAW:
			cols_display (max_col,0);
			lcol = -1;
			break;

		case	LEFT_KEY:
		case	8:
		case	9:
			ccol = (ccol == 0) ? max_col - 1 : ccol - 1;
			break;

		case	RIGHT_KEY:
		case	12:
			ccol = (ccol == max_col - 1) ? 0 : ccol + 1;
			break;

		case	FN16:
		case	'\r':
			crsr_on ();
			return (ccol);

		default:
			putchar (BELL);
			break;
		}
		/*-----------------------------------
		| re display appropriate columns	|
		-----------------------------------*/
		if (lcol != ccol)
		{
			prnt_col (lcol,0,0);
			prnt_col (ccol,1,0);
		}
	}
    return (EXIT_SUCCESS);
}

void
prnt_col (
 int ccol, 
 int rv_flag, 
 int offset)
{
	int		len = col_store [ccol].c_width;
	int		curr_col = col_store [ccol].c_start;
	char	masks [2] [46];
	/*-------------------------------
	| print headings into masks		|
	-------------------------------*/
	sprintf (masks [0],"%-*.*s",len,len,col_store [ccol].c_head [0]);
	sprintf (masks [1],"%-*.*s",len,len,col_store [ccol].c_head [1]);
	/*-----------------------------------
	| print headings & vertical bars	|
	-----------------------------------*/
	rv_pr (masks [0],curr_col + offset,10,rv_flag);
	PGCHAR (5);
	rv_pr (masks [1],curr_col + offset,11,rv_flag);
	PGCHAR (5);
	crsr_off ();
}

void
col_load (
 int ccol)
{
	int		i;
	/*-------------------------------
	| init columns table			|
	-------------------------------*/
	strcpy (local_rec.col_type,col_store [ccol].c_type);
	local_rec.col_width = col_store [ccol].c_width;
	strcpy (local_rec.col_head [0],col_store [ccol].c_head [0]);
	strcpy (local_rec.col_head [1],col_store [ccol].c_head [1]);
	strcpy (local_rec.col_format,col_store [ccol].c_format);
	/*---------------------------
	| get column type			|
	---------------------------*/
	i = get_ctype (local_rec.col_type);
	if (i >= 0)
		sprintf (local_rec.col_type,"%-20.20s",CDESC);
	/*---------------------------
	| get column format			|
	---------------------------*/
	i = get_cformat (local_rec.col_format);
	if (i >= 0)
		sprintf (local_rec.col_format,"%-20.20s",CFDESC);
	/*---------------------------
	| display screen			|
	---------------------------*/
	scn_display (COL_SCN);
}

void
disp_col (
 void)
{
	strcpy (local_rec.col_type," ");
	local_rec.col_width = 0;
	strcpy (local_rec.col_head [0],"                    ");
	strcpy (local_rec.col_head [1],"                    ");
	strcpy (local_rec.col_format," ");
	scn_display (COL_SCN);
}

/*===================================
| Insert Column Before Column ccol	|
===================================*/
void
insert_col (
 int ccol, 
 int max_col)
{
	int		i;
	/*-----------------------------------
	| create space for new column		|
	-----------------------------------*/
	for (i = max_col;i > ccol;i--)
	{
		col_store [i].c_start = col_store [i - 1].c_start;
		strcpy (col_store [i].c_type,col_store [i - 1].c_type);
		col_store [i].c_width = col_store [i - 1].c_width;
		strcpy (col_store [i].c_head [0],col_store [i - 1].c_head [0]);
		strcpy (col_store [i].c_head [1],col_store [i - 1].c_head [1]);
		strcpy (col_store [i].c_format,col_store [i - 1].c_format);
	}

	/*-----------------------------------------
	| Store entered values in allocated space |
	-----------------------------------------*/
	sprintf (col_store [curr_col].c_type,"%-1.1s",tmp_col_store.c_type);
	col_store [curr_col].c_width = tmp_col_store.c_width;
	strcpy (col_store [curr_col].c_head [0],tmp_col_store.c_head [0]);
	strcpy (col_store [curr_col].c_head [1],tmp_col_store.c_head [1]);
	sprintf (col_store [curr_col].c_format,"%-1.1s",tmp_col_store.c_format);
}

/*===================================
| Append Column After Column ccol	|
===================================*/
void
append_col (
 int ccol, 
 int max_col)
{
	int		i;

	MODIFY = FALSE;
	/*-----------------------------------
	| create space for new column		|
	-----------------------------------*/
	for (i = max_col;i > ccol;i--)
	{
		col_store [i].c_start = col_store [i - 1].c_start;
		strcpy (col_store [i].c_type,col_store [i - 1].c_type);
		col_store [i].c_width = col_store [i - 1].c_width;
		strcpy (col_store [i].c_head [0],col_store [i - 1].c_head [0]);
		strcpy (col_store [i].c_head [1],col_store [i - 1].c_head [1]);
		strcpy (col_store [i].c_format,col_store [i - 1].c_format);
	}

	/*-----------------------------------------
	| Store entered values in allocated space |
	-----------------------------------------*/
	sprintf (col_store [curr_col].c_type,"%-1.1s",tmp_col_store.c_type);
	col_store [curr_col].c_width = tmp_col_store.c_width;
	strcpy (col_store [curr_col].c_head [0],tmp_col_store.c_head [0]);
	strcpy (col_store [curr_col].c_head [1],tmp_col_store.c_head [1]);
	sprintf (col_store [curr_col].c_format,"%-1.1s",tmp_col_store.c_format);
}

/*=======================
| Delete Column ccol	|
=======================*/
int
delete_col (
 int ccol, 
 int max_col)
{
	int		i;
	/*-------------------------------
	| prompt for confirmation		|
	-------------------------------*/
	i = prmptmsg (" Confirm Deletion Of Column ? ","YyNn",2,4);
	move (0,4);
	cl_line ();
	/*---------------------------
	| abort deletion			|
	---------------------------*/
	if (i == 'N' || i == 'n')
		return (EXIT_SUCCESS);
	/*-------------------------------
	| delete column from table		|
	-------------------------------*/
	for (i = ccol;i < max_col;i++)
	{
		col_store [i].c_start = col_store [i + 1].c_start;
		strcpy (col_store [i].c_type,col_store [i + 1].c_type);
		col_store [i].c_width = col_store [i + 1].c_width;
		strcpy (col_store [i].c_head [0],col_store [i + 1].c_head [0]);
		strcpy (col_store [i].c_head [1],col_store [i + 1].c_head [1]);
	}
	return (EXIT_FAILURE);
}

/*==============================
| Update all relevent records. |
==============================*/
void
update (
 int page, 
 char *sub_page)
{
	clear ();
	update_inph (page);
	/*---------------------------
	| "normal" page				|
	---------------------------*/
	if (!BLANK_PAGE)
	{
		update_inpi ();
		update_inps (sub_page);
		update_inpt ();
		update_inpc ();
		update_inpl ();
	}
	else
	{
		/*-------------------------------
		| configuration page			|
		-------------------------------*/
		if (DEFAULT_PAGE)
		{
			update_inps (sub_page);
			update_inpc ();
		}
		else
			update_inpi ();
	}
}

void
update_inph (
 int page)
{
	/*---------------------------
	| page header				|
	---------------------------*/
	printf ("\n\r\n\rUpdating Header ... ");
	strcpy (inph_rec.co_no,comm_rec.co_no);
	inph_rec.this_page = page;
	sprintf (inph_rec.blank_page,"%-1.1s",local_rec.blank_page);
	if (new_page)
	{
		putchar ('A');
		fflush (stdout);
		cc = abc_add ("inph",&inph_rec);
		if (cc)
			file_err (cc, "inph", "DBADD");
		/*-----------------------------------
		| re read page for hhph_hash		|
		-----------------------------------*/
		cc = find_rec ("inph",&inph_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, "inph", "DBFIND");
	}
	else
	{
		putchar ('U');
		fflush (stdout);
		cc = abc_update ("inph",&inph_rec);
		if (cc)
			file_err (cc, "inph", "DBUPDATE");
	}
}

void
update_inpi (
 void)
{
	int		add_inpi = FALSE;
	/*---------------------------
	| index page				|
	---------------------------*/
	scn_set (INDEX_SCN);
	printf ("\n\r\n\rUpdating Index References ... ");
	fflush (stdout);
	cc = find_hash ("inpi",&inpi_rec,GTEQ,"u",inph_rec.hhph_hash);
	add_inpi = !valid_inpi ();
	for (line_cnt = 0;line_cnt < lcount [INDEX_SCN];line_cnt++)
	{
		getval (line_cnt);
		if (add_inpi)
		{
			putchar ('A');
			fflush (stdout);
			inpi_rec.hhph_hash = inph_rec.hhph_hash;
			cc = abc_add ("inpi",&inpi_rec);
			if (cc)
				file_err (cc, "inpi", "DBADD");
		}
		else
		{
			putchar ('U');
			fflush (stdout);
			cc = abc_update ("inpi",&inpi_rec);
			if (cc)
				file_err (cc, "inpi", "DBUPDATE");
			
			cc = find_hash ("inpi",&inpi_rec,NEXT,"u",inph_rec.hhph_hash);
			add_inpi = !valid_inpi ();
		}
	}
	/*-------------------------------
	| delete extra columns			|
	-------------------------------*/
	while (!add_inpi)
	{
		cc = find_hash ("inpi",&inpi_rec,NEXT,"u",inph_rec.hhph_hash);
		if (!valid_inpi ())
			break;
		/*-----------------------
		| delete				|
		-----------------------*/
		putchar ('D');
		fflush (stdout);
		abc_unlock ("inpi");
		cc = abc_delete ("inpi");
		if (cc)
			file_err (cc, "inpi", "DBDELETE");
	}
}

void
update_inps (
 char *sub_page)
{
	/*-----------------------
	| sub page				|
	-----------------------*/
	printf ("\n\r\n\rUpdating Sub Page ... ");
	fflush (stdout);
	if (new_sub)
	{
		putchar ('A');
		fflush (stdout);
		inps_rec.hhph_hash = inph_rec.hhph_hash;
		strcpy (inps_rec.sub_page,sub_page);
		sprintf (inps_rec.page_style,"%-1.1s",local_rec.format);
		cc = abc_add ("inps",&inps_rec);
		if (cc)
			file_err (cc, "inps", "DBADD");
		/*-----------------------------------
		| re read sub page for hhpr_hash	|
		-----------------------------------*/
		cc = find_rec ("inps",&inps_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, "inps", "DBFIND");
	}
	else
	{
		putchar ('U');
		fflush (stdout);
		sprintf (inps_rec.page_style,"%-1.1s",local_rec.format);
		cc = abc_update ("inps",&inps_rec);
		if (cc)
			file_err (cc, "inps", "DBUPDATE");

		abc_unlock ("inps");
	}
}

void
update_inpc (
 void)
{
	/*---------------------------
	| page columns				|
	---------------------------*/
	printf ("\n\r\n\rUpdating Column Titles ... ");
	scn_set (COL_SCN);
	for (line_cnt = 0;line_cnt < max_columns;line_cnt++)
	{
		/*---------------------------
		| find page column			|
		---------------------------*/
		inpc_rec.hhpr_hash = inps_rec.hhpr_hash;
		inpc_rec.col_no = line_cnt;
		cc = find_rec ("inpc",&inpc_rec,COMPARISON,"w");
		/*-------------------------------
		| update page column			|
		-------------------------------*/
		strcpy (inpc_rec.col_type,col_store [line_cnt].c_type);
		inpc_rec.width = col_store [line_cnt].c_width;
		strcpy (inpc_rec.heading_1,col_store [line_cnt].c_head [0]);
		strcpy (inpc_rec.heading_2,col_store [line_cnt].c_head [1]);
		strcpy (inpc_rec.format,col_store [line_cnt].c_format);
		if (cc)
		{
			putchar ('A');
			fflush (stdout);
			inpc_rec.hhpr_hash = inps_rec.hhpr_hash;
			inpc_rec.col_no = line_cnt;
			cc = abc_add ("inpc",&inpc_rec);
			if (cc)
				file_err (cc, "inpc", "DBADD");
		}
		else
		{
			putchar ('U');
			fflush (stdout);
			inpc_rec.hhpr_hash = inps_rec.hhpr_hash;
			inpc_rec.col_no = line_cnt;
			cc = abc_update ("inpc",&inpc_rec);
			if (cc)
				file_err (cc, "inpc", "DBUPDATE");
			abc_unlock ("inpc");
		}
	}
	delete_inpc (max_columns);
}

void
update_inpt (
 void)
{
	/*-------------------------------
	| sub page title lines			|
	-------------------------------*/
	printf ("\n\r\n\rUpdating Title Lines ... ");
	fflush (stdout);
	scn_set (TITLE_SCN);
	for (line_cnt = 0;line_cnt < lcount [TITLE_SCN];line_cnt++)
	{
		getval (line_cnt);
		/*-------------------------------
		| read sub page title lines		|
		-------------------------------*/
		inpt_rec.hhpr_hash = inps_rec.hhpr_hash;
		inpt_rec.line_no = line_cnt;
		cc = find_rec ("inpt",&inpt_rec,COMPARISON,"w");
		/*-----------------------------------
		| update sub page title lines		|
		-----------------------------------*/
		sprintf (inpt_rec.format,"%-1.1s",local_rec.title_type);
		strcpy (inpt_rec.title,local_rec.title_text);
		if (cc)
		{
			putchar ('A');
			fflush (stdout);
			inpt_rec.hhpr_hash = inps_rec.hhpr_hash;
			inpt_rec.line_no = line_cnt;
			cc = abc_add ("inpt",&inpt_rec);
			if (cc)
				file_err (cc, "inpt", "DBADD");
		}
		else
		{
			putchar ('U');
			fflush (stdout);
			inpt_rec.hhpr_hash = inps_rec.hhpr_hash;
			inpt_rec.line_no = line_cnt;
			cc = abc_update ("inpt",&inpt_rec);
			if (cc)
				file_err (cc, "inpt", "DBUPDATE");
			abc_unlock ("inpt");
		}
	}
	delete_inpt (lcount [TITLE_SCN]);
}

void
update_inpl (
 void)
{
	int		i;
	int		max_col = (type_page == A_TYPE) ? 6 : 7;
	/*-------------------------------
	| items in price book			|
	-------------------------------*/
	printf ("\n\r\n\rUpdating Price Book Lines ... ");
	scn_set (type_page);
	for (line_cnt = 0;line_cnt < lcount [type_page];line_cnt++)
	{
		getval (line_cnt);
		/*-------------------------------
		| read price book line			|
		-------------------------------*/
		inpl_rec.hhpr_hash = inps_rec.hhpr_hash;
		inpl_rec.line_no = line_cnt;
		cc = find_rec ("inpl",&inpl_rec,COMPARISON,"w");
		if (type_page == C_TYPE || type_page == D_TYPE)
		{
			strcpy (inpl_rec.inpl_length,"        ");
			inpl_hhbr_hash [0] = store [line_cnt].hhbrHash [0];
		}
		else
		{
			strcpy (inpl_rec.inpl_length,local_rec.bolt_length);

			for (i = 0;i < max_col;i++)
				inpl_hhbr_hash [i] = store [line_cnt].hhbrHash [i];
		}

		if (cc)
		{
			putchar ('A');
			fflush (stdout);
			inpl_rec.hhpr_hash = inps_rec.hhpr_hash;
			inpl_rec.line_no = line_cnt;
			cc = abc_add ("inpl",&inpl_rec);
			if (cc)
				file_err (cc, "inpl", "DBADD");
		}
		else
		{
			putchar ('U');
			fflush (stdout);
			inpl_rec.hhpr_hash = inps_rec.hhpr_hash;
			inpl_rec.line_no = line_cnt;
			cc = abc_update ("inpl",&inpl_rec);
			if (cc)
				file_err (cc, "inpl", "DBUPDATE");
		}
	}
	delete_inpl (lcount [type_page]);
}

void
show_inps (
 void)
{
	work_open ();
	inps_rec.hhph_hash = inph_rec.hhph_hash;
	strcpy (inps_rec.sub_page," ");
	cc = find_rec ("inps",&inps_rec,GTEQ,"r");
	while (!cc && inps_rec.hhph_hash == inph_rec.hhph_hash)
	{
		cc = save_rec (inps_rec.sub_page," ");
		if (cc)
			break;
		cc = find_rec ("inps",&inps_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	inps_rec.hhph_hash = inph_rec.hhph_hash;
	sprintf (inps_rec.sub_page,"%-1.1s",temp_str);
	cc = find_rec ("inps",&inps_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "inps", "DBFIND");
}

void
show_inph (
 char *key_val)
{
	work_open ();
	save_rec ("#Page.","# Reference ");
	strcpy (inph_rec.co_no,comm_rec.co_no);
	inph_rec.this_page = atoi (key_val);
	cc = find_rec ("inph",&inph_rec,GTEQ,"r");
	while (!cc && !strcmp (inph_rec.co_no,comm_rec.co_no))
	{
		sprintf (err_str,"%d",inph_rec.this_page);
		if (!strncmp (err_str,key_val,strlen (key_val)))
			save_inph ();
		cc = find_rec ("inph",&inph_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (inph_rec.co_no,comm_rec.co_no);
	inph_rec.this_page = atoi (temp_str);
	cc = find_rec ("inph",&inph_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "inph", "DBFIND");
}

int
srch_price (
 char *key_val)
{
	work_open ();
	save_rec ("#  Price        ","#");
	save_rec (comm_rec.price1_desc, " ");
	save_rec (comm_rec.price2_desc, " ");
	save_rec (comm_rec.price3_desc, " ");
	save_rec (comm_rec.price4_desc, " ");
	save_rec (comm_rec.price5_desc, " ");
	save_rec (comm_rec.price6_desc, " ");
	save_rec (comm_rec.price7_desc, " ");
	save_rec (comm_rec.price8_desc, " ");
	save_rec (comm_rec.price9_desc, " ");
	
	cc = disp_srch ();
	work_close ();
	if (cc)
		return (cc);

	return (EXIT_SUCCESS);
}

int
save_inph (
 void)
{
	int		ref_found = FALSE;
	char	page_no [6];

	sprintf (page_no,"%5d",inph_rec.this_page);
	cc = find_hash ("inpi",&inpi_rec,GTEQ,"r",inph_rec.hhph_hash);
	while (valid_inpi ())
	{
		cc = save_rec (page_no,inpi_rec.reference);
		if (cc)
			return (cc);
		ref_found = TRUE;
		cc = find_hash ("inpi",&inpi_rec,NEXT,"r",inph_rec.hhph_hash);
	}

	if (!ref_found)
	{
		sprintf (err_str,"%-50.50s"," ");
		cc = save_rec (page_no,err_str);
		if (cc)
			return (cc);
	}
	return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{
	int		len = (132 - strlen (screens [scn - 1]._heading)) / 2;

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	printf ("<%s>",PNAME);

	rv_pr (ML (" Price Book Page Maintenance "),51,0,1);

	strcpy (err_str, screens [scn - 1]._heading);
	rv_pr (ML (err_str),len,2,1);

	switch (scn)
	{
	case	PAGE_SCN:
		box (0,3,132,4);
		move (1,5);
		line (131);
		break;

	case	INDEX_SCN:
		tab_col = 13;
		tab_row = 6;
		PV_tlines = 10;
		break;

	case	COL_SCN:
		tab_col = -3;
		box (0,14,132,5);
		if (restart)
			cols_display (max_columns,0);
		break;

	case	TITLE_SCN:
		tab_col = 0;
		tab_row = 13;
		PV_tlines = 4;
		cols_display (max_columns,tab_col + 3);
		title_display (tab_col + 4);
		break;

	case	A_TYPE:
		tab_col = 0;
		tab_row = 6;
		PV_tlines = 10;
		break;

	case	C_TYPE:
		tab_col = 25;
		tab_row = 6;
		PV_tlines = 10;
		break;

	case	B_TYPE:
		tab_col = 0;
		tab_row = 6;
		PV_tlines = 10;
		break;

	case	D_TYPE:
		tab_col = 36;
		tab_row = 6;
		PV_tlines = 10;
		break;

	case	COPY_SCN:
		box (0,3,132,7);
		move (1,8);
		line (131);
		break;

	default:
		break;
	}

	move (0,1);
	line (132);
	move (0,21); 
	line (132);
	move (0,22);
	printf ("Co.: %s  %s",comm_rec.co_no,comm_rec.co_name);
	scn_write (scn);
    return (EXIT_SUCCESS);
}

