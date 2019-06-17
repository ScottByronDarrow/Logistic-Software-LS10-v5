/*=====================================================================
|  Copyright (C) 1988 - 1992 LogisticSoftware Limited.                |
|=====================================================================|
|  Program Name  : ( so_psprint.c   )                                 |
|  Program Desc  : ( Background Process to Print Invoice / Credit )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  sobg,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  sobg,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison. | Date Written  : 25/10/88         |
|---------------------------------------------------------------------|
|  Date Modified : (25/10/88)      | Modified  by : Roger Gibbison.   |
|  Date Modified : (11/03/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (12/09/97)      | Modified  by : Marnie Organo     |
|  Date Modified : (05/10/1999)    | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  Comments      :                                                    |
|  (11/03/92)    : Updated to compile on ICL DRS-6000                 |
|  (12/09/97)    : Updated for Multilingual Conversion.               |
|  (05/10/1999)  : Ported to ANSI standards.                          |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: bg_monitor.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/bg_monitor/bg_monitor.c,v 5.1 2001/08/09 09:26:45 scott Exp $";

#include	<ml_std_mess.h>
#include	<ml_utils_mess.h>
#include	<pslscr.h>
#include	<signal.h>

#define		NUM(x,y)	(( data_str[ x ] - 48) + y )

#define		PA		( !strcmp( sobg_rec.bg_type, "PA" ) )
#define		PC		( !strcmp( sobg_rec.bg_type, "PC" ) )
#define		RC		( !strcmp( sobg_rec.bg_type, "RC" ) )
#define		RO		( !strcmp( sobg_rec.bg_type, "RO" ) )
#define		SU		( !strcmp( sobg_rec.bg_type, "SU" ) )
#define		RP		( !strcmp( sobg_rec.bg_type, "RP" ) )

void	bg_sigset(int x);

void	bg_sigset(int x)
{
}

	/*
	static	char	*tm_str[] = {
		"####","  # ","####","####","#  #","####","#   ","####","####","####",
		"#  #","  # ","   #","   #","#  #","#   ","#   ","   #","#  #","#  #",
		"#  #","  # ","####","####","####","####","####","   #","####","####",
		"#  #","  # ","#   ","   #","   #","   #","#  #","   #","#  #","   #",
		"####","  # ","####","####","   #","####","####","   #","####","   #",
	};
	*/
	char *	tm_str [50];

	int	num_init = FALSE;
	int	pass_num = 0;

	char	*sobg = "sobg",
			*data = "data";

	/*=============================
	| Background Processing file. |
	=============================*/
	struct dbview sobg_list[] ={
		{"sobg_co_no"},
		{"sobg_br_no"},
		{"sobg_type"},
		{"sobg_lpno"},
		{"sobg_hash"}
	};

	int sobg_no_fields = 5;

	struct {
		char	bg_co_no[3];
		char	bg_br_no[3];
		char	bg_type[3];
		int	bg_lpno;
		long	bg_hash;
	} sobg_rec;

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	(void);
void	InitStruct		(void);
void	FreeStruct		(void);
void	OpenDB			(void);
void	CloseDB			(void);
void	draw_screen		(void);
void	process			(void);
void	init_numbers	(void);
void	pr_num			(int x, int y, int val);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int	abort = 1;
	int	key;

	init_scr ();
	set_tty ();
	OpenDB ();
	draw_screen ();
	do 
	{
		signal (SIGALRM, bg_sigset);
	
		process();

		alarm(10);

		key = getkey();
		if ( key == FN16 )
		{
			abort = 0;
			alarm(0);
		}
		putchar(BELL);
		fflush(stdout);

	} while ( abort );
	signal( SIGALRM, SIG_DFL );
	crsr_on();
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
	FreeStruct ();
}

