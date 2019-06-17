/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_priprt.c,v 5.7 2001/11/28 02:58:16 scott Exp $
|  Program Name  : (sk_priprt.c)
|  Program Desc  : (Inventory Price Book Print)
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 17/03/88         |
|---------------------------------------------------------------------|
| $Log: sk_priprt.c,v $
| Revision 5.7  2001/11/28 02:58:16  scott
| Updated for length
|
| Revision 5.6  2001/11/26 07:55:22  cha
| Updated to fix some small errors.
|
| Revision 5.5  2001/11/15 01:12:41  scott
| Updated for define of length
|
| Revision 5.4  2001/11/07 07:17:11  scott
| Updated to remove gets
| Updated to convert to app.schema
| Updated to clean SOME of code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_priprt.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_priprt/sk_priprt.c,v 5.7 2001/11/28 02:58:16 scott Exp $";

#include	<pslscr.h>

#define	MAX_COL		7

#define	PRICE		 (local_rec.price_type [0] >= PRICE1 && local_rec.price_type [0] <= PRICE9)
#define NOT_PRINT	0
#define	PRICE1		1
#define	PRICE2		2
#define	PRICE3		3
#define	PRICE4		4
#define	PRICE5		5
#define	PRICE6		6
#define	PRICE7		7
#define	PRICE8		8
#define	PRICE9		9
#define	ITEM		10
#define	ON_HAND		11
#define	PAGE_LIST	12

#define	MAX_SORT	1000

#define	BLANK_PAGE	 (inph_rec.blank_page [0] == 'Y')
#define	ValidInpi()	 (!cc && inpd_rec.hhph_hash == inpi_rec.hhph_hash)
#define	ValidInpd()	 (!cc && inpd_rec.hhpb_hash == inpb_rec.hhpb_hash)
#define	ValidPage()	 (page_no >= startPage && \
			 (page_no <= endPage || endPage <= 0))


FILE	*fout;
char	*ruler = "---------------------------------------------------------------------------------------------------------------------------------------------------------------";

struct	{
	char	c_type [2];
	int		c_width;
	char	c_head [2][46];
	char	c_format [2];
	int		c_lwidth [2];
} col_store [20];

struct	PriceBook 
{
	char	reference [51];
	int		page_no;
} sort_tab [MAX_SORT];

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct ineiRecord	inei_rec;
struct inprRecord	inpr_rec;
struct inphRecord	inph_rec;
struct inpsRecord	inps_rec;
struct inplRecord	inpl_rec;
struct inptRecord	inpt_rec;
struct inpcRecord	inpc_rec;
struct inpbRecord	inpb_rec;
struct inpdRecord	inpd_rec;
struct inpiRecord	inpi_rec;

	long	*inpl_hhbr_hash	=	&inpl_rec.hhbr_hash_1;
	double	*inpr_qty_brk	=	&inpr_rec.qty_brk1;

	int		MCURR;
	int		p_width = 96;
	int		max_columns;
	int		blank_head [2];

	char	Curr_code [4];

/*==================
| Local structure. |
==================*/
struct	{
	char	dummy [11];
	int		lpno;
	int		ncopies;
	int		pindex;
	char	p_date [11];
	long	l_p_date;
	int		price_type [13];
	char	prc_name [5][16];
	char	usr_head [31];
} local_rec;


