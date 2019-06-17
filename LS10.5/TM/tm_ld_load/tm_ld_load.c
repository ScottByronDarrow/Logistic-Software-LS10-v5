/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: tm_ld_load.c,v 5.3 2001/11/14 05:21:30 scott Exp $
|  Program Name  : (tm_ld_load.c)
|  Program Desc  : (Load leads from ascii file)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 04/11/91         |
|---------------------------------------------------------------------|
| $Log: tm_ld_load.c,v $
| Revision 5.3  2001/11/14 05:21:30  scott
| Updated to convert to app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: tm_ld_load.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_ld_load/tm_ld_load.c,v 5.3 2001/11/14 05:21:30 scott Exp $";

#define DELIMIT		 (load_type [0] == 'D')

#define	MAXLINES	32
#define	TABLINES	13

#define	MOD	10
#include <pslscr.h>
#include <getnum.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_tm_mess.h>

#define	BUFFER_SIZE	10240

FILE	*fin;

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int  	envDbCo = 0,
		wk_no,
		envDbFind = 0;

	char	branchNo [3];

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct tmpmRecord	tmpm_rec;
struct tmdfRecord	tmdf_rec;
struct tmdlRecord	tmdl_rec;

/*-------------------------------------------------------------
| List Of Possible Fields To Load                             |
| ALL fields in dbview must be represented here in same order |
| Fields which can NOT be loaded from ascii file have N in    |
|   load_ok                                                   |
-------------------------------------------------------------*/
struct	{
	char	fld_name [19];
	char	load_ok [2];	/* Y(es) N(o) D (efault OK) */
	int		max_len;
	int		fld_no;
	int		offset;
	int		fld_len;
	char	dflt [41];
	char	*dest_fld;
} lead_list [] = {
	{ "Co No             ", "N", 0,  0, 0, 0, " ", ""  },
	{ "Br No             ", "N", 0,  0, 0, 0, " ", ""  },
	{ "Prospect No       ", "N", 0,  0, 0, 0, " ", ""  },
	{ "HHPM_HASH         ", "N", 0,  0, 0, 0, " ", ""  },
	{ "HHCU_HASH         ", "N", 0,  0, 0, 0, " ", ""  },
	{ "Prospect Name     ", "Y",40,  0, 0, 0, " ", tmpm_rec.name       },
	{ "Acronym           ", "Y", 9,  0, 0, 0, " ", tmpm_rec.acronym    },
	{ "Area Code         ", "Y", 2,  0, 0, 0, " ", tmpm_rec.area_code  },
	{ "Salesman Code     ", "Y", 2,  0, 0, 0, " ", tmpm_rec.sman_code  },
	{ "Business Sector   ", "Y", 3,  0, 0, 0, " ", tmpm_rec.b_sector   },
	{ "Post Code         ", "Y",10,  0, 0, 0, " ", tmpm_rec.post_code  },
	{ "Active Flag       ", "D", 0,  0, 0, 0, " ", tmpm_rec.active_flg },
	{ "Contact Name 1    ", "Y", 30, 0, 0, 0, " ", tmpm_rec.cont_name1 },
	{ "Contact Name 2    ", "Y", 30, 0, 0, 0, " ", tmpm_rec.cont_name2 },
	{ "Contact Name 3    ", "Y", 30, 0, 0, 0, " ", tmpm_rec.cont_name3 },
	{ "Contact Name 4    ", "Y", 30, 0, 0, 0, " ", tmpm_rec.cont_name4 },
	{ "Contact Name 5    ", "Y", 30, 0, 0, 0, " ", tmpm_rec.cont_name5 },
	{ "Contact Name 6    ", "Y", 30, 0, 0, 0, " ", tmpm_rec.cont_name6 },
	{ "Contact Code 1    ", "Y", 3,  0, 0, 0, " ", tmpm_rec.cont_code1 },
	{ "Contact Code 2    ", "Y", 3,  0, 0, 0, " ", tmpm_rec.cont_code2 },
	{ "Contact Code 3    ", "Y", 3,  0, 0, 0, " ", tmpm_rec.cont_code3 },
	{ "Contact Code 4    ", "Y", 3,  0, 0, 0, " ", tmpm_rec.cont_code4 },
	{ "Contact Code 5    ", "Y", 3,  0, 0, 0, " ", tmpm_rec.cont_code5 },
	{ "Contact Code 6    ", "Y", 3,  0, 0, 0, " ", tmpm_rec.cont_code6 },
	{ "Mail Address 1    ", "Y", 40, 0, 0, 0, " ", tmpm_rec.mail1_adr  },
	{ "Mail Address 2    ", "Y", 40, 0, 0, 0, " ", tmpm_rec.mail2_adr  },
	{ "Mail Address 3    ", "Y", 40, 0, 0, 0, " ", tmpm_rec.mail3_adr  },
	{ "Delivery Address1 ", "Y", 40, 0, 0, 0, " ", tmpm_rec.del1_adr   },
	{ "Delivery Address2 ", "Y", 40, 0, 0, 0, " ", tmpm_rec.del2_adr   },
	{ "Delivery Address3 ", "Y", 40, 0, 0, 0, " ", tmpm_rec.del3_adr   },
	{ "Fax Number        ", "Y", 15, 0, 0, 0, " ", tmpm_rec.fax_no     },
	{ "Phone Number      ", "Y", 15, 0, 0, 0, " ", tmpm_rec.phone_no   },
	{ "Phone Freq        ", "D", 0,  0, 0, 0, " ", (char *)&tmpm_rec.phone_freq },
	{ "Phone Date        ", "D", 0,  0, 0, 0, " ", (char *)&tmpm_rec.n_phone_date},
	{ "Phone Time        ", "D", 0,  0, 0, 0," ", tmpm_rec.n_phone_time},
	{ "Visit Freq        ", "D", 0,  0, 0, 0, " ", (char *)&tmpm_rec.visit_freq },
	{ "Visit Date        ", "N", 0,  0, 0, 0, " ", ""},
	{ "Visit Time        ", "N", 0,  0, 0, 0, " ", ""},
	{ "Mail Flag         ", "Y", 1,  0, 0, 0, " ", tmpm_rec.mail_flag  },
	{ "Op Code           ", "D", 0,  0, 0, 0, " ", tmpm_rec.op_code    },
	{ "Last Op           ", "N", 0,  0, 0, 0, " ", ""},
	{ "Call Bk DT        ", "N", 0,  0, 0, 0, " ", ""},
	{ "Call Bk TM        ", "N", 0,  0, 0, 0, " ", ""},
	{ "Call No           ", "N", 0,  0, 0, 0, " ", ""},
	{ "Last Phone        ", "N", 0,  0, 0, 0, " ", ""},
	{ "Origin            ", "Y", 3,  0, 0, 0, " ", tmpm_rec.origin     },
	{ "Date Create       ", "N", 0,  0, 0, 0, " ", ""},
	{ "Best Phone Time   ", "Y", 5,  0, 0, 0, " ",tmpm_rec.best_ph_time},
	{ "Delete Flag       ", "N", 0,  0, 0, 0, " ", ""},
	{ "Tax Code          ", "Y", 1,  0, 0, 0, " ", tmpm_rec.tax_code   },
	{ "Tax Number        ", "Y", 15, 0, 0, 0, " ", tmpm_rec.tax_no     },
	{ "Status Flag       ", "Y", 1,  0, 0, 0, " ", tmpm_rec.stat_flag  },
	{ "", "N", 0, 0, 0, 0, "", (char *)0 }
};

