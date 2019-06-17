/*=====================================================================
|  Copyright (C) 1988 - 1993 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( db_labels.c   )                                  |
|  Program Desc  : ( Creates records for labels                      )|
|                  ( links to cr_labels                              )|
|---------------------------------------------------------------------|
|  Access files  : comm ,lbhr ,lbln ,cumr ,sumr ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  lbhr, lbln,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 29/12/92         |
|---------------------------------------------------------------------|
|  Date Modified : (16/06/93)      | Modified  by : Trevor van Bremen |
|  Date Modified : (12/09/97)      | Modified  by : Roanna Marcelino  |
|                                                                     |
|  (16/06/93)    : PSL 9086. Fix to compile warning on SCO.           |
|  (12/09/97)    : Modified for Multilingual Conversion.              |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: psl_labels.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/psl_labels/psl_labels.c,v 5.2 2001/08/09 09:27:27 scott Exp $";
#define	TXT_REQD
#define	TABLINES 5
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_utils_mess.h>

#define	CUMR_NO_FIELDS	40
#define	SUMR_NO_FIELDS	26

	/*=======
	| Label |
	=======*/
	struct dbview lbhr_list[] ={
		{"lbhr_filename"},
		{"lbhr_across"},
		{"lbhr_down"},
		{"lbhr_x_size"},
		{"lbhr_y_size"},
		{"lbhr_x_offset"},
		{"lbhr_y_offset"},
		{"lbhr_x_sp"},
		{"lbhr_y_sp"},
		{"lbhr_lines"},
	};

	int	lbhr_no_fields = 10;

	struct	{
		char	hr_filename[14];
		int		hr_across;
		int		hr_down;
		int		hr_x_size;
		int		hr_y_size;
		int		hr_x_offset;
		int		hr_y_offset;
		int		hr_x_sp;
		int		hr_y_sp;
		int		hr_lines;
	} lbhr_rec;

	/*=========================
	| Label line descriptions |
	=========================*/
	struct dbview lbln_list[] ={
		{"lbln_filename"},
		{"lbln_line_no"},
		{"lbln_field_no"},
		{"lbln_field_name"},
		{"lbln_fld_master"},
		{"lbln_field_size"},
	};

	int	lbln_no_fields = 6;

	struct	{
		char	ln_filename[14];
		int		ln_line_no;
		int		ln_field_no;
		char	ln_field_name[19];
		int		ln_fld_master;
		int		ln_field_size;
	} lbln_rec;


	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
	};

	int	comm_no_fields = 5;

	struct	{
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
	} comm_rec;

	char	*lbhr	=	"lbhr",
			*lbln	=	"lbln",
			*comm	=	"comm",
			*data	=	"data";
		
	char	cumr_fields[CUMR_NO_FIELDS][19] = {
		"Company No       ","cumr_co_no",
		"Branch No        ","cumr_est_no",
		"Customer No      ","cumr_dbt_no",
		"Customer Name    ","cumr_dbt_name",
		"Acronym          ","cumr_dbt_acronym",
		"Charge Add 1     ","cumr_ch_adr1",
		"Charge Add 2     ","cumr_ch_adr2",
		"Charge Add 3     ","cumr_ch_adr3",
		"Charge Add 4     ","cumr_ch_adr4",
		"Delivery Add 1   ","cumr_dl_adr1",
		"Delivery Add 2   ","cumr_dl_adr2",
		"Delivery Add 3   ","cumr_dl_adr3",
		"Delivery Add 4   ","cumr_dl_adr4",
		"Postal Code      ","cumr_post_code",
		"Contact Name #1  ","cumr_contact_name",
		"Contact Name #2  ","cumr_contact2_name",
		"Contact Name #3  ","cumr_contact3_name",
		"Phone No         ","cumr_phone_no",
		"Fax Number       ","cumr_fax_no",
		"Salesman Number  ","cumr_sman_code",
	};

	char	sumr_fields[SUMR_NO_FIELDS][19] = {
		"Company No       ","sumr_co_no",
		"Branch No        ","sumr_est_no",
		"Suppliers No     ","sumr_crd_no",
		"Suppliers Name   ","sumr_crd_name",
		"Suppliers Acronym","sumr_acronym",
		"Address 1        ","sumr_adr1",
		"Address 2        ","sumr_adr2",
		"Address 3        ","sumr_adr3",
		"Address 4        ","sumr_adr4",
		"Contact Name #1  ","sumr_cont_name",
		"Contact Name #2  ","sumr_cont2_name",
		"Contact Name #3  ","sumr_cont3_name",
		"Phone No         ","sumr_cont_no",
	};

	int		exists    = FALSE;		
	int		Customer  = FALSE;
	int 	Supplier  = FALSE;
	int		no_fields;
	int     fh;
	int     total_fld ;