char		envVarCurrCode [4];
#include	<FindBasePrice.h>
/*=======================
| Function Declarations |
=======================*/
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	Compare 		(const void *, const void *);
void 	PrintIndex 		(void);
void 	Heading 		(int, int);
void 	Process 		(int, int);
void 	ProcessInph		(long, int, int *);
void 	ProcessPage		(long);
void 	LoadColumns 	(long);
void 	PrintTitle 		(long, char *);
void 	ColumnHead 		(int, char *);
void 	PrintData 		(long, char *);
void 	PrintRow 		(int, char *);
double 	GetQuantity		(void);
int  	findItemCode 	(long);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	int		startPage;
	int		endPage;
	int		i;

	/*--------------------------
	| Get local currency code. |
	--------------------------*/
	sprintf (envVarCurrCode, "%-3.3s", get_env ("CURR_CODE"));

	/*---------------------------
	| check args				|
	---------------------------*/
	if (for_chk () == 0)
	{
		printf (" Invoke %s from Pipe\007\n\r", argv [0]);
		printf ("<lpno>\n\r");
		printf ("<no_copies>\n\r");
		printf ("<hhpbHash>\n\r");
		printf ("<PrintIndex> - Y(es\n\r");
		printf ("              - N(o\n\r");
		printf ("<pricebook date>\n\r");
		printf ("<pricebook Heading>\n\r");
		printf ("<price1-9 type> - 0 - Price not printed\n\r");
		printf ("              - 1 - Price 1\n\r");
		printf ("              - 2 - Price 2\n\r");
		printf ("              - 3 - Price 3\n\r");
		printf ("              - 4 - Price 4\n\r");
		printf ("              - 5 - Price 5\n\r");
		printf ("              - 6 - Price 6\n\r");
		printf ("              - 7 - Price 7\n\r");
		printf ("              - 8 - Price 8\n\r");
		printf ("              - 9 - Price 9\n\r");
		printf ("              - I (tem Number\n\r");
		printf ("              - O (n Hand\n\r");
		printf ("              - L (ist of Page Headings\n\r");
		printf ("multiple occurances of the next 2 lines\n\r");
		printf ("<startPage>\n\r");
		printf ("<endPage>    - 0 denotes last page\n\r");
		printf ("-1\n\r");
		return (EXIT_FAILURE);
	}

	OpenDB ();

	if (scanf ("%d", &local_rec.lpno) == EOF || 
		scanf ("%d", &local_rec.ncopies) == EOF ||
		scanf ("%ld", &inpb_rec.hhpb_hash) == EOF)
	{
		CloseDB (); 
		FinishProgram ();
		return (EXIT_FAILURE);
	}
	
	cc = find_rec (inpb, &inpb_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inpb, "DBFIND");
	
	if (scanf ("%s", err_str) == EOF)
	{
		CloseDB (); 
		FinishProgram ();
		return (EXIT_FAILURE);
	}
	/*
	 * pindex (Y ? N)
	 */
	local_rec.pindex = (err_str [0] == 'Y' || err_str [0] == 'y');

	/*
	 * print date.
	 */
	if (scanf ("%ld", &local_rec.l_p_date) == EOF)
	{
		CloseDB (); 
		FinishProgram ();
		return (EXIT_FAILURE);
	}
	strcpy (local_rec.p_date, DateToString (local_rec.l_p_date));

	/*-------------------------------
	| price book Heading   	 		|
	-------------------------------*/
	if (scanf ("%s", err_str) == EOF)
	{
		CloseDB (); 
		FinishProgram ();
		return (EXIT_FAILURE);
	}
	sprintf (local_rec.usr_head, "%-30.30s", err_str);
	
	/*
	 * 9 price types.
	 */
	for (i = 0; i < 9; i++)
	{
		/*
		 * price type.
		 */
		if (scanf ("%s", err_str) == EOF)
		{
			CloseDB (); 
			FinishProgram ();
			return (EXIT_FAILURE);
		}
		switch (err_str [0])
		{
		case	'0':
			local_rec.price_type [i] = NOT_PRINT;
			strcpy (local_rec.prc_name [i], comm_rec.price1_desc);
			break;

		case	'1':
			local_rec.price_type [i] = PRICE1;
			strcpy (local_rec.prc_name [i], comm_rec.price1_desc);
			break;

		case	'2':
			local_rec.price_type [i] = PRICE2;
			strcpy (local_rec.prc_name [i], comm_rec.price2_desc);
			break;

		case	'3':
			local_rec.price_type [i] = PRICE3;
			strcpy (local_rec.prc_name [i], comm_rec.price3_desc);
			break;

		case	'4':
			local_rec.price_type [i] = PRICE4;
			strcpy (local_rec.prc_name [i], comm_rec.price4_desc);
			break;
	
		case	'5':
			local_rec.price_type [i] = PRICE5;
			strcpy (local_rec.prc_name [i], comm_rec.price5_desc);
			break;

		case	'6':
			local_rec.price_type [i] = PRICE5;
			strcpy (local_rec.prc_name [i], comm_rec.price6_desc);
			break;

		case	'7':
			local_rec.price_type [i] = PRICE5;
			strcpy (local_rec.prc_name [i], comm_rec.price7_desc);
			break;

		case	'8':
			local_rec.price_type [i] = PRICE5;
			strcpy (local_rec.prc_name [i], comm_rec.price8_desc);
			break;

		case	'9':
			local_rec.price_type [i] = PRICE5;
			strcpy (local_rec.prc_name [i], comm_rec.price9_desc);
			break;

		case	'I':
			p_width = 158;
			local_rec.price_type [i] = ITEM;
			strcpy (local_rec.prc_name [i], "Item No.");
			break;

		case	'O':
			p_width = 158;
			local_rec.price_type [i] = ON_HAND;
			strcpy (local_rec.prc_name [i], "On Hand");
			break;

		case	'L':
			local_rec.price_type [i] = PAGE_LIST;
			break;

		default:
			CloseDB (); 
			FinishProgram ();
            return (EXIT_FAILURE);
		}
	}

	/*
	 * Process.
	 */
	while (	scanf ("%d", &startPage) != EOF && 
	        scanf ("%d", &endPage) != EOF)
	{
		if (startPage < 0)
			break;

		if (endPage < 0)
			break;

		/*
		 * Process price book		
		 */
		Process (startPage, endPage);
	}
	/*
	 * print index for whole price book
	 */
	if (local_rec.pindex)
		PrintIndex ();

	CloseDB (); 
	FinishProgram ();
    return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (inph, inph_list, INPH_NO_FIELDS, "inph_hhph_hash");
	open_rec (inps, inps_list, INPS_NO_FIELDS, "inps_id_no");
	open_rec (inpl, inpl_list, INPL_NO_FIELDS, "inpl_id_no");
	open_rec (inpt, inpt_list, INPT_NO_FIELDS, "inpt_id_no");
	open_rec (inpc, inpc_list, INPC_NO_FIELDS, "inpc_id_no");
	open_rec (inpb, inpb_list, INPB_NO_FIELDS, "inpb_hhpb_hash");
	open_rec (inpd, inpd_list, INPD_NO_FIELDS, "inpd_id_no");
	open_rec (inpi, inpi_list, INPI_NO_FIELDS, "inpi_hhph_hash");
	open_rec (inpr, inpr_list, INPR_NO_FIELDS, "inpr_id_no2");
	OpenBasePrice ();
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (inpr);
	abc_fclose (ccmr);
	abc_fclose (inph);
	abc_fclose (inps);
	abc_fclose (inpl);
	abc_fclose (inpt);
	abc_fclose (inpc);
	abc_fclose (inpb);
	abc_fclose (inpd);
	abc_fclose (inpi);
	CloseBasePrice ();
	abc_dbclose ("data");
}

