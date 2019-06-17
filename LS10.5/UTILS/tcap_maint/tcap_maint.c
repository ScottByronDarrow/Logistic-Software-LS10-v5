/*=====================================================================
|  Copyright (C) 1988 - 1992 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( tcap_maint.c   )                                 |
|  Program Desc  : ( Maintenance program for terminal entries.      ) |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 08/01/91         |
|---------------------------------------------------------------------|
|  Date Modified : (11/02/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (28/06/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (15/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (12/09/97)      | Modified  by : Roanna Marcelino. |
|                                                                     |
|  Comments      : (11/02/92) User can now update even if all reqd.   |
|                : fields hve not been entered. User is asked to      |
|                : confirm update.                                    |
|  (28/06/91)    : Cursor now turned on for updating of escape        |
|                : sequences.                                         |
|  (15/08/92)    : Changes for HP port. S/C INF 7619                  |
|  (12/09/97)    : Modified for Mutilingual Conversion.               |
|                                                                     |
=====================================================================*/
#define CCMAIN
#include <pslscr.h>
#include <hot_keys.h>
#include <getnum.h>
#include <ml_std_mess.h>
#include <ml_utils_mess.h>

char	*PNAME = "$RCSfile: tcap_maint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/tcap_maint/tcap_maint.c,v 5.2 2001/08/09 09:27:45 scott Exp $";

char	*sptr;
char	*tptr;
char	reqd_desc[41];
char	reqd_atrb[4];

char	*val_1_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890+-_.";
char	*val_2_chars = "^ abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890+-*/%=#$&@'`[]{}_()?<>;,.\\!~\"";


FILE	*descf;
FILE	*tcapf;
FILE	*tcapp;

int	new_term;
int	no_in_desc_tab = 0;

extern	char	*atrb[];

struct ATRB_REC {
	char	t_atrb[4];
	char	t_atrb_desc[41];
	char	t_esc_seq[51];
	char	t_reqd[4];
} t_tab_line[MAX_TA], tmp_line;

struct	{
	int	_val;	/* value representing key			*/
	char	*_key;	/* pointer to entry from termcap		*/
} function_keys[] = {
	{FN1,		"^A@^M"},
	{FN2,		"^AA^M"},
	{FN3,		"^AB^M"},
	{FN4,		"^AC^M"},
	{FN5,		"^AD^M"},
	{FN6,		"^AE^M"},
	{FN7,		"^AF^M"},
	{FN8,		"^AG^M"},
	{FN9,		"^AH^M"},
	{FN10,		"^AI^M"},
	{FN11,		"^AJ^M"},
	{FN12,		"^AK^M"},
	{FN13,		"^AL^M"},
	{FN14,		"^AM^M"},
	{FN15,		"^AN^M"},
	{FN16,		"^AO^M"},
	{FN17,		"^A`^M"},
	{FN18,		"^Aa^M"},
	{FN19,		"^Ab^M"},
	{FN20,		"^Ac^M"},
	{FN21,		"^Ad^M"},
	{FN22,		"^Ae^M"},
	{FN23,		"^Af^M"},
	{FN24,		"^Ag^M"},
	{FN25,		"^Ah^M"},
	{FN26,		"^Ai^M"},
	{FN27,		"^Aj^M"},
	{FN28,		"^Ak^M"},
	{FN29,		"^Al^M"},
	{FN30,		"^Am^M"},
	{FN31,		"^An^M"},
	{FN32,		"^Ao^M"},
	{0}
};

struct	{
	char	dummy[11];
	char	term_name[15];
	char	atrb_desc[41];
	char	esc_seq[51];
} local_rec;

char	directory[100];
char	tdesc_dir[100];
char	filename[100];

static	int	quit_func(int iUnused, KEY_TAB *psUnused);
static	int	update_func(int iUnused, KEY_TAB *psUnused);
static	int	exit_update_func(int iUnused, KEY_TAB *psUnused);
	
static	KEY_TAB edit_keys [] =
{
   { "[Q]UIT",		'Q', quit_func,
	"Exit without updating the termcap.",				"A" },
   { "[U]PDATE",	'U', update_func,
	"Update the line at the current cursor position.",		"E" },
   { NULL,		FN16, exit_update_func,
	"Exit and update the termcap.",				"A" },
   END_KEYS
};
	     
