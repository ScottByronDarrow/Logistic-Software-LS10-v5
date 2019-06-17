/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (so_lostsale.c )                                 |
|  Program Desc  : (Order Entry window to log lost sales.       )   |
|                 (                                             ) |
|                 (                                             ) |
|---------------------------------------------------------------------|
|  Date Written  : (25/08/1998)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
| (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
char	*PNAME = "$RCSfile: lostsale.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_lostsale/lostsale.c,v 5.2 2001/08/09 09:21:30 scott Exp $";
#define	CCMAIN

#include 	<pslscr.h>
#include	<getnum.h>
#include	<ml_so_mess.h>
#include	<Costing.h>

#ifdef GVISION
#	define	Y_POS	0
#else
#	define	Y_POS	18
#endif	/* GVISION */

#include	"schema"

struct commRecord	comm_rec;
struct inlsRecord	inls_rec;
struct exlsRecord	exls_rec;
struct ffdmRecord	ffdm_rec;

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		 (void);
void 	OpenDB 				 (void);
void 	CloseDB 			 (void);
void 	ProgramEntry 		 (void);
int  	FindExls 			 (int, char *);
void 	ProgramEdit 		 (void);
void 	AddInls 			 (void);
void 	ProgramHeading 		 (void);
void 	EntryDetails 		 (void);
void 	EditDetails 		 (void);
void 	heading 			 (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 9)
	{
		print_at (0,0, mlSoMess799,argv [0]);
		return (EXIT_FAILURE);
	}

	init_scr ();
	set_tty ();
	ProgramHeading ();
	OpenDB ();

	prog_exit = FALSE;

	strcpy (inls_rec.co_no,comm_rec.co_no);
	strcpy (inls_rec.est_no,comm_rec.est_no);
	inls_rec.date	= comm_rec.inv_date;
	inls_rec.hhbr_hash	= atol (argv [1]);
	inls_rec.hhcc_hash	= atol (argv [2]);
	inls_rec.hhcu_hash	= atol (argv [3]);
	sprintf (inls_rec.area_code,"%-2.2s",argv [4]);
	sprintf (inls_rec.sale_code,"%-2.2s",argv [5]);
	inls_rec.qty	= (float) atof (argv [6]);
	inls_rec.value	= atof (argv [7]);
	inls_rec.cost	= 	CENTS
						(
							FindIneiCosts
							(
								"L",
								comm_rec.est_no,
								inls_rec.hhbr_hash
							)
						);
sprintf (inls_rec.status,"%-1.1s",argv [8]);
	
	while (!prog_exit)
	{
		prog_exit = FALSE;
		search_ok = 1;
		restart = FALSE;
		ProgramEntry ();
		if (!restart && !prog_exit)
		{
			ProgramEdit ();
			sprintf (exls_rec.description, "%-60.60s", temp_str);
		}

		/*------------------------------------------------------------
		| Don't update as restart or program exited with blank code. |
		------------------------------------------------------------*/
		if (!restart && !prog_exit)
			AddInls ();

		if (!prog_exit)
		{
			print_at (Y_POS + 3,50,"____________________________________________________________");
		}
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=======================
| program exit sequence	|
=======================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*======================
| Open Database files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
 
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (inls, inls_list, INLS_NO_FIELDS, "inls_id_no");
	open_rec (exls, exls_list, EXLS_NO_FIELDS, "exls_id_no");
	open_rec (ffdm, ffdm_list, FFDM_NO_FIELDS, "ffdm_id_no2");
}

/*=======================
| Close Database files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (inls);
	abc_fclose (exls);
	abc_fclose (ffdm);
	CloseCosting ();
	abc_dbclose ("data");
}

/*=================
| Enter all data. |
=================*/
void
ProgramEntry (
 void)
{
	int	first_time = TRUE;
	clear_mess ();
	while (TRUE)
	{
		crsr_on ();
		getalpha (25,Y_POS + 3,"UU",temp_str);
		crsr_off ();

		switch (last_char)
		{
		case	REDRAW:
			ProgramHeading ();
			break;

		case	FN14:
		case	FN15:
		case	SEARCH:
			cc = FindExls (first_time ? GTEQ :
				 ((last_char == FN14) ? GREATER : LT),exls_rec.code);
			first_time = FALSE;
			last_char = SEARCH;
			if (!cc)
			{
				if (exls_rec.demand_ok [0] == 'Y')
					print_at (Y_POS + 1,2, "Note : Code selected will update demand");
				else
					print_at (Y_POS + 1,2, "                                       ");
				strcpy (temp_str, exls_rec.code);
			}
			break;

		case	RESTART:
			restart = 1;
			return;

		case	ENDINPUT:
		case	EOI:
			if (strlen (temp_str) == 0)
			{
				prog_exit = 1;
				return;
			}
			if (!FindExls (COMPARISON,temp_str))
			{
				if (exls_rec.demand_ok [0] == 'Y')
					print_at (Y_POS + 1,2, "Note : Code selected will update demand");
				else
					print_at (Y_POS + 1,2, "                                       ");
				return;
			}
			break;
		default:
			break;
		}
	}
}

int
FindExls (
 int ftype, 
 char *code)
{
	clear_mess ();
	strcpy (exls_rec.co_no,comm_rec.co_no);
	sprintf (exls_rec.code,"%-2.2s",code);
	cc = find_rec (exls,&exls_rec,ftype, "r");
	if (!cc && !strcmp (exls_rec.co_no,comm_rec.co_no))
	{
		print_at (Y_POS + 3,25,"%s",exls_rec.code);
		print_at (Y_POS + 3,50,"%s",exls_rec.description);
		sprintf (temp_str,"%-60.60s",exls_rec.description);
	}
	else
	{
		if (cc == 1)
			print_mess (ML (mlSoMess297));
		else
		{
			print_mess (ML (mlSoMess360));
			if (ftype == GREATER)
				cc = find_rec (exls,&exls_rec,LTEQ, "r");
			else
			{
				strcpy (exls_rec.co_no,comm_rec.co_no);
				strcpy (exls_rec.code,"  ");
				cc = find_rec (exls,&exls_rec,GTEQ, "r");
			}
		}
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

void
ProgramEdit (
 void)
{
	last_char = REDRAW;

	while (TRUE)
	{
		crsr_on ();
		getalpha (50,Y_POS + 3,"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",temp_str);
		crsr_off ();

		switch (last_char)
		{
		case	REDRAW:
			ProgramHeading ();
			print_at (Y_POS + 3,25, "%s", exls_rec.code);
			print_at (Y_POS + 3,50, "%s", exls_rec.description);
			break;

		case	RESTART:
			restart = 1;
			return;

		default:
			return;
			break;
		}
	}
}

void
AddInls (
 void)
{
	prog_exit = 1;
	sprintf (inls_rec.res_code,"%-2.2s",exls_rec.code);
	sprintf (inls_rec.res_desc,"%-60.60s",exls_rec.description);
	cc = abc_add (inls,&inls_rec);
	if (cc)
		file_err (cc, inls, "DBADD");

	if (exls_rec.demand_ok [0] == 'Y')
	{
		ffdm_rec.hhbr_hash	=	inls_rec.hhbr_hash;
		ffdm_rec.hhcc_hash	=	inls_rec.hhcc_hash;
		ffdm_rec.date		=	inls_rec.date;
		strcpy (ffdm_rec.type, "5");
		cc = find_rec (ffdm, &ffdm_rec, COMPARISON, "u");
		if (cc)
		{
			abc_unlock (ffdm);
			ffdm_rec.qty	=	inls_rec.qty;
			cc = abc_add (ffdm, &ffdm_rec);
			if (cc)
				file_err (cc, ffdm, "DBADD");
		}
		else
		{
			ffdm_rec.qty	+=	inls_rec.qty;
			cc = abc_update (ffdm, &ffdm_rec);
			if (cc)
				file_err (cc, ffdm, "DBUPDATE");
		}
	}
}

void 
ProgramHeading (
 void)
{
	register int	i;

#ifdef GVISION 
	swide ();
	clear ();
#else
	for (i = Y_POS; i < (Y_POS + 5); i++)
		print_at (i,0,"%120.120s"," ");
#endif	/* GVISION */
	
	box (0, Y_POS, 120, 3);

	move (1, Y_POS + 2);
	line (119);

	rv_pr (ML (mlSoMess361), 50, Y_POS + 1, 1);
	rv_pr (ML (" [REDRAW][NEXT][PREV][EDIT/END]"), 42, Y_POS + 4, 1);

	print_at (Y_POS + 3, 5, ML (mlSoMess362));

	fflush (stdout);
}
