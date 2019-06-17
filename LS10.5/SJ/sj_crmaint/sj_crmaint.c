/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
| $Id: sj_crmaint.c,v 5.2 2001/08/09 09:17:16 scott Exp $
|  Program Name  : (sj_crmaint.c)
|  Program Desc  : (Service Job Control File Maintenance)
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 30/06/88         |
|---------------------------------------------------------------------|
| $Log: sj_crmaint.c,v $
| Revision 5.2  2001/08/09 09:17:16  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:17  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_crmaint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_crmaint/sj_crmaint.c,v 5.2 2001/08/09 09:17:16 scott Exp $";

/*===============================
|   Include file dependencies   |
===============================*/
#include <pslscr.h>
#include <GlUtils.h>
#include <ml_sj_mess.h>
#include <ml_std_mess.h>

/*===================================
|   Constants, defines and stuff    |
===================================*/
char 	*data = "data";

/*=====================
|   Local variables   |
=====================*/
int 	new_sjcr = 0;

#include	"schema"

struct commRecord	comm_rec;
struct cudpRecord	cudp_rec;
struct sjcrRecord	sjcr_rec;


/*============================
| Local & Screen Structures. |
============================*/
struct 
{
	char	dummy [11];
	char	dept [3];
	char	wip_desc [26];
	char	es_desc [26];
	char	is_desc [26];
	char	ec_desc [26];
	char	ic_desc [26];
	char	ecv_desc [26];
	char	icv_desc [26];
} local_rec;

static struct var vars [] =
{
	{1, LIN, "dept_no",	 4, 20, CHARTYPE,
		"AA", "          ",
		" ", "1", "Department", " ",
		YES, NO, JUSTRIGHT, "0", "99", local_rec.dept},
	{1, LIN, "dept_name",	 4, 55, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "-", " ",
		 NA, NO,  JUSTLEFT, "", "", cudp_rec.dp_name},
	{1, LIN, "dis_pc_markup",	 5, 20, FLOATTYPE,
		"NNN.N", "          ",
		" ", "0", "Markup %", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&sjcr_rec.dis_pc_markup},
	{1, LIN, "gl_wip",	 7, 20, CHARTYPE,
		GlMask, "          ",
		"0", "0", "WIP Account", " GL WIP Account Code ",
		YES, NO,  JUSTLEFT, "1234567890*", "", sjcr_rec.gl_wip},
	{1, LIN, "gl_wip_desc",	 7, 55, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.wip_desc},
	{1, LIN, "gl_ext_sales",	 8, 20, CHARTYPE,
		GlMask, "          ",
		"0", "0", "Ext Sales Account", " GL External Sales Account ",
		YES, NO,  JUSTLEFT, "1234567890*", "", sjcr_rec.gl_ext_sales},
	{1, LIN, "gl_es_desc",	 8, 55, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.es_desc},
	{1, LIN, "gl_int_sales",	 9, 20, CHARTYPE,
		GlMask, "          ",
		"0", "0", "Int Sales Account", " GL Internal Sales Account ",
		YES, NO,  JUSTLEFT, "1234567890*", "", sjcr_rec.gl_int_sales},
	{1, LIN, "gl_is_desc",	 9, 55, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.is_desc},
	{1, LIN, "gl_ext_cos",	10, 20, CHARTYPE,
		GlMask, "          ",
		"0", "0", "Ext COS Account", " GL External Cost of Sales Account ",
		YES, NO,  JUSTLEFT, "1234567890*", "", sjcr_rec.gl_ext_cos},
	{1, LIN, "gl_ec_desc",	10, 55, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.ec_desc},
	{1, LIN, "gl_int_cos",	11, 20, CHARTYPE,
		GlMask, "          ",
		"0", "0", "Int COS Account", " GL Internal Cost of Sales Account ",
		YES, NO,  JUSTLEFT, "1234567890*", "", sjcr_rec.gl_int_cos},
	{1, LIN, "gl_ic_desc",	11, 55, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.ic_desc},
	{1, LIN, "gl_ext_cos_var",	12, 20, CHARTYPE,
		GlMask, "          ",
		"0", "0", "Ext COS Var Account", " GL External Cost of Sales Variance Account ",
		YES, NO,  JUSTLEFT, "1234567890*", "", sjcr_rec.gl_ext_cos_va},
	{1, LIN, "gl_ecv_desc",	12, 55, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.ecv_desc},
	{1, LIN, "gl_int_cos_var",	13, 20, CHARTYPE,
		GlMask, "          ",
		"0", "0", "Int COS Var Account", " GL Internal Cost of Sales Variance Account ",
		YES, NO,  JUSTLEFT, "1234567890*", "", sjcr_rec.gl_int_cos_va},
	{1, LIN, "gl_icv_desc",	13, 55, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.icv_desc},
	{0, LIN, "",	 0, 0, CHARTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*===============================
|   Local function prototypes   |
===============================*/
void 	shutdown_prog 	 (void);
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
int  	spec_valid 		 (int);
int  	GetGlmr 		 (char *, char *, char *);
void 	Update 			 (void);
int  	heading 		 (int);
void 	SrchCudp  		 (char *key_val);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char *argv [])
{


	sprintf (GlMask,   "%-*.*s", MAXLEVEL, MAXLEVEL, "AAAAAAAAAAAAAAAA");
	sprintf (GlFormat, "%-*.*s", MAXLEVEL, MAXLEVEL, "XXXXXXXXXXXXXXXX");

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);
	init_scr ();
	set_tty (); 
	set_masks ();
	init_vars (1);

