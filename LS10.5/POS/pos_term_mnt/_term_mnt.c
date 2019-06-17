/*=====================================================================
|  Copyright (C) 1986 - 1998 SOFTWARE ENGINEERING LIMITED.            |
|=====================================================================|
|  Program name          :  (pos_term_mnt.c           )               |
|  Program Description   :  (POS terminal maintenance )               |
|---------------------------------------------------------------------|
| Author         : Primo O. Esteria  : Date written  : 26/08/1998     |
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: _term_mnt.c,v $
| Revision 5.1  2001/08/09 09:50:25  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:12:27  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:41  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:16  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:49  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  1999/11/19 06:26:22  scott
| Updated for warnings.
|
| Revision 1.10  1999/11/17 06:40:32  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.9  1999/10/16 01:49:13  scott
| Updated from ansi
|
| Revision 1.8  1999/06/18 02:05:27  scott
| Updated for log.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _term_mnt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/POS/pos_term_mnt/_term_mnt.c,v 5.1 2001/08/09 09:50:25 scott Exp $";

#include <pslscr.h>
#include <ring_menu.h>

#define JUST_READY	    0
#define ADD_NEW		    1
#define EDIT_REC	    2
#define DELETE_REC	    3
#define MAX_ROW			9 
#define MAX_TERMINALS   50 

char 	*posterm = "posterm",
		*comr    = "comr",
	    *esmr    = "esmr",	
		*ccmr    = "ccmr";
 
struct dbview esmr_list[] =
{
   {"esmr_co_no"},
   {"esmr_est_no"},
   {"esmr_est_name"}
};

int esmr_no_fields = 3;

struct 
{
   char co_no[3];
   char est_no[3];
   char est_name[41];
} esmr_rec;

struct dbview ccmr_list[] =
{
   {"ccmr_co_no"},
   {"ccmr_est_no"},
   {"ccmr_cc_no"},
   {"ccmr_name"}     /* branch_name */
};

int ccmr_no_fields = 4;

struct
{
   char co_no[3];
   char est_no[3];
   char cc_no[3];
   char cc_name[41];
} ccmr_rec;

struct dbview comr_list[] =
{
   {"comr_co_no"},
   {"comr_co_name"},
};

int comr_no_fields = 2;

struct 
{
   char co_no[3];
   char co_name[41];
} comr_rec;

struct dbview posterm_list[] =
{
   {"pos_no"},
   {"ip_address"},
   {"description"},
   {"co_no"},
   {"br_no"},
   {"wh_no"},
   {"last_user"},
   {"last_logoff"},
   {"last_login"}
};

int posterm_no_fields= 9;

int   cur_row = 5;
char  inp_buf[256];
char  res_buf[256];
char  *val_chrs;
char  com_no[3];
char  bra_no[3];
char  war_no[3];
int   pos_terminals[MAX_TERMINALS];
int   row_used;
int   start_data_row =0 ;
int   act_data;
int   SUPER_USER = FALSE;
int   mode =0;

struct 
{
   int  pos_term_no;
   char ip_address[16];
   char description[41];
   char co_no[3];
   char br_no[3];
   char wh_no[3]; 
   char last_user[15];
   long last_logoff;
   long last_login;

} posterm_rec;

int   Add(void);
int   Edit(void);
int   Delete(void);
int   Update(void);
int   Prev(void);
int   Next(void);
int   Up(void);
int   Down(void);
void  screen(void);
int   get_inp(int , int , int );
char  *GLookup(int);
int   DisplayCompany(char *);
int   DisplayBranch(char *, char *);
int   DisplayWarehouse(char *, char *, char *);
char  *GetBranch(void);
char  *GetWarehouse(void);
void  Getint(int ,int, int *, int *); 
void  Getalpha (int, int, char *, char *, int);
int   main (int, char *c []);
void  GetStatus (void);
void  GetCommands (void);
void  open_db (void);
void  close_db (void);
void  DisplayTerminals (void);
void  GetRecord (int);

