/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: dflt_fmt.c,v 5.3 2001/11/14 05:05:38 scott Exp $
|  Program Name  : (tm_dflt_fmt.c)
|  Program Desc  : (Set up default format for tm_ld_load) 
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 07/11/91         |
|---------------------------------------------------------------------|
| $Log: dflt_fmt.c,v $
| Revision 5.3  2001/11/14 05:05:38  scott
| Updated to convert to app.schema
| Updated to clean code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: dflt_fmt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_dflt_fmt/dflt_fmt.c,v 5.3 2001/11/14 05:05:38 scott Exp $";

#define DELIMIT		 (local_rec.formatType [0] == 'D')

#define	MAXLINES	38
#define	TABLINES	13
#include <ml_std_mess.h>
#include <ml_tm_mess.h>
#include <pslscr.h>
#include <getnum.h>
#include <dsp_screen.h>
#include <dsp_process2.h>

#define	DELIMITED	2
#define	FIXED		3

	/*
	 * Special fields and flags 
	 */
   	int  	envDbCo = 0,
			wk_no,
			envDbFind = 0;

	char	branchNo [3];

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct tmdfRecord	tmdf_rec;
struct tmdlRecord	tmdl_rec;

/*
 * List Of Possible Fields To Load                            
 * Fields with a load_ok field set to D can only have a default
 * value and not a position to load from in the ascii file.   
 */
struct	{
	char	fld_name [19];
	char	load_ok [2];	/* Y(es) D (efault OK) */
	int		offset;
	int		fld_len;
	char	dflt [41];
	char	tmpm_fld [19];
} lead_list [] = {
	{ "Prospect Name     ", "Y", 0, 0, " ", "tmpm_name"         },
	{ "Acronym           ", "Y", 0, 0, " ", "tmpm_acronym"      },
	{ "Area Code         ", "Y", 0, 0, " ", "tmpm_area_code"    },
	{ "Salesman Code     ", "Y", 0, 0, " ", "tmpm_sman_code"    },
	{ "Business Sector   ", "Y", 0, 0, " ", "tmpm_b_sector"     },
	{ "Post Code         ", "Y", 0, 0, " ", "tmpm_post_code"    },
	{ "Active Flag       ", "D", 0, 0, " ", "tmpm_active_flg"   },
	{ "Contact Name 1    ", "Y", 0, 0, " ", "tmpm_cont_name1"   },
	{ "Contact Name 2    ", "Y", 0, 0, " ", "tmpm_cont_name2"   },
	{ "Contact Name 3    ", "Y", 0, 0, " ", "tmpm_cont_name3"   },
	{ "Contact Name 4    ", "Y", 0, 0, " ", "tmpm_cont_name4"   },
	{ "Contact Name 5    ", "Y", 0, 0, " ", "tmpm_cont_name5"   },
	{ "Contact Name 6    ", "Y", 0, 0, " ", "tmpm_cont_name6"   },
	{ "Contact Code 1    ", "Y", 0, 0, " ", "tmpm_cont_code1"   },
	{ "Contact Code 2    ", "Y", 0, 0, " ", "tmpm_cont_code2"   },
	{ "Contact Code 3    ", "Y", 0, 0, " ", "tmpm_cont_code3"   },
	{ "Contact Code 4    ", "Y", 0, 0, " ", "tmpm_cont_code4"   },
	{ "Contact Code 5    ", "Y", 0, 0, " ", "tmpm_cont_code5"   },
	{ "Contact Code 6    ", "Y", 0, 0, " ", "tmpm_cont_code6"   },
	{ "Mail Address 1    ", "Y", 0, 0, " ", "tmpm_mail1_adr"    },
	{ "Mail Address 2    ", "Y", 0, 0, " ", "tmpm_mail2_adr"    },
	{ "Mail Address 3    ", "Y", 0, 0, " ", "tmpm_mail3_adr"    },
	{ "Delivery Address1 ", "Y", 0, 0, " ", "tmpm_del1_adr"     },
	{ "Delivery Address2 ", "Y", 0, 0, " ", "tmpm_del2_adr"     },
	{ "Delivery Address3 ", "Y", 0, 0, " ", "tmpm_del3_adr"     },
	{ "Fax Number        ", "Y", 0, 0, " ", "tmpm_fax_no"       },
	{ "Phone Number      ", "Y", 0, 0, " ", "tmpm_phone_no"     },
	{ "Phone Freq        ", "D", 0, 0, " ", "tmpm_phone_freq"   },
	{ "Phone Date        ", "D", 0, 0, " ", "tmpm_n_phone_date" },
	{ "Phone Time        ", "D", 0, 0, " ", "tmpm_n_phone_time" },
	{ "Visit Freq        ", "D", 0, 0, " ", "tmpm_visit_freq"   },
	{ "Mail Flag         ", "Y", 0, 0, " ", "tmpm_mail_flag"    },
	{ "Op Code           ", "D", 0, 0, " ", "tmpm_op_code"      },
	{ "Origin            ", "Y", 0, 0, " ", "tmpm_origin"       },
	{ "Best Phone Time   ", "Y", 0, 0, " ", "tmpm_best_ph_time" },
	{ "Tax Code          ", "Y", 0, 0, " ", "tmpm_tax_code"     },
	{ "Tax Number        ", "Y", 0, 0, " ", "tmpm_tax_no"       },
	{ "Status Flag       ", "Y", 0, 0, " ", "tmpm_stat_flag"    },
	{ "",                   "N", 0, 0, "",  "" }
};