int		
Compare (
 const void *left, 
 const void *right)
{
	int		resault;
	
	const struct PriceBook a = * (const struct PriceBook *) left;
	const struct PriceBook b = * (const struct PriceBook *) right;

	resault = strcmp (a.reference, b.reference);
	return (resault);
}

void
PrintIndex (
 void)
{
	int		i;
	int		page_no = 1;
	int		indx = 0;
	int		last_alpha = 0;
	/*---------------------------
	| open output				|
	---------------------------*/
	p_width = 96;
	Heading (1, TRUE);
	fprintf (fout, "%-*.*s\n", p_width, p_width, ruler);
	fflush (fout);

	/*-----------------------------------
	| Process indexing information		|
	-----------------------------------*/
	inpd_rec.hhpb_hash = inpb_rec.hhpb_hash;
	inpd_rec.seq_no = 0;
	cc = find_rec (inpd, &inpd_rec, GTEQ, "r");
	while (indx < MAX_SORT && ValidInpd ())
	{
		/*-------------------------------
		| Process index references		|
		-------------------------------*/
		cc = find_hash (inpi, &inpi_rec, GTEQ, "r", inpd_rec.hhph_hash);
		while (indx < MAX_SORT && ValidInpi ())
		{
			inpi_rec.reference [0] = toupper (inpi_rec.reference [0]);
			strcpy (sort_tab [indx].reference, inpi_rec.reference);
			sort_tab [indx++].page_no = page_no;
			cc = find_hash (inpi, &inpi_rec, NEXT, "r", inpd_rec.hhph_hash);
		}
		page_no++;
		cc = find_rec (inpd, &inpd_rec, NEXT, "r");
	}
	/*-------------------------------
	| sort sort_tab table			|
	-------------------------------*/
	qsort ((char *)sort_tab, (unsigned)indx, sizeof (struct PriceBook), Compare);

	/*-----------------------------------
	| output sorted index references	|
	-----------------------------------*/
	for (i = 0;i < indx;i++)
	{
		/*-------------------------------
		| new section in index			|
		-------------------------------*/
		if (last_alpha != sort_tab [i].reference [0])
		{
			/*---------------------------
			| not first time			|
			---------------------------*/
			if (last_alpha != 0)
			{
				fprintf (fout, "|%-94.94s|\n", " ");
				fprintf (fout, ".LPR5\n");
				fflush (fout);
			}
			last_alpha = sort_tab [i].reference [0];
		}
		/*-------------------------------
		| print index reference			|
		-------------------------------*/
		fprintf (fout, "| %-50.50s ", sort_tab [i].reference);
		fprintf (fout, " %5d ", sort_tab [i].page_no);
		fprintf (fout, "%-35.35s|\n", " ");
		fflush (fout);
	}
	fprintf (fout, ".EOF\n");
	pclose (fout);
}

