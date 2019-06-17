/*=====================================================================
|  Copyright (C) 1996 - 1997 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( fx_inv_no.c )                                    |
|  Program Desc  : (                                              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : (30/06/94)       |
|---------------------------------------------------------------------|
|  Date Modified : (28/09/1999)    | Modified by : edge cabalfin      |
|                :                                                    |
|  Comments      :                                                    |
|  (28/09/1999)  : ANSIfication of the code                           |
|                :      - potential problems marked with QUERY        |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN

char	*PNAME = "$RCSfile: fx_inv_no.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MISC/fx_inv_no/fx_inv_no.c,v 5.2 2001/08/09 09:49:46 scott Exp $";

/*==============================
|   Include file dependencies   |
 ==============================*/
#define		MOD	1

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

/*==================================
|   Constants, defines and stuff    |
 ==================================*/

#define	    _isdigit(x)	 (x >= '0' && x <= '9')

/*  QUERY
    these should be declared as const char*
    to minimize potential problems.
*/
	char	*comm	= "comm",
			*arhr	= "arhr",
			*cnre	= "cnre",
			*cohr	= "cohr",
			*cucc	= "cucc",
			*cudr	= "cudr",
			*cuin	= "cuin",
			*curh	= "curh",
			*cuwk	= "cuwk",
			*insf	= "insf",
			*mhdr	= "mhdr",
			*sohr	= "sohr",
			*somi	= "somi",
			*somc	= "somc",
			*data  = "data";


	/*==================================
	| file comm {System Common file}    |
	 ===================================*/
	struct dbview comm_list[] =
    {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"}
	};

	const int comm_no_fields = 6;

	struct 
    {
		int	term;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		long	tdbt_date;
	} comm_rec;

	/*================================================
	| Customer P-slip/ Invoice/Credit Archive Header |
	================================================*/
	struct dbview arhr_list [] =
	{
		{"arhr_inv_no"},
		{"arhr_app_inv_no"},
		{"arhr_hhco_hash"},
	};

	const int arhr_no_fields = 3;

	struct tag_arhrRecord
	{
		char	inv_no [9];
		char	app_inv_no [9];
		long	hhco_hash;
	} arhr_rec;

	/*====================================
	| Customer Contract Rebates Due File |
	====================================*/
	struct dbview cnre_list [] =
	{
		{"cnre_hhch_hash"},
		{"cnre_inv_no"},
	};

	const int cnre_no_fields = 2;

	struct tag_cnreRecord
	{
		long	hhch_hash;
		char	inv_no [9];
	} cnre_rec;

	/*=============================================
	| Customer P-slip/ Invoice/Credit Header File |
	=============================================*/
	struct dbview cohr_list [] =
	{
		{"cohr_inv_no"},
		{"cohr_app_inv_no"},
		{"cohr_hhco_hash"},
	};

	const int cohr_no_fields = 3;

	struct tag_cohrRecord
	{
		char	inv_no [9];
		char	app_inv_no [9];
		long	hhco_hash;
	} cohr_rec;

	/*================================
	| Customer Credit Control Record |
	================================*/
	struct dbview cucc_list [] =
	{
		{"cucc_hhcu_hash"},
		{"cucc_hold_ref"}
	};

	const int cucc_no_fields = 2;

	struct tag_cuccRecord
	{
		long	hhcu_hash;
		char	hold_ref [9];
	} cucc_rec;

	/*===================================
	| Customer Receipts Work File Record |
	===================================*/
	struct dbview cudr_list [] =
	{
		{"cudr_co_no"},
		{"cudr_invoice"},
	};

	const int cudr_no_fields = 2;

	struct tag_cudrRecord
	{
		char	co_no [3];
		char	invoice [9];
	} cudr_rec;

	/*================================================
	| Customer Invoice Accounting Invoice/Credit file |
	================================================*/
	struct dbview cuin_list [] =
	{
		{"cuin_hhcu_hash"},
		{"cuin_inv_no"},
	};

	const int cuin_no_fields = 2;

	struct tag_cuinRecord
	{
		long	hhcu_hash;
		char	inv_no [9];
	} cuin_rec;

	/*===============================
	| Customers Rebate History File |
	===============================*/
	struct dbview curh_list [] =
	{
		{"curh_hhbr_hash"},
		{"curh_inv_no"},
	};

	const int curh_no_fields = 2;

	struct tag_curhRecord
	{
		long	hhbr_hash;
		char	inv_no [9];
	} curh_rec;

	/*===================================
	| Customer Invoice/Credits Work File |
	===================================*/
	struct dbview cuwk_list [] =
	{
		{"cuwk_co_no"},
		{"cuwk_inv_no"},
	};

	const int cuwk_no_fields = 2;

	struct tag_cuwkRecord
	{
		char	co_no [3];
		char	inv_no [9];
	} cuwk_rec;
	/*==============================
	| Inventory Serial Number File |
	==============================*/
	struct dbview insf_list [] =
	{
		{"insf_hhsf_hash"},
		{"insf_invoice_no"},
	};

	const int insf_no_fields = 2;

	struct tag_insfRecord
	{
		long	hhsf_hash;
		char	invoice_no [9];
	} insf_rec;

	/*====================================
	| Machine History Detail Record File |
	====================================*/
	struct dbview mhdr_list [] =
	{
		{"mhdr_co_no"},
		{"mhdr_inv_no"},
	};

	const int mhdr_no_fields = 2;

	struct tag_mhdrRecord
	{
		char	co_no [3];
		char	inv_no [9];
	} mhdr_rec;

	/*=========================
	| Sales Order Header File |
	=========================*/
	struct dbview sohr_list [] =
	{
		{"sohr_hhso_hash"},
		{"sohr_inv_no"},
	};

	const int sohr_no_fields = 2;

	struct tag_sohrRecord
	{
		long	hhso_hash;
		char	inv_no [9];
	} sohr_rec;

	/*================================================
	| Sales Order Processing Missing Invoice Control |
	================================================*/
	struct dbview somi_list [] =
	{
		{"somi_co_no"},
		{"somi_br_no"},
		{"somi_inv_no"}
	};

	const int somi_no_fields = 3;

	struct tag_somiRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	inv_no [9];
	} somi_rec;


	/*================================================
	| Sales Order Processing Missing Invoice Control |
	================================================*/
	struct dbview somc_list [] =
	{
		{"somc_co_no"},
		{"somc_br_no"},
		{"somc_active"},
		{"somc_start_seq"},
		{"somc_end_seq"},
		{"somc_stat_flag"}
	};

	const int somc_no_fields = 6;

	struct tag_somcRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	active [2];
		char	start_seq [9];
		char	end_seq [9];
		char	stat_flag [2];
	} somc_rec;


