/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: SkIrCodes.c,v 5.1 2001/08/06 22:49:50 scott Exp $
|  Author            : Scott Darrow.   | Date Written  : 24/01/91     |
|  Source Name       : sk_ir_code.c                                   |
|  Source Desc       : Parse source code for : sk_ir_trans.c &        |
|                    :                         sk_ir_mctran.c         |
|---------------------------------------------------------------------|
| $Log: SkIrCodes.c,v $
| Revision 5.1  2001/08/06 22:49:50  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 06:51:25  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:59:21  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/12 13:28:52  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/10/11 07:24:39  scott
| New include to allow compile with app.schema
|
=====================================================================*/
#include <sk_ir_inc.h>

char	dflt_qty [15];
char	rep_qty [30];

/*===========================
| Function prototypes       |
===========================*/
void	parse		 (char *	wrk_prt);
int		valid_cmd	 (char *	wk_str);
void	subst_cmd	 (int cmd);
void	pr_line		 (char * ln_sptr);


/*===========================
| Parse all relevent lines. |
===========================*/
void
parse  (
 char *	wrk_prt)
{
	char	*sptr,
			*chk_env  (char *);
	int		after,
			before;
	int		cmd;
	char	*cptr;
	char	*dptr;
	char	*wk_prt = p_strsave  (wrk_prt);

	sptr = chk_env  ("SK_QTY_MASK");
	if  (sptr ==  (char *)0)
		strcpy  (dflt_qty, "NNNNNNN.NNNNNN");
	else
		strcpy  (dflt_qty, sptr);
	before = strlen  (dflt_qty);
	sptr = strrchr  (dflt_qty, '.');
	if  (sptr)
		after =  (int)  ((sptr + strlen  (sptr) - 1) - sptr);
	else
		after = 0;
	if  (after == 0)
		sprintf  (rep_qty, "%%%df", before);
	else
		sprintf  (rep_qty, "%%%d.%df", before, after);

	part_printed = TRUE;

	/*-------------------------------
	|	look for caret command	|
	-------------------------------*/
	cptr = strchr  (wk_prt,'.');
	dptr = wk_prt;
	while  (cptr)
	{
		part_printed = FALSE;
		/*-------------------------------
		|	print line up to now	|
		-------------------------------*/
		*cptr =  (char) NULL;

		if  (cptr != wk_prt)
		{
			part_printed = TRUE;
			fprintf (fout,"%s",dptr);
		}

		/*-------------------------------
		|	check if valid .command	|
		-------------------------------*/
		cmd = valid_cmd (cptr + 1);
		if  (cmd >= ISS_CO_NUMB)
		{
			if  (cmd == FORMAT_LN_1)
			{
				sprintf (format_line[0],"%-150.150s", cptr + 12);
				part_printed = FALSE;
				free  (wk_prt);
				return;
			}
			if  (cmd == FORMAT_LN_2)
			{
				sprintf (format_line[1],"%-150.150s", cptr + 12);
				part_printed = FALSE;
				free  (wk_prt);
				return;
			}
			if  (cmd == FORMAT_LN_3)
			{
				sprintf (format_line[2],"%-150.150s", cptr + 12);
				part_printed = FALSE;
				free  (wk_prt);
				return;
			}
			subst_cmd (cmd);
			if  (cmd >= LINE_SPACE1 && cmd <= LINE_SPACE3)
			{
				free  (wk_prt);
				return;
			}

			dptr = cptr + 12;
		}
		else
		{
			fprintf (fout,".");
			part_printed = TRUE;
			dptr = cptr + 1;
		}

		cptr = strchr  (dptr,'.');
	}

	/*-------------------------------
	|	print rest of line	|
	-------------------------------*/
	if  (part_printed)
	{
		if  (dptr)
			fprintf (fout,"%s\n",dptr);
		else
			fprintf (fout,"\n");
	}
	free  (wk_prt);
}

/*=====================
| Validate .commands. |
=====================*/
int
valid_cmd  (
char *	wk_str)
{
	int	i;

	for  (i = 0;i < N_CMDS;i++)
		if  (!strncmp (wk_str,dot_cmds[i],11))
			return (i);

	return (-1);
}