void
Heading (
 int page_no, 
 int p_index)
{
	/*---------------------------
	| open output				|
	---------------------------*/
	if ((fout = popen ("pformat", "w")) == 0)
		sys_err ("Error in pformat during (POPEN)", errno, PNAME);
	/*---------------------------
	| print Heading				|
	---------------------------*/
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.lpno);
	fprintf (fout, ".PG%d\n", page_no);
	fprintf (fout, ".NC%d\n", local_rec.ncopies);
	if (p_index)
		fprintf (fout, ".7\n");
	else
	{
		if (PRICE)
			fprintf (fout, ".7\n");
		else
			fprintf (fout, ".6\n");
	}
	fprintf (fout, ".L%d\n", p_width);
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".E%s - %s\n", comm_rec.co_no, clip (comm_rec.co_name));
	fprintf (fout, ".E%s - %s\n", clip (inpb_rec.book), clip (inpb_rec.description));
	fprintf (fout, ".EAS AT %s\n", SystemTime ());
	if (!p_index && PRICE)
		fprintf (fout, ".E%s\n", clip (local_rec.usr_head));
	fprintf (fout, ".R%s\n", ruler);
	fflush (fout);
}

void
Process (
 int startPage, 
 int endPage)
{
	int		page_no = 1;
	int		first_time = TRUE;
	/*---------------------------
	| Process pages				|
	---------------------------*/
	inpd_rec.hhpb_hash = inpb_rec.hhpb_hash;
	inpd_rec.seq_no = 0;
	cc = find_rec (inpd, &inpd_rec, GTEQ, "r");
	while (!cc && inpd_rec.hhpb_hash == inpb_rec.hhpb_hash)
	{
		/*---------------------------
		| valid page				|
		---------------------------*/
		if (ValidPage ())
			ProcessInph (inpd_rec.hhph_hash, page_no, &first_time);
		page_no++;
		cc = find_rec (inpd, &inpd_rec, NEXT, "r");
	}
	fprintf (fout, ".EOF\n");
	pclose (fout);
}

void
ProcessInph (
 long hhph_hash, 
 int page_no, 
 int *first_time)
{
	/*-----------------------
	| find page				|
	-----------------------*/
	cc = find_hash (inph, &inph_rec, COMPARISON, "r", hhph_hash);
	if (!cc)
	{
		/*-------------------------------
		| ignore blank pages			|
		-------------------------------*/
		if (BLANK_PAGE)
			return;
		/*-------------------------------
		| not first page printed		|
		-------------------------------*/
		if (*first_time)
		{
			Heading (page_no, FALSE);
			*first_time = FALSE;
		}
		else
		{
			fprintf (fout, ".PG%d\n", page_no);
/*
			if (local_rec.price_type [0] == PAGE_LIST)
				fprintf (fout, ".LRP5\n");
			else
				fprintf (fout, ".PA\n");
*/
		}
		/*---------------------------
		| Process page				|
		---------------------------*/
		ProcessPage (inph_rec.hhph_hash);
	}
}