/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
} local_rec;

/*==============================
|   Local function prototypes   |
 ==============================*/
void proc_arhr (void);
void proc_cnre (void);
void proc_cohr (void);
void proc_cucc (void);
void proc_cudr (void);
void proc_cuin (void);
void proc_curh (void);
void proc_cuwk (void);
void proc_insf (void);
void proc_mhdr (void);
void proc_sohr (void);
void proc_somi (void);
void proc_somc (void);
char *conv_invoice (char *inv_no);

/*==============================
|   Main Processing Function    |
 ==============================*/
int
main (
 int   argc, 
 char *argv[])
{
	init_scr ();
	clear ();
	dsp_screen ("Converting invoice numbers", " ", " ");

	abc_dbopen (data);

	proc_arhr ();
	proc_cnre ();
	proc_cohr ();
	proc_cucc ();
	proc_cudr ();
	proc_cuin ();
	proc_curh ();
	proc_cuwk ();
	proc_insf ();
	proc_mhdr ();
	proc_sohr ();
	proc_somi ();
	proc_somc ();

	abc_dbclose (data);

	return (EXIT_SUCCESS);
}

void
proc_arhr (
 void)
{
	open_rec (arhr,  arhr_list, arhr_no_fields, "arhr_hhco_hash");

	arhr_rec.hhco_hash = 0L;

	cc = find_rec (arhr, &arhr_rec, GTEQ, "u");
	while (!cc)
	{
		dsp_process (arhr, arhr_rec.inv_no);
		conv_invoice (arhr_rec.inv_no);

		cc = abc_update (arhr, &arhr_rec);
		if (cc)
		{
			printf ("\n\n\nError = [%d] file = [%s] update = [%s] \n", cc, arhr, "DBUPDATE");
		}

		cc = find_rec (arhr, &arhr_rec, NEXT, "u");
	}
	abc_fclose (arhr);
		
	return;
}

void
proc_cnre (
 void)
{
	open_rec (cnre,  cnre_list, cnre_no_fields, "cnre_hhch_hash");

	cnre_rec.hhch_hash = 0L;

	cc = find_rec (cnre, &cnre_rec, GTEQ, "u");
	while (!cc)
	{
		dsp_process (cnre, cnre_rec.inv_no);
		conv_invoice (cnre_rec.inv_no);

		cc = abc_update (cnre, &cnre_rec);
		if (cc)
		{
			printf ("\n\n\nError = [%d] file = [%s] update = [%s] \n", cc, cnre, "DBUPDATE");
		}

		cc = find_rec (cnre, &cnre_rec, NEXT, "u");
	}
	abc_fclose (cnre);

	return;
}