/*==============================================
| Substitute valid .commands with actual data. |
==============================================*/
void
subst_cmd  (
 int	cmd)
{
	part_printed = TRUE;

	switch  (cmd)
	{
	case	ISS_CO_NUMB:
			pr_line (iss_co);
		break;

	case	ISS_CO_NAME:
			pr_line (iss_coname);
		break;

	case	ISS_CO_SHRT:
			pr_line (iss_coshort);
		break;

	case	ISS_CO_ADR1:
			pr_line (iss_co_adr[0]);
		break;

	case	ISS_CO_ADR2:
			pr_line (iss_co_adr[1]);
		break;

	case	ISS_CO_ADR3:
			pr_line (iss_co_adr[2]);
		break;

	case	REC_CO_NUMB:
			pr_line (rec_co);
		break;

	case	REC_CO_NAME:
			pr_line (rec_coname);
		break;

	case	REC_CO_SHRT:
			pr_line (rec_coshort);
		break;

	case	REC_CO_ADR1:
			pr_line (rec_co_adr[0]);
		break;

	case	REC_CO_ADR2:
			pr_line (rec_co_adr[1]);
		break;

	case	REC_CO_ADR3:
			pr_line (rec_co_adr[2]);
		break;

	case	ISS_BR_NUMB:
			pr_line (iss_br);
		break;

	case	ISS_BR_NAME:
			pr_line (iss_brname);
		break;

	case	ISS_BR_SHRT:
			pr_line (iss_brshort);
		break;

	case	ISS_BR_ADR1:
			pr_line (iss_br_adr[0]);
		break;

	case	ISS_BR_ADR2:
			pr_line (iss_br_adr[1]);
		break;

	case	ISS_BR_ADR3:
			pr_line (iss_br_adr[2]);
		break;

	case	REC_BR_NUMB:
			pr_line (rec_br);
		break;

	case	REC_BR_NAME:
			pr_line (rec_brname);
		break;

	case	REC_BR_SHRT:
			pr_line (rec_brshort);
		break;

	case	REC_BR_ADR1:
			pr_line (rec_br_adr[0]);
		break;

	case	REC_BR_ADR2:
			pr_line (rec_br_adr[1]);
		break;

	case	REC_BR_ADR3:
			pr_line (rec_br_adr[2]);
		break;

	case	ISS_WH_NUMB:
			pr_line (iss_wh);
		break;

	case	ISS_WH_NAME:
			pr_line (iss_whname);
		break;

	case	ISS_WH_SHRT:
			pr_line (iss_whshort);
		break;

	case	REC_WH_NUMB:
			pr_line (rec_wh);
		break;

	case	REC_WH_NAME:
			pr_line (rec_whname);
		break;

	case	REC_WH_SHRT:
			pr_line (rec_whshort);
		break;

	case	LN_INMR_PAK:
			pr_line (inmr_rec.pack_size);
		break;

	case	LN_MAK_NUMB:
			pr_line (inmr_rec.maker_no);
		break;

	case	LN_ITM_NUMB:
			pr_line (inmr_rec.item_no);
		break;

	case	LN_ITM_NAME:
			pr_line (local_rec.item_desc);
		break;

	case	LN_QT_ORDER:
			sprintf  (err_str, rep_qty,
					n_dec  (local_rec.qty_ord + local_rec.qty_bord,
					inmr_rec.dec_pt));
			fprintf (fout, "%14s", err_str);
		break;

	case	LN_QT_BORDR:
			sprintf  (err_str, rep_qty, 
					n_dec  (local_rec.qty_bord, inmr_rec.dec_pt));
			fprintf (fout, "%14s", err_str);
		break;

	case	LN_QT_SUPPY:
			sprintf  (err_str, rep_qty,
					n_dec  (local_rec.qty_ord, inmr_rec.dec_pt));
			fprintf (fout, "%14s", err_str);
		break;

	case	LN_SER_NUMB:
			pr_line (local_rec.serial_no);
		break;

	case	LN_LOCATION:
			pr_line (local_rec.location);
		break;

	case	LN_REF_NUMB:
			pr_line (itln_rec.tran_ref);
		break;

	case	LN_CUST_STK:
			if  (itln_rec.stock[0] == 'S')
				pr_line ("S");
			else
				pr_line ("C");
		break;

	case	TRAN_N_DATE:
			fprintf (fout,"%10.10s",DateToString (local_rec.j_date));
		break;

	case	TRAN_M_DATE:
			fprintf (fout,"%10.10s",DateToString (comm_rec.inv_date));
		break;

	case	TRAN_C_DATE:
			fprintf (fout,"%10.10s",local_rec.systemDate);
		break;

	case	TRAN_C_TIME:
			fprintf (fout,"%5.5s",local_rec.sys_time);
		break;

	case	DTE_ENTERED:
			fprintf (fout,"%10.10s",DateToString (ithr_rec.date_create));
		break;

	case	TME_ENTERED:
			fprintf (fout,"%5.5s",ithr_rec.time_create);
		break;

	case	OPERATOR_ID:
			fprintf (fout,"%-14.14s",ithr_rec.op_id);
		break;

	case	DOCKET_NUMB:
			fprintf (fout,"%06ld", ithr_rec.del_no);
		break;

	case	PAGE_NUMBER:
			fprintf (fout,"#");
		break;

	case	TR_TRAN_REF:
			pr_line (ithr_rec.tran_ref);
		break;

	case	TR_MESSAGES:
			pr_line (local_rec.message);
		break;

	case	TR_FULL_SUP:
			pr_line (local_rec.full_supply);
		break;

	case	TR_CAR_CODE:
			pr_line (ithr_rec.carr_code);
		break;

	case	FRGHT_VALUE:
			fprintf (fout,"%7.2f", ithr_rec.frt_cost);
		break;

	case	FRT_TOT_WGT:
			fprintf (fout,"%7.2f", ithr_rec.no_kgs);
		break;

	case	FRT_CONS_NO:
			pr_line (ithr_rec.cons_no);
		break;

	case	FRT_CARTONS:
			fprintf (fout,"%4d", ithr_rec.no_cartons);
		break;

	case	LINE_SPACE1:
			line_spacing = 1;
		break;

	case	LINE_SPACE2:
			line_spacing = 2;
		break;

	case	LINE_SPACE3:
			line_spacing = 3;
		break;

	case	LN_U_O_MEAS:
			pr_line (local_rec.uom);
		break;

	case	ISS_WH_ADR1:
			pr_line (iss_whadd1);
		break;

	case	ISS_WH_ADR2:
			pr_line (iss_whadd2);
		break;

	case 	ISS_WH_ADR3:
			pr_line (iss_whadd3);
		break;

	case	REC_WH_ADR1:
			pr_line (rec_whadd1);
		break;

	case	REC_WH_ADR2:
			pr_line (rec_whadd2);
		break;

	case 	REC_WH_ADR3:
			pr_line (rec_whadd3);
		break;

	default:
		break;
	}
	fflush (fout);
}

void
pr_line  (
 char * ln_sptr)
{
	fprintf (fout,"%-*.*s", (int) strlen (ln_sptr), (int) strlen (ln_sptr),ln_sptr);
}