void
ProcessPage (
 long hhph_hash)
{
	int		first_sub = TRUE;
	/*---------------------------
	| Process sub page			|
	---------------------------*/
	inps_rec.hhph_hash = hhph_hash;
	strcpy (inps_rec.sub_page, " ");
	cc = find_rec (inps, &inps_rec, GTEQ, "r");
	while (!cc && inps_rec.hhph_hash == hhph_hash)
	{
		/*-----------------------------------
		| blank line between sub pages		|
		-----------------------------------*/
		if (!first_sub)
		{
			fprintf (fout, "%-*.*s\n", p_width, p_width, ruler);
			fprintf (fout, ".B1\n");
		}
		else
			first_sub = FALSE;
		/*-----------------------------------
		| load column layout definition		|
		-----------------------------------*/
		LoadColumns (inps_rec.hhpr_hash);
		/*-------------------------------
		| Process sub page title		|
		-------------------------------*/
		PrintTitle (inps_rec.hhpr_hash, inps_rec.page_style);
		/*-------------------------------
		| print sub page data			|
		-------------------------------*/
		if (local_rec.price_type [0] != PAGE_LIST)
		       PrintData (inps_rec.hhpr_hash, inps_rec.page_style);

		cc = find_rec (inps, &inps_rec, NEXT, "r");
	}
}

void
LoadColumns (
 long hhpr_hash)
{
	int		i;
	char	*sptr;
	/*---------------------------------------
	| both Heading lines assumed to be blank|
	---------------------------------------*/
	max_columns = 0;
	blank_head [0] = TRUE;
	blank_head [1] = TRUE;
	/*---------------------------
	| Process columns			|
	---------------------------*/
	inpc_rec.hhpr_hash = hhpr_hash;
	inpc_rec.col_no = 0;
	cc = find_rec (inpc, &inpc_rec, GTEQ, "r");
	while (!cc && inpc_rec.hhpr_hash == hhpr_hash)
	{
		strcpy (col_store [max_columns].c_type, inpc_rec.col_type);
		/*-----------------------------------
		| printing one of the 9 prices		|
		-----------------------------------*/
		if (PRICE)
			col_store [max_columns].c_width = inpc_rec.width;
		else
		{
			if (col_store [max_columns].c_type [0] == '$')
				col_store [max_columns].c_width = 16;
			else
				col_store [max_columns].c_width = inpc_rec.width;
		}
		strcpy (col_store [max_columns].c_head [0], inpc_rec.heading_1);
		strcpy (col_store [max_columns].c_head [1], inpc_rec.heading_2);
		strcpy (col_store [max_columns++].c_format, inpc_rec.format);
		cc = find_rec (inpc, &inpc_rec, NEXT, "r");
	}
	/*-------------------------------
	| store column into table		|
	-------------------------------*/
	for (p_width = 0, i = 0;i < max_columns;i++)
	{
		p_width++;
		p_width += col_store [i].c_width;
		/*-------------------------------
		| first column Heading			|
		-------------------------------*/
		sptr = clip (col_store [i].c_head [0]);
		if (strlen (col_store [i].c_head [0]) != 0)
			blank_head [0] = FALSE;
		/*-------------------------------
		| second column Heading			|
		-------------------------------*/
		sptr = clip (col_store [i].c_head [1]);
		if (strlen (col_store [i].c_head [1]) != 0)
			blank_head [1] = FALSE;
		/*-------------------------------
		| column justification			|
		-------------------------------*/
		switch (col_store [i].c_format [0])
		{
		case	'R':
			col_store [i].c_lwidth [0] = col_store [i].c_width;
			col_store [i].c_lwidth [1] = col_store [i].c_width;
			col_store [i].c_lwidth [0] -= strlen (col_store [i].c_head [0]);
			col_store [i].c_lwidth [1] -= strlen (col_store [i].c_head [1]);
			break;

		case	'C':
			col_store [i].c_lwidth [0] = col_store [i].c_width;
			col_store [i].c_lwidth [1] = col_store [i].c_width;
			col_store [i].c_lwidth [0] -= strlen (col_store [i].c_head [0]);
			col_store [i].c_lwidth [1] -= strlen (col_store [i].c_head [1]);
			col_store [i].c_lwidth [0] /= 2;
			col_store [i].c_lwidth [1] /= 2;
			break;

		case	'L':
		default:
			col_store [i].c_lwidth [0] = 0;
			col_store [i].c_lwidth [1] = 0;
			break;
		}
	}
	p_width++;
}