static	struct	var	vars[]	={	

	{1, LIN, "term_name", 2, 20, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "", "Terminal Name:", "Enter name of terminal you wish to maintain", 
		NE, NO, JUSTLEFT, "", "", local_rec.term_name}, 
	{2, LIN, "esc_seq", 21, 60, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NO, NO, JUSTLEFT, "", "", local_rec.dummy}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};
/*=======================
| Function Declarations |
=======================*/
int  spec_valid (int field);
void srch_term (char *key_val);
void process (void);
void create_table (void);
void erase_btm_scn (void);
int  chck_reqd (void);
void load_term_desc (void);
void add_desc (int position);
void init_t_table (void);
void load_term (void);
int  load_line (char *line);
int  heading (int scn);
void get_esc_seq (int x, int y, char *mask, char *buf);

/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc,
 char * argv [])
{
	char	*sptr = getenv("PROG_PATH");

	sprintf(tdesc_dir,"%s/BIN/SCN/term_desc",(sptr != (char *)0) ? sptr : "/usr/LS10.5");
	sprintf(directory,"%s/BIN/MENUSYS",(sptr != (char *)0) ? sptr : "/usr/LS10.5");

	SETUP_SCR(vars);

	init_scr();			/*  sets terminal from termcap	*/
	set_tty();
	set_masks();			/*  setup print using masks	*/
	init_vars(1);			/*  set default values		*/
	swide();

	/*---------------------------
	| WARNING!!! The next line  |
	| alters the standard range |
	| of valid chars within the |
	| screen generator!!!!!!!!! |
	---------------------------*/
	alpha = val_1_chars;

	/*--------------------------------
	| Load term_desc file into array |
	--------------------------------*/
	init_t_table();
	set_help(FN6, "FN6");

	while (prog_exit == 0)
	{
		/*=====================
		| Reset control flags |
		=====================*/
		search_ok = 1;
		entry_exit = 1;
		prog_exit = 0;
		restart = 0;

		init_vars(1);	

		heading(1);
		entry(1);

		if (prog_exit || restart)
			continue;

		if (!restart) 
			process();
	}
	FinishProgram ();
	return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	if (LCHECK("term_name")) 
        {
		if (last_char == SEARCH)
		{	
                        srch_term(temp_str);
			return(0);
		}
	
		sptr = clip(local_rec.term_name);
		while (*sptr)
		{
			if (*sptr == ' ')
				return(1);
			sptr++;
		}
	
		sprintf(filename, "%s/TERM/%s", directory, local_rec.term_name);
		load_term();
		return(0);
	}

	return(0);
}

/*===================================
| Search for existing termcap files |
===================================*/
void
srch_term (
 char *key_val)
{
	char	tmpname[15];
	char	*lclptr;

	work_open();
	save_rec("#Terminal Name ","#");

	sprintf(filename,"ls %s/TERM/%s* 2> /dev/null",directory,key_val);

	if ((tcapp = popen(filename,"r")) == 0)
	{
		sprintf(err_str,"Error in %s during (POPEN)",filename);
		sys_err(err_str,errno,PNAME);
	}
		
	sptr = fgets(err_str,80,tcapp);

	while (sptr)
	{
		*(sptr + strlen(sptr) - 1) = '\0';
		lclptr = strrchr(sptr, '/');
		if (lclptr)
			lclptr++;

		sprintf(tmpname,"%-14.14s",lclptr);

		cc = save_rec(tmpname," ");
		if (cc)
			break;

		sptr = fgets(err_str,80,tcapp);
	}
	pclose(tcapp);
	
	cc = disp_srch();
	sprintf(local_rec.term_name,"%-14.14s",temp_str);
	work_close();
}

void
process (
 void)
{
	create_table();

	if (new_term)
		/* NEW TERMCAP*/
		rv_pr(ML(mlUtilsMess115),60,2,1);

	/*----------------------
	| Display Instructions |
	----------------------*/
	box (4,15,125,3);
	/*INSTRUCTIONS*/
	rv_pr(ML(mlUtilsMess116),60,15,0);

	/* For ESCAPE please enter \134E*/
	rv_pr(ML(mlUtilsMess117),5,16,0);

	/* For a CONTROL character please enter a carat(^) followed by the desired character  eg. CTRL A should be entered as ^A.*/
	rv_pr(ML(mlUtilsMess118),5,17,0);

	/* For dW# please enter TRUE if there is to be a delay for 80/132 change or FALSE if there isn't.*/
	rv_pr(ML(mlUtilsMess119),5,18,0);

	tab_scan("tcap");
	tab_close("tcap",TRUE);
}

