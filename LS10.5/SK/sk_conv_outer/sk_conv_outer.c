/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_conv_outer.c,v 5.1 2002/05/09 07:38:45 scott Exp $
|  Program Name  : (sk_conv_outer.c)           
|  Program Desc  : (Convert Product outer sizes)
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow  | Date Written  : 8th May 2001     |
|---------------------------------------------------------------------|
| $Log: sk_conv_outer.c,v $
| Revision 5.1  2002/05/09 07:38:45  scott
| .
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_conv_outer.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_conv_outer/sk_conv_outer.c,v 5.1 2002/05/09 07:38:45 scott Exp $";

#include	<pslscr.h>
#include 	<twodec.h>
/*
#include	<log.h>
*/

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct ineiRecord	inei_rec;
struct inafRecord	inaf_rec;
struct incfRecord	incf_rec;
struct intrRecord	intr_rec;
struct pocaRecord	poca_rec;
struct poglRecord	pogl_rec;
struct polnRecord	poln_rec;
struct suphRecord	suph_rec;
struct inccRecord	incc_rec;
struct solnRecord	soln_rec;
struct colnRecord	coln_rec;
struct inprRecord	inpr_rec;
struct arlnRecord	arln_rec;
struct cmrdRecord	cmrd_rec;
struct cmtrRecord	cmtr_rec;
struct ddlnRecord	ddln_rec;
struct qtlnRecord	qtln_rec;
struct qtpdRecord	qtpd_rec;
struct incpRecord	incp_rec;

	char 	*data	= "data";

	float	convertFactor	=	0.00;

void 	CloseDB				(void);
void 	OpenDB 				(void);
void 	shutdown_prog 		(void);
void	Process				(long, float);
void	ProcessInaf 		(long);
void	ProcessInei 		(long);
void	ProcessIntr 		(long);
void	ProcessSoln 		(long);
void	ProcessColn 		(long);
void	ProcessInpr 		(long);
void	ProcessPogl 		(long);
void	ProcessPoln 		(long);
void	ProcessSuph 		(long);
void	ProcessIncf 		(long);
void	ProcessCmrd 		(long);
void	ProcessCmtr 		(long);
void	ProcessDdln 		(long);
void	ProcessQtln 		(long);
void	ProcessQtpd 		(long);
void	ProcessIncp 		(long);
Money 	CONVERT 			(Money);

/*
 * Main Processing Routine. 
 */