void
PrintTitle (
 long hhpr_hash, 
 char *format)
{
	int		printed = FALSE;
	char	*sptr;
	/*---------------------------
	| Heading info				|
	---------------------------*/
	fprintf (fout, ".L%d\n", p_width);
	fprintf (fout, ".R%-*.*s\n", p_width, p_width, ruler);
	/*-------------------------------
	| Process title lines			|
	-------------------------------*/
	inpt_rec.hhpr_hash = hhpr_hash;
	inpt_rec.line_no = 0;
	cc = find_rec (inpt, &inpt_rec, GTEQ, "r");
	while (!cc && inpt_rec.hhpr_hash == hhpr_hash)
	{
		sptr = clip (inpt_rec.title);
		/*---------------------------
		| title formatting			|
		---------------------------*/
		switch (inpt_rec.format [0])
		{
		case	'E':
			fprintf (fout, ".e%s\n", sptr);
			printed++;
			break;

		case	'C':
			fprintf (fout, ".C%s\n", sptr);
			printed++;
			break;

		case	'B':
			fprintf (fout, ".E%s\n", sptr);
			printed++;
			break;

		case	'N':
		default:
			printed++;
			fprintf (fout, "%s\n", sptr);
			break;
		}
		cc = find_rec (inpt, &inpt_rec, NEXT, "r");
	}
	/*---------------------------
	| page listing				|
	---------------------------*/
	if (local_rec.price_type [0] == PAGE_LIST)
		return;
	if (printed)
		fprintf (fout, ".B1\n");
	/*---------------------------------------
	| at least one line in column Heading	|
	---------------------------------------*/
	if (blank_head [0] == 0 || blank_head [1] == 0)
		fprintf (fout, "%-*.*s\n", p_width, p_width, ruler);
	ColumnHead (0, format);
	ColumnHead (1, " ");
	fprintf (fout, "%-*.*s\n", p_width, p_width, ruler);
}

void
ColumnHead (
 int line_no, 
 char *format)
{
	int		i;
	int		cc_width;
	int		cl_width;
	int		p_count = 0;

	/*-------------------------------
	| column Heading line			|
	-------------------------------*/
	for (i = 0;blank_head [line_no] == 0 && i < max_columns;i++)
	{
		/*-------------------------------
		| print start of line			|
		-------------------------------*/
		if (i == 0)
			fprintf (fout, "|");
		/*-----------------------------------
		| calc width of column to print		|
		-----------------------------------*/
		cc_width = col_store [i].c_width;
		cl_width = col_store [i].c_lwidth [line_no];
		cc_width -= cl_width;
		/*-------------------------------
		| print column Heading			|
		-------------------------------*/
		if (format [0] == 'D' && col_store [i].c_type [0] == '$' && p_count < 3)
		{
			fprintf 
			(
				fout, 
				"%-*.*s%-*.*s", 
				cl_width, 
				cl_width, 
				" ", 
				cc_width, 
				cc_width, 
				local_rec.prc_name [p_count++]
			);
		}
		else
		{
			fprintf 
			(
				fout, 
				"%-*.*s%-*.*s", 
				cl_width, 
				cl_width, 
				" ", 
				cc_width, 
				cc_width, 
				col_store [i].c_head [line_no]
			);
		}

		/*-------------------------------
		| not the last column			|
		-------------------------------*/
		if (i != max_columns - 1)
			fprintf (fout, " ");
	}
	/*---------------------------
	| print end of line			|
	---------------------------*/
	if (!blank_head [line_no])
		fprintf (fout, "|\n");
}

void
PrintData (
	long	hhprHash, 
	char 	*format)
{
	int		max_col = (format [0] == 'A') ? 6 : 7;

	/*
	 * Process data records.
	 */
	inpl_rec.hhpr_hash 	= hhprHash;
	inpl_rec.line_no 	= 0;
	cc = find_rec (inpl, &inpl_rec, GTEQ, "r");
	while (!cc && inpl_rec.hhpr_hash == hhprHash)
	{
		if (format [0] == 'C')
			PrintRow (1, format);
		else
		{
			if (format [0] == 'D')
				PrintRow (3, format);
			else
				PrintRow (max_col, format);
		}

		cc = find_rec (inpl, &inpl_rec, NEXT, "r");
	}
}

