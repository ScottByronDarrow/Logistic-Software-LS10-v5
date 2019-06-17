/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( mend_disp.c    )                                 |
|  Program Desc  : ( Month End Status Display.                    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm,     ,     ,     ,     ,     ,     ,         |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (10/05/86)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (28/08/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (10/05/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (12/09/97)      | Modified  by  : Leah Manibog.    |
|  Date Modified : (22/05/1998)    | Modified  by  : Elizabeth D. Paid|
|  Date Modified : (30/08/1999)    | Modified  by  : Alvin Misalucha  |
|                                                                     |
|  Comments      : (28/08/90) - General Update for New Scrgen. S.B.D. |
|                : (10/05/91) -                                       |
|  (12/09/97)    : Updated for Multilingual Conversion.               |
|                : header display                                     |
|  (30/08/1999)  : Converted to ANSI format.                          |
|                :                                                    |
|                                                                     |
| $Log: mend_disp.c,v $
| Revision 5.1  2001/08/09 05:13:30  scott
| Updated to use FinishProgram ();
|
| Revision 5.0  2001/06/19 08:08:26  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:42  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:03  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:14  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  1999/12/06 01:47:14  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/10/20 02:06:48  nz
| Updated for final changes on date routines.
|
| Revision 1.11  1999/10/16 04:56:35  nz
| Updated for pjulmdy and pmdyjul routines.
|
| Revision 1.10  1999/09/29 10:11:07  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/17 07:26:59  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.8  1999/09/16 04:11:39  scott
| Updated from Ansi Project
|
| Revision 1.7  1999/06/15 02:36:00  scott
| Update to add log + change database name + general look.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: mend_disp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/mend_disp/mend_disp.c,v 5.1 2001/08/09 05:13:30 scott Exp $";

#define		NO_SCRGEN
#include 	<pslscr.h>
#include 	<get_lpno.h>
#include 	<ml_std_mess.h>
#include 	<ml_menu_mess.h>

#define	NOT_ACTIVE	0
#define	OPEN		1
#define	TO_CLOSE	2
#define	CLOSING		3
#define	CLOSED 		4
#define	ERR_FND 	5

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
		{"comm_dbt_date"},
		{"comm_crd_date"},
		{"comm_inv_date"},
		{"comm_gl_date"},
		};

	int comm_no_fields = 11;
	
	struct {
		int  	termno;
		char 	tco_no[3];
		char 	tco_name[41];
		char 	tco_short[16];
		char 	tes_no[3];
		char 	tes_name[41];
		char 	tes_short[16];
		long	tdbt_date;
		long	tcrd_date;
		long	tinv_date;
		long	tgl_date;
	} comm_rec;

	/*====================================
	| Month End Information File Record. |
	====================================*/
	struct dbview mect_list[] ={
		{"mect_co_no"},
		{"mect_br_no"},
		{"mect_module_type"},
		{"mect_status"},
		{"mect_start_time"},
		{"mect_end_time"},
		{"mect_closed_mth"},
		{"mect_txt_file"},
		{"mect_prog_stat"}
	};

	int mect_no_fields = 9;

	struct {
		char	ct_co_no[3];
		char	ct_br_no[3];
		char	ct_module_type[3];
		int	ct_status;
		char	ct_start_time[6];
		char	ct_end_time[6];
		int	ct_closed_mth;
		char	ct_txt_file[31];
		char	ct_prog_stat[2];
	} mect_rec;

	/*---------------------------------------
	| Set up Array to hold Months of Year . |
	---------------------------------------*/
	static char *mth[] = {
		"January","February","March","April",
		"May","June","July","August",
		"September","October","November","December"
	};

	int	flag = 0,
		total_counter = 0;

	long	inp_client;

	char 	mect_type[3];

	FILE	*pp;


/*=========================
| Function prototypes .   |
=========================*/
int		main			(int argc, char * argv []);
void	process			(void);
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
void	ReadMisc		(void);


/*-------------------------
| Main Processing Routine |
-------------------------*/
int
main (
 int	argc,
 char * argv [])
{

	if (argc != 2)
	{
		print_at(0,0, mlMenuMess701, argv[0] );
		shutdown_prog ();
		return (EXIT_FAILURE);
	}

	strcpy(mect_type, argv[1]);

	ReadMisc();

	/*---------------------
  	| open database files |
	---------------------*/
	OpenDB();
	set_tty();
	init_scr();

	process();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
process(void)
{
	long	date_used = 0L;
	char	disp_str[ 300 ];
	char	err_type[ 9 ];
	int		this_mth,
			last_mth,
			this_year,
			last_year;
	int		periodYear;

	sprintf(err_str,"%s ", ML(mlMenuMess230));

	if (!strcmp(mect_type, "DB"))
	{
		strcat(err_str, ML(mlMenuMess231));
		date_used = comm_rec.tdbt_date;
	}

	if (!strcmp(mect_type, "SK"))
	{
		strcat(err_str, ML(mlMenuMess232));
		date_used = comm_rec.tinv_date;
	}

	if (!strcmp(mect_type, "GL"))
	{
		strcat(err_str, ML(mlMenuMess233));
		date_used = comm_rec.tgl_date;
	}

	if (!strcmp(mect_type, "CR"))
	{
		strcat(err_str, ML(mlMenuMess234));
		date_used = comm_rec.tcrd_date;
	}

	clear();
	print_at( 0, (80 - strlen(err_str)) / 2, "%R %s", err_str );
	move(0,1);
	line(80);

	Dsp_prn_open( 0, 3, 15, err_str, comm_rec.tco_no, comm_rec.tco_name,
					 (char *) 0, (char *) 0, 
					 (char *) 0, (char *) 0 );

	Dsp_saverec(" CO | BR | MODULE | START |  END  |   LAST  MONTH    |    NEXT MONTH    ");
	Dsp_saverec(" NO | NO | STATUS | TIME. |  TIME |      CLOSED      |     TO CLOSE     ");

	Dsp_saverec(" [FN5-Print]   [FN14-Next]   [FN15-Prev]   [FN16-END] ");
	
	/*----------------------------------
	| Find by Company, Month End Type. |
	----------------------------------*/
	strcpy(mect_rec.ct_co_no, comm_rec.tco_no); 
	strcpy(mect_rec.ct_module_type, mect_type);
	cc = find_rec("mect", &mect_rec, GTEQ, "r");
	while ( !cc && !strcmp(mect_rec.ct_co_no, comm_rec.tco_no) &&
	 	       !strcmp(mect_rec.ct_module_type, mect_type))
	{
		switch ( mect_rec.ct_status )
		{
			case	NOT_ACTIVE:
				strcpy( err_type, "INACTIVE");
				break;

			case	OPEN:
				strcpy( err_type, "OPEN    ");
				break;

			case	TO_CLOSE:
				strcpy( err_type, "TO CLOSE");
				break;

			case	CLOSING:
				strcpy( err_type, "CLOSING ");
				break;

			case	CLOSED:
				strcpy( err_type, " CLOSED ");
				break;

			case	ERR_FND:
				strcpy( err_type, " ERROR. ");
				break;
		}

		DateToDMY ( date_used, NULL, NULL, &periodYear);

		last_mth  = mect_rec.ct_closed_mth;
		last_year = ( last_mth == 12 ) ? periodYear - 1 : periodYear;
		this_mth  = ( last_mth == 12 ) ? 1 : last_mth + 1;
		this_year = periodYear;


		sprintf( disp_str, " %s ^E %s ^E%s%8.8s^E %5.5s ^E %5.5s ^E %-9.9s - %02d %s^E %-9.9s - %02d ",
				mect_rec.ct_co_no, 
				mect_rec.ct_br_no,
				( mect_rec.ct_status < CLOSED ) ? "^1" : "^6",
				err_type,
				mect_rec.ct_start_time,
				mect_rec.ct_end_time,
				mth[ last_mth - 1 ], last_year,
				"^6",
				mth[ this_mth - 1 ], this_year );

		Dsp_saverec( disp_str );

		cc = find_rec("mect", &mect_rec, NEXT, "r");
	}
	Dsp_srch();
	Dsp_close();
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

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	open_rec("mect", mect_list, mect_no_fields, "mect_id_no2");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose("mect");
	abc_dbclose("data");
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (void)
{

	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
}