void
InitStruct (
 void)
{
	int i;
	for (i=0; i<50; i++)
		tm_str [i] = (char *) malloc (10);

	strcpy (tm_str [0],  "####");
	strcpy (tm_str [10], "#  #");
	strcpy (tm_str [20], "#  #");
	strcpy (tm_str [30], "#  #");
	strcpy (tm_str [40], "####");

	strcpy (tm_str [1],  "  # ");
	strcpy (tm_str [11], "  # ");
	strcpy (tm_str [21], "  # ");
	strcpy (tm_str [31], "  # ");
	strcpy (tm_str [41], "  # ");

	strcpy (tm_str [2],  "####");
	strcpy (tm_str [12], "   #");
	strcpy (tm_str [22], "####");
	strcpy (tm_str [32], "#   ");
	strcpy (tm_str [42], "####");

	strcpy (tm_str [3],  "####");
	strcpy (tm_str [13], "   #");
	strcpy (tm_str [23], "####");
	strcpy (tm_str [33], "   #");
	strcpy (tm_str [43], "####");

	strcpy (tm_str [4],  "#  #");
	strcpy (tm_str [14], "#  #");
	strcpy (tm_str [24], "####");
	strcpy (tm_str [34], "   #");
	strcpy (tm_str [44], "   #");

	strcpy (tm_str [5],  "####");
	strcpy (tm_str [15], "#   ");
	strcpy (tm_str [25], "####");
	strcpy (tm_str [35], "   #");
	strcpy (tm_str [45], "####");

	strcpy (tm_str [6],  "#   ");
	strcpy (tm_str [16], "#   ");
	strcpy (tm_str [26], "####");
	strcpy (tm_str [36], "#  #");
	strcpy (tm_str [46], "####");

	strcpy (tm_str [7],  "####");
	strcpy (tm_str [17], "   #");
	strcpy (tm_str [27], "   #");
	strcpy (tm_str [37], "   #");
	strcpy (tm_str [47], "   #");

	strcpy (tm_str [8],  "####");
	strcpy (tm_str [18], "#  #");
	strcpy (tm_str [28], "####");
	strcpy (tm_str [38], "#  #");
	strcpy (tm_str [48], "####");

	strcpy (tm_str [9],  "####");
	strcpy (tm_str [19], "#  #");
	strcpy (tm_str [29], "####");
	strcpy (tm_str [39], "   #");
	strcpy (tm_str [49], "   #");
}

void
FreeStruct (
 void)
{
	int i;
	for (i=0; i<50; i++)
		free (tm_str [i]);
}

void
OpenDB (
 void)
{
	InitStruct ();
	abc_dbopen (data);
	open_rec (sobg, sobg_list, sobg_no_fields, "sobg_id_no_2");
}

void
CloseDB (
 void)
{
	abc_fclose (sobg);
	abc_dbclose (data);
}

void
draw_screen (
 void)
{
	init_numbers();
	clear();
	crsr_off();

	/*print_at(0,14,"%R B A C K G R O U N D   P R O C E S S   M O N I T O R ");*/
	print_at(0,14,ML(mlUtilsMess028));
	move(0,1);
	line(79);

	box(0,  3, 25, 5);
	box(27, 3, 25, 5);
	box(54, 3, 25, 5);

	box(0,  10, 25, 5);
	box(27, 10, 25, 5);
	box(54, 10, 25, 5);

	box(0, 19, 25, 1);
	box(54, 19, 25, 1);

/*
	rv_pr( " One Step P/Slip. ", 	 3,  3, TRUE);
	rv_pr( " Two Step P/Slip. ",	30,  3, TRUE);
	rv_pr( " Stock Update. ", 	59,  3, TRUE);
	rv_pr( " Recalc of Stock. ", 	 3, 10, TRUE);
	rv_pr( " Recalc of Customers ", 29, 10, TRUE);
	rv_pr( " Misc BG Records. ", 	59, 10, TRUE);
*/

	rv_pr(ML(mlUtilsMess029), 	 3,  3, TRUE);
	rv_pr(ML(mlUtilsMess030),	30,  3, TRUE);
	rv_pr(ML(mlUtilsMess031), 	59,  3, TRUE);
	rv_pr(ML(mlUtilsMess032), 	 3, 10, TRUE);
	rv_pr(ML(mlUtilsMess033), 29, 10, TRUE);
	rv_pr(ML(mlUtilsMess034), 	59, 10, TRUE);
}