menu_type mmenu [] = 
{
 {"   ","",_no_option,"",255,SHOW},
 {"Add","     Add new POS terminal", Add,"Aa",},
 {"Edit","     Modify current record",Edit,"Ee",},
 {"Delete","     Delete existing POS terminal",Delete,"Dd",},
 {"Update","     Update last operation",Update,"Uu",},
 {"Prev Scn","     Previous screen",Prev,"Pp",},
 {"Next Scn","     Next screen",Next,"Nn",},
 {"Up","     ", Up,"Uu",UP_KEY,},
 {"Down","     ", Down,"wW",DOWN_KEY,},
 {"Quit","     Exit",_no_option,"Qq",FN16,EXIT|SELECT|SHOW },
 {"",}
};

int 
main (
 int argc, 
 char *argv [])
{
	if (!getuid () || !geteuid ())
	{
	   SUPER_USER = TRUE;
	}
	else
	{
       mmenu [1].flag = mmenu [2].flag = mmenu [3].flag = mmenu [4].flag =
		  DISABLED;
	}


	init_scr ();
    
	open_db ();
    
	set_tty ();

	crsr_off ();

	screen (); 
  
	DisplayTerminals ();

	GetStatus ();
    
    GetRecord (1); 

	run_menu (mmenu,"",15); 
    
	close_db ();
 
	crsr_on ();
 
    rset_tty ();
 
    clear ();

	return EXIT_SUCCESS;
}

void 
screen (
 void)
{
    char *title = "POS Terminal Maintenance";

    clear ();

	rv_pr (title,40-strlen(title)/2,0,1);

    line_at (1,0,80);   
    box (2,2,76,11);
    print_at (3,3,"POS #");
	print_at (3,10,"|");
	print_at (3,12,"IP Address");
	print_at (3,26, "|");
	print_at (3,28,"Description");
	print_at (3,62, "|");
	print_at (3,64, "Co");
	print_at (3,67, "|");
	print_at (3,69, "Br");
	print_at (3,72, "|");
	print_at (3,74, "Wh");
    line_at (4,3,75);

	box (2,17,76,23); 
    
	print_at (18,4,"POS #");
	print_at (19,4,"Description");
	print_at (20,4,"Company");
	print_at (21,4,"Branch");
	print_at (22,4,"Warehouse");
	print_at (18,40,"IP Address"); 
   
} 

void 
open_db (
 void)
{
	abc_dbopen ("data");
	open_rec (posterm,posterm_list,posterm_no_fields,"pos_no");
	open_rec (comr,comr_list,comr_no_fields,"comr_co_no");
	open_rec (esmr,esmr_list,esmr_no_fields,"esmr_id_no");
    open_rec (ccmr,ccmr_list,ccmr_no_fields,"ccmr_id_no");
}

void 
close_db (
 void)
{
   abc_fclose (esmr);
   abc_fclose (ccmr);
   abc_fclose (comr);
   abc_fclose (posterm);
   abc_fclose ("data");
}

void 
DisplayTerminals (
 void)
{
  int cc;
  int row = 5;
  int i=0,j=0,r=0;

  row_used = 5;

  posterm_rec.pos_term_no = 0;

  cc = find_rec (posterm,&posterm_rec,GTEQ,"u");
  act_data = 0;

  while (!cc)
  {
    if ( r >= start_data_row)
    {
		pos_terminals [r] = posterm_rec.pos_term_no;
    
		print_at (row, 3,"%d",posterm_rec.pos_term_no);
		print_at (row,12,"%15.15s",posterm_rec.ip_address);
		print_at (row,28,"%40.40s",posterm_rec.description);
		print_at (row,64, "%2.2s", posterm_rec.co_no );
		print_at (row,69, "%2.2s", posterm_rec.br_no);
		print_at (row,74, "%2.2s", posterm_rec.wh_no);
    	row++;
		act_data++;
	}

    row_used++; 
   
	r++;
	i++;
    j++;

	if (row - 5 == 9)
	/* if (j == MAX_ROW) */
	{
		break;
	}

	cc = find_rec (posterm,&posterm_rec,NEXT,"u");
  }
  
  cur_row = 5;

  GetRecord (1);

}
   