int		no_fields = MAXLINES;
int		new_format;
int		fmt_screen;
int		tab_only;
extern	int		TruePosition;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char	systemDate [11];
	long	lsystemDate;
	char	fld_name [19];
	int		offset;
	int		fld_len;
	char	fld_dflt [41];
	char	formatType [2];
	char	formatTypeDesc [13];
	char	delimit [2];
	char	fmt_name [11];
	char	fmt_desc [41];
} local_rec;

static	struct	var vars [] =
{
	{1, LIN, "formatType", 4, 2, CHARTYPE,
		"U", "          ",
		" ", "D", "Fixed / Delimited  ", "",
		 NE, NO,  JUSTLEFT, "", "", local_rec.formatType},
	{1, LIN, "formatTypeDesc", 4, 26, CHARTYPE,
		"AAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.formatTypeDesc},

	{1, LIN, "fmt_name", 5, 2, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "",  "Format Name        ", "",
		 NE, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_", "", local_rec.fmt_name},
	{1, LIN, "fmt_desc", 6, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "Description        ", "",
		 YES, NO,  JUSTLEFT, "", "", local_rec.fmt_desc},
	{1, LIN, "delimit", 7, 2, CHARTYPE,
		"A", "          ",
		" ", "",  "Delimiter          ", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.delimit},

	{2, TAB, "d_fld",	 MAXLINES, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Field Name    ", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.fld_name},
	{2, TAB, "d_offset", 0, 0, INTTYPE,
		"NNNNN", "          ",
		" ", "", "Field No", "",
		 NO, NO,  JUSTRIGHT, "0", "99999", (char *)&local_rec.offset},
	{2, TAB, "d_fld_dflt", 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Default                  ", "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.fld_dflt},

	{3, TAB, "f_fld",	 MAXLINES, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Field Name    ", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.fld_name},
	{3, TAB, "f_offset", 0, 0, INTTYPE,
		"NNNNN", "          ",
		" ", "", "Offset", "",
		 NO, NO,  JUSTRIGHT, "0", "99999", (char *)&local_rec.offset},
	{3, TAB, "fld_len", 0, 0, INTTYPE,
		"NNN", "          ",
		" ", "", "Length", "",
		 NO, NO,  JUSTRIGHT, "0", "999", (char *)&local_rec.fld_len},
	{3, TAB, "f_fld_dflt", 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Default                  ", "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.fld_dflt},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

FILE	*fin;

void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	InitList 		(void);
void 	LoadDefaults 	(void);
void 	LoadFields 		(void);
void 	Update 			(void);
int 	spec_valid 		(int);
void 	tab_other 		(int);
void 	SrchTmdf 		(char *);
int 	CheckFields 	(void);
int 	heading 		(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int argc,
	char *argv [])
{
	TruePosition	=	TRUE;

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind = atoi (get_env ("DB_FIND"));

	OpenDB ();
	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();

	tab_row = 4;
	tab_col = 2;

	tab_only = FALSE;
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

		InitList ();

		if (!tab_only)
		{
			/*-------------------------------
			| Enter screen 1 linear input . |
			-------------------------------*/
			init_vars (1);
			heading (1);
			entry (1);
			if (prog_exit || restart)
				continue;
	
			heading (1);
			scn_display (1);
			edit (1);
			if (restart)
				continue;

			init_vars (DELIMITED);
			init_vars (FIXED);

			if (!new_format)
				LoadDefaults ();

			if (DELIMIT)
				fmt_screen = DELIMITED;
			else
				fmt_screen = FIXED;
		}

		LoadFields ();

		heading (fmt_screen);
		scn_display (fmt_screen);
		edit (fmt_screen);
		if (restart)
		{
			tab_only = FALSE;
			continue;
		}
	
		if (!CheckFields ()) 
		{
			/*print_mess ("\007 No Fields Allocated ");*/
			print_mess (ML (mlTmMess004));
			sleep (sleepTime);
			clear_mess ();
			tab_only = TRUE;
			continue;
		}

		Update ();
		tab_only = FALSE;
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
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
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
	abc_fclose (tmdf);
	abc_fclose (tmdl);
	abc_fclose (comr);
	abc_dbclose ("data");
}

/*----------------------
| Initialise lead_list |
----------------------*/
void
InitList (void)
{
	int		i;

	for (i = 0; i < no_fields; i++)
	{
		lead_list [i].offset = 0;
		lead_list [i].fld_len = 0;
		sprintf (lead_list [i].dflt, "%-40.40s", " ");
	}

	return;
}

/*-------------------------
| Load defaults from tmdl |
-------------------------*/
void
LoadDefaults (
	void)
{
	int		i;

	tmdl_rec.hhdf_hash = tmdf_rec.hhdf_hash;
	tmdl_rec.line_no = 0;
	cc = find_rec (tmdl, &tmdl_rec, GTEQ, "r");
	while (!cc && tmdl_rec.hhdf_hash == tmdf_rec.hhdf_hash)
	{
		clip (tmdl_rec.field_name);
		for (i = 0; i < no_fields; i++)
		{
			if (!strcmp (tmdl_rec.field_name,lead_list [i].tmpm_fld))
			{
		  	   	sprintf (lead_list [i].dflt, "%-40.40s", tmdl_rec.tmdl_default);
				
			   	if (lead_list [i].load_ok [0] == 'D')
				  	break;
			
				lead_list [i].offset = tmdl_rec.offset;
				if (!DELIMIT)
					lead_list [i].fld_len = tmdl_rec.tmdl_length;

			   break;
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
LoadFields (
	void)
{
	int		i;

	scn_set (fmt_screen);
	lcount [ fmt_screen ] = 0;

	for (i = 0; i < no_fields; i++)
	{
		sprintf (local_rec.fld_name, "%-18.18s", lead_list [i].fld_name);
		local_rec.offset = lead_list [i].offset;
		local_rec.fld_len = lead_list [i].fld_len;
		sprintf (local_rec.fld_dflt, "%-40.40s", lead_list [i].dflt);
			
		putval (lcount [ fmt_screen ]++);
	}

	return;
}

/*----------------------
| Update tmdf and tmdl |
----------------------*/
void
Update (	
	void)
{
	int		i;

	if (new_format)
	{
		strcpy (tmdf_rec.co_no, comm_rec.co_no);
		strcpy (tmdf_rec.type, local_rec.formatType);
		sprintf (tmdf_rec.fmt_name, "%-10.10s", local_rec.fmt_name);
		sprintf (tmdf_rec.fmt_desc, "%-40.40s", local_rec.fmt_desc);
		sprintf (tmdf_rec.delimit, "%-1.1s", local_rec.delimit);
		cc = abc_add (tmdf, &tmdf_rec);
		if (cc)
			file_err (cc, tmdf, "DBADD");

		strcpy (tmdf_rec.co_no, comm_rec.co_no);
		strcpy (tmdf_rec.type, local_rec.formatType);
		sprintf (tmdf_rec.fmt_name, "%-10.10s", local_rec.fmt_name);
		cc = find_rec (tmdf, &tmdf_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, tmdf, "DBFIND");
	}
	else
	{
		sprintf (tmdf_rec.fmt_desc, "%-40.40s", local_rec.fmt_desc);
		sprintf (tmdf_rec.delimit, "%-1.1s", local_rec.delimit);
		cc = abc_update (tmdf, &tmdf_rec);
		if (cc)
			file_err (cc, tmdf, "DBUPDATE");

		tmdl_rec.hhdf_hash = tmdf_rec.hhdf_hash;
		tmdl_rec.line_no = 0;
		cc = find_rec (tmdl, &tmdl_rec, GTEQ, "r");
		while (!cc && tmdl_rec.hhdf_hash == tmdf_rec.hhdf_hash)
		{
			cc = abc_delete (tmdl);
			if (cc)
				file_err (cc, tmdl, "DBDELETE");

			tmdl_rec.hhdf_hash = tmdf_rec.hhdf_hash;
			tmdl_rec.line_no = 0;
			cc = find_rec (tmdl, &tmdl_rec, GTEQ, "r");
		}
	}

	scn_set (fmt_screen);
	for (i = 0; i < no_fields; i++)
	{
		tmdl_rec.hhdf_hash = tmdf_rec.hhdf_hash;
		tmdl_rec.line_no = i;
		sprintf (tmdl_rec.field_name,"%-18.18s",lead_list [i].tmpm_fld);
		tmdl_rec.offset = lead_list [i].offset;
		tmdl_rec.tmdl_length = 0;
		if (!DELIMIT)
			tmdl_rec.tmdl_length = lead_list [i].fld_len;

		sprintf (tmdl_rec.tmdl_default,"%-40.40s",lead_list [i].dflt);

		cc = abc_add (tmdl, &tmdl_rec);
		if (cc)
			file_err (cc, tmdl, "DBADD");
	}

	return;
}

int
spec_valid (
	int field)
{
	int		i;

	if (LCHECK ("formatType"))
	{
		if (DELIMIT)
		{
			strcpy (local_rec.formatTypeDesc, "Delimited   ");
			FLD ("delimit") = YES;
			DSP_FLD ("formatTypeDesc");
			display_prmpt (label ("delimit"));
		}
		else
		{
			strcpy (local_rec.formatTypeDesc, "Fixed Format");
			FLD ("delimit") = ND;
			DSP_FLD ("formatTypeDesc");
			rv_pr ("                                  ", 1, 7, 0);
		}

		DSP_FLD ("formatType");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fmt_name"))
	{
		new_format = FALSE;
		if (SRCH_KEY)
		{
			SrchTmdf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmdf_rec.co_no, comm_rec.co_no);
		strcpy (tmdf_rec.type, local_rec.formatType);
		sprintf (tmdf_rec.fmt_name, "%-10.10s", local_rec.fmt_name);
		cc = find_rec (tmdf, &tmdf_rec, COMPARISON, "r");
		if (cc)
			new_format = TRUE;
		else
		{
			sprintf (local_rec.fmt_desc, "%-40.40s", tmdf_rec.fmt_desc);
			sprintf (local_rec.delimit,"%-1.1s",tmdf_rec.delimit);
			entry_exit = TRUE;
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("d_offset") || LCHECK ("f_offset"))
	{
		for (i = 0; i < no_fields; i++)
		{
			if (!strcmp (lead_list [i].fld_name, local_rec.fld_name))
			{
				lead_list [i].offset = local_rec.offset;

				if (local_rec.offset == 0)
					break;

				sprintf (lead_list [i].dflt, "%-40.40s", " ");
				sprintf (local_rec.fld_dflt, "%-40.40s", " ");
				if (DELIMIT)
					DSP_FLD ("d_fld_dflt");
				else
					DSP_FLD ("f_fld_dflt");
				break;
			}
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fld_len"))
	{
		for (i = 0; i < no_fields; i++)
		{
			if (!strcmp (lead_list [i].fld_name, local_rec.fld_name))
			{
				lead_list [i].fld_len = local_rec.fld_len;

				if (local_rec.fld_len == 0)
					break;

				sprintf (lead_list [i].dflt, "%-40.40s", " ");
				sprintf (local_rec.fld_dflt, "%-40.40s", " ");
				DSP_FLD ("f_fld_dflt");
				break;
			}
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("d_fld_dflt") || LCHECK ("f_fld_dflt"))
	{
		for (i = 0; i < no_fields; i++)
		{
			if (!strcmp (lead_list [i].fld_name, local_rec.fld_name))
			{
				sprintf (lead_list [i].dflt, "%-40.40s", local_rec.fld_dflt);

				if (strlen (clip (local_rec.fld_dflt)) == 0)
					break;

				if (DELIMIT)
				{
					lead_list [i].offset = 0;
					local_rec.offset = 0;
					DSP_FLD ("d_offset");
				}
				else
				{
					lead_list [i].offset = 0;
					local_rec.offset = 0;
					lead_list [i].fld_len = 0;
					local_rec.fld_len = 0;
					DSP_FLD ("f_offset");
					DSP_FLD ("fld_len");
				}
				break;
			}
		}

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}	

void
tab_other (
	int line_no)
{
	if (DELIMIT)
	{
		move (0,22);
		cl_line ();
	
		FLD ("d_offset") = NO;
		if (lead_list [line_no].load_ok [0] == 'D')
		{
			/*rv_pr (" DEFAULT ONLY FIELD ", 30, 22, 1);*/
			rv_pr (ML (mlTmMess005), 30, 22, 1);
			FLD ("d_offset") = NA;
		}
	}
	else
	{
		move (0,22);
		cl_line ();
	
		FLD ("f_offset") = NO;
		FLD ("fld_len") = NO;
		if (lead_list [line_no].load_ok [0] == 'D')
		{
			/*rv_pr (" DEFAULT ONLY FIELD ", 30, 22, 1);*/
			rv_pr (ML (mlTmMess005), 30, 22, 1);
			FLD ("f_offset") = NA;
			FLD ("fld_len") = NA;
		}
	}
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
	strcpy (tmdf_rec.type, local_rec.formatType);
	sprintf (tmdf_rec.fmt_name, "%-10.10s", key_val);
	save_rec ("#Format Name.","#Format Description.");
	cc = find_rec (tmdf, &tmdf_rec, GTEQ, "r");
	while (!cc && !strcmp (tmdf_rec.co_no, comm_rec.co_no) &&
		          tmdf_rec.type [0] == local_rec.formatType [0] &&
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
	strcpy (tmdf_rec.type, local_rec.formatType);
	sprintf (tmdf_rec.fmt_name, "%-10.10s", temp_str);
	cc = find_rec (tmdf, &tmdf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tmdf, "DBFIND");
}

/*-----------------------------------------------------
| Check that at least one valid field has been chosen |
-----------------------------------------------------*/
int
CheckFields (
	void)
{
	int		flds_ok;
	int		i;

	flds_ok = FALSE;
	for (i = 0; i < no_fields; i++)
	{
		if (lead_list [i].load_ok [0] != 'Y')
			continue;

		if (DELIMIT)
		{
			if (lead_list [i].offset != 0)
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

int
heading (
	int scn)
{
	if (restart)
		return (EXIT_FAILURE);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	rv_pr (ML (mlTmMess006), 18, 0, 1);
	move (0,1);
	line (80);

	if (scn == 1)
		box (0, 3, 80, 4);

	move (0,20);
	line (80);

	print_at (21,0,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}

