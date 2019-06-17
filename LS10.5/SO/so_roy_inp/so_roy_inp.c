/*=====================================================================
|  Copyright (C) 1989 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( so_roy_inp.c   )                                 |
|  Program Desc  : ( Royalty File Maintenance              )          |
|---------------------------------------------------------------------|
|  Access files  :  comm, rymr, dbry,     ,     ,     ,               |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates files :  rymr,     ,                                       |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Bee Chwee Lim   | Date Written  : 02/09/88         |
|---------------------------------------------------------------------|
|  Date Modified : 15/11/88        | Modified  by  : B.C.Lim.         |
|  Date Modified : 21/12/88        | Modified  by  : B.C.Lim.         |
|  Date Modified : 27/02/89        | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (03/08/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (05/09/97)      | Modified  by  : Rowena S Maandig |
|  Date Modified : (27/10/1997)    | Modified by : Campbell Mander.   |
|                :                                                    |
|  Comments      : Change to use new screen generator.                |
|                : Change the case stmt in get_glca() i.e case '1' to |
|                : case 1 etc.                                        |
|  (03/08/90)    : General Update for New Scrgen. S.B.D.              |
|  (05/09/97)    : Incorporate  multilingual conversion.              |
|  (27/10/1997)  : SEL. 9.9.3 Update for 8 character invoice numbers. |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_roy_inp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_roy_inp/so_roy_inp.c,v 5.2 2001/08/09 09:21:48 scott Exp $";

#include <pslscr.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_cc_no"},
		{"comm_cc_name"},
	};

	int comm_no_fields = 7;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tcc_no[3];
		char	tcc_name[41];
	} comm_rec;

	/*=======================================
	| Royalty Type Master File Base Record. |
	=======================================*/
	struct dbview rymr_list[] ={
		{"rymr_co_no"},
		{"rymr_code"},
		{"rymr_desc"},
		{"rymr_qty1"},
		{"rymr_qty2"},
		{"rymr_qty3"},
		{"rymr_qty4"},
		{"rymr_qty5"},
		{"rymr_qty6"},
		{"rymr_pc1"},
		{"rymr_pc2"},
		{"rymr_pc3"},
		{"rymr_pc4"},
		{"rymr_pc5"},
		{"rymr_pc6"},
		{"rymr_stat_flag"}
	};

	int rymr_no_fields = 16;

	struct {
		char	rm_co_no[3];
		char	rm_code[10];
		char	rm_desc[41];
		float	rm_qty[6];
		float	rm_pc[6];
		char	rm_stat_flag[2];
	} rymr_rec;

	/*==============================================
	| Customer Royalty Type Master File Base Record. |
	==============================================*/
	struct dbview dbry_list[] ={
		{"dbry_co_no"},
		{"dbry_cr_type"},
		{"dbry_desc"},
		{"dbry_stat_flag"}
	};

	int dbry_no_fields = 4;

	struct {
		char	ry_co_no[3];
		char	ry_cr_type[4];
		char	ry_desc[41];
		char	ry_stat_flag[2];
	} dbry_rec;

   	int  	new_item = 0;
	int	envDbCo;
	char	branchNo[3];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	prev_code[10];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "roy_code", 4, 20, CHARTYPE, 
		"UUUUUUUUU", "          ", 
		" ", " ", "Royalty Code.", " ", 
		YES, NO, JUSTLEFT, "A", "Z", rymr_rec.rm_code}, 
	{1, LIN, "roy_desc1", 5, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Class Desc. ", " ", 
		NA, NO, JUSTLEFT, "", "", dbry_rec.ry_desc}, 
	{1, LIN, "roy_desc2", 6, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Royalty Desc.", " ", 
		NA, NO, JUSTLEFT, "", "", rymr_rec.rm_desc}, 
	{1, LIN, "qty_1", 8, 20, FLOATTYPE, 
		"NNNNNNNN.NN", "          ", 
		" ", "0", "Quantity 1.", " ", 
		YES, NO, JUSTRIGHT, "0", "99999999.99", (char *)&rymr_rec.rm_qty[0]}, 
	{1, LIN, "disc_1", 8, 55, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "0", "Royalty Percent 1  ", " ", 
		YES, NO, JUSTRIGHT, "0", "100.00", (char *)&rymr_rec.rm_pc[0]}, 
	{1, LIN, "qty_2", 9, 20, FLOATTYPE, 
		"NNNNNNNN.NN", "          ", 
		" ", "0", "Quantity 2.", " ", 
		YES, NO, JUSTRIGHT, "0", "99999999.99", (char *)&rymr_rec.rm_qty[1]}, 
	{1, LIN, "disc_2", 9, 55, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "0", "Royalty Percent 2  ", " ", 
		YES, NO, JUSTRIGHT, "0", "100.00", (char *)&rymr_rec.rm_pc[1]}, 
	{1, LIN, "qty_3", 10, 20, FLOATTYPE, 
		"NNNNNNNN.NN", "          ", 
		" ", "0", "Quantity 3.", " ", 
		YES, NO, JUSTRIGHT, "0", "99999999.99", (char *)&rymr_rec.rm_qty[2]}, 
	{1, LIN, "disc_3", 10, 55, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "0", "Royalty Percent 3  ", " ", 
		YES, NO, JUSTRIGHT, "0", "100.00", (char *)&rymr_rec.rm_pc[2]}, 
	{1, LIN, "qty_4", 11, 20, FLOATTYPE, 
		"NNNNNNNN.NN", "          ", 
		" ", "0", "Quantity 4.", " ", 
		YES, NO, JUSTRIGHT, "0", "99999999.99", (char *)&rymr_rec.rm_qty[3]}, 
	{1, LIN, "disc_4", 11, 55, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "0", "Royalty Percent 4  ", " ", 
		YES, NO, JUSTRIGHT, "0", "100.00", (char *)&rymr_rec.rm_pc[3]}, 
	{1, LIN, "qty_5", 12, 20, FLOATTYPE, 
		"NNNNNNNN.NN", "          ", 
		" ", "0", "Quantity 5.", " ", 
		YES, NO, JUSTRIGHT, "0", "99999999.99", (char *)&rymr_rec.rm_qty[4]}, 
	{1, LIN, "disc_5", 12, 55, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "0", "Royalty Percent 5  ", " ", 
		YES, NO, JUSTRIGHT, "0", "100.00", (char *)&rymr_rec.rm_pc[4]}, 
	{1, LIN, "qty_6", 13, 20, FLOATTYPE, 
		"NNNNNNNN.NN", "          ", 
		" ", "0", "Quantity 6.", " ", 
		YES, NO, JUSTRIGHT, "0", "99999999.99", (char *)&rymr_rec.rm_qty[5]}, 
	{1, LIN, "disc_6", 13, 55, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "0", "Royalty Percent 6  ", " ", 
		YES, NO, JUSTRIGHT, "0", "100.00", (char *)&rymr_rec.rm_pc[5]}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};


