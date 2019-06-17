/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_glmrCheck.c,v 5.5 2001/08/26 23:20:16 scott Exp $
|  Program Name  : (glmr_check.c)
|  Program Desc  : (Global integrity check of the glmr file)
|---------------------------------------------------------------------|
|  Author        : Trevor van Bremen       | Date Written : 09/01/91  |
|---------------------------------------------------------------------|
| $Log: gl_glmrCheck.c,v $
| Revision 5.5  2001/08/26 23:20:16  scott
| Updated from scotts machine - ongoing WIP release 10.5
|
| Revision 5.4  2001/08/20 23:12:48  scott
| Updated for development related to bullet proofing
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_glmrCheck.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_glmrCheck/gl_glmrCheck.c,v 5.5 2001/08/26 23:20:16 scott Exp $";

/*===============================
|   Include file dependencies   |
===============================*/

#include <pslscr.h>
#include <GlUtils.h>

/*===================================
|   Constants, defines and stuff    |
===================================*/

#define	EACCT_NUM       0
#define	EACCT_LEN       1
#define	ENO_PARENT      2
#define	EBAD_CLASS      3
#define	WNO_DESC        4
#define	WLINK_PARENT    5
#define	WLINK_CHILD     6
#define	BAD_INTERFACE   7

#include "schema"

struct	comrRecord	comr_rec;
char    *glln2 = "glln2",
        *glmr2 = "glmr2",
        *data  = "data";


/*=====================
|   Local variables   |
=====================*/

int     error_cnt = 0;
int     PV_max_len,
    	PV_max_lvl,
        PV_acc_lvl;
int     lvl_len [17],
    	tot_len [18];
char    kid_class [4],
    	dad_class [4],
        acc_bit [16][17],
		dispStr [256];

char    *valid_class [] =
{
	"CC ",
	"FAS",
	"FAP",
	"FES",
	"FEP",
	"FIS",
	"FIP",
	"FLS",
	"FLP",
	"NFC",
	"NFS",
	"NFP",
	 (char *) 0
};

	GLMR_STRUCT glmr2Rec;
	GLLN_STRUCT	glln2Rec;

/*===============================
|   Local function prototypes   |
===============================*/
void 	OpenDB 					 (void);
void 	CloseDB 				 (void);
void 	shutdown_prog 			 (void);
void 	InitPrintout 			 (void);
void 	Process 				 (void);
int  	CheckControlAccount 	 (void);
void 	CheckCompany 			 (char *, int);
int  	StructureCheck 			 (void);
int  	AccountError 			 (int);
void 	DescriptionCheck 		 (char *, long, int);
int  	ClassCheck 				 (void);
void 	LinkCheck 				 (void);
void	CheckInterface 			 (char *);