void 
GetStatus (
 void)
{
}

int 
Add(
 void)
{
   char desc [41],
		ip [15],
		co [3],
		br [3],
		wh [3];
   int i = 0,
	   cc;

   strcpy (desc,"                                       ");
   strcpy (ip,"               ");
   strcpy (co,"  ");
   strcpy (br,"  ");
   strcpy (wh,"  ");

   /* while (!do_exit) */ 
   {
	  while (1)
	  {
 	     Getint (19,18,&i,&i);
         if (i == 0)
		 {
			errmess ("Invalid terminal number");
		    sleep(2);	
			clear_mess();
			box(2,17,76,23); 
			continue;
		 }
         posterm_rec.pos_term_no = i;
		 cc = find_rec(posterm,&posterm_rec,EQUAL,"r");
		 if (!cc)
		 {
	        errmess("You may create a duplicate terminal no.");	
            sleep(2); 
			clear_mess();
			box(2,17,76,23); 
		    continue; 
		 }

		 break;
      }

  	  Getalpha(53,18,ip,ip,15);
      Getalpha(19,19,desc,desc,40);
      
	  while (1)
	  {
	     Getalpha(19,20,co,co,2);
		 clip(co);
		 if (strlen(co) == 0)
         {
			break;
		 }
		 if (DisplayCompany(co))
		 {
			break;
		 }
		 else
		 {
			putchar(BELL);
		 }
      }
      
	  strcpy(com_no,co);

	  while (1)
	  {
          Getalpha(19,21,br,br,2);
		  clip(br);

		  if (strlen(br) == 0)
		  {
			 break;
		  }

		  if (DisplayBranch(co,br))
		  {
			  break;
		  }
		  else
		  {
			  putchar(BELL);
		  }
	  }

	  strcpy(bra_no,br);

	  while (1)
	  {
          Getalpha(19,22,wh,wh,2);
          clip(wh);

		  if (strlen(wh) == 0)
		  {
			   break;
		  }
          
		  if (DisplayWarehouse(co,br,wh))
		  {
			   break;
		  }
		  else
		  {
			 putchar(BELL);
		  }
	  }
   }
 

   posterm_rec.pos_term_no = i;
   strcpy(posterm_rec.ip_address,ip);
   strcpy(posterm_rec.description,desc);
   strcpy(posterm_rec.co_no,co);
   strcpy(posterm_rec.br_no,br);
   strcpy(posterm_rec.wh_no,wh);
   strcpy(posterm_rec.last_user," ");
   posterm_rec.last_logoff = 0;
   posterm_rec.last_login  = 0;

   mode = ADD_NEW;

   return 0;
}

int 
Edit(
 void)
{
   while (1)
   {
      Getint(19,18,&posterm_rec.pos_term_no,&posterm_rec.pos_term_no);

	  if (posterm_rec.pos_term_no == 0)
	  {
		 errmess("Invalid POS terminal no.");
		 sleep(2);
		 clear_mess();
		 box(2,17,76,23); 
		 continue;
	  }
	  break;
   }

   Getalpha(53,18,posterm_rec.ip_address,posterm_rec.ip_address,15);
   Getalpha(19,19,posterm_rec.description,posterm_rec.description,40);
   
   while (1)
   {
	     Getalpha(19,20,posterm_rec.co_no,posterm_rec.co_no,2);
		 clip(posterm_rec.co_no);
		 if (strlen(posterm_rec.co_no) == 0)
         {
			break;
		 }
		 if (DisplayCompany(posterm_rec.co_no))
		 {
			break;
		 }
   }

   while (1)
   {
         Getalpha(19,21,posterm_rec.br_no,posterm_rec.br_no,2);
         clip(posterm_rec.br_no);
		 if (strlen(posterm_rec.br_no) == 0)
		 {
			break;
		 }

		 if (DisplayBranch(posterm_rec.co_no, posterm_rec.br_no))
		 {
			break;
		 }


   }

   while (1)
   {
         Getalpha(19,22,posterm_rec.wh_no,posterm_rec.wh_no,2);
		 clip(posterm_rec.wh_no);
         if (strlen(posterm_rec.wh_no)== 0)
		 {
			 break;
		 }

		 if (DisplayWarehouse(posterm_rec.co_no,
							  posterm_rec.br_no,
							  posterm_rec.wh_no))
		 {
		     break;						
		 }

   }

   strcpy(posterm_rec.last_user," ");
   posterm_rec.last_logoff = 0;
   posterm_rec.last_login  = 0;
   mode = EDIT_REC;
   
   return 0;
}