void
proc_cohr (
 void)
{
	open_rec (cohr,  cohr_list, cohr_no_fields, "cohr_hhco_hash");
	cohr_rec.hhco_hash = 0L;

	cc = find_rec (cohr, &cohr_rec, GTEQ, "u");
	while (!cc)
	{
		dsp_process (cohr, cohr_rec.inv_no);
		conv_invoice (cohr_rec.inv_no);

		cc = abc_update (cohr, &cohr_rec);
		if (cc)
		{
			printf ("\n\n\nError = [%d] file = [%s] update = [%s] \n", cc, cohr, "DBUPDATE");
		}

		cc = find_rec (cohr, &cohr_rec, NEXT, "u");
	}
	abc_fclose (cohr);
	return;
}

void
proc_cucc (
 void)
{
	open_rec (cucc,  cucc_list, cucc_no_fields, "cucc_hhcu_hash");
	cucc_rec.hhcu_hash = 0L;

	cc = find_rec (cucc, &cucc_rec, GTEQ, "u");
	while (!cc)
	{
		dsp_process (cucc, cucc_rec.hold_ref);
		conv_invoice (cucc_rec.hold_ref);

		cc = abc_update (cucc, &cucc_rec);
		if (cc)
		{
			printf ("\n\n\nError = [%d] file = [%s] update = [%s] \n", cc, cucc, "DBUPDATE");
		}

		cc = find_rec (cucc, &cucc_rec, NEXT, "u");
	}
	abc_fclose (cucc);
	return;
}


void
proc_cudr (
 void)
{
	open_rec (cudr,  cudr_list, cudr_no_fields, "cudr_co_no");
	strcpy (cudr_rec.co_no, "  ");
	cc = find_rec (cudr, &cudr_rec, GTEQ, "u");
	while (!cc)
	{
		dsp_process (cudr, cudr_rec.invoice);
		conv_invoice (cudr_rec.invoice);

		cc = abc_update (cudr, &cudr_rec);
		if (cc)
		{
			printf ("\n\n\nError = [%d] file = [%s] update = [%s] \n", cc, cudr, "DBUPDATE");
		}

		cc = find_rec (cudr, &cudr_rec, NEXT, "u");
	}
	abc_fclose (cudr);
	return;
}

void
proc_cuin (
 void)
{
	open_rec (cuin,  cuin_list, cuin_no_fields, "cuin_hhcu_hash");
	cuin_rec.hhcu_hash = 0L;

	cc = find_rec (cuin, &cuin_rec, GTEQ, "u");
	while (!cc)
	{
		dsp_process (cuin, cuin_rec.inv_no);
		conv_invoice (cuin_rec.inv_no);

		cc = abc_update (cuin, &cuin_rec);
		if (cc)
		{
			printf ("\n\n\nError = [%d] file = [%s] update = [%s] \n", cc, cuin, "DBUPDATE");
		}

		cc = find_rec (cuin, &cuin_rec, NEXT, "u");
	}
	abc_fclose (cuin);
	return;
}

void
proc_curh (
 void)
{
	open_rec (curh,  curh_list, curh_no_fields, "curh_hhbr_hash");
	curh_rec.hhbr_hash = 0L;

	cc = find_rec (curh, &curh_rec, GTEQ, "u");
	while (!cc)
	{
		dsp_process (curh, curh_rec.inv_no);
		conv_invoice (curh_rec.inv_no);

		cc = abc_update (curh, &curh_rec);
		if (cc)
		{
			printf ("\n\n\nError = [%d] file = [%s] update = [%s] \n", cc, curh, "DBUPDATE");
		}

		cc = find_rec (curh, &curh_rec, NEXT, "u");
	}
	abc_fclose (curh);
	return;
}

void
proc_cuwk (
 void)
{
	open_rec (cuwk,  cuwk_list, cuwk_no_fields, "cuwk_co_no");
	strcpy (cuwk_rec.co_no, "  ");

	cc = find_rec (cuwk, &cuwk_rec, GTEQ, "u");
	while (!cc)
	{
		dsp_process (cuwk, cuwk_rec.inv_no);
		conv_invoice (cuwk_rec.inv_no);

		cc = abc_update (cuwk, &cuwk_rec);
		if (cc)
		{
			printf ("\n\n\nError = [%d] file = [%s] update = [%s] \n", cc, cuwk, "DBUPDATE");
		}

		cc = find_rec (cuwk, &cuwk_rec, NEXT, "u");
	}
	abc_fclose (cuwk);
	return;
}