char	line_buffer [ BUFFER_SIZE + 1 ];
char	load_type [2];
int		file_open;
int		first_time = TRUE;
int		curr_field;
int		fmt_chosen;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char	systemDate [11];
	long	lsystemDate;
	char	fld_name [19];
	int		fld_no;
	int		offset;
	int		fld_len;
	char	fld_dflt [41];
	char	delimit [2];
	char	filename [41];
	char	fmt_name [15];
} local_rec;

static	struct	var vars [] =
{
	{1, LIN, "filename", 4, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "File Name         :", "",
		 YES, NO,  JUSTLEFT, "", "", local_rec.filename},
	{1, LIN, "fmt_name", 5, 20, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "Default Format    :", "",
		 YES, NO,  JUSTLEFT, "", "", local_rec.fmt_name},
	{1, LIN, "delimit", 6, 20, CHARTYPE,
		"A", "          ",
		" ", "", "Delimit Character :", "",
		 YES, NO,  JUSTLEFT, "", "", local_rec.delimit},

	{2, TAB, "fld",	 MAXLINES, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Field Name    ", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.fld_name},
	{2, TAB, "fld_no", 0, 0, INTTYPE,
		"NNN", "          ",
		" ", "", "Field Number", "",
		 NO, NO,  JUSTRIGHT, "0", "999", (char *)&local_rec.fld_no},
	{2, TAB, "offset", 0, 0, INTTYPE,
		"NNNNN", "          ",
		" ", "", "Offset", "",
		 ND, NO,  JUSTRIGHT, "0", "99999", (char *)&local_rec.offset},
	{2, TAB, "fld_len", 0, 0, INTTYPE,
		"NNN", "          ",
		" ", "", "Length", "",
		 ND, NO,  JUSTRIGHT, "0", "999", (char *)&local_rec.fld_len},
	{2, TAB, "fld_dflt", 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Default                  ", "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.fld_dflt},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

#include	<std_decs.h>
void OpenDB (void);
void CloseDB (void);
void load_dflts (void);
void load_fields (void);
int spec_valid (int field);
void SrchTmdf (char *key_val);
int chk_fields (void);
void process (void);
void init_record (void);
void parse_delimit (char *sptr);
void extract_data (char *sptr, char *tptr);
void parse_fixed (char *sptr);
int heading (int scn);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int argc,
	char *argv [])
{
	if (argc != 2)
	{
		print_at (0,0,"\007Usage: %s <D (elimited) | F (ixed format)>\n",argv [0]);
		return (EXIT_FAILURE);
	}
	sprintf (load_type, "%-1.1s", argv [1]);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind = atoi (get_env ("DB_FIND"));
	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	
	OpenDB (); 	
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	init_vars (1);

	tab_row = 4;
	tab_col = 2;

	if (!DELIMIT)
	{
		FLD ("delimit") = ND;
		vars [label ("fmt_name")].row = 5;
		FLD ("fld_no") = ND;
		FLD ("offset") = NO;
		FLD ("fld_len") = NO;
	}

	/*--------------------
	| Main control loop. |
	--------------------*/
	while (!prog_exit)	
	{
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars (1);
		file_open = FALSE;

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		fmt_chosen = FALSE;
		heading (1);
		entry (1);
		if (prog_exit || restart)
		{
			if (file_open)
				fclose (fin);
			continue;
		}

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
		{
			if (file_open)
				fclose (fin);
			continue;
		}

		if (fmt_chosen)
			load_dflts ();

		load_fields ();

		heading (2);
		scn_display (2);
		edit (2);
		if (restart)
			continue;
	
		if (!chk_fields ()) 
		{
			/* No Fields Allocated */
			print_mess (ML (mlTmMess004));
			sleep (sleepTime);
			clear_mess ();
			continue;
		}

		process ();
		prog_exit = TRUE;
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
	void)
{
	abc_dbopen ("data");

	open_rec (tmpm, tmpm_list, TMPM_NO_FIELDS, (envDbFind) ? "tmpm_id_no3" 
							      : "tmpm_id_no");
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (tmdf, tmdf_list, TMDF_NO_FIELDS, "tmdf_id_no");
	open_rec (tmdl, tmdl_list, TMDL_NO_FIELDS, "tmdl_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
	void)
{
	abc_fclose (tmpm);
	abc_fclose (tmdf);
	abc_fclose (tmdl);
	abc_fclose (comr);
	abc_dbclose ("data");
}

/*-------------------------
| Load defaults from tmdl |
-------------------------*/
void
load_dflts (
	void)
{
	int		i;

	tmdl_rec.hhdf_hash = tmdf_rec.hhdf_hash;
	tmdl_rec.line_no = 0;
	cc = find_rec (tmdl, &tmdl_rec, GTEQ, "r");
	while (!cc && tmdl_rec.hhdf_hash == tmdf_rec.hhdf_hash)
	{
		clip (tmdl_rec.field_name);
		for (i = 0; i < TMPM_NO_FIELDS; i++)
		{
			if (!strcmp (tmdl_rec.field_name,tmpm_list [i].vwname))
			{
				if (DELIMIT)
				      lead_list [i].fld_no = tmdl_rec.offset;
				else
				{
				      lead_list [i].offset  = tmdl_rec.offset;
				      lead_list [i].fld_len = tmdl_rec.tmdl_length;
				}
				sprintf (lead_list [i].dflt, "%-40.40s", tmdl_rec.tmdl_default);
			}
		}
		
		cc = find_rec (tmdl, &tmdl_rec, NEXT, "r");
	}

	return;
}

/*----------------------------------------------
| Load lead_list structure into tabular screen |
----------------------------------------------*/
void
load_fields (
	void)
{
	int		i;

	scn_set (2);
	lcount [2] = 0;

	for (i = 0; i < TMPM_NO_FIELDS; i++)
	{
		if (lead_list [i].load_ok [0] != 'Y')
			continue;

		sprintf (local_rec.fld_name, "%-18.18s", lead_list [i].fld_name);
		local_rec.fld_no = lead_list [i].fld_no;
		local_rec.offset = lead_list [i].offset;
		local_rec.fld_len = lead_list [i].fld_len;
		sprintf (local_rec.fld_dflt, "%-40.40s", lead_list [i].dflt);
			
		putval (lcount [2]++);
	}

	return;
}

int
spec_valid (
	int field)
{
	int		i;

	if (LCHECK ("filename"))
	{
		if ((fin = fopen (clip (local_rec.filename), "r")) == 0)
		{
			/* Invalid File Name */
			print_mess (ML (mlStdMess143));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		file_open = TRUE;
	
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("delimit"))
	{
		if (FLD ("delimit") == ND)
			return (EXIT_SUCCESS);

		/*Delimiter :*/
		rv_pr (ML (mlTmMess068), 60, 0, 0);
		sprintf (err_str, "%-1.1s", local_rec.delimit);
		rv_pr (err_str, 72, 0, 1);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fmt_name"))
	{
		if (dflt_used)
		{
			fmt_chosen = FALSE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTmdf (temp_str);
			return (EXIT_SUCCESS);
		}
		fmt_chosen = FALSE;

		strcpy (tmdf_rec.co_no, comm_rec.co_no);
		sprintf (tmdf_rec.type, "%-1.1s", load_type);
		sprintf (tmdf_rec.fmt_name, "%-10.10s", local_rec.fmt_name);
		cc = find_rec (tmdf, &tmdf_rec, COMPARISON, "r");
		if (cc)
		{
			/* Format Does Not Exist On File */
			print_mess (ML (mlTmMess067));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.delimit, "%-1.1s", tmdf_rec.delimit);
		/*Delimiter :*/
		rv_pr (ML (mlTmMess068), 60, 0, 0);
		sprintf (err_str, "%-1.1s", local_rec.delimit);
		rv_pr (err_str, 72, 0, 1);

		fmt_chosen = TRUE;
		entry_exit = TRUE;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fld_no"))
	{
		if (!DELIMIT)
			return (EXIT_SUCCESS);

		for (i = 0; i < TMPM_NO_FIELDS; i++)
		{
			if (!strcmp (lead_list [i].fld_name, local_rec.fld_name))
			{
				lead_list [i].fld_no = local_rec.fld_no;
				if (local_rec.fld_no == 0)
					break;
				sprintf (lead_list [i].dflt, "%-40.40s", " ");
				sprintf (local_rec.fld_dflt, "%-40.40s", " ");
				DSP_FLD ("fld_dflt");
				break;
			}
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("offset") || LCHECK ("fld_len"))
	{
		if (DELIMIT)
			return (EXIT_SUCCESS);

		for (i = 0; i < TMPM_NO_FIELDS; i++)
		{
			if (!strcmp (lead_list [i].fld_name, local_rec.fld_name))
			{
				lead_list [i].offset = local_rec.offset;
				lead_list [i].fld_len = local_rec.fld_len;

				if (LCHECK ("offset") && local_rec.offset == 0)
					break;

				if (LCHECK ("fld_len") && local_rec.fld_len == 0)
					break;

				sprintf (lead_list [i].dflt, "%-40.40s", " ");
				sprintf (local_rec.fld_dflt, "%-40.40s", " ");
				DSP_FLD ("fld_dflt");
				break;
			}
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fld_dflt"))
	{
		for (i = 0; i < TMPM_NO_FIELDS; i++)
		{
			if (!strcmp (lead_list [i].fld_name, local_rec.fld_name))
			{
				sprintf (lead_list [i].dflt, "%-40.40s", local_rec.fld_dflt);

				if (strlen (clip (local_rec.fld_dflt)) == 0)
					break;

				if (DELIMIT)
				{
					lead_list [i].fld_no = 0;
					local_rec.fld_no = 0;
					DSP_FLD ("fld_no");
				}
				else
				{
					lead_list [i].offset = 0;
					local_rec.offset = 0;
					lead_list [i].fld_len = 0;
					local_rec.fld_len = 0;
					DSP_FLD ("offset");
					DSP_FLD ("fld_len");
				}
				break;
			}
		}

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}	

/*=========================================
| Search routine for Default Format File. |
=========================================*/
void
SrchTmdf (
	char *key_val)
{
	work_open ();

	strcpy (tmdf_rec.co_no, comm_rec.co_no);
	sprintf (tmdf_rec.type, "%-1.1s", load_type);
	sprintf (tmdf_rec.fmt_name, "%-10.10s", key_val);
	save_rec ("#Format Name.","#Format Description.");
	cc = find_rec (tmdf, &tmdf_rec, GTEQ, "r");
	while (!cc && !strcmp (tmdf_rec.co_no, comm_rec.co_no) &&
		!strcmp (tmdf_rec.type, load_type) &&
		strncmp (tmdf_rec.fmt_name, key_val, strlen (key_val)) >= 0)
	{
		cc = save_rec (tmdf_rec.fmt_name, tmdf_rec.fmt_desc);
		if (cc)
			break;

		cc = find_rec (tmdf, &tmdf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmdf_rec.co_no,comm_rec.co_no);
	sprintf (tmdf_rec.type, "%-1.1s", load_type);
	sprintf (tmdf_rec.fmt_name, "%-10.10s", temp_str);
	cc = find_rec (tmdf, &tmdf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tmdf, "DBFIND");
}

/*-----------------------------------------------------
| Check that at least one valid field has been chosen |
-----------------------------------------------------*/
int
chk_fields (
	void)
{
	int		flds_ok;
	int		i;

	flds_ok = FALSE;
	for (i = 0; i < TMPM_NO_FIELDS; i++)
	{
		if (lead_list [i].load_ok [0] != 'Y')
			continue;

		if (DELIMIT)
		{
			if (lead_list [i].fld_no != 0)
				flds_ok = TRUE;
		}
		else
		{
			if (lead_list [i].fld_len != 0)
				flds_ok = TRUE;
		}
	}

	return (flds_ok);
}

/*---------------
| Process File. |
---------------*/
void
process (
	void)
{
	char	*sptr;

	dsp_screen ("Creating Leads", comm_rec.co_no, comm_rec.co_name);

	sptr = fgets (line_buffer, BUFFER_SIZE, fin);
	while (sptr)
	{
		if (* (sptr + strlen (sptr) - 2) == '\r')
			* (sptr + strlen (sptr) - 2) = '\0';
		else
			* (sptr + strlen (sptr) - 1) = '\0';

		init_record ();
		dsp_process ("Lead", tmpm_rec.pro_no);

		curr_field = 0;
		if (DELIMIT)
			parse_delimit (sptr);
		else
			parse_fixed (sptr);

		cc = abc_add (tmpm, &tmpm_rec);
		if (cc)
			file_err (cc, tmpm, "DBADD");

		sptr = fgets (line_buffer, BUFFER_SIZE, fin);
	}

	fclose (fin);

	return;
}

/*-----------------------------------------------
| Initialise tmpm record ready to accept values |
-----------------------------------------------*/
void
init_record (
	void)
{
	char	next_pro_no [9];
	char	tmp_date [11];
	long	nxt_num;
	int		i;

	/*----------------------------
	| Get next lead no from comr |
	----------------------------*/
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, comr, "DBFIND");

	sprintf (next_pro_no, "%-6.6s", comr_rec.prospect_no);
	nxt_num = atol (next_pro_no + 1);
	nxt_num++;
	sprintf (comr_rec.prospect_no, "%-1.1s%05ld", next_pro_no, nxt_num);
	cc = abc_update (comr, &comr_rec);
	if (cc)
		file_err (cc, comr, "DBUPDATE");

	/*---------------------------
	| Initialise fields in tmpm |
	---------------------------*/
	strcpy (tmpm_rec.co_no, comm_rec.co_no);
	strcpy (tmpm_rec.br_no, (envDbCo) ? comm_rec.est_no : " 0");
	sprintf (tmpm_rec.pro_no, "%-8.8s", next_pro_no);
	tmpm_rec.hhcu_hash = 0L;

	/*---------------------------------------
	| Initialise all fields that can be set |
	| ie Any Y or N fields in lead_list     |
	---------------------------------------*/
	for (i = 0; i < TMPM_NO_FIELDS; i++)
	{
		if (lead_list [i].load_ok [0] == 'N')
			continue;

		switch (tmpm_list [i].vwtype)
		{
		case INTTYPE:
			* ((int *) (lead_list [i].dest_fld)) = atoi (lead_list [i].dflt);
			break;

		case DATETYPE:
		case EDATETYPE:
		case YDATETYPE:
			sprintf (tmp_date, "%-10.10s", lead_list [i].dflt);
			* ((long *) (lead_list [i].dest_fld)) = StringToDate (tmp_date);
			break;

		case LONGTYPE:
			* ((long *) (lead_list [i].dest_fld)) = atol (lead_list [i].dflt);
			break;

		case CHARTYPE:
			sprintf (lead_list [i].dest_fld, lead_list [i].dflt);
			break;

		default:
			break;
		}
	}

	/*----------------------------------------
	| Still have to initialise type N fields |
	---------------------------------------*/
	tmpm_rec.n_visit_date = 0;
	strcpy (tmpm_rec.n_visit_time, "00:00");
	sprintf (tmpm_rec.lst_op_code,  "%-14.14s", " ");
	tmpm_rec.call_bk_date = 0;
	strcpy (tmpm_rec.call_bk_time, "00:00");
	tmpm_rec.call_no = 0;
	tmpm_rec.lphone_date = 0;
	tmpm_rec.date_create = local_rec.lsystemDate;
	strcpy (tmpm_rec.delete_flag, " ");

	return;
}

/*--------------------------------------------
| Get fields from a line of a delimited file |
--------------------------------------------*/
void
parse_delimit (
	char *sptr)
{
	char	*tptr;
	char	sep_char;

	while (*sptr)
	{
		curr_field++;

		/*-----------------------------
		| Fields may be delimited by  |
		| a character and ALSO MAY be |
		| enclosed in ""              |
		-----------------------------*/
		sep_char = local_rec.delimit [0];
		if (*sptr == '"')
		{
			sptr++;
			sep_char = '"';
		}

		/*-------------------
		| Find end of field |
		-------------------*/
		tptr = sptr;
		while (*tptr && *tptr != sep_char)
			tptr++;

		/*---------------------------------------
		| Extract data and copy into dest field |
		---------------------------------------*/
		extract_data (sptr, tptr);

		/*------------------------------
		| Find next delimiter if field |
		| was enclosed in ""           |
		------------------------------*/
		if (sep_char == '"')
		{
			sep_char = local_rec.delimit [0];
			while (*tptr && *tptr != sep_char)
				tptr++;
		}

		/*---------------------------------
		| Set sptr to start of next field |
		---------------------------------*/
		if (*tptr)
			sptr = tptr + 1;
		else
			sptr = tptr;

	}

	return;
}

/*---------------------------------------
| Extract data from line given pointers |
| to start and end of field             |
---------------------------------------*/
void
extract_data (
	char *sptr, 
	char *tptr)
{
	int		i;
	int		tmp_len;
	int		fld_len;
	char	tmp_fld [100];

	tmp_len = (tptr - sptr);
	sprintf (tmp_fld, "%-*.*s", tmp_len, tmp_len, sptr);

	for (i = 0; i < TMPM_NO_FIELDS; i++)
	{
		if (lead_list [i].load_ok [0] != 'Y')
			continue;

		if (lead_list [i].fld_no == curr_field)
		{
			fld_len = lead_list [i].max_len;
			sprintf (lead_list [i].dest_fld, "%-*.*s", fld_len, fld_len, tmp_fld);
		}
	}

	return;
}

/*-----------------------------------------------
| Get fields from a line of a fixed format file |
-----------------------------------------------*/
void
parse_fixed (
	char *sptr)
{
	int		i;

	for (i = 0; i < TMPM_NO_FIELDS; i++)
	{
		if (lead_list [i].fld_len == 0 || lead_list [i].load_ok [0] != 'Y')
			continue;

		sprintf (lead_list [i].dest_fld, "%-*.*s", lead_list [i].max_len,
												 lead_list [i].fld_len,
												 (sptr + lead_list [i].offset));
	}

	return;
}

int
heading (
	int scn)
{
	if (restart)
	{
		abc_unlock (tmpm);
		return (EXIT_FAILURE);
	}

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	rv_pr (ML (mlTmMess069),25,0,1);
	move (0,1);
	line (80);

	if (scn == 1)
	{
		if (DELIMIT)
			box (0, 3, 80, 3);
		else
			box (0, 3, 80, 2);
	}
	else
	{
		if (DELIMIT)
		{
			/*Delimiter :*/
			rv_pr (ML (mlTmMess068), 60, 0, 0);
			sprintf (err_str, "%-1.1s", local_rec.delimit);
			rv_pr (err_str, 72, 0, 1);
		}
	}

	move (1,input_row);

	move (0,20);
	line (80);
	print_at (21,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}