void 
GetRecord (
 int rv)
{
   int cc;
   char buf[80];

   posterm_rec.pos_term_no = pos_terminals[cur_row-5+start_data_row];
   
   cc = find_rec(posterm,&posterm_rec,EQUAL, "u");

   if (!cc)
   {
          sprintf(buf,"%-5d    %15.15s %35.35s %2.2s   %2.2s   %2.2s",
		  posterm_rec.pos_term_no, posterm_rec.ip_address, 
		  posterm_rec.description,
		  posterm_rec.co_no,
		  posterm_rec.br_no,
		  posterm_rec.wh_no);

	  rv_pr(buf,3,cur_row,rv);

	  if (rv)
	  {
   		 print_at(18,19,"%-d",posterm_rec.pos_term_no);
   		 print_at(18,53,posterm_rec.ip_address);
   		 print_at(19,19,posterm_rec.description);
   		 print_at(20,19,posterm_rec.co_no);
   		 print_at(21,19,posterm_rec.br_no);
   		 print_at(22,19,posterm_rec.wh_no);

		 DisplayCompany(posterm_rec.co_no);
         DisplayBranch(posterm_rec.co_no,posterm_rec.br_no);
         DisplayWarehouse(posterm_rec.co_no,
						  posterm_rec.br_no,
						  posterm_rec.wh_no);
	  }
   }
}

int 
Delete (
 void)
{ 
   int cc;
   
   posterm_rec.pos_term_no = pos_terminals[cur_row-5];
   
   cc = find_rec(posterm,&posterm_rec,EQUAL,"u");
   if (!cc)
   {
       abc_delete(posterm);
   }

   screen();
   DisplayTerminals();
   mode = DELETE_REC;

   return 0;
}

int 
Next(
 void)
{
	mode = JUST_READY;
    if (act_data == 9)
	{
		start_data_row += 9;
	    screen();
		DisplayTerminals();
    	GetRecord(1);
    }

	return 0;
}

int 
Update (
 void)
{
   int cc = 0;

   switch(mode)
   {
	  case ADD_NEW:
		 if ((cc = abc_add(posterm,&posterm_rec))) 
		 {
			errmess("Add record failed");
		 }
		 break;
	  case EDIT_REC:
		 if ((cc = abc_update(posterm,&posterm_rec)))
		 {
		   errmess("Modify record failed");
		 }
         break; 
	  case DELETE_REC:
		 if ((cc = abc_delete(posterm)))
		 { 
			errmess("Unable to delete record");
		 }
         break;
      default:
		 errmess("Get into Add or Edit to update a record");
		 break;
   }

   if (cc)
   {
	 sleep(1);
   }

   mode = JUST_READY;

   screen();
   DisplayTerminals();

   return 0;
}

int 
Prev (
 void)
{
   mode = JUST_READY; 
   
   if (start_data_row > 0)
   {
      start_data_row -= 9;
	  screen ();
	  DisplayTerminals ();
      GetRecord(1);
   }
   
   return 0;
}

void 
GetRow (
 int row)
{
}

int 
Up(
 void)
{
   if (cur_row > 5)
   {
	   GetRecord(0);
       cur_row--;
   }
   else
   {
	   putchar(BELL);
   }

   GetRecord(1);

   mode = JUST_READY;

   return 0;
}