    swide ();
    
    OpenDB ();

	GL_SetMask (GlFormat);

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{

		/*-----------------------
		| Reset control flags . |
		-----------------------*/
        /*  NOTES
            these flags are better off incorporated into a 
            global struct instead of lying around like a 
            local variable.
            something along the lines of :

                typedef struct 
                {
            		int search_ok,
		            int entry_exit, 
                    int prog_exit, 
                    int restart,
                    ...etc...etc..
                } global_t;

                extern global_t Global;

             access then would be along the lines of:

                Global.search_ok    = 1;
                Global.prog_exit    = 0;
                ...etc...etc...

             or better yet:
                
                Global.search_ok    = TRUE;
                Global.prog_exit    = FALSE;
                ...etc...etc...
        */
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
        {
			continue;
        }

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
        {
			continue;
        }

		Update ();
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void 
OpenDB (
 void)
{
    abc_dbopen (data);
    read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

    open_rec (sjcr, sjcr_list, SJCR_NO_FIELDS, "sjcr_id_no");
    open_rec (cudp, cudp_list, CUDP_NO_FIELDS, "cudp_id_no");
	OpenGlmr ();
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (sjcr);
	abc_fclose (cudp);
	GL_Close ();
	abc_dbclose (data);
}
int
spec_valid (
 int field)
{
	/*----------------------------------------------
	| Validate department Number and allow search. |
	----------------------------------------------*/
	if (LCHECK ("dept_no"))
	{
		if (SRCH_KEY)
		{
			SrchCudp (temp_str);
		}

		strcpy (cudp_rec.co_no, comm_rec.co_no);
		strcpy (cudp_rec.br_no, comm_rec.est_no);
		strcpy (cudp_rec.dp_no, local_rec.dept);
		cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
		if (cc)
		{
			/*------------------------------
			| Department Number not found. |
			------------------------------*/
			errmess (ML (mlStdMess084));
			return (EXIT_FAILURE); 
		}

		strcpy (sjcr_rec.co_no, comm_rec.co_no);
		strcpy (sjcr_rec.est_no, comm_rec.est_no);
		sprintf (sjcr_rec.dp_no, "%-2.2s", local_rec.dept);

		cc = find_rec (sjcr, &sjcr_rec, COMPARISON, "u");
		if (!cc)
		{
			if (GetGlmr (sjcr_rec.gl_wip, local_rec.wip_desc, "gl_wip"))
            {
				return (EXIT_FAILURE);
            }

			if (GetGlmr (sjcr_rec.gl_ext_sales, local_rec.es_desc, "gl_ext_sales"))
            {
				return (EXIT_FAILURE);
            }

			if (GetGlmr (sjcr_rec.gl_int_sales, local_rec.is_desc, "gl_int_sales"))
            {
                return (EXIT_FAILURE);
            }

			if (GetGlmr (sjcr_rec.gl_ext_cos, local_rec.ec_desc, "gl_ext_cos"))
            {
                return (EXIT_FAILURE);
            }

			if (GetGlmr (sjcr_rec.gl_int_cos, local_rec.ic_desc, "gl_int_cos"))
            {
                return (EXIT_FAILURE);
            }

			if (GetGlmr (sjcr_rec.gl_ext_cos_va, local_rec.ecv_desc, "gl_ext_cos_var"))
            {
                return (EXIT_FAILURE);
            }

			if (GetGlmr (sjcr_rec.gl_int_cos_va, local_rec.icv_desc, "gl_int_cos_var"))
            {
                return (EXIT_FAILURE);
            }

			new_sjcr = 0;
			entry_exit = 1;
		}
		else
        {
			new_sjcr = 1;
        }

		display_field (label ("dept_name"));
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("gl_wip"))
	{
		if (SRCH_KEY)
        {
			return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));
        }

		if (GetGlmr (sjcr_rec.gl_wip,
                       local_rec.wip_desc,
                       "gl_wip_desc"))
        {
			return (EXIT_FAILURE);
        }

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("gl_ext_sales"))
	{
		if (SRCH_KEY)
        {
            return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));
        }

		if (GetGlmr (sjcr_rec.gl_ext_sales,
                       local_rec.es_desc,
                       "gl_es_desc"))
        {
            return (EXIT_FAILURE);
        }

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("gl_int_sales"))
	{
		if (SRCH_KEY)
        {
            return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));
        }

		if (GetGlmr (sjcr_rec.gl_int_sales,
                       local_rec.is_desc,
                       "gl_is_desc"))
        {
            return (EXIT_FAILURE);
        }

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("gl_ext_cos"))
	{
		if (SRCH_KEY)
        {
            return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));
        }

		if (GetGlmr (sjcr_rec.gl_ext_cos,
                       local_rec.ec_desc,
                       "gl_ec_desc"))
        {
            return (EXIT_FAILURE);
        }

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("gl_int_cos"))
	{
		if (SRCH_KEY)
        {
            return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));
        }

		if (GetGlmr (sjcr_rec.gl_int_cos,
                       local_rec.ic_desc,
                       "gl_ic_desc"))
        {	
            return (EXIT_FAILURE);
        }

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("gl_ext_cos_var"))
	{
		if (SRCH_KEY)
        {
            return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));
        }

		if (GetGlmr (sjcr_rec.gl_ext_cos_va,
                       local_rec.ecv_desc,
                       "gl_ecv_desc"))
        {
            return (EXIT_FAILURE);
        }

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("gl_int_cos_var"))
	{
		if (SRCH_KEY)
        {
            return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));
        }

		if (GetGlmr (sjcr_rec.gl_int_cos_va,
                       local_rec.icv_desc,
                       "gl_icv_desc"))
        {
            return (EXIT_FAILURE);
        }

		return (EXIT_SUCCESS);
	}

    /*  NOTES
        this return will occur when none of the conditions checked
        by the if (..) were met. will that ever happen? 

        good place to put dubugging stuff/error code
    */
    return (EXIT_SUCCESS);             
}

