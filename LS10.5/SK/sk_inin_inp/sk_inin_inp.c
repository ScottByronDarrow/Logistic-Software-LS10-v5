/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_inin_inp.c,v 5.2 2002/07/25 11:17:35 scott Exp $
|  Program Name  : (sk_inin_inp.c)
|  Program Desc  : (Stock Indent Maintenance)
|---------------------------------------------------------------------|
|  Author        : Choo.           | Date Written  : 01/11/89         |
|---------------------------------------------------------------------|
| $Log: sk_inin_inp.c,v $
| Revision 5.2  2002/07/25 11:17:35  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.1  2001/12/12 03:24:46  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_inin_inp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_inin_inp/sk_inin_inp.c,v 5.2 2002/07/25 11:17:35 scott Exp $";

#define	GST	 (gst [0] == 'Y')

#include <ml_sk_mess.h>
#include <ml_std_mess.h>
#include <pslscr.h>
#include <number.h>
#include <twodec.h>

#define		SCN_MAIN	1

	char	*data	=	"data",
			*inum2	=	"inum2",
			*inum3	=	"inum3";

   	int 	newIndent = FALSE;

	char	gstPc [7];
	char	gstCode [4];
	char	gstPrompt [30];
	char	gst [2];
	int		gstInclude = 0;

#include	"schema"

struct commRecord	comm_rec;
struct inasRecord	inas_rec;
struct excfRecord	excf_rec;
struct ingpRecord	ingp_rec;
struct ininRecord	inin_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct inumRecord	inum3_rec;

	struct	{
		char	*_sunit;
		char	*_sdesc;
	
	} sale_unit [] = {
		{"KG  ","Kilograms        "},
		{"L   ","Litres"},
		{"ML  ","Millilitres"},
		{"GM  ","Grams"},
		{"EA  ","Each"},
		{"THD ","Thousand"},
		{""," "},
	};

	struct	{
		char	*_source;
		char	*_sdesc;
	} source [] = {
		{"BM"," (Bulk Manufactured)"},
		{"BP"," (Bulk Product)"},
		{"MC"," (Manufactured Component)"},
		{"MP"," (Manufactured Product)"},
		{"PP"," (Purchased Product)"},
		{"RM"," (Raw Material)"},
		{""," "},
	};

	extern	int		TruePosition;
	int		envQcApply = FALSE;


/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char	lprev_item [17];
	char	item_no [17];
	char	std_uom [5];
	char	alt_uom [5];
	char	src_desc [27];
	char	selldesc [41];
	char	buydesc [41];
	float	cnv_fact;
} local_rec;