/*---------------------------------------------
| Create table of escape sequences from array |
---------------------------------------------*/
void
create_table (
 void)
{
	int	i;

	tab_open("tcap",edit_keys,3,10,7,FALSE);
	tab_add("tcap","#%-5.5s  %-40.40s  %-50.50s   %-8.8s ","Atrb.","Attribute Description","Escape Sequence","Required");

	/*-----------------------
	| Load table from array |
	-----------------------*/
	for (i = 0; i < MAX_TA; i++)
	tab_add("tcap",
		" %-3.3s   %-40.40s  %-50.50s      %-3.3s ",
		t_tab_line[i].t_atrb,
		t_tab_line[i].t_atrb_desc,
		t_tab_line[i].t_esc_seq,
		t_tab_line[i].t_reqd);

	return;
}

/*-----------------------
| Quit without updating |
-----------------------*/
static	int
quit_func (
 int iUnused, 
 KEY_TAB *psUnused)
{
	return(FN16);
}

/*-----------------------
| Quit without updating |
-----------------------*/
static	int
update_func (
 int iUnused, 
 KEY_TAB *psUnused)
{
	int	pos_atrb = 0;
	char	dummy[133];
	char	tmp_mask[51];

	tab_get("tcap", dummy, CURRENT, 0);
	pos_atrb = tab_tline("tcap");

	memcpy ((char *) &tmp_line, (char *) &t_tab_line[pos_atrb], sizeof (struct ATRB_REC));   

	strcpy(local_rec.atrb_desc, tmp_line.t_atrb_desc);
	strcpy(local_rec.esc_seq, tmp_line.t_esc_seq);

	if (!strncmp (tmp_line.t_atrb, "dW#", 3))
		strcpy(tmp_mask, "UUUUU");
	else
		strcpy(tmp_mask, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");

	heading(2);
	init_ok = FALSE;
	last_char = REDRAW;


	while (TRUE)
	{
		strcpy(temp_str, local_rec.esc_seq);
		get_esc_seq(70,21,tmp_mask,temp_str);
		crsr_off();
		if (!strncmp (tmp_line.t_atrb, "dW#", 3))
		{
			if (!strncmp(temp_str, "TRUE", 4) || !strncmp(temp_str, "FALSE", 5))
				break;
		}
		else
			break;
	}
	strcpy(local_rec.esc_seq, temp_str);

	init_ok = TRUE;

	strcpy(tmp_line.t_esc_seq, local_rec.esc_seq);

	memcpy ((char *) &t_tab_line[pos_atrb], (char *) &tmp_line, sizeof (struct ATRB_REC));   

	tab_update("tcap",
		" %-3.3s   %-40.40s  %-50.50s      %-3.3s ",
		   tmp_line.t_atrb, 
		   tmp_line.t_atrb_desc, 
		   tmp_line.t_esc_seq, 
		   tmp_line.t_reqd);


	erase_btm_scn();
	
	return (EXIT_FAILURE);
}

void
erase_btm_scn (
 void)
{
	int	i;

	for (i = 20; i < 23; i++)
	{
		move(0,i);
		cl_line();
	}
}

/*-----------------------------
| Exit and update the termcap |
-----------------------------*/
static	int
exit_update_func (
 int iUnused, 
 KEY_TAB *psUnused)
{
	int	i;
	int	reqd_ok;

	reqd_ok = chck_reqd();
	if (!reqd_ok)
	{
		move(0,20);
		cl_line();
		/* All required escape sequences have not been entered. Please confirm update.*/
		sprintf(err_str,ML(mlUtilsMess120));
		rv_pr(err_str, 28,20,1);
		sleep(1);
	}

	move(0,21);
	cl_line();
	sprintf(filename, "%s/TERM/%s", directory, local_rec.term_name);
	/*  Write to %s ?  ", filename*/
	sprintf(err_str,ML(mlUtilsMess019), filename);
	/*Please enter Y to update termcap or N to continue editing.*/
	print_mess(ML(mlUtilsMess114));
	crsr_on();
	i = prmptmsg(err_str,"YyNn",43,21);

	if (i == 'N' || i == 'n')
	{
		move(0,20);
		cl_line();
		move(0,21);
		cl_line();
		clear_mess();
		return(0);
	}

	sprintf(filename, "%s/TERM/%s", directory, clip(local_rec.term_name));
	if ((tcapf = fopen(filename,"w")) == 0)
	{
		sprintf(err_str,"Error in %s during (FOPEN)",filename);
		sys_err(err_str,errno,PNAME);
	}

	move(0,20);
	cl_line();
	move(0,21);
	cl_line();
	clear_mess();
	/* UPDATING %s*/
	sprintf(err_str,ML(mlUtilsMess121),filename);
	rv_pr(err_str,40,21,1);

	fprintf(tcapf,"%s:\134\n",local_rec.term_name);
	for (i = 0; i < MAX_TA; i++)
	{
		if (!strncmp (t_tab_line[i].t_atrb, "dW#", 3))
		{
			if (!strncmp (t_tab_line[i].t_esc_seq, "TRUE", 4))
				fprintf(tcapf, "\t:%s:\134\n", t_tab_line[i].t_atrb);
		}
		else
			fprintf(tcapf, "\t:%s%s:%s\n", t_tab_line[i].t_atrb, t_tab_line[i].t_esc_seq, (i == (MAX_TA - 1)) ? " " : "\134");

	}

	fclose(tcapf);

	return(FN16);
}

/*----------------------------------------------------------------
| Check if all required attributes have escape sequences entered |
----------------------------------------------------------------*/
int
chck_reqd (
 void)
{
	int	i;

	for (i = 0; i < MAX_TA; i++)
	{
		if (!strcmp(t_tab_line[i].t_reqd, "YES") && strlen(t_tab_line[i].t_esc_seq) == 0)
		{
				strcpy(reqd_atrb,t_tab_line[i].t_atrb);
				strcpy(reqd_desc,t_tab_line[i].t_atrb_desc);
				return(0);
		}
	}
	return(1);
}

/*=================================
| Loads term_desc file into array |
=================================*/
void
load_term_desc (
 void)
{
	int	loop = 0;
	int	found_atrb = FALSE;
	int	pos_atrb = 0;
	char	data[81];
	char	tmp_atrb[4];
	char	*lclptr;

	if ((descf = fopen(tdesc_dir,"r")) == 0)
	{
		fprintf(stdout,"Error in %s during (FOPEN)\n\r",tdesc_dir);
		fprintf(stdout,"%s %d\n\r",PNAME,errno);
		fflush(stdout);
		return;
	}

	/*---------------------------------
	| load term_desc file into arrays | 
	---------------------------------*/
	lclptr = fgets(data,80,descf);

	while (lclptr)
	{
		/*-----------------------
		| Ignore comments ie. # |
		-----------------------*/
		if (*lclptr == '#')
		{
			lclptr = fgets(data,80,descf);
			continue;
		}

		sprintf(tmp_atrb, "%-3.3s", lclptr);

		/*------------------
		| Look for first " |
		------------------*/
		tptr = lclptr;
		while (*tptr && *tptr != '"')
			tptr++;

		if (*tptr)
		{
			*tptr = '\0';

			found_atrb = FALSE;
			for(pos_atrb = 0; pos_atrb < MAX_TA; pos_atrb++)
				if (!strncmp(t_tab_line[pos_atrb].t_atrb, tmp_atrb, 3))
				{
					found_atrb = TRUE;
					break;
				}

			if (found_atrb)
				add_desc(pos_atrb);
		}
		loop++;
		lclptr = fgets(data,80,descf);
	}
	no_in_desc_tab = loop;
	fclose(descf);
}

/*--------------------------------------
| Add description of atribute to array |
--------------------------------------*/
void
add_desc (
 int position)
{
	char	*lclptr;

	/*-----------------------------
	| Look for end of description |
	-----------------------------*/
	lclptr = ++tptr;
	while (*tptr && *tptr != '"')
		tptr++;

	if (*tptr)
	{
		*tptr = '\0';

		/*-------------------------------
		| attribute desc is 2nd on line |
		-------------------------------*/
		sprintf(t_tab_line[position].t_atrb_desc, "%-40.40s",lclptr);


		lclptr = ++tptr;
		while (*tptr && *tptr != '(')
			tptr++;

		if (*tptr)
		{
			tptr++;
			if (!strncmp (tptr, "NEEDED", 6))
				strcpy(t_tab_line[position].t_reqd, "YES");
		}
	}
}

/*-------------------------------------------------
| Initialise attribute to those defined in tcap.c |
-------------------------------------------------*/
void
init_t_table (
 void)
{
	int 	i;
	int 	loop;
	int	func_found = TRUE;

	for (i = 0; i < MAX_TA; i++)
	{
		sprintf(t_tab_line[i].t_atrb, "%-3.3s", atrb[i]);
		sprintf(t_tab_line[i].t_atrb_desc, "%-40.40s", " ");

		if (!strncmp (t_tab_line[i].t_atrb, "dW#", 3))
			strcpy(t_tab_line[i].t_esc_seq, "FALSE");
		else
			strcpy(t_tab_line[i].t_esc_seq, "");
		sprintf(t_tab_line[i].t_reqd, "%-3.3s", " ");
	}


	/*-----------------------------------------
	| Find position of function keys in table |
	-----------------------------------------*/
	for (i = 0; i < MAX_TA; i++)
		if (!strcmp(t_tab_line[i].t_atrb, "k0="))
		{
			func_found = TRUE;
			break;
		}

	/*------------------------------------------------------------
	| Set default values for function keys 1-16 and shifted 1-16 |
	------------------------------------------------------------*/
	if (func_found)
	{
		for ( loop = 0; loop < 32; loop++)
		{
			strcpy(t_tab_line[i].t_esc_seq, function_keys[loop]._key);
			i++;
		}
	}

	load_term_desc();
}


/*==========================================================================
| Load terminal form file in MENUSYS/TERM if exists or from termcap.cprogs |
==========================================================================*/
void
load_term (
 void)
{
	char 	rd_line[81];
	int	wk_len = 0;

	new_term = TRUE;

	if ((tcapf = fopen(filename,"r")) == 0)
	{
		sprintf(filename,"%s/termcap.cprogs",directory);
		/*----------------------------------------------
		| Open /usr/LS10.5/BIN/MENUSYS/termcapcprogs file. |
		----------------------------------------------*/
		if ((tcapf = fopen(filename,"r")) == 0)
		{
			fclose(tcapf);
			return;
		}
	}

	/*=======================
	| Find terminals entry. |
	=======================*/
	wk_len = strlen(local_rec.term_name);
	sptr = fgets(rd_line,80,tcapf);
	while (sptr != (char *)0 && strncmp(rd_line,local_rec.term_name,wk_len))
		sptr = fgets(rd_line,80,tcapf);

	if (sptr == (char *)0)
	{
	 	fclose(tcapf);
		return;
	}

	init_t_table();   

	new_term = FALSE;

	/*=================================
	| Read entry into array elements. |
	=================================*/
	while (sptr != (char *)0 && load_line(sptr) != 1)
		sptr = fgets(rd_line,80,tcapf);

	/*=========================================
	| Close file /usr/LS10.5/BIN/MENUSYS/termcap. |
	=========================================*/
	fclose(tcapf);
}

/*================================================
| Load a line from term entry and store in array |
================================================*/
int 
load_line (
 char *line)
{
	int	flag = 0;
	int	loop;
	int	line_len = strlen(line);

	if (*(line + line_len - 2) == ':' || line_len == 0)
		flag = 1;/*	return 1 if last line in entry	*/

	while (*(line + line_len) != ':')
		--line_len;
	/*--------------------- 
	| Remove end of line. |
	---------------------*/
	*(line + line_len) = '\0'; 

	while (line_len)
	{
		if (*(line + line_len) == ':' )
		{
			*(line + line_len) = '\0';
			for (loop = 0; loop < MAX_TA; loop++)
			{
				if (strncmp(t_tab_line[loop].t_atrb,line + line_len + 1 ,3) == 0)
				{
					if (!strncmp (t_tab_line[loop].t_atrb, "dW#", 3))
						strcpy(t_tab_line[loop].t_esc_seq, "TRUE ");
					else
						strcpy(t_tab_line[loop].t_esc_seq, line + line_len + 4);
					break;
				}
			}
		}
		--line_len;
	}
	return(flag);
}

/*=========================
| Display Screen Heading  |
=========================*/
int
heading (
 int scn)
{
	char	hdng_date[11];

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		if (scn == 1)	
		{
			clear();
			strcpy(hdng_date,DateToString(TodaysDate()));

			sprintf(err_str,ML(mlUtilsMess122));
			rv_pr(err_str,(132 - strlen(err_str)) / 2,0,1);
			rv_pr(hdng_date,121,0,0);
			disp_help(110);
			box(0,1,132,1); 

		}

		if (scn == 2)
		{
			box(0,20,132,1);
			/*Change Escape Sequence*/
			rv_pr(ML(mlUtilsMess123),54,20,0);
			/* Escape Sequence for :*/
			rv_pr(ML(mlUtilsMess124),1,21,0);
			rv_pr(local_rec.atrb_desc,26,21,1);
		}

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_SUCCESS);
}