void
PrintRow (
	int 	max_col, 
	char 	*format)
{
	int		i;
	int		j;
	int		ccol;
	int		cwidth;		/* width of field	*/
	int		prc_no;
	double	lcl_qty = 0.00;
	double	printPrice	=	0.00;

	/*---------------------------
	| Process columns			|
	---------------------------*/
	fprintf (fout, "|");
	for (ccol = 0, i = 0;ccol < max_columns;ccol++)
	{
		/*---------------------------
		| column width				|
		---------------------------*/
		cwidth = col_store [ccol].c_width;
		switch (col_store [ccol].c_type [0])
		{
		/*-----------------------
		| length				|
		-----------------------*/
		case	'L':
			fprintf (fout, "%-*.*s", cwidth, cwidth, inpl_rec.inpl_length);
			break;

		/*---------------------------
		| item number				|
		---------------------------*/
		case	'I':
			if (i < max_col)
			{
				cc = findItemCode (inpl_hhbr_hash [i]);
				if (cc)
					fprintf (fout, "%-*.*s", cwidth, cwidth, " ");
				else
					fprintf (fout, "%-*.*s", cwidth, cwidth, inmr_rec.item_no);
			}
			break;

		/*---------------------------
		| item description			|
		---------------------------*/
		case	'D':
			fprintf (fout, "%-*.*s", cwidth, cwidth, inmr_rec.description);
			break;

		/*-----------------------
		| $ price			 	|
		-----------------------*/
		case	'$':
			if (i < max_col)
			{
				if (format [0] == 'D')
					prc_no = i;
				else
					prc_no = 0;

				if (format [0] != 'D' || (format [0] == 'D' && i == 0))
					cc = findItemCode (inpl_hhbr_hash [i++]);
				else
				{
					cc = 0;
					i++;
				}

				if (cc)
					fprintf (fout, "%-*.*s", cwidth, cwidth, " ");
				else
				{
					switch (local_rec.price_type [prc_no])
					{
					case	PRICE1:
					case	PRICE2:
					case	PRICE3:
					case	PRICE4:
					case	PRICE5:
					case	PRICE6:
					case	PRICE7:
					case	PRICE8:
					case	PRICE9:
						printPrice	= 	FindBasePrice
										 (	
											comm_rec.est_no, 
											comm_rec.cc_no, 
											inmr_rec.hhbr_hash, 
											local_rec.price_type [prc_no], 
											envVarCurrCode
										);
						fprintf (fout, "%*.2f", cwidth, DOLLARS (printPrice));
						break;

					case	ITEM:
						fprintf (fout, "%-*.*s", cwidth, cwidth, inmr_rec.item_no);
						break;

					case	ON_HAND:
						fprintf (fout, "%*.2f", cwidth, inmr_rec.on_hand);
						break;
					default:
						fprintf (fout, "%-*.*s", cwidth, cwidth, " ");
						break;
					}
				}
			}
			break;

		/*-----------------------
		| pack size				|
		-----------------------*/
		case	'P':
			if (i < max_col)
			{
				j = i;
				for (j = i;j < max_col && inpl_hhbr_hash [j] == 0L;j++);
				cc = findItemCode (inpl_hhbr_hash [j]);
				if (cc || j == max_col)
					strcpy (inmr_rec.sale_unit, "    ");
			}
			else
				strcpy (inmr_rec.sale_unit, "    ");

			fprintf (fout, "%-*.*s", cwidth, cwidth, inmr_rec.sale_unit);
			break;

		/*-------------------------------
		| cost price        			|
		-------------------------------*/
		case	'C':
			fprintf (fout, "%*.2f", cwidth, inei_rec.std_cost);

			break;

		/*-------------------------------
		| quantity break       			|
		-------------------------------*/
		case	'Q':
			lcl_qty = GetQuantity ();
			fprintf (fout, "%*.2f", cwidth, lcl_qty);
			break;

		/*-------------------------------
		| formatting columns			|
		-------------------------------*/
		default:
		case	'O':
			fprintf (fout, "%-*.*s", cwidth, cwidth, " ");
			break;

		}
		if (ccol != max_columns - 1)
			fprintf (fout, " ");
	}
	fprintf (fout, "|\n");
}

double	
GetQuantity (void)
{
	double	qty_brk = (double) 0;

	inpr_rec.hhbr_hash	=	inpl_hhbr_hash [0];
	inpr_rec.price_type	=	1;
	cc = find_rec (inpr, &inpr_rec, GTEQ, "r");
	if (!cc && inpr_rec.hhbr_hash	==	inpl_hhbr_hash [0] &&
			   inpr_rec.price_type	==	1)
	{
		qty_brk = inpr_qty_brk [0];
	}
	return (qty_brk);
}

int
findItemCode (
	long	hhbrHash)
{
	inmr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (!cc)
	{
		strcpy (inei_rec.est_no, comm_rec.est_no);
		inei_rec.hhbr_hash	=	hhbrHash;
		cc = find_rec (inei, &inei_rec, COMPARISON, "r");
		if (cc)
			inei_rec.std_cost	=	0.00;
	}
	return (cc);
}