static	struct	var	vars [] =
{
	{SCN_MAIN, LIN, "itemno",	 3, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "",  "Base Indent No   ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{SCN_MAIN, LIN, "next_indent",	 3, 36, LONGTYPE,
		"NNNNNN", "          ",
		" ", "0", "Last Seq. No     ", " ",
		YES, NO, JUSTRIGHT, "0", "999999", (char *)&inin_rec.next_indent},
	{SCN_MAIN, LIN, "_class",	 5, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Class            ", "",
		YES, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", inin_rec.inin_class},
	{SCN_MAIN, LIN, "category",	 6, 2, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "Category         ", " ",
		YES, NO,  JUSTLEFT, "", "", inin_rec.category},
	{SCN_MAIN, LIN, "cat_desc",	 6, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", excf_rec.cat_desc},
	{SCN_MAIN, LIN, "sellgrp",	 7, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Selling Group    ", " ",
		YES, NO,  JUSTLEFT, "", "", inin_rec.sellgrp},
	{SCN_MAIN, LIN, "selldesc",	 7, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.selldesc},
	{SCN_MAIN, LIN, "buygrp",	 8, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Buying Group     ", " ",
		YES, NO,  JUSTLEFT, "", "", inin_rec.buygrp},
	{SCN_MAIN, LIN, "buydesc",	 8, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.buydesc},
	{SCN_MAIN, LIN, "source",	 10, 2, CHARTYPE,
		"UU", "          ",
		" ", "PP", "Source           ", "Use search Key for valid source Codes and descriptions. ",
		 NO, NO,  JUSTLEFT, "", "", inin_rec.source},
	{SCN_MAIN, LIN, "src_desc",	10 , 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.src_desc},
	{SCN_MAIN, LIN, "active_status",	 11, 2, CHARTYPE,
		"U", "          ",
		" ", "A",  "Active Status    ", "Enter Active Status. Full Search Available",
		NO, NO,  JUSTLEFT, "", "", inin_rec.active_status},
	{SCN_MAIN, LIN, "act_desc",	 11, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", inas_rec.description},
	{SCN_MAIN, LIN, "abc_code",	 12, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "ABC Code         ", "Enter ABC Code (A-D)",
		NO, NO,  JUSTLEFT, "ABCD", "", inin_rec.abc_code},
	{SCN_MAIN, LIN, "abc_update",	 12, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "ABC Update       ", "Automatic Update of ABC Code (Y/N)",
		NO, NO,  JUSTLEFT, "YN", "", inin_rec.abc_update},
	{SCN_MAIN, LIN, "serial",	13, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Serial Item      ", "Y(es) - Serial Item  / N(o) not serial item",
		YES, NO, JUSTRIGHT, "YN", "", inin_rec.serial_item},
	{SCN_MAIN, LIN, "costing_flag",	13, 36, CHARTYPE,
		"U", "          ",
		" ", "F", "Costing Type     ", "L (ast) A (verage) F (IFO) I-LIFO B (OM) S (erial)",
		YES, NO, JUSTRIGHT, "LAFIBS", "", inin_rec.costing_flag},
	{SCN_MAIN, LIN, "lot_ctrl",	14, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Lot Control      ", " Enter Y(es) if Lot Control Required.",
		 NO, NO, JUSTRIGHT, "YN", "", inin_rec.lot_ctrl},
	{SCN_MAIN, LIN, "dec_pt",	14, 36, INTTYPE,
		"N", "          ",
		" ", "2", "Decimal Places   ", " (0-6) Number of decimal places for Stock. ",
		 NO, NO, JUSTRIGHT, "0", "6", (char *)&inin_rec.dec_pt},
	{SCN_MAIN, LIN, "reorder_flag",	 15, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Reorder Item ?   ", "Y(es) = Reorder item, N(o) = Prompt for reorder.",
		NO, NO,  JUSTLEFT, "YN", "", inin_rec.reorder},
	{SCN_MAIN, LIN, "gst",	15, 36, FLOATTYPE,
		"NN.NN", "          ",
		" ", gstPc, gstPrompt, " ",
		YES, NO, JUSTRIGHT, "0", "99.99", (char *)&inin_rec.gst_pc},
	{SCN_MAIN, LIN, "taxp",	16, 2, FLOATTYPE,
		"NN.NN", "          ",
		" ", "0", "Tax %            ", " ",
		YES, NO, JUSTRIGHT, "0", "99.99", (char *)&inin_rec.tax_pc},
	{SCN_MAIN, LIN, "tax_amount",	16, 36, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Last WS Price    ", " ",
		YES, NO, JUSTRIGHT, "0", "9999999.99", (char *)&inin_rec.tax_amount},
	{SCN_MAIN, LIN, "std_uom",	17, 2, CHARTYPE,
		"AAAA", "          ",
		" ", "EA", "Standard  UOM    ", "Use Search key for valid unit of Measures ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.std_uom},
	{SCN_MAIN, LIN, "alt_uom",	17, 36, CHARTYPE,
		"AAAA", "          ",
		" ", local_rec.std_uom, "Alternate UOM    ", "Use Search key for valid unit of Measures. ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.alt_uom},
	{SCN_MAIN, LIN, "cnv_fact",	18, 2, FLOATTYPE,
		"NNNNNNN.NNN", "          ",
		" ", "",   "Conv Factor      ", "Enter Conversion Factor. ",
		 NO, NO,  JUSTLEFT, "", "", (char *)&local_rec.cnv_fact},
	{SCN_MAIN, LIN, "outer",	18, 36, FLOATTYPE,
		"NNNNN.N", "          ",
		" ", "1.00", "Pricing Conv     ", " Cost & Sale quantity / price conversion.",
		 NO, NO, JUSTRIGHT, "0", "99999.9", (char *)&inin_rec.outer_size},
	{SCN_MAIN, LIN, "qcReqd",	19, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "QC Check Reqd    ", "Y(es) QC Checking Required. N(o) Not required",
		YES, NO,  JUSTLEFT, "NY", "", inin_rec.qc_reqd},
	{SCN_MAIN, LIN, "qcTime",	19, 36, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0.00", "QC Check Time    ", "QC Check Time In Weeks.",
		YES, NO, JUSTRIGHT, "0.00", "999.99", (char *) &inin_rec.qc_time},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		 (void);
void 	OpenDB 				 (void);
void 	CloseDB 			 (void);
int  	spec_valid 			 (int);
void 	Update 				 (void);
void 	SrchForIndent 		 (char *);
void 	SrchExcf 			 (char *);
void 	SrchSource 			 (void);
void 	SrchInum 			 (char *);
void 	SrchIngp 			 (char *, int);
void	SrchInas 			 (char *);
int  	CalculateConversion (void);
int  	ValidUOM 			 (void);
void 	CheckSource 		 (void);
int  	heading 			 (int);

/*
 * Main Processing Routine.
 */
int
main (
 int argc,
 char * argv [])
{
	char	*sptr;

	SETUP_SCR (vars);

	TruePosition	=	TRUE;
	gstInclude = atoi (get_env ("GST_INCLUSIVE"));
	sprintf (gst, "%-1.1s", get_env ("GST"));
	sprintf (gstCode, "%-3.3s", get_env ("GST_TAX_NAME"));
	sprintf (gstPrompt, "%-3.3s %%          : ", gstCode);
	envQcApply = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;
	FLD ("qcReqd")		= envQcApply ? YES : ND;
	FLD ("qcTime")		= envQcApply ? YES : ND;

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty (); 			/*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	/*
	 * Take Out Gst for Non New-Zealand Clients.
	 */
	if (GST)
	{
		FLD ("gst")     	= YES;
		FLD ("taxp")    	= ND;
		FLD ("tax_amount") 	= ND;
	}
	else
	{
		FLD ("gst")     	= ND;
		FLD ("taxp")    	= YES;
		FLD ("tax_amount") 	= YES;
	}

	/*
	 * Set up Default for Gst.
	 */
	sprintf (gstPc, "%3.2f", (gstInclude) ? 0.00 : comm_rec.gst_rate);

	/*
	 * open main database files.
	 */
	OpenDB ();

	/*
	 * Beginning of input control loop.
	 */
	while (!prog_exit)
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		newIndent	= FALSE;
		search_ok	= TRUE;
		init_vars (1);

		/*
		 * Enter screen 1 linear input.
		 */
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		/*
		 * Update stock master record.
		 */
		Update (); 
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (inum2, inum);
	abc_alias (inum3, inum);
 
	open_rec (inas,  inas_list, INAS_NO_FIELDS, "inas_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_id_no");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_uom");
	open_rec (inum3, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inin , inin_list, ININ_NO_FIELDS, "inin_id_no");
	open_rec (ingp , ingp_list, INGP_NO_FIELDS, "ingp_id_no2");
	open_rec (excf , excf_list, EXCF_NO_FIELDS, "excf_id_no");
}	

/*
 * Close data base files.
 */
void
CloseDB (void)
{
    abc_unlock (inas);
    abc_unlock (inin);
    abc_unlock (ingp);
    abc_unlock (inum);
    abc_unlock (inum2);
    abc_unlock (inum3);
	abc_fclose (excf);
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	int	i;
	char	*sptr;

	/*
	 * Validate Item Number.
	 */
	if (LCHECK ("itemno"))
	{
		if (SRCH_KEY)
		{
			SrchForIndent (temp_str);
			return (EXIT_SUCCESS);
		}
		newIndent = FALSE;

		/*
		 * check if item_no contains "INDENT"
		 */
		sptr = clip (local_rec.item_no);
		sptr = strstr (local_rec.item_no, "INDENT");
		if (!sptr)
		{
			print_mess (ML (mlSkMess543));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		sprintf (inin_rec.item_no,"%-16.16s",local_rec.item_no);
		strcpy (inin_rec.co_no,comm_rec.co_no);
		cc = find_rec (inin ,&inin_rec,COMPARISON,"w");
		if (cc) 
			newIndent = TRUE;
		else
		{
			strcpy (excf_rec.co_no,comm_rec.co_no);
			strcpy (excf_rec.cat_no,inin_rec.category);
			cc = find_rec (excf ,&excf_rec,COMPARISON,"r");
			if (cc) 
				file_err (cc, excf, "DBFIND");

			strcpy (ingp_rec.co_no, comm_rec.co_no);
			strcpy (ingp_rec.code, inin_rec.sellgrp);
			strcpy (ingp_rec.type, "S");
			cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
			if (cc)
				strcpy (local_rec.selldesc, " ");
			else
				strcpy (local_rec.selldesc, ingp_rec.desc);

			strcpy (ingp_rec.co_no, comm_rec.co_no);
			strcpy (ingp_rec.code, inin_rec.buygrp);
			strcpy (ingp_rec.type, "B");
			cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
			if (cc)
				strcpy (local_rec.buydesc, " ");
			else
				strcpy (local_rec.buydesc, ingp_rec.desc);

			strcpy (inas_rec.co_no, comm_rec.co_no);
			sprintf (inas_rec.act_code, "%-1.1s", inin_rec.active_status);
			cc = find_rec (inas, &inas_rec, COMPARISON, "r");
			if (cc)
				strcpy (inas_rec.description, " ");

			DSP_FLD ("buydesc");
			DSP_FLD ("selldesc");
			DSP_FLD ("act_desc");
			entry_exit = TRUE;
		}

		for (i = 0;strlen (source [i]._source);i++)
		{
			if (!strcmp (source [i]._source,inin_rec.source))
				sprintf (local_rec.src_desc,
					"%-26.26s",
			           	source [i]._sdesc);
		}
		CheckSource ();

		inum2_rec.hhum_hash	=	inin_rec.std_uom;
		cc = find_rec (inum3, &inum2_rec, COMPARISON, "r");
		if (cc)
			strcpy (local_rec.std_uom, "    ");
		else
			sprintf (local_rec.std_uom, "%-4.4s", inum2_rec.uom);

		inum3_rec.hhum_hash	=	inin_rec.alt_uom;
		cc = find_rec (inum3, &inum3_rec, COMPARISON, "r");
		if (cc)
			strcpy (local_rec.alt_uom, "    ");
		else
			sprintf (local_rec.alt_uom, "%-4.4s", inum3_rec.uom);

		if (!strcmp (inum2_rec.uom_group,inum3_rec.uom_group))
			FLD ("cnv_fact") = NA;
		else
		{
			if (FLD ("cnv_fact") != ND)
				FLD ("cnv_fact") = YES;
		}
		local_rec.cnv_fact = inin_rec.uom_cfactor;
		return (EXIT_SUCCESS);
	}
			
	/*
	 * Validate Category.
	 */
	if (LCHECK ("category"))
	{
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,inin_rec.category);
		cc = find_rec (excf ,&excf_rec,COMPARISON,"r");
		if (cc) 
		{
			errmess (ML (mlStdMess004));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (excf_rec.item_alloc [0] == 'N')
		{
			print_mess (ML (mlSkMess480));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sellgrp"))
	{
		if (SRCH_KEY)
		{
			SrchIngp (temp_str, FALSE);
			return (EXIT_SUCCESS);
		}
		strcpy (ingp_rec.co_no, comm_rec.co_no);
		strcpy (ingp_rec.code, inin_rec.sellgrp);
		strcpy (ingp_rec.type, "S");
		cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess208));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.selldesc, ingp_rec.desc);
		DSP_FLD ("selldesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("buygrp"))
	{
		if (SRCH_KEY)
		{
			SrchIngp (temp_str, TRUE);
			return (EXIT_SUCCESS);
		}
		strcpy (ingp_rec.co_no, comm_rec.co_no);
		strcpy (ingp_rec.code, inin_rec.buygrp);
		strcpy (ingp_rec.type, "B");
		cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess207));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.buydesc, ingp_rec.desc);
		DSP_FLD ("buydesc");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Costing flag.
	 */
	if (LCHECK ("costing_flag"))
	{
		if (inin_rec.costing_flag [0] == 'S' && inin_rec.serial_item [0] != 'Y')
		{
			errmess (ML (mlSkMess542));
			sleep (sleepTime);
			strcpy (inin_rec.serial_item,"Y");
			DSP_FLD ("serial");
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}
		
	/*
	 * Validate G.S.T.  
	 */
	if (LCHECK ("gst"))
	{
		if (FLD ("gst") == ND)
	  		inin_rec.gst_pc = 0.00;
	
		if (gstInclude)
		{
			if (inin_rec.gst_pc != 0.00)
			{
				errmess (ML (mlSkMess481));
				sleep (sleepTime);
				return (EXIT_FAILURE); 
			}
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("std_uom")) 
	{
		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}
		sprintf (inum2_rec.uom, "%-4.4s", local_rec.std_uom);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		inin_rec.std_uom = inum2_rec.hhum_hash;

		CalculateConversion ();
		if (!ValidUOM ())
			return (EXIT_FAILURE);

		DSP_FLD ("std_uom");
		DSP_FLD ("cnv_fact");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("alt_uom"))
	{
		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}
		sprintf (inum2_rec.uom, "%-4.4s", local_rec.alt_uom);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		inin_rec.alt_uom = inum2_rec.hhum_hash;

		CalculateConversion ();
		if (!ValidUOM ())
			return (EXIT_FAILURE);

		DSP_FLD ("alt_uom");
		DSP_FLD ("cnv_fact");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("source"))
	{
		if (SRCH_KEY)
		{
			SrchSource ();
			return (EXIT_SUCCESS);
		}
		for (i = 0;strlen (source [i]._source);i++)
		{
			if (!strcmp (source [i]._source,inin_rec.source))
			{
				sprintf (local_rec.src_desc, "%-26.26s", source [i]._sdesc);
				DSP_FLD ("src_desc");
				CheckSource ();

				return (EXIT_SUCCESS);
			}
		}
		print_mess (ML (mlSkMess552));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (LCHECK ("lot_ctrl"))
	{
		if (inin_rec.lot_ctrl [0] == 'Y')
		{
			strcpy (inin_rec.costing_flag, "F");
			FLD ("costing_flag") = NA;
			DSP_FLD ("costing_flag");
		}
		else
			FLD ("costing_flag") = NO;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("qcReqd"))
	{
		if (!envQcApply)
			return (EXIT_SUCCESS);

		if (inin_rec.qc_reqd [0] == 'Y')
		{
			FLD ("qcTime") = YES;
			if (prog_status != ENTRY)
			{
				do
				{
					get_entry (label ("qcTime"));
					cc = spec_valid (label ("qcTime"));
				} while (cc && !restart);
			}
		}
		else
		{
			FLD ("qcTime") = NA;
			inin_rec.qc_time = 0.00;
		}
		DSP_FLD ("qcTime");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate active status.
	 */
	if (LCHECK ("active_status"))
	{
		if (SRCH_KEY)
		{
			SrchInas (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (inas_rec.co_no, comm_rec.co_no);
		sprintf (inas_rec.act_code, "%-1.1s", inin_rec.active_status);
		cc = find_rec (inas, &inas_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlSkMess312));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("act_desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Update All inventory files.
 */
void
Update (void)
{
	clear ();

	inin_rec.uom_cfactor = local_rec.cnv_fact;
	strcpy (inin_rec.sale_unit, local_rec.std_uom);
	strcpy (inin_rec.pack_size, local_rec.std_uom);
	strcpy (inin_rec.stat_flag,"0");

	/*
	 * Add inventory master record 
	 */
	if (newIndent == TRUE) 
	{
		cc = abc_add (inin ,&inin_rec);
		if (cc) 
			file_err (cc, inin, "DBADD");

		strcpy (inin_rec.co_no,comm_rec.co_no);
		cc = find_rec (inin ,&inin_rec,COMPARISON,"r");
		if (cc) 
			file_err (cc, inin, "DBFIND");
	}
	else 
	{
		/*
		 * update inventory master record 
		 */
		strcpy (inin_rec.co_no,comm_rec.co_no);
		cc = abc_update (inin ,&inin_rec);
		if (cc) 
			file_err (cc, inin, "DBUPDATE");

	}
	abc_unlock (inin);
	strcpy (local_rec.lprev_item,inin_rec.item_no);
}

void
SrchInas (
	char 	*keyValue)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Active Status Description");
	strcpy (inas_rec.co_no, comm_rec.co_no);
	strcpy (inas_rec.act_code, " ");
	cc = find_rec (inas, &inas_rec, GTEQ, "r");
	while (!cc && !strcmp (inas_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (inas_rec.act_code, inas_rec.description);
		if (cc)
			break;

		cc = find_rec (inas, &inas_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (inas_rec.co_no, comm_rec.co_no);
	strcpy (inas_rec.act_code, temp_str);
	cc = find_rec (inas, &inas_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inas, "DBFIND");
}
void
SrchForIndent (
	char	*keyValue)
{
	_work_open (16,0,8);
	strcpy (inin_rec.co_no,comm_rec.co_no);
	sprintf (inin_rec.item_no,"%-16.16s",keyValue);
	save_rec ("#Base Indent No","#Last Sequence No");
	cc = find_rec (inin ,&inin_rec,GTEQ,"r");
	while (!cc && !strncmp (inin_rec.item_no,keyValue,strlen (keyValue)) && 
		      !strcmp (inin_rec.co_no,comm_rec.co_no))
	{
		sprintf (err_str,"%06ld",inin_rec.next_indent);

		cc = save_rec (inin_rec.item_no,err_str);
		if (cc)
			break;
		cc = find_rec (inin ,&inin_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (inin_rec.co_no,comm_rec.co_no);
	sprintf (inin_rec.item_no,"%-16.16s",temp_str);
	cc = find_rec (inin ,&inin_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, inin, "DBFIND");
}

/*
 * Search for Category master file.
 */
void
SrchExcf (
	char	*keyValue)
{
	_work_open (11,0,40);
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no,"%-11.11s",keyValue);
	save_rec ("#Category","#Category Description");
	cc = find_rec (excf ,&excf_rec,GTEQ,"r");
	while (!cc && !strncmp (excf_rec.cat_no,keyValue,strlen (keyValue)) && 
		      !strcmp (excf_rec.co_no,comm_rec.co_no))
	{
		cc = save_rec (excf_rec.cat_no,excf_rec.cat_desc);
		if (cc)
			break;
		cc = find_rec (excf ,&excf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no,"%-11.11s",temp_str);
	cc = find_rec (excf ,&excf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, excf, "DBFIND");
}

void
SrchSource (
 void)
{
	int	i;
	_work_open (2,0,40);
	save_rec ("#Sr","#Description");
	for (i = 0;strlen (source [i]._source);i++)
	{
		cc = save_rec (source [i]._source,source [i]._sdesc);
		if (cc)
			break;
	}
	disp_srch ();
	work_close ();
}

void
SrchInum (
	char	*keyValue)
{
	_work_open (4,0,40);
	save_rec ("#UOM","#UOM Description ");
	sprintf (inum_rec.uom_group, "%-20.20s", " ");
	inum_rec.hhum_hash = 0L;

	cc = find_rec (inum, &inum_rec, GTEQ, "r");
	while (!cc)
	{
		cc = save_rec (inum_rec.uom,inum_rec.desc);
		if (cc)
			break;

		cc = find_rec (inum, &inum_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
}

void
SrchIngp (
	char	*keyValue, 
	int 	isbuy)
{
	_work_open (6,0,40);
	save_rec ("#Code", "#Description ");

	strcpy (ingp_rec.co_no, comm_rec.co_no);
	strcpy (ingp_rec.type, (isbuy) ? "B" : "S");
	sprintf (ingp_rec.code, "%-6.6s", keyValue);
	cc = find_rec (ingp, &ingp_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ingp_rec.co_no, comm_rec.co_no) &&
	       !strncmp (ingp_rec.code, keyValue, strlen (keyValue)))
	{
		if ((isbuy && ingp_rec.type [0] == 'B') ||
			 (!isbuy && ingp_rec.type [0] == 'S'))
				cc = save_rec (ingp_rec.code, ingp_rec.desc);
		else
			break;

		if (cc)
			break;

		cc = find_rec (ingp, &ingp_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ingp_rec.co_no, comm_rec.co_no);
	strcpy (ingp_rec.type, (isbuy) ? "B" : "S");
	sprintf (ingp_rec.code, "%-6.6s", temp_str);
	cc = find_rec (ingp, &ingp_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ingp, "DBFIND");
}

int
CalculateConversion (
 void)
{
	number	cnv_fct;
	number	std_cnv_fct;
	number	alt_cnv_fct;

	inum2_rec.hhum_hash	=	inin_rec.std_uom;
	cc = find_rec (inum3, &inum2_rec, EQUAL, "r");
	if (cc)
	{
		local_rec.cnv_fact = 0.00;
		return (EXIT_SUCCESS);
	}

	inum3_rec.hhum_hash	=	inin_rec.alt_uom;
	cc = find_rec (inum3, &inum3_rec, EQUAL, "r");
	if (cc)
	{
		local_rec.cnv_fact = 0.00;
		return (EXIT_SUCCESS);
	}

	if (strcmp (inum2_rec.uom_group, inum3_rec.uom_group))
	{
		if (FLD ("cnv_fact") != ND)
			FLD ("cnv_fact") = YES;
		local_rec.cnv_fact = 0.00;
		return (EXIT_SUCCESS);
	}

	/*
	 * converts a float to arbitrary precision number
	 * defined as number.                           
	 */
	NumFlt (&std_cnv_fct, inum2_rec.cnv_fct);
	NumFlt (&alt_cnv_fct, inum3_rec.cnv_fct);

	/*
	 * a function that divides one number by another and places
	 * the result in another number defined variable          
	 * std uom cnv_fct / alt uom cnv_fct = conversion factor 
	 */
	NumDiv (&std_cnv_fct, &alt_cnv_fct, &cnv_fct);

	/*
	 * converts a arbitrary precision number to a float
	 */
	local_rec.cnv_fact = NumToFlt (&cnv_fct);

	if (!strcmp (inum2_rec.uom_group, inum3_rec.uom_group))
		FLD ("cnv_fact") = NA;

	return (EXIT_SUCCESS);
}

/*
 * Check whether uom is valid compared with
 * the dec_pt and the conversion factor.   
 * eg. std uom = kg     iss uom = gm        
 *     conv.fact = 1000 dec_pt = 2          
 *     issue 5 gm, converts to 0.005 kg     
 *     round to 2 dec_pt, new qty = 0.01 kg 
 *     or 10gm                              
 *This is incorrect and not allowed.        
 */
int
ValidUOM (
 void)
{
	long	numbers [7];

	numbers [0] = 1;
	numbers [1] = 10;
	numbers [2] = 100;
	numbers [3] = 1000;
	numbers [4] = 10000;
	numbers [5] = 100000;
	numbers [6] = 1000000;

	if (local_rec.cnv_fact > numbers [inin_rec.dec_pt])
	{
		sprintf (err_str, ML (mlSkMess482),local_rec.alt_uom, inin_rec.dec_pt);
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}
	return (TRUE);
}

void
CheckSource (void)
{
	int	i;

	for (i = 0;strlen (source [i]._source);i++)
		if (!strcmp (source [i]._source,inin_rec.source))
			break;
}
/*
 * Heading concerns itself with clearing the screen,painting the 
 * screen overlay in preparation for input.                     
 */
int
heading (
	int		scn)
{
	if (restart) 
	{
       	abc_unlock (inin);
    	return (EXIT_SUCCESS);
	}
	if (scn != cur_screen)
		scn_set (scn);
	clear ();

	rv_pr (ML (mlSkMess483),16, 0, 1);
	print_at (0,52,ML (mlSkMess484), local_rec.lprev_item);

	box (0, 2, 80, 17);
	line_at (1,0,80);
	line_at (4,1,79);
	line_at (9,1,79);
	line_at (21,0,80);

	if (GST)
		line_at (16,1,79);

	strcpy (err_str,ML (mlStdMess038));
	print_at (22,0, err_str,comm_rec.co_no, comm_rec.co_name);
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
