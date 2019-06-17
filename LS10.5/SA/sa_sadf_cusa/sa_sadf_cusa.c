/*=====================================================================
|  Copyright (C) 1988 - 1995 Pinnacle Software Limited.               |
|=====================================================================|
|  Program Name  : ( sa_sadf_cusa.c )                                 |
|  Program Desc  : ( Updates cusa from sadf.                      )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cumr, cusa, sadf,     ,     ,     ,         |
|  Database      : (sale)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  cusa,     ,     ,     ,     ,     ,     ,         |
|  Database      : (sale)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 13/12/91         |
|---------------------------------------------------------------------|
|  Date Modified : (10/07/92)      | Modified  by  : Simon Dubey.     |
|  Date Modified : (14/08/93)      | Modified  by  : Anneliese Allen. |
|  Date Modified : (28/03/95)      | Modified  by  : Campbell Mander. |
|                                                                     |
| (10/07/92)     : To accept and process a range of debtors as passed |
|                : by sa_update.i SC DFH 6303.                        |
|                :                                                    |
| (14/08/93)     : Updated to account for bonus items.                |
|                :                                                    |
| (28/03/95)     : DFT 10559.  Changed such that if a cusa record     |
|                : exists and the sadf total is 0.00 then cusa record |
|                : gets updated. Zero records are NOT created.        |
|                :                                                    |
|                                                                     |
|---------------------------------------------------------------------|
| $Id: sa_sadf_cusa.c,v 5.3 2002/09/09 05:15:29 scott Exp $
| $Log: sa_sadf_cusa.c,v $
| Revision 5.3  2002/09/09 05:15:29  scott
| .
|
| Revision 5.2  2001/08/09 09:17:05  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/07 00:06:40  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:42  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:49  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:00  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:30  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.1  2000/03/04 05:07:21  ambhet
| SC# 16059 - 2600 : Added program name in v9.10
|
| Revision 1.1.1.1  1998/01/22 00:58:20  jonc
| Version 10 start
|
| Revision 2.1  1998/01/16 00:39:13  kirk
| PSL 14546 - Update version number
|
| Revision 1.1  1998/01/16 00:38:40  kirk
| PSL 14546 - Brought from version 7
|
|
=====================================================================*/
#define CCMAIN
char    *PNAME = "$RCSfile: sa_sadf_cusa.c,v $",
        *PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_sadf_cusa/sa_sadf_cusa.c,v 5.3 2002/09/09 05:15:29 scott Exp $";

#define		NO_SCRGEN
#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

	/*===================================
	| File comm { System Common file }. |
	===================================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"},
		{"comm_fiscal"},
		};

	int comm_no_fields = 8;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tco_short[16];
		char	tes_no[3];
		char	tes_name[41];
		long	tdbt_date;
		int	tfiscal;
	} comm_rec;

	/*=====================================
	| File cumr {Customer Master Record}. |
	=====================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
		{"cumr_stat_flag"}
		};

	int cumr_no_fields = 7;

	struct {
		char	mr_co_no[3];
		char	mr_est_no[3];
		char	mr_dbt_no[7];
		long	mr_hhcu_hash;
		char	mr_name[41];
		char	mr_dbt_acronym[10];
		char	mr_stat_flag[2];
	} cumr_rec;

	/*======================================
	| File cusa {Customer Sales Analysis}. |
	======================================*/
	struct dbview cusa_list[] ={
		{"cusa_hhcu_hash"},
		{"cusa_year"},
		{"cusa_val1"},
		{"cusa_val2"},
		{"cusa_val3"},
		{"cusa_val4"},
		{"cusa_val5"},
		{"cusa_val6"},
		{"cusa_val7"},
		{"cusa_val8"},
		{"cusa_val9"},
		{"cusa_val10"},
		{"cusa_val11"},
		{"cusa_val12"},
		{"cusa_stat_flag"}
		};

	int cusa_no_fields = 15;

	struct {
		long	cu_hhcu_hash;
		char	cu_year[2];
		double	cu_val[12];		/*  Money field  */
		char	cu_stat_flag[2];
	} cusa_rec;

	/*==============================================
	| Sales Analysis Detail file By Item/Customer. |
	==============================================*/
	struct dbview sadf_list[] ={
		{"sadf_co_no"},
		{"sadf_br_no"},
		{"sadf_year"},
		{"sadf_hhbr_hash"},
		{"sadf_hhcu_hash"},
		{"sadf_sal_per1"},
		{"sadf_sal_per2"},
		{"sadf_sal_per3"},
		{"sadf_sal_per4"},
		{"sadf_sal_per5"},
		{"sadf_sal_per6"},
		{"sadf_sal_per7"},
		{"sadf_sal_per8"},
		{"sadf_sal_per9"},
		{"sadf_sal_per10"},
		{"sadf_sal_per11"},
		{"sadf_sal_per12"},
	};

	int sadf_no_fields = 17;

	struct {
		char	df_co_no[3];
		char	df_br_no[3];
		char	df_year[2];
		long	df_hhbr_hash;
		long	df_hhcu_hash;
		double	df_sales[12];
	} sadf_rec;

	struct {
		char	st_dbt_no[7];
		char	end_dbt_no[7];
	} local_rec;

	double	sa_hist[ 24 ];

	void OpenDB(void);
	void CloseDB(void);
	void procfile(void);
	int calc_sadf(long);

	char	*data = "data";