/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int  spec_valid (int field);
int  find_dbry (void);
void update (void);
void save_page (char *key_val);
int  heading (int scn);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char	*argv [])
{

	envDbCo = atoi(get_env("DB_CO"));

	strcpy(local_rec.prev_code,"         ");

	SETUP_SCR (vars);
	
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr();
	set_tty();
	set_masks();
	init_vars(1);

	OpenDB();

	strcpy(branchNo,(!envDbCo) ? " 0" : comm_rec.test_no);

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading(1);
		entry(1);
		if (prog_exit)
			/*shutdown_prog(); <-- old line */
            break; /* <-- new line */

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading(1);
		scn_display(1);
		edit(1);

		if (restart)
			continue;

		update();
	}
	shutdown_prog();
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
	abc_dbopen("data");
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec("rymr",rymr_list,rymr_no_fields,"rymr_id_no");
	open_rec("dbry", dbry_list, dbry_no_fields, "dbry_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("rymr");
	abc_fclose("dbry");
	abc_dbclose("data");
}

int
spec_valid (
 int field)
{
	int	i;
	int	indx;

        if (strcmp(FIELD.label,"roy_code") == 0)
        {
		if (last_char == SEARCH)
		{
			save_page(temp_str);
			return(0);
		}

		strcpy(rymr_rec.rm_co_no,comm_rec.tco_no);
		cc = find_rec("rymr",&rymr_rec,COMPARISON,"u");
		if (cc)
		{
			cc = find_dbry();
			if (cc)
				return(1);
			display_field(label("roy_desc1"));
			vars[label("roy_desc2")].required = YES;
			new_item = 1;
		}
		else
		{
			cc = find_dbry();
			if (cc)
				return(1);
			vars[label("roy_desc2")].required  = NA;
			display_field(label("roy_desc1"));
			display_field(label("roy_desc2"));
			new_item = 0;
			entry_exit = 1;
		}
	}

	if (strncmp(FIELD.label,"qty_",4) == 0)
	{
		indx = FIELD.label[4] - '1';

		rymr_rec.rm_qty[indx] = (float) (twodec(rymr_rec.rm_qty[indx]));
		
		if (rymr_rec.rm_qty[indx] == 0.00)
		{
			entry_exit = 1;
			for (i = indx;i < 6;i++)
			{
				rymr_rec.rm_qty[i] = 0.00;
				rymr_rec.rm_pc[i] = 0.00;
			}

			for (i = label("qty_1");i <= label("disc_6");i++)
				display_field(i);
			return(0);
		}


		if (prog_status == ENTRY)
		{
			if (indx == 0)
				return(0);

			if (rymr_rec.rm_qty[indx] <= rymr_rec.rm_qty[indx - 1])
			{
				/*print_mess("Qty must be greater than Previous Qty");*/
				print_mess(ML(mlSoMess305));
				return(1);
			}
			return(0);
		}
		else
		{
			if (indx != 0 && rymr_rec.rm_qty[indx -1] == 0.00)
			{
				/*print_mess("Previous Qty must is Zero");*/
				print_mess(ML(mlSoMess334));
				return(1);
			}

			if (indx != 0 && rymr_rec.rm_qty[indx] <= rymr_rec.rm_qty[indx - 1])
			{
				/*print_mess("Qty must be greater than Previous Qty");*/
				print_mess(ML(mlSoMess305));
				return(1);
			}

			if (indx != 5 && rymr_rec.rm_qty[indx + 1] != 0.00 && rymr_rec.rm_qty[indx] >= rymr_rec.rm_qty[indx + 1])
			{
				/*print_mess("Qty must be less than Previous Qty");*/
				print_mess(ML(mlSoMess306));
				return(1);
			}
			return(0);
		}
	}
	return(0);
}