int 
Down (
 void)
{
  /* if (cur_row - 5 < MAX_ROW) */
  if (cur_row - 4 < act_data)
  {
	  GetRecord(0);
      cur_row++;
  }
  else
  {
	  putchar(BELL);
  }
 
  GetRecord(1);

  mode = JUST_READY;

  return 0;
}

void 
GetCommands (
 void)
{
}

int 
DisplayCompany(
 char *co)
{
	int i;
	
	strcpy(comr_rec.co_no,co);
    if (!find_rec(comr,&comr_rec,EQUAL,"r"))
	{
	   print_at(20,23,comr_rec.co_name);
	   i = 1;
	}
	else
	{
	   print_at(20,23,"%-40.40s", " ");
	   i = 0;
    }

	return i;
     
}

int 
DisplayBranch (
 char *co, 
 char *br)
{
   int ret;

   strcpy(esmr_rec.co_no,co);
   strcpy(esmr_rec.est_no,br);

   if (!find_rec(esmr,&esmr_rec,EQUAL,"r"))
   {
	  print_at (21,23,esmr_rec.est_name);
	  ret = 1;
   }
   else
   {
	  print_at(21,23,"%-40.40s", " ");
	  ret = 0;
   }

   return ret;
}


int 
DisplayWarehouse(
 char *co, 
 char *br, 
 char *wh)
{
    int ret;

	strcpy(ccmr_rec.co_no,co);
	strcpy(ccmr_rec.est_no,br);
	strcpy(ccmr_rec.cc_no,wh);

	if (!find_rec(ccmr,&ccmr_rec,EQUAL,"r"))
	{
	   ret = 1;
	   print_at(22,23,ccmr_rec.cc_name);
	}
	else
	{ 
	   print_at(22,23,"%-40.40s", " ");
	   ret = 0;
	}

	return ret;
}


/* adopted from pinform */
void 
Getalpha (
 int x, 
 int y, 
 char *cptr, 
 char *cres, 
 int len )
{
	 int cc;
	 val_chrs = " !\"#$%&'()*+,-./0123456789;:<=>?@ABCDEF"
			 	      "GHIJKLMNOPQRSTUVWXYZ[\\]"
			          "^_`abcdefghijklmnopqrstuvwxyz{|}~";
	 
	 strcpy (res_buf, cres);
	 strcpy (inp_buf, cptr);
	 cc = get_inp (x, y, len);
	 if (cc != FN1)
	 {
		 sprintf (cptr, "%-*.*s", len, len, inp_buf);
	 }
 }

