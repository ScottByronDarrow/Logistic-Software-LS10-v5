/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: dsp_xml_err.c,v 5.1 2001/08/09 05:13:25 scott Exp $
|  Program Name  : (dsp_xml_err.c)
|  Program Desc  : (Display XML Errors)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : May 14th 2001    |
|---------------------------------------------------------------------|
| $Log: dsp_xml_err.c,v $
| Revision 5.1  2001/08/09 05:13:25  scott
| Updated to use FinishProgram ();
|
| Revision 5.0  2001/06/19 08:08:16  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 1.1  2001/05/14 05:45:46  scott
| New program to display XML errors.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: dsp_xml_err.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/dsp_xml_err/dsp_xml_err.c,v 5.1 2001/08/09 05:13:25 scott Exp $";

#include 	<pslscr.h>

#include	"schema"

struct xmleRecord	xmle_rec;

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	 (void);
void	LoadErrors		 (void);
void	OpenDB					(void);
void	CloseDB					(void);

/*======================
| Open Database Files. |
======================*/


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	init_scr ();			/*  sets terminal from termcap	*/
	swide ();
	set_tty ();
	OpenDB ();

	LoadErrors ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	open_rec (xmle,  xmle_list, XMLE_NO_FIELDS, "xmle_id_no");
}
/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (xmle);
	abc_dbclose ("data");
}

void
LoadErrors (
 void)
{
	strcpy (xmle_rec.program, " ");
	strcpy (xmle_rec.time_err, " ");
	strcpy (xmle_rec.user_name, " ");
	xmle_rec.date_err	=	0L;
	
	Dsp_open (0,0,18);
	Dsp_saverec ("     PROGRAM NAME     |  USER  NAME  |   DATE   |TIME |                          ERROR DESCRIPTION                           ");
	Dsp_saverec ("");
	Dsp_saverec (" [REDRAW] [NEXT SCREEN] [PREV SCREEN] [EDIT/END] ");

	cc = find_rec (xmle, &xmle_rec, GTEQ, "r");
	while (!cc)
	{
		sprintf (err_str, " %-20.20s ^E%-14.14s^E%s^E%s^E %s",
				xmle_rec.program,
				xmle_rec.user_name,
				DateToString (xmle_rec.date_err),
				xmle_rec.time_err,
				xmle_rec.desc_1);

		Dsp_saverec (err_str);

		if (strlen (clip (xmle_rec.desc_2)))
		{
			sprintf (err_str, "%22.22s^E%14.14s^E%10.10s^E%5.5s^E %s",
					" ",
					" ",
					" ",
					" ",
					xmle_rec.desc_2);
			Dsp_saverec (err_str);
		}
		if (strlen (clip (xmle_rec.desc_3)))
		{
			sprintf (err_str, "%22.22s^E%14.14s^E%10.10s^E%5.5s^E %s",
					" ",
					" ",
					" ",
					" ",
					xmle_rec.desc_3);
			Dsp_saverec (err_str);
		}
		if (strlen (clip (xmle_rec.desc_4)))
		{
			sprintf (err_str, "%22.22s^E%14.14s^E%10.10s^E%5.5s^E %s",
					" ",
					" ",
					" ",
					" ",
					xmle_rec.desc_4);
			Dsp_saverec (err_str);
		}
		Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGGGEGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
		cc = find_rec (xmle, &xmle_rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();
}