int    
main (
 int argc, 
 char * argv [])
{

	if (argc < 3)
	{
		print_at (0,0,"Usage : %s <hhbrHash> <convertTo>", argv [0]);
		return (EXIT_FAILURE);
	}
	OpenDB ();

	init_scr ();

	Process (atol (argv [1]), atof (argv [2]));

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
{
	CloseDB ();
	clear ();
	crsr_on ();
}

/*
 * Open data base files 
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	open_rec (inaf, inaf_list, INAF_NO_FIELDS, "inaf_id_no");
	open_rec (incf, incf_list, INCF_NO_FIELDS, "incf_hhwh_hash");
	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_hhbr_hash");
	open_rec (intr, intr_list, INTR_NO_FIELDS, "intr_hhbr_hash");
	open_rec (poca, poca_list, POCA_NO_FIELDS, "poca_id_no");
	open_rec (pogl, pogl_list, POGL_NO_FIELDS, "pogl_hhbr_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhbr_hash");
	open_rec (suph, suph_list, SUPH_NO_FIELDS, "suph_hhbr_hash");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhbr_hash");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_hhbr_hash");
	open_rec (inpr, inpr_list, INPR_NO_FIELDS, "inpr_hhbr_hash");
	open_rec (arln, arln_list, ARLN_NO_FIELDS, "arln_hhbr_hash");
	open_rec (cmrd, cmrd_list, CMRD_NO_FIELDS, "cmrd_hhbr_hash");
	open_rec (cmtr, cmtr_list, CMTR_NO_FIELDS, "cmtr_hhhr_hash");
	open_rec (ddln, ddln_list, DDLN_NO_FIELDS, "ddln_hhbr_hash");
	open_rec (qtln, qtln_list, QTLN_NO_FIELDS, "qtln_hhbr_hash");
	open_rec (qtpd, qtpd_list, QTPD_NO_FIELDS, "qtpd_hhbr_hash");
	open_rec (incp, incp_list, INCP_NO_FIELDS, "incp_hhbr_hash");
}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (inaf);
	abc_fclose (incf);
	abc_fclose (inei);
	abc_fclose (intr);
	abc_fclose (poca);
	abc_fclose (pogl);
	abc_fclose (poln);
	abc_fclose (soln);
	abc_fclose (coln);
	abc_fclose (inpr);
	abc_fclose (arln);
	abc_fclose (cmrd);
	abc_fclose (cmtr);
	abc_fclose (ddln);
	abc_fclose (qtln);
	abc_fclose (qtpd);
	abc_fclose (incp);
	abc_dbclose (data);
}

/*
 * Main Processing Routine. 
 */
void
Process (
	long	hhbrHash,
	float	convertTo)
{

	inmr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
	{
		fprintf (stdout, "Find inmr - inmr_hhbr_hash = [%ld], cc = [%d]",
							inmr_rec.hhbr_hash, cc);
		return;
	}
	if (inmr_rec.outer_size == 0.00)
	{
		fprintf (stdout, "item [%s] outer size should be greater than one\n", inmr_rec.item_no);
		return;
	}
	fprintf (stdout, "Found item [%s]\n", inmr_rec.item_no);

	convertFactor = convertTo / inmr_rec.outer_size;

	ProcessInaf (inmr_rec.hhbr_hash);
	ProcessInei (inmr_rec.hhbr_hash);
	ProcessIntr (inmr_rec.hhbr_hash);
	ProcessPogl (inmr_rec.hhbr_hash);
	ProcessPoln (inmr_rec.hhbr_hash);
	ProcessSuph (inmr_rec.hhbr_hash);
	ProcessSoln (inmr_rec.hhbr_hash);
	ProcessColn (inmr_rec.hhbr_hash);
	ProcessInpr (inmr_rec.hhbr_hash);
	ProcessCmrd (inmr_rec.hhbr_hash);
	ProcessCmtr (inmr_rec.hhbr_hash);
	ProcessDdln (inmr_rec.hhbr_hash);
	ProcessQtln (inmr_rec.hhbr_hash);
	ProcessQtpd (inmr_rec.hhbr_hash);
	ProcessIncp (inmr_rec.hhbr_hash);

	incc_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	cc = find_rec (incc, &incc_rec, GTEQ, "r");
	while (!cc && incc_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		ProcessIncf (incc_rec.hhwh_hash);
		cc = find_rec (incc, &incc_rec, NEXT, "r");
	}
	inmr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "u");
	if (!cc)
	{
		inmr_rec.outer_size = convertTo;
		cc = abc_update (inmr, &inmr_rec);
		if (cc)
			file_err (cc, inmr, "DBUPDATE");
	}
}

void
ProcessInaf (
	long	hhbrHash)
{
	strcpy (inaf_rec.co_no, comm_rec.co_no);
	strcpy (inaf_rec.br_no, "  ");
	strcpy (inaf_rec.wh_no, "  ");
	inaf_rec.sys_date	=	0L;
	strcpy (inaf_rec.ref1, "               ");
	cc = find_rec (inaf, &inaf_rec, GTEQ, "u");
	while (!cc)
	{
		if (inaf_rec.hhbr_hash != hhbrHash)
		{
			abc_unlock (inaf);
			cc = find_rec (inaf, &inaf_rec, NEXT, "u");
			continue;
		}
		inaf_rec.cost_price = CONVERT (inaf_rec.cost_price);
		inaf_rec.sale_price = CONVERT (inaf_rec.sale_price);
		cc = abc_update (inaf, &inaf_rec);
		if (cc)
			file_err (cc, inaf, "DBUPDATE");

		cc = find_rec (inaf, &inaf_rec, NEXT, "u");
	}
	abc_unlock (inaf);
}

