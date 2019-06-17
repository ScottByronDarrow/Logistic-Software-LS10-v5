/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: pc_glcons.c,v 5.4 2001/09/04 08:15:39 scott Exp $
|  Program Name  : (pc_glcons.c) 
|  Program Desc  : (Consolidation of pcgl records into glwk)
|---------------------------------------------------------------------|
|  Date Written  : (05/05/92)      | Author        : Mike Davy        |
|---------------------------------------------------------------------|
| $Log: pc_glcons.c,v $
| Revision 5.4  2001/09/04 08:15:39  scott
| Updated as structure needed to be same as glwk in app.schema
|
| Revision 5.3  2001/08/09 09:14:35  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:34:59  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_glcons.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_glcons/pc_glcons.c,v 5.4 2001/09/04 08:15:39 scott Exp $";

#include	<pslscr.h>
#include	<GlUtils.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct pcglRecord	pcgl_rec;


char	*data = "data";

char	locCurr [4];
int		printerNo;

/*=====================
| function prototypes |
=====================*/
void shutdown_prog 		(void);
void ProcessAll 		(void);
void ProcessJob 		(void);
void ProcessWorks 		(void);
void OpenDB 			(void);
void CloseDB 			(void);

/*==========================
| Main processing routine. |
==========================*/
int
main (
 int   argc,
 char *argv [])
{
	char	*chk_env (char *),
			type [2],
			*sptr = chk_env ("PC_GLPOST");

	OpenDB ();

	init_scr ();
	set_tty ();

	if (argc < 2)
	{
		print_at (0,0, "Usage: %s <LPNO>", argv [ 0]);
		exit (0);
	}

	printerNo = atoi (argv [1]);
	if (!sptr)
		strcpy (type, "A");
	else
		strcpy (type, sptr);

	switch (type [0])
	{
	case 'j':
	case 'J':
		ProcessJob ();
		break;

	case 'w':
	case 'W':
		ProcessWorks ();
		break;

	default:
		ProcessAll ();
		break;
	}

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*==========================
| Flush all pcgl to glwk   |
==========================*/
void
ProcessAll (void)
{
	sprintf (pcgl_rec.tran_type,"%-2.2s"," ");
	sprintf (pcgl_rec.sys_ref,"%-10.10s"," ");

	cc = find_rec (pcgl, &pcgl_rec, GTEQ, "u");
	while (!cc)
	{
		pin_bcopy ((char *) &glwkRec, (char *) &pcgl_rec, sizeof (glwkRec));
		GL_AddBatch ();

		/*-------------
		| flush pcgl  |
		-------------*/
		cc = abc_delete (pcgl);
		if (cc)
			file_err (cc, pcgl, "DBDELETE");

		/*-------------------------------
		| Get the next (1st) pcgl rec.	|
		-------------------------------*/
		sprintf (pcgl_rec.tran_type,"%-2.2s"," ");
		sprintf (pcgl_rec.sys_ref,	"%-10.10s"," ");
		cc = find_rec (pcgl, &pcgl_rec, GTEQ, "u");
	}
	return;
}

/*=======================================
| Consolidate pcgl based on job (narr)  |
=======================================*/
void
ProcessJob (void)
{
	char	tmp_narr [sizeof glwkRec.narrative],
			tmp_user_ref [sizeof glwkRec.user_ref];
	long 	tmp_hhgl,
			tmp_post_date;
	double	subtotal;
	int 	eof = FALSE;

	abc_selfield (pcgl, "pcgl_narrative");
	strcpy (tmp_user_ref,"CONSOL.MANUFG. ");
	sprintf (pcgl_rec.narrative,"%20.20s", " ");
	cc = find_rec (pcgl, &pcgl_rec, GTEQ, "u");
	if (cc)
		eof = TRUE;

	while (!eof)
	{
		/*------------------------
		| consolidation criteria |
		------------------------*/
		strcpy (tmp_narr, pcgl_rec.narrative);
		tmp_hhgl = pcgl_rec.hhgl_hash;
		tmp_post_date = pcgl_rec.post_date;
		subtotal = 0.00;
		eof = FALSE;

		/*---------------------------------------
		| set glwk rec to 1st pcgl rec in group |
		---------------------------------------*/
		pin_bcopy ((char *) &glwkRec, (char *) &pcgl_rec, sizeof (glwkRec));
		while (	!cc &&
	       		!strcmp (tmp_narr, pcgl_rec.narrative) &&
	      	 	tmp_hhgl == pcgl_rec.hhgl_hash &&
	       		tmp_post_date == pcgl_rec.post_date)
		{
			subtotal += pcgl_rec.amount;
			/*-------------
			| flush pcgl  |
			-------------*/
			cc = abc_delete (pcgl);
			if (cc)
				file_err (cc, pcgl, "DBDELETE");

			strcpy (pcgl_rec.narrative,"                    ");
			cc = find_rec (pcgl, &pcgl_rec, GTEQ, "u");
			if (cc)
				eof = TRUE;
		}
		strcpy (glwkRec.user_ref, tmp_user_ref);

		/*------------------------
		| summary info for group |
		------------------------*/
		if (subtotal != 0.00)
		{
			if (subtotal < 0.00)
			{
				subtotal *= -1.00;
				strcpy (glwkRec.jnl_type, "2");
			}
			else
				strcpy (glwkRec.jnl_type, "1");

			glwkRec.amount 		= subtotal;
			glwkRec.loc_amount  = subtotal;
			glwkRec.exch_rate	=	1.00;
			strcpy (glwkRec.currency, locCurr);

			strcpy (glwkRec.alt_desc1, " ");
			strcpy (glwkRec.alt_desc2, " ");
			strcpy (glwkRec.alt_desc3, " ");
			strcpy (glwkRec.batch_no,  " ");
			GL_AddBatch ();
		}
	}
	return;
}
			
/*=======================================
| Consolidate pcgl based on work centre |
=======================================*/
void
ProcessWorks (
 void)
{
	char	tmp_narr [sizeof glwkRec.narrative],
			tmp_user_ref [sizeof glwkRec.user_ref];
	long 	tmp_hhgl,
			tmp_post_date;
	double	subtotal;
	int 	eof = FALSE;

	abc_selfield (pcgl, "pcgl_user_ref");
	strcpy (tmp_narr,"CONSOLIDATED MANUFG.");
	strcpy (pcgl_rec.user_ref,"               ");
	cc = find_rec (pcgl, &pcgl_rec, GTEQ, "u");
	if (cc)
		eof = TRUE;
	while (!eof)
	{
		/*------------------------
		| consolidation criteria |
		------------------------*/
		strcpy (tmp_user_ref, pcgl_rec.user_ref);
		tmp_hhgl = pcgl_rec.hhgl_hash;
		tmp_post_date = pcgl_rec.post_date;
		subtotal = 0.00;
		eof = FALSE;

		/*---------------------------------------
		| set glwk rec to 1st pcgl rec in group |
		---------------------------------------*/
		pin_bcopy ((char *) &glwkRec, (char *) &pcgl_rec, sizeof (glwkRec));
		while (	!cc &&
	       		!strcmp (tmp_user_ref, pcgl_rec.user_ref) &&
	      	 	tmp_hhgl == pcgl_rec.hhgl_hash &&
	       		tmp_post_date == pcgl_rec.post_date)
		{
			subtotal += pcgl_rec.amount;
			/*-------------
			| flush pcgl  |
			-------------*/
			cc = abc_delete (pcgl);
			if (cc)
				file_err (cc, pcgl, "DBDELETE");

			strcpy (pcgl_rec.user_ref,"               ");
			cc = find_rec (pcgl, &pcgl_rec, GTEQ, "u");
			if (cc)
				eof = TRUE;
		}
		strcpy (glwkRec.narrative, tmp_narr);
		/*------------------------
		| summary info for group |
		------------------------*/
		if (subtotal != 0.00)
		{
			if (subtotal < 0.00)
			{
				subtotal *= -1.00;
				strcpy (glwkRec.jnl_type, "2");
			}
			else
				strcpy (glwkRec.jnl_type, "1");

			glwkRec.amount 		= subtotal;
			glwkRec.loc_amount  = subtotal;
			glwkRec.exch_rate	=	1.00;
			strcpy (glwkRec.currency, locCurr);

			strcpy (glwkRec.alt_desc1, " ");
			strcpy (glwkRec.alt_desc2, " ");
			strcpy (glwkRec.alt_desc3, " ");
			strcpy (glwkRec.batch_no,  " ");
			GL_AddBatch ();
		}
	}
	return;
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{	
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (locCurr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (locCurr, "%-3.3s", comr_rec.base_curr);

	abc_fclose (comr);

	OpenGlmr ();
	OpenGlwk ();
	open_rec (pcgl, pcgl_list, PCGL_NO_FIELDS, "pcgl_id_no");
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (pcgl);
	GL_CloseBatch (printerNo);
	GL_Close ();

	abc_dbclose (data);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}