/*===========================
| Local & Screen Structures |
===========================*/
struct {		
	char	dummy[11];
	char	txt[81];
	char	filename[11];
	char	FileMask[11];
	int		fld_no;
} local_rec;            

static	struct	var	vars[] =
{
	{1, LIN, "filename",	 3, 22, CHARTYPE,
		local_rec.FileMask, "          ",
		" ", "", " Label Code        :", "Enter label code [SEARCH] available.",
		 NE, NO,  JUSTLEFT, "", "", local_rec.filename},
	{2, LIN, "lbls_across",	 3, 26, INTTYPE,
		"NN", "          ",
		"", "", "No. of Labels Across    :", "Enter the number of labels across the page ",
		 YES, NO,  JUSTLEFT, "", "", (char *) &lbhr_rec.hr_across},
	{2, LIN, "lbls_down",	 4, 26, INTTYPE,
		"NN", "          ",
		"", "", "No. of Labels Down      :", "Enter the number of labels down the page ",
		 YES, NO,  JUSTLEFT, "", "", (char *) &lbhr_rec.hr_down},
	{2, LIN, "top_off",       6, 26, INTTYPE,
		"NN", "          ",
		"", "", "Offset from Top         :", "Enter the number of lines offset for the top of page ",
		 YES, NO,  JUSTLEFT, "", "", (char *) &lbhr_rec.hr_y_offset},
	{2, LIN, "side_off",	 7, 26, INTTYPE,
		"NN", "          ",
		"", "", "Offset from Side        :", "Enter the number of lines offset for the side of page ",
		 YES, NO,  JUSTLEFT, "", "", (char *) &lbhr_rec.hr_x_offset},
	{2, LIN, "width",	 9, 26, INTTYPE,
		"NNN", "          ",
		"", "", "Width of Label          :", "Enter the number of characters in width each label can be",
		 YES, NO,  JUSTLEFT, "1", "72", (char *) &lbhr_rec.hr_x_size},
	{2, LIN, "height",       10, 26, INTTYPE,
		"NNN", "          ",
		"", "", "Height of Label         :", "Enter the number of lines in height each label can be ",
		 YES, NO,  JUSTLEFT, "1", "12", (char *) &lbhr_rec.hr_y_size},
	{2, LIN, "sp_x",	 	12, 26, INTTYPE,
		"NN", "          ",
		"", "", "Horz. Inter-Label Space :", "Enter the number of characters between labels across the page. ",
		 YES, NO,  JUSTLEFT, "", "", (char *) &lbhr_rec.hr_x_sp},
	{2, LIN, "sp_y",         13, 26, INTTYPE,
		"NN", "          ",
		"", "", "Vert. Inter-Label Space :", "Enter the number of characters between labels down the page. ",
		 YES, NO,  JUSTLEFT, "", "", (char *) &lbhr_rec.hr_y_sp},
	{2, LIN, "lines",        15, 26, INTTYPE,
		"NNN", "          ",
		" ", " ", "Lines Per Page          :", "Enter the number of printable lines down the page ",
		 YES, NO,  JUSTLEFT, "", "", (char *) &lbhr_rec.hr_lines},
	{3, TAB, "lcl_fld",	MAXLINES, 0, INTTYPE,
		"NN", "          ",
		 "", "", "Field", "",
		 NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.fld_no},
	{3, TAB, "mr_fld",	0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAA", "          ",
		 "", "", "Filled With           ", "",
		 YES, NO, JUSTLEFT, "", "", lbln_rec.ln_field_name},
	{3, TAB, "lbln_line",	0, 0, INTTYPE,
		"NN", "          ",
		 "", "", "", "",
		 ND, NO, JUSTLEFT, "", "", (char *) &lbln_rec.ln_line_no},
	{3, TAB, "lbln_fld_no",	0, 0, INTTYPE,
		"NN", "          ",
		 "", "", "", "",
		 ND, NO, JUSTLEFT, "", "", (char *) &lbln_rec.ln_field_no},
	{3, TAB, "lbln_fld_mr",	0, 0, INTTYPE,
		"NN", "          ",
		 "", "", "", "",
		 ND, NO, JUSTLEFT, "", "", (char *) &lbln_rec.ln_fld_master},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
int design (void);
void shutdown_prog (void);
int heading (int scn);
int spec_valid (int field);
void OpenDB (void);
void CloseDB (void);
void update (void);
void update_design (int lines);
void save_lbln (int line_no, int fld_no, int blank, int lngth);
void load_lbln (void);
void delete_rest_lbln (int line_no, int fld_no);
void srch_lbhr (char *key_val);
void srch_fields (char *key_val);
void down_shift (char *str);
void load_into_tab (void);
void save_descs (void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int                argc,
 char*              argv[])
{
	tab_row = 14;
	tab_col = 25;

	if ( argc < 2 )
	{
		/*printf ("Usage : %s C(ustomer) S)upplier \n", argv[0]);*/
		print_at (0,0,ML(mlUtilsMess142), argv[0]);
        return (EXIT_FAILURE);
	}

	if ( !strncmp ( argv[1], "C", 1 ))
	{
		Customer = TRUE;
		no_fields = CUMR_NO_FIELDS;
		strcpy (local_rec.FileMask, "C-AAAAAAAA");

	}
	else
	{
		Supplier = TRUE;
		no_fields = SUMR_NO_FIELDS;
		strcpy (local_rec.FileMask, "S-AAAAAAAA");
	}

	OpenDB ();

	SETUP_SCR (vars);

	init_scr();
	set_tty();
	set_masks();

	prog_exit = 0;
	while (prog_exit == 0)
	{
		search_ok = 1;
		init_ok = TRUE;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_vars(1);	
		init_vars(2);	
		init_vars(3);	

		heading(1);
		entry(1);
		if (prog_exit || restart)
			continue;
		
		no_edit (1);

		heading(2);
		if ( exists )
		{
			scn_display (2);
			edit (2);
		}
		else
			entry(2);

		if (restart)
			continue;

		if ( !exists )
			edit (2);

		if (restart)
			continue;
		
		if ( design ())
			continue;

		heading (3);
		scn_display (3);
		edit (3);

		txt_close (fh, FALSE);
		if ( lbhr_rec.hr_x_size > 78 )
			snorm ();

		if (restart)
			continue;

		save_descs ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

int
design (void)
{
	int	scn_width = 78;
	int 	y_coord;
	int 	x_coord;

	heading (99);

	if ( lbhr_rec.hr_x_size > 78 )
	{
		scn_width = 130;
		swide ();
	}

	y_coord =  2;
	x_coord =  ( scn_width - lbhr_rec.hr_x_size ) / 2;
	tab_row = lbhr_rec.hr_y_size + 5;
	if (tab_row > 13)
		tab_row = 13;

	fh = txt_open ( y_coord,
					x_coord,
		        	lbhr_rec.hr_y_size,
		        	lbhr_rec.hr_x_size,
		        	lbhr_rec.hr_y_size,
		        	" Label Design " 
		     	);

		  /*"Enter \"xxxxx....\" to show position and length of fields ");*/
	centre_at ( y_coord + lbhr_rec.hr_y_size + 2, 80, "%s", 
		  ML(mlUtilsMess132));

	if ( fh < 0 )
		return(1);

	load_lbln ();

	txt_edit (fh);

	centre_at ( y_coord + lbhr_rec.hr_y_size + 2, 80, "%s", 
		  "                                                          ");
	
	if ( !restart )
		update_design ( lbhr_rec.hr_y_size );
	
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

/*================
| Print Heading. |
================*/
int
heading (
 int                scn)
{
	if ( scn != 3 )
		clear ();

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		if (Customer)
			strcpy ( err_str,ML(mlUtilsMess137));
		else
			strcpy(err_str, ML(mlUtilsMess138));

		rv_pr(err_str,40 - ( strlen(err_str) / 2 ),0,1);
		
		move(0,1);
		line(80);

		if ( scn == 1 )
			box(0,2,80,1);
		if ( scn == 2 )
		{
			move(0,5);
			line(80);
			move(0,8);
			line(80);
			move(0,11);
			line(80);
			move(0,14);
			line(80);
			box(0,2,80,13);
		}
		if ( scn == 3 )
		{
			move( tab_col, tab_row -1 );
			PGCHAR ( 0 );
			move( tab_col + 1, tab_row - 1 );
			line ( 6 );
			PGCHAR ( 8 );
			line ( 23 );
			PGCHAR ( 1 );

			move( tab_col, tab_row + TABLINES + 2);
			PGCHAR ( 2 );
			move( tab_col + 1, tab_row + TABLINES + 2);
			line ( 6 );
			PGCHAR ( 9 );
			line ( 23 );
			PGCHAR ( 3 );
		}
		move(0,21);
		if ( scn == 3 )
		{
			line ( 26 );
			move ( 55, 21 );
			line ( 24 );
		}
		else
			line(80);

		print_at ( 22,0, ML(mlStdMess038),
				   comm_rec.tco_no, clip (comm_rec.tco_name));


		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int                field)
{
	int	i;  

	if ( (LCHECK ("lbls_down") ||
	      LCHECK ("top_off")   ||
	      LCHECK ("height")    ||
	      LCHECK ("sp_y")) && prog_status != ENTRY
	   )
	{
		if ( lbhr_rec.hr_lines < lbhr_rec.hr_y_offset +
				       ( lbhr_rec.hr_down *
				       ( lbhr_rec.hr_y_size +
					 lbhr_rec.hr_y_sp ))
		   )
		   {
			lbhr_rec.hr_lines = lbhr_rec.hr_y_offset +
					  ( lbhr_rec.hr_down *
					  ( lbhr_rec.hr_y_size +
					    lbhr_rec.hr_y_sp ));

			DSP_FLD ("lines");
			/*print_mess ( "Number of Lines Automatically Adjusted ");*/
			print_mess (ML(mlUtilsMess131));
			sleep (sleepTime);
			clear_mess ();
		   }

		return (EXIT_SUCCESS);
	}

	if ( LCHECK ("mr_fld"))
	{
		if ( SRCH_KEY )
		{
			srch_fields ( temp_str );
			clear ();
			txt_display ( fh, 1 );
			return (EXIT_SUCCESS);
		}

		if ( Customer )
		{
			for (i = 0;i < no_fields;i += 2)
				if ( !strcmp ( temp_str, cumr_fields[ i + 1 ] ))
				{
					lbln_rec.ln_fld_master = i/2;
					return (EXIT_SUCCESS);
				}
		}
		else
		{
			for (i = 0;i < no_fields;i += 2)
				if ( !strcmp ( temp_str, sumr_fields[ i + 1 ] ))
				{
					lbln_rec.ln_fld_master = i/2;
					return (EXIT_SUCCESS);
				}
		}

		/*print_mess ( "Not a Valid Field ");*/
		print_mess (ML(mlUtilsMess083));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if ( LCHECK ("lines"))
	{
		if ( dflt_used )
		{
			/*================================
			| equals top offset +            |
			| number of labels *             |
			| ( hgt of label + inter space ) |
			================================*/

			lbhr_rec.hr_lines = lbhr_rec.hr_y_offset +
					    ( lbhr_rec.hr_down *
					    ( lbhr_rec.hr_y_size +
					      lbhr_rec.hr_y_sp ));
		}
		if ( lbhr_rec.hr_lines < lbhr_rec.hr_y_offset +
					 ( lbhr_rec.hr_down *
					 ( lbhr_rec.hr_y_size +
					   lbhr_rec.hr_y_sp )))
		   {
			/*print_err ( "Label Specifications Equate To More Lines Than Entered\007");*/
			print_err (ML(mlUtilsMess133));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		   }

		DSP_FLD ( "lines" );
		return (EXIT_SUCCESS);
	}

	if ( LCHECK ("sp_x"))
	{
		if((lbhr_rec.hr_x_size + lbhr_rec.hr_x_sp) * lbhr_rec.hr_across > 158 )
		{
			
			/*print_err ( "Label Specifications Equate To Wider Than 158 Chars ");*/
			print_err (ML(mlUtilsMess134));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}

	if ( LCHECK ("lbls_across"))
	{
		if ( prog_status == ENTRY )
			lbhr_rec.hr_x_sp = 0;

		if((lbhr_rec.hr_x_size + lbhr_rec.hr_x_sp) * lbhr_rec.hr_across > 158 )
		{
			
			/*print_err ( "Label Specifications Equate To Wider Than 158 Chars ");*/
			print_err (ML(mlUtilsMess134));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}

	if ( LCHECK ("width"))
	{
		if((lbhr_rec.hr_x_size + lbhr_rec.hr_x_sp) * lbhr_rec.hr_across > 158 )
		{
			
			/*print_err ( "Label Specifications Equate To Wider Than 158 Chars ");*/
			print_err (ML(mlUtilsMess134));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	if ( LCHECK ("filename"))
	{
		if ( SRCH_KEY )
		{
			srch_lbhr (temp_str);
			return (EXIT_SUCCESS);
		}

		if ( Customer )
		{
			if (local_rec.filename[0] != 'c' &&
					local_rec.filename[0] != 'C'  )
			{
				/*print_mess ("This Label Code Is Not A Customer Label - 1st Char Must Be C ");*/
				print_mess (ML(mlUtilsMess135));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			;
		}
		else
		{
			if (local_rec.filename[0] != 's' &&
					local_rec.filename[0] != 'S'  )
			{
				/*print_mess ("This Label Code Is Not A Supplier Label - 1st Char Must Be S ");*/
				print_mess (ML(mlUtilsMess136));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			;
		}

		sprintf ( lbhr_rec.hr_filename, "%2.2s%s", comm_rec.tco_no,
						          local_rec.filename );

		cc = find_rec ( lbhr, &lbhr_rec, COMPARISON, "r" );

		if ( !cc )
			exists = TRUE;
		else
			exists = FALSE;		

	}

	return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen(data);

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec (lbln, lbln_list, lbln_no_fields, "lbln_id_no");
	open_rec (lbhr, lbhr_list, lbhr_no_fields, "lbhr_filename");
}

void
CloseDB (void)
{
	abc_fclose (lbhr);
	abc_fclose (lbln);
	abc_dbclose (data);
}

void
update (void)
{

	if ( exists )
		cc = abc_update ( lbhr, &lbhr_rec );
	else
		cc = abc_add ( lbhr, &lbhr_rec );

	if ( cc )
		file_err ( cc, lbhr, (exists) ? "DBUPDATE" : "DBADD" );
}

void
update_design (
 int                lines)
{
	int     count;
	int     lgth;
	int     total = 0;
	int     field ;
	int     is_blank;
	char    txt[133];
	char    *tmp;

	init_vars (3);
	scn_set (3);
	lcount[3] = 0;

	for ( count = 1; count <= lines; count++ )
	{
		field = 1;
		strcpy ( txt , txt_gval ( fh, count ));

		/*=================================
		| see whether line starts with     |
		| blank or char then break line    |
		| up into blocks of blanks & chars |
		==================================*/

		while ( TRUE )
		{
			if ( txt[0] == ' ' )
				is_blank = TRUE;
			else
				is_blank = FALSE;

			if ( is_blank )
				tmp = strchr ( txt, 'x' );
			else
				tmp = strchr ( txt, ' ' );

			/*=================================
			| means field goes to end of line |
			=================================*/
			if ( !tmp )
			{
				lgth = lbhr_rec.hr_x_size - total;
				save_lbln ( count, field++, is_blank, lgth );
				if ( !is_blank )
					load_into_tab ();
				total = 0;
				break;
			}

			lgth = tmp - txt;
			total += lgth;
			strcpy ( txt , txt + lgth );

			save_lbln ( count, field++, is_blank, lgth );
			if ( !is_blank )
				load_into_tab ();
		}
		delete_rest_lbln ( count, field );
	}

	update ();
}

void
save_lbln (
 int                line_no,
 int                fld_no,
 int                blank,
 int                lngth)
{
	int non_exist;

	strcpy ( lbln_rec.ln_filename, lbhr_rec.hr_filename );
	lbln_rec.ln_line_no  = line_no;
	lbln_rec.ln_field_no = fld_no;

	non_exist = find_rec ( lbln, &lbln_rec, COMPARISON, "u" );

	lbln_rec.ln_field_size = lngth;

	/*==============================
	| if changing field from blank  |
	| to non blank then reset field |
	===============================*/
	if ( !strncmp ( lbln_rec.ln_field_name, "blank", 5 ))
		if ( !blank )
			sprintf( lbln_rec.ln_field_name, "%18.18s", " " );

	if ( blank )
	{
		sprintf( lbln_rec.ln_field_name, "%-18.18s", "blank" );
		 lbln_rec.ln_fld_master = 99;
	}

	if ( non_exist )
		cc = abc_add ( lbln, &lbln_rec );
	else
		cc = abc_update ( lbln, &lbln_rec );

	if ( cc )
		file_err ( cc, lbln, (non_exist) ? "DBADD" : "DBUPDATE" );

}

void
load_lbln (void)
{
	char    tmp_str[133];
	int     line_no = 1;
	int     i;

	tmp_str[0] = '\0';
	strcpy ( lbln_rec.ln_filename, lbhr_rec.hr_filename );
	lbln_rec.ln_line_no  = line_no;
	lbln_rec.ln_field_no = 0;

	cc = find_rec ( lbln, &lbln_rec, GTEQ, "r" );

	while ( !cc )
	{
		if (strcmp (lbln_rec.ln_filename,lbhr_rec.hr_filename))
			break;

		while ( !cc && lbln_rec.ln_line_no == line_no )
		{
			if ( !strncmp ( lbln_rec.ln_field_name, "blank", 5 ))
			{
				for (i = 0; i < lbln_rec.ln_field_size; i++ )
					strcat ( tmp_str, " " );
			}
			else
			{
				for (i = 0; i < lbln_rec.ln_field_size; i++ )
					strcat ( tmp_str, "x" );
			}
			cc = find_rec ( lbln, &lbln_rec, NEXT, "r" );
		}
		txt_pval ( fh, tmp_str, line_no++ );
		tmp_str[0] = '\0';

	}
}

void
delete_rest_lbln (
 int                line_no,
 int                fld_no)
{
	strcpy ( lbln_rec.ln_filename, lbhr_rec.hr_filename );
	lbln_rec.ln_line_no  = line_no;
	lbln_rec.ln_field_no = fld_no;
	cc = find_rec ( lbln, &lbln_rec, GTEQ, "u" );

	while ( !cc && lbln_rec.ln_line_no == line_no &&
		!strcmp (lbln_rec.ln_filename, lbhr_rec.hr_filename) )
	{
		cc = abc_delete ( lbln );
		if ( cc )
			file_err ( cc, lbln, "DBDELETE" );

		strcpy ( lbln_rec.ln_filename, lbhr_rec.hr_filename );
		lbln_rec.ln_line_no  = line_no;
		lbln_rec.ln_field_no = fld_no++;
		cc = find_rec ( lbln, &lbln_rec, GTEQ, "u" );

	}
	abc_unlock ( lbln );
}

/*======================
| Search for Label Code |
=======================*/
void
srch_lbhr (
 char*              key_val)
{
	char    tmp_str[13];

	work_open();
	save_rec("#Label Code   ","#");
	sprintf ( tmp_str, "%2.2s%s", comm_rec.tco_no, key_val );
	clip ( tmp_str );
	strcpy ( lbhr_rec.hr_filename, tmp_str );

	cc = find_rec("lbhr",&lbhr_rec,GTEQ,"r");
	while (!cc && !strncmp( lbhr_rec.hr_filename, tmp_str, strlen(tmp_str)) )
	{
		if ( Customer && ( lbhr_rec.hr_filename[2] != 'C' && 
		 		lbhr_rec.hr_filename[2] != 'c' ))
		{
			cc = find_rec("lbhr",&lbhr_rec,NEXT,"r");
			continue;	
		}
		if ( !Customer && ( lbhr_rec.hr_filename[2] != 'S' && 
		 		lbhr_rec.hr_filename[2] != 's' ))
		{
			cc = find_rec("lbhr",&lbhr_rec,NEXT,"r");
			continue;	
		}
		/*===========================
		| plus 2 b'cos we don't want |
		| co no showing ( do we ?? ) |
		============================*/
		cc = save_rec( lbhr_rec.hr_filename + 2, " " );
		if (cc)
			break;
		cc = find_rec("lbhr",&lbhr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	sprintf ( lbhr_rec.hr_filename , "%2.2s%s", comm_rec.tco_no, temp_str );
	cc = find_rec("lbhr",&lbhr_rec,COMPARISON,"r");
	if (cc)
		file_err(cc,"lbhr","DBFIND");
}

void
srch_fields (
 char*              key_val)
{
	int	i;

	work_open();
	save_rec("#Database Field","#Description");

	for (i = 0;i < no_fields;i += 2)
	{
		if ( Customer )
			sprintf(err_str,"%-18.18s",cumr_fields [i + 1]);
		else
			sprintf(err_str,"%-18.18s",sumr_fields [i + 1]);

		down_shift (err_str);
		if (!strncmp(err_str,key_val,strlen(key_val)))
		{
			if ( Customer )
				cc = save_rec(err_str,cumr_fields [ i ]);
			else
				cc = save_rec(err_str,sumr_fields [ i ]);
			if (cc)
				break;
		}
	}
	cc = disp_srch();
	work_close();
	sprintf ( lbln_rec.ln_field_name, "%-18.18s", temp_str);
}

void
down_shift (
 char*              str)
{
	char	*sptr = str;

	while (*sptr)
	{
		*sptr = tolower(*sptr);
		sptr++;
	}
}

void
load_into_tab (void)
{
	total_fld = local_rec.fld_no = lcount[3] + 1;
	putval ( lcount[3]++ );
}

void
save_descs (void)
{
	int 	line_cnt;
	int	fld_mr;
	char	tmp[19];

	scn_set (3);

	for ( line_cnt = 0; line_cnt < total_fld; line_cnt++ )
	{
		getval ( line_cnt );
		strcpy ( tmp, lbln_rec.ln_field_name );
		fld_mr = lbln_rec.ln_fld_master;
		strcpy ( lbln_rec.ln_filename, lbhr_rec.hr_filename );

		cc = find_rec ( lbln, &lbln_rec, COMPARISON, "u" );
		if ( cc )
			file_err ( cc, lbln, "DBFIND" );

		strcpy ( lbln_rec.ln_field_name, tmp);
		lbln_rec.ln_fld_master = fld_mr;
		cc = abc_update ( lbln, &lbln_rec );
		if ( cc )
			file_err ( cc, lbln, "DBUPDATE" );
	}
}

		