/*===============================
|   Main Processing Routine     |
===============================*/
int
main (
 int argc, 
 char *argv [])
{
	OpenDB ();

	init_scr ();		/*  sets terminal from termcap	  */
	set_tty ();

	InitPrintout ();
	Process ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=======================================
|   Standard program exit sequence      |
=======================================*/
void
shutdown_prog (void)
{	
	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
	if (error_cnt)
    {
        sprintf 
		(
			dispStr, 
			 "%-29s^E Total errors encountered %5d%-57s",
			 " ",
			 error_cnt,
			 " "
		 );
		Dsp_saverec (dispStr);
    }
	else
    {
        sprintf 
		(
			dispStr, 
			"%-29s^E No error encountered %-66s\n",
			" ",
			" "
		);
		Dsp_saverec (dispStr);
    }
	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
	Dsp_srch ();
	Dsp_close ();
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open data base files	|
=======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	OpenGlbl (); abc_selfield (glbl, "glbl_acc_no");
	OpenGlbh (); abc_selfield (glbh, "glbh_hhbh_hash");
	OpenGlih (); abc_selfield (glih, "glih_hhih_hash");
	OpenGlid (); abc_selfield (glid, "glid_acct_no");
	OpenGlca (); 
	OpenGlct (); 
	OpenGlln (); 
	OpenGlmr (); 

	abc_alias (glln2, glln);
	abc_alias (glmr2, glmr);
	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (glln2, glln_list, GLLN_NO_FIELDS, "glln_id_no2");
	open_rec (glmr2, glmr_list, GLMR_NO_FIELDS, "glmr_id_no");
}

/*=======================
| Close data base files	|
=======================*/
void
CloseDB (void)
{
	GL_Close ();
	abc_fclose (comr);
	abc_fclose (glln2);
	abc_fclose (glmr2);
	abc_dbclose (data);
}

/*===============================
|   Initialise pformat pipe     |
===============================*/
void
InitPrintout (void)
{
	clear ();
	swide ();
	rv_pr ("General ledger structure integrity check", 40, 0,1);
	Dsp_prn_open (0, 1, 17, dispStr, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0);
	
	Dsp_saverec (" ACCOUNT NUMBER              | DESCRIPTION OF ERROR                                                                               ");
	Dsp_saverec ("");
	Dsp_saverec (" [FN03]  [FN05]  [FN14]  [FN15]  [FN16] ");
}

/*===============================
|   Process the glmr records    |
===============================*/
void
Process (void)
{
	int     first_co = TRUE;
	char	last_co [3];

	cc = find_rec (glct, &glctRec, FIRST, "r");
	if (cc)
	{
		sprintf 
		(
			dispStr, 
			"%-29.29s^E%-88.88s",
			"FATAL ERROR",
            "No control record found. Cannot continue"
		);
		Dsp_saverec (dispStr);
        error_cnt++;
		return;
	}
	if (CheckControlAccount ())
	{
		sprintf 
		(
			dispStr, 
			"%-29.29s^E%-88.88s",
			"FATAL ERROR",
            "control record INVALID. Cannot continue"
		);
		Dsp_saverec (dispStr);
        error_cnt++;
		return;
	}
	
	strcpy (last_co, "");
	strcpy (glmrRec.co_no, "  ");
	strcpy (glmrRec.acc_no, "                ");

    cc = find_rec (glmr, &glmrRec, GTEQ, "r");
	while (!cc)
	{
		if (strcmp (last_co, glmrRec.co_no))
		{
			CheckCompany (glmrRec.co_no, first_co);
			strcpy (last_co, glmrRec.co_no);
			first_co = FALSE;
		}
		CheckInterface (glmrRec.acc_no);

		if (!ClassCheck ())
		{
			StructureCheck ();
			LinkCheck ();
		}
		cc = find_rec (glmr, &glmrRec, NEXT, "r");
	}
}

/*=======================
| Validate glct_format	|
=======================*/
int 
CheckControlAccount (void)
{
	int	    lvl_no,
            len_tot = 0;
	char    *sptr,
            *tptr;

	for (lvl_no = 0; lvl_no < 17; lvl_no++)
    {
        tot_len [lvl_no] = lvl_len [lvl_no] = 0;
    }

	/*-------------------------------
	| Format MUST consist only of a	|
	| combination of '-' and 'X'.	|
	-------------------------------*/
	tptr = sptr = clip (glctRec.format);
	while (*tptr)
	{
		if (*tptr == 'X' || *tptr == '-')
        {
            tptr++;
        }
		else
        {
            return (EXIT_FAILURE);
        }
	}

	/*-------------------------------
	| Each level MUST be > 0 chars	|
	-------------------------------*/
	lvl_no = 0;
	while (TRUE)
	{
		tptr = strchr (sptr, '-');
		if (tptr)
		{
			*tptr = 0;
			lvl_len [lvl_no] = strlen (sptr);
			if (! (lvl_len [lvl_no]))
            {
                return (EXIT_FAILURE);
            }
			len_tot+= strlen (sptr);
			lvl_no++;
			tot_len [lvl_no] = len_tot;
			sptr = tptr + 1;
		}
		else
		{
			lvl_len [lvl_no] = strlen (sptr);
			len_tot+= strlen (sptr);
			tot_len [lvl_no + 1] = len_tot;
			PV_max_len = len_tot;
			PV_max_lvl = lvl_no;
			break;
		}
	}

	return (EXIT_SUCCESS);
}

/*==================================
| Make sure co_no is valid in comr |
==================================*/
void
CheckCompany (
	char   *co_no, 
	int    first_time)
{
	strcpy (comr_rec.co_no, co_no);
    cc = find_rec (comr, &comr_rec, EQUAL, "r");
	if (cc)
    {
		sprintf 
		(
			dispStr, 
            " Co: %-2.2s %-21.21s^E%-88.88s",
             comr_rec.co_no,
             comr_rec.co_short_name,
          	(cc) ? " FATAL ERROR NOT ON FILE " : " "
		);
		Dsp_saverec (dispStr);
        error_cnt++;
	}
}

/*===============================================
| Make sure that the next highest level exists	|
| and also, check that current class is valid.	|
===============================================*/
int
StructureCheck (void)
{
	char    acc_no [17],
            lcl_no [17];
    int     acc_len,
            acc_lvl;

	strcpy (acc_no, glmrRec.acc_no);
	clip (acc_no);
	acc_len = strlen (acc_no);

    if (acc_len != PV_max_len)
        return (AccountError (EACCT_LEN));

	/*-----------------------
	| Split account number	|
	| into level-sized bits	|
	-----------------------*/
	for (PV_acc_lvl = 0; PV_acc_lvl < 16; PV_acc_lvl++)
	{
		sprintf (acc_bit [PV_acc_lvl], 
                 "%-*.*s",
                 lvl_len [PV_acc_lvl],
                 lvl_len [PV_acc_lvl],
                 &acc_no [tot_len [PV_acc_lvl]]);
	}

	/*-----------------------
	| Ascertain which level	|
	| this account is	|
	-----------------------*/
	PV_acc_lvl = 16;
	while (PV_acc_lvl > 0)
	{
		if (atol (acc_bit [PV_acc_lvl- 1]) == 0L)
        {
            PV_acc_lvl--;
        }
		else
		{
			DescriptionCheck (glmrRec.co_no, 
                        atol (acc_bit [PV_acc_lvl - 1]), 
                        PV_acc_lvl);
			break;
		}
	}

	/*-----------------------
	| Build parent acc_no	|
	-----------------------*/
	strcpy (acc_no, "");
	for (acc_lvl = 0; acc_lvl <= PV_max_lvl; acc_lvl++)
	{
		if (acc_lvl < (PV_acc_lvl - 1))
        {
            strcat (acc_no, acc_bit [acc_lvl]);
        }
		else
		{
			sprintf 
			(
				lcl_no, 
				"%-*.*s",
				lvl_len [acc_lvl],
				lvl_len [acc_lvl],
				"0000000000000000"
			);
            strcat (acc_no, lcl_no);
		}
	}

	if (!strcmp (kid_class, "CC "))
    {
        return (EXIT_SUCCESS);
    }

	strcpy (glmr2Rec.co_no, glmrRec.co_no);
	sprintf (glmr2Rec.acc_no, "%-16.16s", acc_no);

    cc = find_rec (glmr2, &glmr2Rec, EQUAL, "r");
	if (cc)
        return (AccountError (ENO_PARENT));

	sprintf (dad_class, 
             "%s%s%s",
             glmr2Rec.glmr_class [0],
             glmr2Rec.glmr_class [1],
             glmr2Rec.glmr_class [2]);

	if (!strcmp (kid_class, "NFC"))
	{
		if (strcmp (dad_class, "CC "))
        {
            return (AccountError (EBAD_CLASS));
        }
        else
        {
            return (EXIT_SUCCESS);
        }
	}

	if (!strcmp (kid_class, "NFS"))
	{
		if (strcmp (dad_class, "NFC") &&
		    strcmp (dad_class, "NFS"))
        {
            return (AccountError (EBAD_CLASS));
        }
		else
        {
            return (EXIT_SUCCESS);
        }
	}

	if (!strcmp (kid_class, "NFP"))
	{
		if (strcmp (dad_class, "NFS"))
        {
            return (AccountError (EBAD_CLASS));
        }
		else
        {
            return (EXIT_SUCCESS);
        }
	}

	if (!strcmp (kid_class, "FLP"))
	{
		if (strcmp (dad_class, "FLS") &&
		    strcmp (dad_class, "CC "))
        {
            return (AccountError (EBAD_CLASS));
        }
        else
        {
            return (EXIT_SUCCESS);
        }
	}

	if (!strcmp (kid_class, "FIP"))
	{
		if (strcmp (dad_class, "FLS") &&
		    strcmp (dad_class, "FIS") &&
		    strcmp (dad_class, "CC "))
        {
            return (AccountError (EBAD_CLASS));
        }
		else
        {
            return (EXIT_SUCCESS);
        }
	}

	if (!strcmp (kid_class, "FEP"))
	{
		if (strcmp (dad_class, "FLS") &&
		    strcmp (dad_class, "FIS") &&
		    strcmp (dad_class, "FES") &&
		    strcmp (dad_class, "FAS") &&
		    strcmp (dad_class, "CC "))
        {
            return (AccountError (EBAD_CLASS));
        }
		else
        {
            return (EXIT_SUCCESS);
        }
	}

	if (!strcmp (kid_class, "FAP"))
	{
		if (strcmp (dad_class, "FLS") &&
		    strcmp (dad_class, "FAS") &&
		    strcmp (dad_class, "CC "))
        {
            return (AccountError (EBAD_CLASS));
        }
		else
        {
            return (EXIT_SUCCESS);
        }
	}

	if (!strcmp (kid_class, "FLS"))
	{
		if (strcmp (dad_class, "FLS") &&
		    strcmp (dad_class, "CC "))
        {
            return (AccountError (EBAD_CLASS));
        }
		else
        {
            return (EXIT_SUCCESS);
        }
	}

	if (!strcmp (kid_class, "FIS"))
	{
		if (strcmp (dad_class, "FLS") &&
		    strcmp (dad_class, "FIS") &&
		    strcmp (dad_class, "CC "))
        {
            return (AccountError (EBAD_CLASS));
        }
		else
        {
            return (EXIT_SUCCESS);
        }
	}

	if (!strcmp (kid_class, "FES"))
	{
		if (strcmp (dad_class, "FLS") &&
		    strcmp (dad_class, "FES") &&
		    strcmp (dad_class, "FAS") &&
		    strcmp (dad_class, "CC "))
        {
            return (AccountError (EBAD_CLASS));
        }
		else
        {
            return (EXIT_SUCCESS);
        }
	}

	if (!strcmp (kid_class, "FAS"))
	{
		if (strcmp (dad_class, "FLS") &&
		    strcmp (dad_class, "FAS") &&
		    strcmp (dad_class, "CC "))
        {
            return (AccountError (EBAD_CLASS));
        }
		else
        {
            return (EXIT_SUCCESS);
        }
	}

    return (EXIT_SUCCESS);
}

/*=======================================
| Print an error about the current acct	|
=======================================*/
int
AccountError (
 int error_no)
{
	error_cnt++;

	switch (error_no)
	{
	case	EACCT_NUM:
		sprintf 
		(
			dispStr, 
			" %-28.28s^E %-100.100s ",
			glmrRec.acc_no,
			"Class 'CC ' accounts MUST be account no. 0000000000000000"
		);
		Dsp_saverec (dispStr);
		break;


	case	EACCT_LEN:
		sprintf
		(
			dispStr, 
			" %-28.28s^E %-100.100s",
			glmrRec.acc_no,
			"Illegal account number format. (Length wrong!)"
		);
		Dsp_saverec (dispStr);
		break;

	case	ENO_PARENT:
		sprintf 
		(
			dispStr, 
			" %-28.28s^E %-100.100s",
			glmrRec.acc_no,
			"Could not locate parent record!"
		);
		Dsp_saverec (dispStr);
		break;

	case	EBAD_CLASS:
		sprintf 
		(
			dispStr, 
			" %-28.28s^E %-100.100s",
			glmrRec.acc_no,
			"Invalid class for parent account!"
		);
		Dsp_saverec (dispStr);
		break;

	case	WNO_DESC:
		error_cnt--;
		sprintf 
		(
			dispStr,
			" %-28.28s^E %-100.100s",
			glmrRec.acc_no,
			"WARNING: No description at lowest level! (Please create a record)"
		);
		Dsp_saverec (dispStr);
		break;

	case	WLINK_PARENT:
		error_cnt--;
		sprintf 
		(
			dispStr, 
			" %-28.28s^E %-100.100s",
			glmrRec.acc_no,
			"WARNING: Parent link count was wrong! I have repaired it for you!"
		);
		Dsp_saverec (dispStr);
		break;

	case	WLINK_CHILD:
		error_cnt--;
		sprintf 
		(
			dispStr, 
			" %-28.28s^E %-100.100s",
			glmrRec.acc_no,
			"WARNING: Child link count was wrong! I have repaired it for you!"
		);
		Dsp_saverec (dispStr);
		break;
	}

	return (EXIT_FAILURE);
}

void
DescriptionCheck (
 char *co_no, 
 long acc_no, 
 int  acc_lvl)
{
	strcpy (glcaRec.co_no, co_no);
	glcaRec.level_no = acc_lvl;
	glcaRec.acc_no = acc_no;

	if (find_rec (glca, &glcaRec, EQUAL, "r"))
    {
        AccountError (WNO_DESC);
    }
}

int
ClassCheck (
 void)
{
	int	class_indx;

	sprintf (kid_class, "%s%s%s",
		glmrRec.glmr_class [0],
		glmrRec.glmr_class [1],
		glmrRec.glmr_class [2]);

	/*-------------------------------
	| Validate class against list	|
	-------------------------------*/
	for (class_indx = 0; valid_class [class_indx]; class_indx++)
	{
		if (strcmp (kid_class, valid_class [class_indx]))
        {
            continue;
        }
		break;
	}
	if (! (valid_class [class_indx]))
    {
        return (AccountError (EBAD_CLASS));
    }

	/*-------------------------------
	| Class CC must be A/c # 0	|
	-------------------------------*/
	if (!strcmp (kid_class, valid_class [0]))
    {
        if (strncmp (glmrRec.acc_no, "0000000000000000", PV_max_len))
        {
            return (AccountError (EACCT_NUM));
        }
    }

	return (EXIT_SUCCESS);
}

void
LinkCheck (
 void)
{
	int cnt,
        update = FALSE;

	cnt = 0;
	if (glmrRec.glmr_class [0][0] == 'N')
	{
		gllnRec.parent_hash = glmrRec.hhmr_hash;
		gllnRec.child_hash = 0L;
		
        cc = find_rec (glln, &gllnRec, GTEQ, "r");
		while (!cc && 
            (gllnRec.parent_hash == glmrRec.hhmr_hash))
		{
			cnt++;
			cc = find_rec (glln, &gllnRec, NEXT, "r");
		}

        if (cnt != glmrRec.child_cnt)
		{
			update = TRUE;
			glmrRec.child_cnt = cnt;
			AccountError (WLINK_CHILD);
		}
	}
	else
	{
		if (glmrRec.child_cnt != 0)
		{
			update = TRUE;
			glmrRec.child_cnt = 0;
			AccountError (WLINK_CHILD);
		}
	}

	cnt = 0;
	glln2Rec.child_hash = glmrRec.hhmr_hash;
	glln2Rec.parent_hash = 0L;

    cc = find_rec (glln2, &glln2Rec, GTEQ, "r");
	while (!cc && 
        (glln2Rec.child_hash == glmrRec.hhmr_hash))
	{
		cnt++;
		cc = find_rec (glln2, &glln2Rec, NEXT, "r");
	}
	if (cnt != glmrRec.parent_cnt)
	{
		update = TRUE;
		glmrRec.parent_cnt = cnt;
		AccountError (WLINK_PARENT);
	}

	if (update)
	{
		cc = abc_update (glmr, &glmrRec);
		if (cc)
        {
            sys_err ("Error in glmr During (DBUPDATE)", cc, PNAME);
        }
	}
}

void
CheckInterface (
	char	*accountNo)
{

	sprintf (glidRec.acct_no, "%-16.16s", accountNo);
	cc = find_rec (glid, &glidRec, COMPARISON, "r");
	if (!cc)
	{
		glihRec.hhih_hash	=	glidRec.hhih_hash;
		cc = find_rec (glih, &glihRec, COMPARISON, "r");
		if (!cc) 
		{
			if (!strcmp (glihRec.co_no, glmrRec.co_no))
			{
				if (glmrRec.glmr_class [2][0] != 'P')
				{
					error_cnt++;
					sprintf (err_str, "Account no longer a posting level account yet interface ^1 %s ^6 exists.", glihRec.int_code);
					sprintf 
					(
						dispStr, 
						" %-28.28s^E %-100.100s",
						glmrRec.acc_no,
						err_str
					);
					Dsp_saverec (dispStr);
				}	
				if (glmrRec.system_acc [0] == 'M')
				{
					error_cnt++;
					sprintf (err_str, "Account is defined in interface ^1 %s ^6 but account is set as a manual posting account.", glihRec.int_code);
					sprintf 
					(
						dispStr, 
						" %-28.28s^E %-100.100s",
						glmrRec.acc_no,
						err_str
					);
					Dsp_saverec (dispStr);
				}	
			}	
		}	
	}	
	if (glmrRec.glmr_class [2][0] != 'P')
	{
		strcpy (glblRec.acc_no, glmrRec.acc_no);
		cc = find_rec (glbl, &glblRec, GTEQ, "r");
		while (!cc && !strcmp (glblRec.acc_no, glmrRec.acc_no))
		{
			glbhRec.hhbh_hash	=	glblRec.hhbh_hash;
			cc = find_rec (glbh, &glbhRec, COMPARISON, "r");
			if (cc || strcmp (glbhRec.co_no, glmrRec.co_no))
			{
				cc = find_rec (glbl, &glblRec, NEXT, "r");
				continue;
			}
			error_cnt++;
			sprintf (err_str, "Batch number %s Journal type %s Dated %s has an invalid posting account.", 
							glbhRec.batch_no,
							glbhRec.jnl_type,
							DateToString (glbhRec.glbh_date));
			sprintf 
			(
				dispStr, 
				"| %-28.28s| %-100.100s |\n",
				glmrRec.acc_no, 
				err_str
			);
			Dsp_saverec (dispStr);

			cc = find_rec (glbl, &glblRec, NEXT, "r");
		}	
	}	
}	

/* [ end of file ] */