/* adopted from pinform */
int 
get_inp (
 int x, 
 int y, 
 int len)
{
	int j,
	init = TRUE,
			pos,
			chr,
			insert = FALSE,
			do_exit = FALSE;

			print_at (y, x, inp_buf);
			move (x, y);
  		    pos = 0;
			crsr_on ();
			while (!do_exit)
			{
				chr = getkey ();
			    last_char = chr;
	   	        switch (chr)
				{
					case    INSCHAR:
					case    INSLINE:
							init = FALSE;
							insert = TRUE;
	 					    break;

					case    DELCHAR:
					case    DELLINE:
							init = FALSE;
							insert = FALSE;
							break;
                    case    FN4:
							/*
							res = GLookup(y);
							strcpy(inp_buf,res);
							move(x, y);
							*/
							break;
					case    FN2:
							strcpy (inp_buf, res_buf);
  						    print_at (y, x, inp_buf);
							last_char = DOWN_KEY;
					case    FN1:
					case    FN16:
					case    UP_KEY:
					case    DOWN_KEY:
					case    '\r':
							do_exit = TRUE;
							break;

					case    LEFT_KEY:
					case    BS:
							init = FALSE;
	  					    if (pos > 0)
                            {
							    putchar (BS);
							    pos--;
							}
							else
								putchar (BELL);
								break;

					case    RIGHT_KEY:
							init = FALSE;
				 		    if (pos < len)
							{
								pos++;
								move (x + pos, y);
							}
							else
								putchar (BELL);
							break;

					case    0x18:       /* Ctrl (X) */
							init = FALSE;
							for (j = pos; j < len; j++)
								inp_buf[j] = inp_buf[j + 1];
							inp_buf[len] = ' ';
																											print_at (y, x, "%-*.*s", len, len, inp_buf);
							move (x + pos, y);
  						    break;

					default:
					  	   if (strchr (val_chrs, chr) && pos < len)
					 	   {
								if (init)
								{
									sprintf (inp_buf, "%-*.*s", len, len, " ");
									print_at (y, x, inp_buf);
									move (x, y);
									init = FALSE;
								}

 		                        if (insert)
								{
									for (j = len; j > pos; j--)
										inp_buf[j] = inp_buf[j - 1];
									inp_buf[pos] = chr;
									print_at (y, x, "%-*.*s",len, len, inp_buf);
							  	    pos++;
									move (x + pos, y);
								}
								else
								{
									inp_buf[pos] = chr;
									pos++;
						  		    putchar (chr);
								}
							}
							else
							{
								init = FALSE;
								putchar (BELL);
							}
							break;
					}
																							}
			crsr_off ();
			dflt_used = init;
			return (chr);
}


void 
Getint (
 int x, 
 int y, 
 int *iptr, 
 int *ires)
{
	int cc;

	val_chrs = "-0123456789";
	sprintf (inp_buf, "%-6d", *iptr);
	sprintf (res_buf, "%-6d", *ires);
	cc = get_inp (x, y, 6);
	if (cc != FN1)
	{
		*iptr = atoi (inp_buf);
	}
}

char *
GLookup (
 int row)
{
   int 	  i = cur_row;
   static char result[41];
   /* company */
   if (row == 20)
   {
   }
   /* branch */
   else if (row == 21)
   {
	   strcpy(result,GetBranch());
   }
   /* warehouse */ 
   else if (row == 22)
   {
	   strcpy(result,GetWarehouse());
   }
   
   screen();

   DisplayTerminals();
   cur_row = i;

   return result;
}
 

char *
GetBranch (
 void)
{
   int cc; 
   static char buf[3];

   strcpy(esmr_rec.co_no,com_no);
   strcpy(esmr_rec.est_no,"");
 
   work_open();
   save_rec("#Branch","#Name");

   cc = find_rec(esmr,&esmr_rec,GTEQ,"r");
   
   while (cc == 0 && strcmp(esmr_rec.est_no,com_no) == 0)
   {
      cc = save_rec(esmr_rec.est_no,esmr_rec.est_name);

	  if (cc)
	  {
		 break;
	  }

	  cc = find_rec(esmr, &esmr_rec,NEXT, "r"); 
   }
   
   cc = disp_srch();
   
   sprintf(buf,"%-2.2s",temp_str);
   work_close();
    
   return buf;
}

char *
GetWarehouse (
 void)
{
	int cc;
    static char buf[3];

	strcpy(ccmr_rec.co_no,com_no);
	strcpy(ccmr_rec.est_no,bra_no);
	strcpy(ccmr_rec.cc_no,"");

    work_open();

    save_rec("#Warehouse No","#Name");    

	cc = find_rec(ccmr,&ccmr_rec,GTEQ,"r");

	while (cc == 0 && 
		   strcmp(ccmr_rec.co_no,com_no)== 0 && 
		   strcmp(ccmr_rec.est_no,bra_no) == 0)
	{
	   cc = save_rec(ccmr_rec.cc_no, ccmr_rec.cc_name);

	   if (cc)
	   {
		  break;
	   }

	   cc = find_rec(ccmr,&ccmr_rec, NEXT, "r");
	}
     
    cc = disp_srch();
    
	sprintf(buf,"%-2.2s",temp_str);
    work_close();
     
    return buf;
}

/**/