void
process (
 void)
{
	int	tot_pa = 0,
		tot_pc = 0,
		tot_ro = 0,
		tot_rc = 0,
		tot_su = 0,
		tot_misc = 0;

	strcpy (sobg_rec.bg_type, "  ");
	sobg_rec.bg_lpno = 0;
	cc = find_rec(sobg,&sobg_rec,GTEQ,"r");
	while (!cc)
	{
		if ( PA )
			tot_pa++;

		else if ( PC )
			tot_pc++;

		else if ( RO )
			tot_ro++;

		else if ( RC || RP )
			tot_rc++;

		else if ( SU )
			tot_su++;
		else
			tot_misc++;

		cc = find_rec(sobg,&sobg_rec,NEXT,"r");
	}
	pr_num (  3,  4, tot_pa);
	pr_num ( 30,  4, tot_pc);
	pr_num ( 57,  4, tot_su);
	pr_num (  3, 11, tot_rc);
	pr_num ( 30, 11, tot_ro);
	pr_num ( 57, 11, tot_misc);
	/*print_at( 20, 2, "%R Number Records %4d ", */
	print_at (20, 2, ML(mlUtilsMess035), 
					tot_pa + tot_pc + tot_ro +
					tot_rc + tot_su + tot_misc);
	/*print_at( 20, 56, "%R Number Passes %5d ", ++pass_num);*/
	print_at( 20, 56, ML(mlUtilsMess036), ++pass_num);
	return;
}

/*===============
| Init numbers. |
===============*/
void
init_numbers (
 void)
{
	int	k,j;

	if ( num_init )
		return;

	for ( k = 0 ; k < 50; k++ )
	    	for ( j = 0; j < 4; j++ )
			tm_str[k][j] = (tm_str[k][j] == '#') ? ta[12][13] : ' ';

	num_init = 1;
}

/*================
| Process clock. |
================*/
void
pr_num (
 int x,
 int y,
 int val)
{
	char	data_str[5];
	int	i;
		
	sprintf(data_str, "%04d", val);

	gr_on();
	for ( i = 0; i < 5; i++ )
	{
		/*move( x ,y + i );*/
		if ( val > 999 )
		{
			print_at(y + i, x,"%-4.4s %-4.4s %-4.4s %-4.4s",
			    			tm_str[ NUM( 0, i*10 ) ], 
						tm_str[ NUM( 1, i*10 ) ],
			    			tm_str[ NUM( 2, i*10 ) ], 
						tm_str[ NUM( 3, i*10 ) ] );
		}
		else if ( val > 99 )
		{
			print_at(y + i, x,"%-4.4s %-4.4s %-4.4s %-4.4s",
						"    ",
						tm_str[ NUM( 1, i*10 ) ],
			    			tm_str[ NUM( 2, i*10 ) ], 
						tm_str[ NUM( 3, i*10 ) ] );
		}
		else if ( val > 9 )
		{
			print_at(y + i, x,"%-4.4s %-4.4s %-4.4s %-4.4s",
						"    ",
						"    ",
			    			tm_str[ NUM( 2, i*10 ) ], 
						tm_str[ NUM( 3, i*10 ) ] );
		}
		else 
		{
			print_at(y + i, x,"%-4.4s %-4.4s %-4.4s %-4.4s",
						"    ",
						"    ",
						"    ",
						tm_str[ NUM( 3, i*10 ) ] );
		}
	}
	gr_off();
	fflush( stdout );
}