int
GetGlmr (
 char *acc_code, 
 char *acc_desc, 
 char *label_name)
{
	if (!strncmp (acc_code, "0000000000000000", MAXLEVEL))
	{
		sprintf (acc_desc,"%-25.25s"," ");
		DSP_FLD (label_name);
		return (EXIT_SUCCESS);
	}

	strcpy (glmrRec.co_no, comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", MAXLEVEL, MAXLEVEL, acc_code);

	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc) 
	{
		/*------------------------
		| G/L Account not found. |
		------------------------*/
		errmess (ML (mlStdMess024));
		sleep (sleepTime);		

		return (EXIT_FAILURE);
	}
		
	if (check_class ("G/L"))
    {
        return (EXIT_FAILURE);
    }

	strcpy (acc_desc, glmrRec.desc);
	DSP_FLD (label_name);
	
    return (EXIT_SUCCESS);
}


void
Update (
 void)
{
	strcpy (sjcr_rec.co_no, comm_rec.co_no);
	strcpy (sjcr_rec.est_no, comm_rec.est_no);
	strcpy (sjcr_rec.dp_no, local_rec.dept);
	if (new_sjcr)
	{
		cc = abc_add (sjcr, &sjcr_rec);
		if (cc)
        {
		   file_err (cc, sjcr, "DBADD");
        }
	}
	else
	{
		cc = abc_update (sjcr, &sjcr_rec);
		if (cc)
        {
            file_err (cc, sjcr, "DBUPDATE");
        }
	}
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
        {
            scn_set (scn);
        }

		swide ();
		clear ();

		/*---------------------------------
		| Service Job Control Maintenance |
		---------------------------------*/
		sprintf (err_str, " %s ", ML (mlSjMess039));
		rv_pr (err_str, 50,0,1);
		line_at (1,0,132);

		box (0,3,132,10);

		line_at (6,1,131);
		line_at (20,0,132);

		sprintf (err_str, 
                 ML (mlStdMess038),
                 comm_rec.co_no,
                 comm_rec.co_name);
		print_at (21,0, "%s", err_str);
		print_at (22,0, 
                  ML (mlStdMess039),
                  comm_rec.est_no,
                  comm_rec.est_name);

		line_at (22,0,132);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}

int
check_class (
 char *msg)
{
	if (glmrRec.glmr_class [2][0] != 'P')
	{
		/*-------------------------------------
		| account is not a posting account ...|
		-------------------------------------*/
		errmess (ML (mlStdMess025));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	
	return (EXIT_SUCCESS);
}

			
/*============================================
| Search routine for Department Master File. |
============================================*/
void
SrchCudp (
 char *key_val)
{
	_work_open (2,0,40);
	strcpy (cudp_rec.co_no,comm_rec.co_no);
	strcpy (cudp_rec.br_no,comm_rec.est_no);
	sprintf (cudp_rec.dp_no,"%-2.2s",key_val);
	save_rec ("#Dp.","#Department Description");

	cc = find_rec (cudp, &cudp_rec, GTEQ, "r");
	while (!cc && 
           !strncmp (cudp_rec.dp_no, key_val,strlen (key_val)) && 
           !strcmp (cudp_rec.co_no, comm_rec.co_no) && 
           !strcmp (cudp_rec.br_no,comm_rec.est_no))
	{
		cc = save_rec (cudp_rec.dp_no, cudp_rec.dp_name);
		if (cc)	
        {
            break;
        }
		cc = find_rec (cudp, &cudp_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
    {
		return;
    }
	strcpy (cudp_rec.co_no,comm_rec.co_no);
	strcpy (cudp_rec.br_no,comm_rec.est_no);
	sprintf (cudp_rec.dp_no,"%-2.2s",temp_str);
	
    cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
	if (cc)
    {
        sys_err ("Error in cudp During (DBFIND)",cc,PNAME);
    }
}

/* [ end of file ] */