int
find_dbry (
 void)
{
	strcpy(dbry_rec.ry_co_no,comm_rec.tco_no);
	sprintf(dbry_rec.ry_cr_type,"%-3.3s",rymr_rec.rm_code);
	cc = find_rec("dbry",&dbry_rec,COMPARISON,"r");
	if (cc)
	{
		/*sprintf(err_str,"Customer Royalty Type %s is not on file.",dbry_rec.ry_cr_type);*/
		print_mess(ML(mlSoMess307));
		sleep(3);
		return(1);
	}
	return(0);
}

void
update (
 void)
{
	strcpy(rymr_rec.rm_co_no,comm_rec.tco_no);
	strcpy(rymr_rec.rm_stat_flag,"0");
	if (new_item)
	{
		cc = abc_add("rymr",&rymr_rec);
		if (cc)
			sys_err("Error in rymr During (DBADD)",cc,PNAME);
	}
	else
	{
		cc = abc_update("rymr",&rymr_rec);
		if (cc)
			sys_err("Error in rymr During (DBUPDATE)",cc,PNAME);
	}
	abc_unlock("rymr");
	strcpy(local_rec.prev_code,rymr_rec.rm_code);
}

void
save_page (
 char *key_val)
{
	work_open();
	strcpy(rymr_rec.rm_co_no,comm_rec.tco_no);
	sprintf(rymr_rec.rm_code,"%-9.9s",key_val);
	cc = find_rec("rymr", &rymr_rec, GTEQ,"r");
	save_rec("#Roy Code ", "#Royalty Description   ");
	while (!cc && !strncmp(rymr_rec.rm_code,key_val,strlen(key_val)) && !strcmp(rymr_rec.rm_co_no, comm_rec.tco_no))
	{
		cc = save_rec(rymr_rec.rm_code, rymr_rec.rm_desc);
		if (cc)
			break;

		cc = find_rec("rymr", &rymr_rec, NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy(rymr_rec.rm_co_no,comm_rec.tco_no);
	sprintf(rymr_rec.rm_code,"%-9.9s",temp_str);
	cc = find_rec("rymr", &rymr_rec, COMPARISON,"r");
	if (cc)
	       sys_err("Error in rymr During (DBFIND)",cc,PNAME);
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();
		rv_pr(ML(mlSoMess180),28,0,1);
		print_at(0,55,ML(mlSoMess308),local_rec.prev_code);

		move(0,1);
		line(80);

		box(0,3,80,10);

		move(1,7);
		line(79);

		move(0,18);
		line(80);
		/*print_at(19,0,"Company   : %s %s",comm_rec.tco_no,comm_rec.tco_name);
		print_at(20,0,"Branch    : %s %s",comm_rec.test_no,comm_rec.test_name);
		print_at(21,0,"Warehouse : %s %s",comm_rec.tcc_no,comm_rec.tcc_name);
		*/
		print_at(19,0,ML(mlStdMess038),comm_rec.tco_no,comm_rec.tco_name);
		print_at(20,0,ML(mlStdMess039),comm_rec.test_no,comm_rec.test_name);
		print_at(21,0,ML(mlStdMess099),comm_rec.tcc_no,comm_rec.tcc_name);
		move(0,22);
		line(80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}