/*-------------------------------
| Get escape sequence from user |
-------------------------------*/
void
get_esc_seq (
 int x, 
 int y, 
 char *mask, 
 char *buf)
{
	char    *sptr = buf;
	char    *dpos = NULL;
	int		i;
	int		bufsize = strlen(mask);
	int     c = 0;
	int     decflg = FALSE;

	/*-------------------------------
	| Redisplay after Redraw	|
	-------------------------------*/
	if (last_char == REDRAW || last_char == SEARCH)
	{
		print_at(y,x,"%s", sptr);

		dpos = strchr (sptr,'.');
		decflg = (dpos != (char *) NULL);
		sptr = buf + strlen(buf);
	}
	else
		*sptr = (char) NULL;

	/*-----------------------
	| print '_' for mask	|
	-----------------------*/
	move(x + strlen(buf),y);
	for (i = strlen(buf);i < bufsize;i++)
		putchar((mask[i] == '.') ? '.' : US);
	fflush (stdout);

	crsr_on();
	/*-------------------------------
	| Move to appropriate position	|
	-------------------------------*/
	move(x + strlen(buf),y);
        while (TRUE)
	{
                c = getkey();
		last_char = c;

		/*-----------------------
		| Perform Upshift	|
		-----------------------*/
		if (c < 2000 && mask[sptr - buf] == 'U')
			c = toupper(c);

		/*-------------------------------
		| At a decimal point		|
		| and input is not valid	|
		-------------------------------*/
		if (mask[sptr - buf] == '.' && !decflg && c != BS && c != '.' && c != REDRAW && c != SEARCH && c != ENDINPUT && c != FN9 && c != FN10 && c != FN11 && c != FN12)
		{
                        putchar(BELL);
			fflush (stdout);
                        continue;
                }

		/*-------------------------------
		| Special key excluding BS	|
		-------------------------------*/
		if ((c >= 2000 || c < ' ') && c != BS)
		{
			/*---------------
			| Search Key	|
			---------------*/
			if (c == FN4 || c == FN9 || c == FN10 || c == FN11 || c == FN12)
			{
				if (!search_ok)
				{
					putchar(BELL);
					fflush (stdout);
					continue;
				}
				search_key = last_char;
				last_char = SEARCH;
			}
			return;
		}

                switch (c)
		{
                case ENDINPUT:
			return;
			break;

                case BS:
                        if (sptr == buf)
			{
                                putchar(BELL);
				fflush (stdout);
			}
                        else
			{
                                sptr--;
                                *sptr = (char) NULL;
				putchar(BS);
				putchar((mask[sptr - buf] == '.') ? '.' : US);
				putchar(BS);
				fflush (stdout);
                        }
                        break;

                default:
                        if (sptr - buf < bufsize)
			{
				if (strchr (val_2_chars, c) != (char *)0)
				{
					*sptr++ = (char) c;
					*sptr = (char) NULL;
					putchar(c);
				}
				else
				{
					if (c == ':' && (sptr + 4 - buf <= bufsize))
					{
						strcat (sptr, "\\072");
						sptr += 4;
						printf ("\\072");
					}
					else
					{
						putchar(BELL);
					}
				}
			}
			else
				putchar(BELL);
			fflush(stdout);
                        break;
                }
        }
}
