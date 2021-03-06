/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Include Name  : ( srch_pcwo.h    )                                 |
|  Include Desc  : ( Interactively allow searching on specific    )   |
|                  ( status(es) of pcwo records.                  )   |
|---------------------------------------------------------------------|
|  Access files  :  inmr, pcwo,     ,     ,     ,     ,     ,     ,   |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files : NEVER,     ,     ,     ,     ,     ,     ,     ,   |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Date Written  : 20/01/94        | Author       : Aroha Merrilees.  |
|---------------------------------------------------------------------|
|  Date Modified : (16/02/94)      | Modified  by : Campbell Mander,  |
|  Date Modified : (05/10/94)      | Modified  by : Aroha Merrilees.  |
|                                                                     |
|  Comments      : Two parameters are required,                       |
|                : 1: The partial order number (Maybe blank!!)        |
|                : 2: A string of allowable statuses                  |
|                :    Consisting of PFIARCDZ                          |
|           NOTE : This routine will change the inmr index to the     |
|                : inmr_id_no index unconditionally!!                 |
|  (16/02/94)    : PSL 10366. Fix search on batch number.             |
|  (05/10/94)    : PSL 11299 - mfg cutover - add br and wh no to srch |
|                :                                                    |
|                                                                     |
=====================================================================*/
void	srch_order (char *, char *, char *, char *);
void	srch_batch (char *, char *, char *, char *);

void
srch_order (char *key_val, char *val_sts, char *br_no, char *wh_no)
{
	abc_selfield (inmr, "inmr_hhbr_hash");
	abc_selfield (pcwo, "pcwo_id_no");

	work_open ();
	save_rec ("#W/O No.", "#Batch No.  (Item Number     ) Item Description");

	strcpy (pcwo_rec.wo_co_no, comm_rec.tco_no);
	strcpy (pcwo_rec.wo_br_no, br_no);
	strcpy (pcwo_rec.wo_wh_no, wh_no);
	sprintf (pcwo_rec.wo_order_no, "%-7.7s", key_val);
	cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (pcwo_rec.wo_co_no, comm_rec.tco_no) &&
		!strcmp (pcwo_rec.wo_br_no, br_no) &&
		!strcmp (pcwo_rec.wo_wh_no, wh_no) &&
		!strncmp (pcwo_rec.wo_order_no, key_val, strlen (key_val)))
	{
		if (strchr (val_sts, pcwo_rec.wo_order_status[0]))
		{
			cc = find_hash (inmr, &inmr_rec, EQUAL, "r", pcwo_rec.wo_hhbr_hash);
			if (cc)
			{
				sprintf (inmr_rec.mr_item_no, "%-16.16s", " ");
				sprintf (inmr_rec.mr_description, "%-40.40s",
					"DELETED ITEM!!");
			}
			sprintf (err_str, "%-10.10s (%-16.16s) %-40.40s",
				pcwo_rec.wo_batch_no,
				inmr_rec.mr_item_no,
				inmr_rec.mr_description);
			cc = save_rec (pcwo_rec.wo_order_no, err_str);
			if (cc)
			    break;
		}
		cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	    return;
	strcpy (pcwo_rec.wo_co_no, comm_rec.tco_no);
	strcpy (pcwo_rec.wo_br_no, br_no);
	strcpy (pcwo_rec.wo_wh_no, wh_no);
	sprintf (pcwo_rec.wo_order_no, "%-7.7s", temp_str);
	cc = find_rec (pcwo, &pcwo_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, pcwo, "DBFIND");
}

void	srch_batch (char *, char *, char *, char *);

void
srch_batch (char *key_val, char *val_sts, char *br_no, char *wh_no)
{
	abc_selfield (inmr, "inmr_hhbr_hash");
	abc_selfield (pcwo, "pcwo_id_no3");

	work_open ();
	save_rec ("#Batch No.", "#W/O No. (Item Number     ) Item Description");

	strcpy (pcwo_rec.wo_co_no, comm_rec.tco_no);
	strcpy (pcwo_rec.wo_br_no, br_no);
	strcpy (pcwo_rec.wo_wh_no, wh_no);
	sprintf (pcwo_rec.wo_batch_no, "%-10.10s", key_val);
	cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (pcwo_rec.wo_co_no, comm_rec.tco_no) &&
		!strcmp (pcwo_rec.wo_br_no, br_no) &&
		!strcmp (pcwo_rec.wo_wh_no, wh_no) &&
		!strncmp (pcwo_rec.wo_batch_no, key_val, strlen (key_val)))
	{
		if (strchr (val_sts, pcwo_rec.wo_order_status[0]))
		{
			cc = find_hash (inmr, &inmr_rec, EQUAL, "r", pcwo_rec.wo_hhbr_hash);
			if (cc)
			{
				sprintf (inmr_rec.mr_item_no, "%-16.16s", " ");
				sprintf (inmr_rec.mr_description, "%-40.40s",
					"DELETED ITEM!!");
			}
			sprintf (err_str, "%-7.7s (%-16.16s) %-40.40s",
				pcwo_rec.wo_order_no,
				inmr_rec.mr_item_no,
				inmr_rec.mr_description);
			cc = save_rec (pcwo_rec.wo_batch_no, err_str);
			if (cc)
				break;
		}
		cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	    return;
	strcpy (pcwo_rec.wo_co_no, comm_rec.tco_no);
	strcpy (pcwo_rec.wo_br_no, br_no);
	strcpy (pcwo_rec.wo_wh_no, wh_no);
	sprintf (pcwo_rec.wo_batch_no, "%-10.10s", temp_str);
	cc = find_rec (pcwo, &pcwo_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, pcwo, "DBFIND");
}
#include	<wild_search.h>