void
proc_insf (
 void)
{
	open_rec (insf,  insf_list, insf_no_fields, "insf_hhsf_hash");
	insf_rec.hhsf_hash = 0L;

	cc = find_rec (insf, &insf_rec, GTEQ, "u");
	while (!cc)
	{
		dsp_process (insf, insf_rec.invoice_no);
		conv_invoice (insf_rec.invoice_no);

		cc = abc_update (insf, &insf_rec);
		if (cc)
		{
			printf ("\n\n\nError = [%d] file = [%s] update = [%s] \n", cc, insf, "DBUPDATE");
		}

		cc = find_rec (insf, &insf_rec, NEXT, "u");
	}
	abc_fclose (insf);
	return;
}

void
proc_mhdr (
 void)
{
	open_rec (mhdr,  mhdr_list, mhdr_no_fields, "mhdr_co_no");
	strcpy (mhdr_rec.co_no, "  ");

	cc = find_rec (mhdr, &mhdr_rec, GTEQ, "u");
	while (!cc)
	{
		dsp_process (mhdr, mhdr_rec.inv_no);
		conv_invoice (mhdr_rec.inv_no);

		cc = abc_update (mhdr, &mhdr_rec);
		if (cc)
		{
			printf ("\n\n\nError = [%d] file = [%s] update = [%s] \n", cc, mhdr, "DBUPDATE");
		}

		cc = find_rec (mhdr, &mhdr_rec, NEXT, "u");
	}
	abc_fclose (mhdr);
	return;
}

void
proc_sohr (
 void)
{
	open_rec (sohr,  sohr_list, sohr_no_fields, "sohr_hhso_hash");
	sohr_rec.hhso_hash = 0L;

	cc = find_rec (sohr, &sohr_rec, GTEQ, "u");
	while (!cc)
	{
		dsp_process (sohr, sohr_rec.inv_no);
		conv_invoice (sohr_rec.inv_no);

		cc = abc_update (sohr, &sohr_rec);
		if (cc)
		{
			printf ("\n\n\nError = [%d] file = [%s] update = [%s] \n", cc, sohr, "DBUPDATE");
		}

		cc = find_rec (sohr, &sohr_rec, NEXT, "u");
	}
	abc_fclose (sohr);
	return;
}

void
proc_somi (
 void)
{
	open_rec (somi,  somi_list, somi_no_fields, "somi_id_no");
	strcpy (somi_rec.co_no, "  ");
	strcpy (somi_rec.br_no, "  ");
	strcpy (somi_rec.inv_no, "        ");

	cc = find_rec (somi, &somi_rec, GTEQ, "u");
	while (!cc)
	{
		dsp_process (somi, somi_rec.inv_no);
		conv_invoice (somi_rec.inv_no);

		cc = abc_update (somi, &somi_rec);
		if (cc)
		{
			printf ("\n\n\nError = [%d] file = [%s] update = [%s] \n", cc, somi, "DBUPDATE");
        }

		cc = find_rec (somi, &somi_rec, NEXT, "u");
	}
	abc_fclose (somi);
	return;
}

void
proc_somc (
 void)
{
	open_rec (somc,  somc_list, somc_no_fields, "somc_id_no");
	strcpy (somc_rec.co_no, "  ");
	strcpy (somc_rec.br_no, "  ");
	strcpy (somc_rec.start_seq, "        ");
	strcpy (somc_rec.end_seq  , "        ");

	cc = find_rec (somc, &somc_rec, GTEQ, "u");
	while (!cc)
	{
		dsp_process (somc, somc_rec.start_seq);
		conv_invoice (somc_rec.start_seq);
		dsp_process (somc, somc_rec.end_seq);
		conv_invoice (somc_rec.end_seq);

		cc = abc_update (somc, &somc_rec);
		if (cc)
		{
			printf ("\n\n\nError = [%d] file = [%s] update = [%s] \n", cc, somc, "DBUPDATE");
		}

		cc = find_rec (somc, &somc_rec, NEXT, "u");
	}
	abc_fclose (somc);
	return;
}


/*=========================================
| Convert invoice number to 8 characters. |
=========================================*/
char*
conv_invoice (
 char   *inv_no)
{
	char	*sptr = inv_no;

	while (*sptr)
    {
        if (!_isdigit (*sptr) && *sptr != ' ')
        {
			if (strncmp (inv_no, "DP", 2))
			{
                if (atol (inv_no + 1) == 0L)
				{
					return (inv_no);
				}

				sprintf (inv_no, "%c%07ld", inv_no[0], atol (inv_no + 1));
			}
			else
			{
				if (atol (inv_no + 2) == 0L)
				{
					return (inv_no);
				}

				sprintf (inv_no, "%-2.2s%06ld", inv_no, atol (inv_no + 2));
			}
			return (inv_no);
		}
		sptr++;
	}
	if (atol (inv_no) == 0L)
	{
		return (inv_no);
	}

	sprintf (inv_no,"%08ld",atol (inv_no));

	return (inv_no);
}

/* [ end of file ] */