/*========================== 
| Main Processing Routine. |
==========================*/
int
main(
	int	argc,
	char* argv[])
{

	if (argc < 3)
	{
		printf(" USAGE: <Start Customer> <End Customer> \007\n");
		exit(0);
	}

	strcpy( local_rec.st_dbt_no , argv[1] );
	strcpy( local_rec.end_dbt_no, argv[2] );

	dsp_screen("Update of sadf to cusa.", 
			comm_rec.tco_no, comm_rec.tco_name);

	/*-----------------------------
	| Open main database files  . |
	-----------------------------*/
	OpenDB();

	cumr_rec.mr_hhcu_hash = 0L;
	cc = find_hash("cumr", &cumr_rec, GTEQ, "r", 0L);
	while (!cc)
	{
		if (strcmp (cumr_rec.mr_dbt_no , local_rec.st_dbt_no) < 0 ||
                    strcmp (cumr_rec.mr_dbt_no , local_rec.end_dbt_no) > 0)
		{
			cc = find_hash("cumr", &cumr_rec, NEXT, "r", 0L);
			continue;
		}

		dsp_process("Customer : ", cumr_rec.mr_dbt_acronym);

		calc_sadf( cumr_rec.mr_hhcu_hash );

		procfile();

		cc = find_hash("cumr", &cumr_rec, NEXT, "r", 0L);
	}
	CloseDB ();
	FinishProgram ();
	return(EXIT_SUCCESS);

}

void
OpenDB(void)
{
	abc_dbopen (data);
    read_comm( comm_list, comm_no_fields, (char *) &comm_rec );        
	open_rec("cumr", cumr_list, cumr_no_fields, "cumr_hhcu_hash");
	open_rec("cusa", cusa_list, cusa_no_fields, "cusa_id_no");
	open_rec("sadf", sadf_list, sadf_no_fields, "sadf_id_no3");
}

void
CloseDB(void)
{
	abc_fclose("cumr");
	abc_fclose("cusa");
	abc_fclose("sadf");
	abc_dbclose (data);
}

void
procfile(void)
{
	int	i;
	int	cusa_err;
	double	total = 0.00;

	/*=================================================================
	| Read Customer sales record with a type of 'C' for current Year. |
	=================================================================*/
	cusa_rec.cu_hhcu_hash = cumr_rec.mr_hhcu_hash;
	strcpy(cusa_rec.cu_year, "C");
	cusa_err = find_rec("cusa", &cusa_rec, COMPARISON,"u");

	total = 0.00;
	for ( i = 0; i < 12; i++ )
	{
		cusa_rec.cu_val[ i ] = CENTS( sa_hist[ i ] );
		total += sa_hist[ i ];
	}

	if (total == 0.00)
	{
		if (!cusa_err)
			cc = abc_update( "cusa", &cusa_rec );
		abc_unlock( "cusa" );
	}
	else
	{
		if ( cusa_err )
			cc = abc_add( "cusa", &cusa_rec );
		else
			cc = abc_update( "cusa", &cusa_rec );

		if ( cc )
			sys_err("Error in cusa During (DBADD/UPDATE)",cc,PNAME);

		if ( cusa_err )
			abc_unlock( "cusa" );
	}
	
	/*=================================================================
	| Read Customer sales record with a type of 'L' for current Year. |
	=================================================================*/
	cusa_rec.cu_hhcu_hash = cumr_rec.mr_hhcu_hash;
	strcpy(cusa_rec.cu_year, "L");
	cusa_err = find_rec("cusa", &cusa_rec, COMPARISON,"u");

	total = 0.00;
	for ( i = 0; i < 12; i++ )
	{
		cusa_rec.cu_val[ i ] = CENTS( sa_hist[ i + 12 ] );
		total += sa_hist[ i + 12 ];

	}
	
	if (total == 0.00)
	{
		if (!cusa_err)
			cc = abc_update( "cusa", &cusa_rec );
		abc_unlock( "cusa" );
	}
	else
	{
		if ( cusa_err )
			cc = abc_add( "cusa", &cusa_rec );
		else
			cc = abc_update( "cusa", &cusa_rec );

		if ( cc )
			sys_err("Error in cusa During (DBADD/UPDATE)",cc,PNAME);

		if ( cusa_err )
			abc_unlock( "cusa" );
	}

	return;
}

int
calc_sadf( long hhcu_hash )
{
	int	i;
	int	OFFSET;

	for ( i = 0; i < 24; i++ )
		sa_hist[ i ] = 0.00;
	
	sadf_rec.df_hhcu_hash = hhcu_hash;
	sadf_rec.df_hhbr_hash = 0L;
	cc = find_rec( "sadf", &sadf_rec, GTEQ, "r" );
	while ( !cc && sadf_rec.df_hhcu_hash == hhcu_hash )
	{
		if ( sadf_rec.df_year[0] == 'C' ||
		     sadf_rec.df_year[0] == 'c' )
			OFFSET = 0;
		else
			OFFSET = 12;

		for ( i = 0; i < 12; i++ )
			sa_hist[ i + OFFSET ] += sadf_rec.df_sales[ i ];

		cc = find_rec( "sadf", &sadf_rec, NEXT, "r" );
	}
	return(FALSE);
}