void
ProcessInei (
	long	hhbrHash)
{
	inei_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inei, &inei_rec, GTEQ, "u");
	while (!cc)
	{
		if (inei_rec.hhbr_hash != hhbrHash)
		{
			abc_unlock (inei);
			cc = find_rec (inei, &inei_rec, NEXT, "u");
			continue;
		}

		inei_rec.avge_cost 	= CONVERT (inei_rec.avge_cost);
		inei_rec.last_cost 	= CONVERT (inei_rec.last_cost);
		inei_rec.prev_cost 	= CONVERT (inei_rec.prev_cost);
		inei_rec.std_cost 	= CONVERT (inei_rec.std_cost);

		cc = abc_update (inei, &inei_rec);
		if (cc)
			file_err (cc, inei, "DBUPDATE");

		cc = find_rec (inei, &inei_rec, NEXT, "u");
	}
	abc_unlock (inei);
}
void
ProcessIntr (
	long	hhbrHash)
{
	intr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (intr, &intr_rec, GTEQ, "u");
	while (!cc && intr_rec.hhbr_hash == hhbrHash)
	{

		intr_rec.cost_price 	= CONVERT (intr_rec.cost_price);
		intr_rec.sale_price 	= CONVERT (intr_rec.sale_price);

		cc = abc_update (intr, &intr_rec);
		if (cc)
			file_err (cc, intr, "DBUPDATE");

		cc = find_rec (intr, &intr_rec, NEXT, "u");
	}
	abc_unlock (intr);
}
void
ProcessSoln (
	long	hhbrHash)
{
	soln_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (soln, &soln_rec, GTEQ, "u");
	while (!cc && soln_rec.hhbr_hash == hhbrHash)
	{

		soln_rec.gsale_price 	= CONVERT (soln_rec.gsale_price);
		soln_rec.sale_price 	= CONVERT (soln_rec.sale_price);
		soln_rec.cost_price 	= CONVERT (soln_rec.cost_price);

		cc = abc_update (soln, &soln_rec);
		if (cc)
			file_err (cc, soln, "DBUPDATE");

		cc = find_rec (soln, &soln_rec, NEXT, "u");
	}
	abc_unlock (soln);
}
void
ProcessColn (
	long	hhbrHash)
{
	coln_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (coln, &coln_rec, GTEQ, "u");
	while (!cc && coln_rec.hhbr_hash == hhbrHash)
	{
		coln_rec.gsale_price 	= CONVERT (coln_rec.gsale_price);
		coln_rec.sale_price 	= CONVERT (coln_rec.sale_price);
		coln_rec.cost_price 	= CONVERT (coln_rec.cost_price);

		cc = abc_update (coln, &coln_rec);
		if (cc)
			file_err (cc, coln, "DBUPDATE");

		cc = find_rec (coln, &coln_rec, NEXT, "u");
	}
	abc_unlock (coln);
}
void
ProcessArln (
	long	hhbrHash)
{
	arln_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (arln, &arln_rec, GTEQ, "u");
	while (!cc && arln_rec.hhbr_hash == hhbrHash)
	{
		arln_rec.gsale_price 	= CONVERT (arln_rec.gsale_price);
		arln_rec.sale_price 	= CONVERT (arln_rec.sale_price);
		arln_rec.cost_price 	= CONVERT (arln_rec.cost_price);

		cc = abc_update (arln, &arln_rec);
		if (cc)
			file_err (cc, arln, "DBUPDATE");

		cc = find_rec (arln, &arln_rec, NEXT, "u");
	}
	abc_unlock (arln);
}
void
ProcessCmrd (
	long	hhbrHash)
{
	cmrd_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (cmrd, &cmrd_rec, GTEQ, "u");
	while (!cc && cmrd_rec.hhbr_hash == hhbrHash)
	{
		cmrd_rec.gsale_price 	= CONVERT (cmrd_rec.gsale_price);
		cmrd_rec.sale_price 	= CONVERT (cmrd_rec.sale_price);
		cmrd_rec.cost			= CONVERT (cmrd_rec.cost);

		cc = abc_update (cmrd, &cmrd_rec);
		if (cc)
			file_err (cc, cmrd, "DBUPDATE");

		cc = find_rec (cmrd, &cmrd_rec, NEXT, "u");
	}
	abc_unlock (cmrd);
}
void
ProcessCmtr (
	long	hhbrHash)
{
	cmtr_rec.hhhr_hash	=	0L;
	cc = find_rec (cmtr, &cmtr_rec, GTEQ, "u");
	while (!cc)
	{
		if (cmtr_rec.hhbr_hash	==	hhbrHash)
		{
			cmtr_rec.sale_price 	= CONVERT (cmtr_rec.sale_price);
			cmtr_rec.cost_price 	= CONVERT (cmtr_rec.cost_price);
	
			cc = abc_update (cmtr, &cmtr_rec);
			if (cc)
				file_err (cc, cmtr, "DBUPDATE");
		}
		else
			abc_unlock (cmtr);
	
		cc = find_rec (cmtr, &cmtr_rec, NEXT, "u");
	}
	abc_unlock (cmtr);
}
void
ProcessDdln (
	long	hhbrHash)
{
	ddln_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (ddln, &ddln_rec, GTEQ, "u");
	while (!cc && ddln_rec.hhbr_hash == hhbrHash)
	{
		ddln_rec.gsale_price 	= CONVERT (ddln_rec.gsale_price);
		ddln_rec.sale_price 	= CONVERT (ddln_rec.sale_price);
		ddln_rec.cost_price 	= CONVERT (ddln_rec.cost_price);

		cc = abc_update (ddln, &ddln_rec);
		if (cc)
			file_err (cc, ddln, "DBUPDATE");

		cc = find_rec (ddln, &ddln_rec, NEXT, "u");
	}
	abc_unlock (ddln);
}
void
ProcessQtln (
	long	hhbrHash)
{
	qtln_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (qtln, &qtln_rec, GTEQ, "u");
	while (!cc && qtln_rec.hhbr_hash == hhbrHash)
	{
		qtln_rec.gsale_price 	= CONVERT (qtln_rec.gsale_price);
		qtln_rec.sale_price 	= CONVERT (qtln_rec.sale_price);
		qtln_rec.cost_price 	= CONVERT (qtln_rec.cost_price);

		cc = abc_update (qtln, &qtln_rec);
		if (cc)
			file_err (cc, qtln, "DBUPDATE");

		cc = find_rec (qtln, &qtln_rec, NEXT, "u");
	}
	abc_unlock (qtln);
}
void
ProcessQtpd (
	long	hhbrHash)
{
	qtpd_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (qtpd, &qtpd_rec, GTEQ, "u");
	while (!cc && qtpd_rec.hhbr_hash == hhbrHash)
	{
		qtpd_rec.gsale_price 	= CONVERT (qtpd_rec.gsale_price);
		qtpd_rec.sale_price 	= CONVERT (qtpd_rec.sale_price);

		cc = abc_update (qtpd, &qtpd_rec);
		if (cc)
			file_err (cc, qtpd, "DBUPDATE");

		cc = find_rec (qtpd, &qtpd_rec, NEXT, "u");
	}
	abc_unlock (qtpd);
}
void
ProcessIncp (
	long	hhbrHash)
{
	incp_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (incp, &incp_rec, GTEQ, "u");
	while (!cc && incp_rec.hhbr_hash == hhbrHash)
	{
		incp_rec.price1 	= CONVERT (incp_rec.price1);
		incp_rec.price2 	= CONVERT (incp_rec.price2);
		incp_rec.price3 	= CONVERT (incp_rec.price3);
		incp_rec.price4 	= CONVERT (incp_rec.price4);
		incp_rec.price5 	= CONVERT (incp_rec.price5);
		incp_rec.price6 	= CONVERT (incp_rec.price6);
		incp_rec.price7 	= CONVERT (incp_rec.price7);
		incp_rec.price8 	= CONVERT (incp_rec.price8);
		incp_rec.price9 	= CONVERT (incp_rec.price9);

		cc = abc_update (incp, &incp_rec);
		if (cc)
			file_err (cc, incp, "DBUPDATE");

		cc = find_rec (incp, &incp_rec, NEXT, "u");
	}
	abc_unlock (incp);
}
void
ProcessInpr (
	long	hhbrHash)
{
	inpr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inpr, &inpr_rec, GTEQ, "u");
	while (!cc && inpr_rec.hhbr_hash == hhbrHash)
	{
		inpr_rec.base 		= CONVERT (inpr_rec.base);
		inpr_rec.price1 	= CONVERT (inpr_rec.price1);
		inpr_rec.price2 	= CONVERT (inpr_rec.price2);
		inpr_rec.price3 	= CONVERT (inpr_rec.price3);
		inpr_rec.price4 	= CONVERT (inpr_rec.price4);
		inpr_rec.price5 	= CONVERT (inpr_rec.price5);
		inpr_rec.price6 	= CONVERT (inpr_rec.price6);
		inpr_rec.price7 	= CONVERT (inpr_rec.price7);
		inpr_rec.price8 	= CONVERT (inpr_rec.price8);
		inpr_rec.price9 	= CONVERT (inpr_rec.price9);

		cc = abc_update (inpr, &inpr_rec);
		if (cc)
			file_err (cc, inpr, "DBUPDATE");

		cc = find_rec (inpr, &inpr_rec, NEXT, "u");
	}
	abc_unlock (inpr);
}
void
ProcessPogl (
	long	hhbrHash)
{
	pogl_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (pogl, &pogl_rec, GTEQ, "u");
	while (!cc && pogl_rec.hhbr_hash == hhbrHash)
	{

		pogl_rec.land_cst       =	CONVERT (pogl_rec.land_cst);
		pogl_rec.act_cst        =	CONVERT (pogl_rec.act_cst);
		pogl_rec.fob_fgn_cst    =	CONVERT (pogl_rec.fob_fgn_cst);
		pogl_rec.fob_nor_cst    =	CONVERT (pogl_rec.fob_nor_cst);
		pogl_rec.frt_ins_cst    =	CONVERT (pogl_rec.frt_ins_cst);
		pogl_rec.lcost_load     =	CONVERT (pogl_rec.lcost_load);
		pogl_rec.duty           =	CONVERT (pogl_rec.duty);
		pogl_rec.licence        =	CONVERT (pogl_rec.licence);

		cc = abc_update (pogl, &pogl_rec);
		if (cc)
			file_err (cc, pogl, "DBUPDATE");

		cc = find_rec (pogl, &pogl_rec, NEXT, "u");
	}
	abc_unlock (pogl);
}
void
ProcessPoln (
	long	hhbrHash)
{
	poln_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (poln, &poln_rec, GTEQ, "u");
	while (!cc && poln_rec.hhbr_hash == hhbrHash)
	{
		poln_rec.grs_fgn_cst    =	CONVERT (poln_rec.grs_fgn_cst);
		poln_rec.fob_fgn_cst    =	CONVERT (poln_rec.fob_fgn_cst);
		poln_rec.fob_nor_cst    =	CONVERT (poln_rec.fob_nor_cst);
		poln_rec.frt_ins_cst    =	CONVERT (poln_rec.frt_ins_cst);
		poln_rec.duty           =	CONVERT (poln_rec.duty);
		poln_rec.licence        =	CONVERT (poln_rec.licence);
		poln_rec.lcost_load     =	CONVERT (poln_rec.lcost_load);
		poln_rec.land_cst       =	CONVERT (poln_rec.land_cst);

		cc = abc_update (poln, &poln_rec);
		if (cc)
			file_err (cc, poln, "DBUPDATE");

		cc = find_rec (poln, &poln_rec, NEXT, "u");
	}
	abc_unlock (poln);
}
void
ProcessSuph (
	long	hhbrHash)
{
	suph_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (suph, &suph_rec, GTEQ, "u");
	while (!cc && suph_rec.hhbr_hash == hhbrHash)
	{
		suph_rec.net_cost    =	CONVERT (suph_rec.net_cost);
		suph_rec.land_cost    =	CONVERT (suph_rec.land_cost);

		cc = abc_update (suph, &suph_rec);
		if (cc)
			file_err (cc, suph, "DBUPDATE");

		cc = find_rec (suph, &suph_rec, NEXT, "u");
	}
	abc_unlock (suph);
}
void
ProcessIncf (
	long	hhwhHash)
{
	incf_rec.hhwh_hash	=	hhwhHash;
	cc = find_rec (incf, &incf_rec, GTEQ, "u");
	while (!cc && incf_rec.hhwh_hash == hhwhHash)
	{
		incf_rec.fifo_cost      =	CONVERT (incf_rec.fifo_cost);
		incf_rec.act_cost       =	CONVERT (incf_rec.act_cost);
		incf_rec.fob_nor_cst    =	CONVERT (incf_rec.fob_nor_cst);
		incf_rec.frt_ins_cst    =	CONVERT (incf_rec.frt_ins_cst);
		incf_rec.duty           =	CONVERT (incf_rec.duty);
		incf_rec.licence        =	CONVERT (incf_rec.licence);
		incf_rec.lcost_load     =	CONVERT (incf_rec.lcost_load);
		incf_rec.land_cst       =	CONVERT (incf_rec.land_cst);

		cc = abc_update (incf, &incf_rec);
		if (cc)
			file_err (cc, incf, "DBUPDATE");

		cc = find_rec (incf, &incf_rec, NEXT, "u");
	}
	abc_unlock (incf);
}

Money
CONVERT (
Money	convertValue)
{
	convertValue	*=	(double) convertFactor;
	return (twodec (convertValue));
}
