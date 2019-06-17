/*=====================================================================
|  Copyright (C) 1996 - 1999 SOFTWARE ENGINEERING LIMITED.            |
|=====================================================================|
|  Program Name  :  ( pos_monitor.c  )                                |
|  Description   :  ( POS terminals monitor  )                        |
|---------------------------------------------------------------------|
| Author        :  Primo O. Esteria    : Date written  : 27/08/1998   |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: _monitor.c,v $
| Revision 5.1  2001/08/09 09:50:22  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:12:25  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:38  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:15  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:48  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  2000/02/18 02:22:08  scott
| Updated to fix small compile warings errors found when compiled under Linux.
|
| Revision 1.8  1999/11/19 06:16:08  scott
| Updated for warning errors.
|
| Revision 1.7  1999/11/17 06:40:32  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.6  1999/10/16 01:49:09  scott
| Updated from ansi
|
| Revision 1.5  1999/06/18 02:05:22  scott
| Updated for log.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _monitor.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/POS/pos_monitor/_monitor.c,v 5.1 2001/08/09 09:50:22 scott Exp $";

#include <pslscr.h>
#include <ring_menu.h>
#include <std_decs.h>

#define TOP_ROW				4	
#define MAX_TERMINALS		50
#define SEND				1
#define RECEIVE				2
#define PAGE_LEN			16

char *data    = "data",
	 *posterm = "posterm",
	 *comr    = "comr";

int  start_data_row = 0,
  	 act_data,
  	 mode,
  	 page_no = 0,
  	 row_displayed;

int pos_terminals [MAX_TERMINALS],
	cur_row,
	posterm_fields_no = 7;

struct dbview posterm_list [] =
{
   {"pos_term_no"},
   {"co_no"},
   {"br_no"},
   {"wh_no"},
   {"last_user"},
   {"last_logoff"},
   {"last_login"}
};

struct
{
  int pos_term_no;
  char co_no [3];
  char br_no [3];
  char wh_no [3];
  char last_user [15];
  long last_logoff;
  long last_login;

} posterm_rec;

struct dbview comr_list [] =
{
  {"comr_co_no"},
  {"comr_co_name"}
};

int comr_fields_no = 2;

struct
{
  char co_no [3];
  char co_name [41];
} comr_rec;

/* prototypes */
int main (int argc, char *argv []);
void screen (void);
int  Refresh (void);
int  SendLog (void);
int  ReceiveLog (void);
int  DataStatus (void);
int  Up (void);
int  Down (void);
int  Next (void);
int  Prev (void);
void DisplayTerminals (void);
void open_db (void);
void close_db (void);
void GetRecord (int);
char *GetPosStatus (int);
char *GetCompany (void);
void GetSendLog (int);
void GetReceiveLog (int);
void DisplaySLog (void);
void DisplayRLog (void);
int  NextPage (void);
int  PrevPage (void);

menu_type mmenu [] =
{
  {"Refresh","Refresh view",Refresh,"rR",},
  {"Send log", "View send log details", SendLog,"sS",},
  {"receiVe log","Receive log details", ReceiveLog,"vV",},
  {"Data status", "Data status",DataStatus,"dD",},
  {"Up"," ",Up,"uU",UP_KEY},
  {"doWn"," ",Down,"wW",DOWN_KEY},
  {"Next","Next page",Next,"nN",},
  {"Prev","Previous page", Prev,"pP",},
  {"Quit"," ",_no_option,"qQ",FN16,EXIT|SELECT|SHOW},
  {"",}
};
 
menu_type mmenuLog [] =
{
  {"Next","Next page on log file",NextPage,"nN", },
  {"Prev","Previous page on log file", PrevPage, "pP",},
  {"Quit","Back to main menu",_no_option,"qQ",FN16,EXIT|SELECT|SHOW},
  {"",}
};

int 
main (
 int argc, 
 char *argv [])
{
   init_scr ();

   crsr_off ();

   set_tty ();

   screen ();

   open_db ();
   
   DisplayTerminals ();
 
   GetRecord (1);

   run_menu (mmenu,"",20);
 
   close_db ();

   rset_tty ();
   
   crsr_on ();
   
   clear ();

   return EXIT_SUCCESS;
}


void
screen (
 void)
{
   char *title = "POS Terminal Monitoring";

   clear ();

   rv_pr (title,40-strlen (title)/2,0,1);
   
   line_at (1,0,80);
   
   box (0,1,79,17);
   print_at (2,2,"POS #");
   print_at (2,10,"|");
   print_at (2,12,"Status");
   print_at (2,37,"|");
   print_at (2,39,"Last User");
   print_at (2,51,"|");
   print_at (2,53,"Last Login");
   print_at (2,65, "|");
   print_at (2,67, "Last Logoff");
   line_at (3,1,78); 
   line_at (22,0,80);
   print_at (23,1,"Company : ");
} 

int 
Refresh (
 void)
{
   screen ();
   DisplayTerminals ();
   GetRecord (1);

   return 0;
}

int 
SendLog (
 void)
{
   
   clear ();
   screen ();

   GetSendLog (0); 

   return 0;
}

void 
GetSendLog (
 int m)
{
    char 	path [256];
    FILE 	*fp;
    char    temp [81];
    int     i;
    int     j; 
    int     x;

	strcpy (path,getenv ("PROG_PATH"));
	
	sprintf (path,"%-s/BIN/LOG/POS/upldData%03d.log",
		 path,
		 posterm_rec.pos_term_no);
   
    box (0,3,79,15);
    
	fp = fopen (path,"r");

	if (fp)
	{
		memset (temp,0,81);
        
		i = 0;
        j = 0;
        x = 0;

        while (!feof (fp))
		{
           for (i=0; i < 81; i++)
		   {
               temp [i] = getc (fp);
			   if (temp [i] == '\n' || feof (fp))
			   {
				  if (j >= page_no*PAGE_LEN )
				  {
					temp [i+1] = '\0';
				    print_at (4+x,1, temp); 
				    memset (temp,0,81);
					x++;
					row_displayed = x;
			 	  }
			   }
		   }
           
		   j++;

		   if ( x == 14)
		   {
			  break;
		   }
		}

        mode = SEND;
 
		if (m == 0)
		{
        	run_menu (mmenuLog,"[Send Log]",20);
        }
	}
	else
	{
	    print_at (4,2,"Transmit log file error"); 
	}
   
	fclose (fp);
}

void 
DisplayRLog (
 void)
{
   GetReceiveLog (1);
}

void 
DisplaySLog (
 void)
{
   GetSendLog (1);
}

int 
NextPage (
 void)
{
   page_no ++;

   if (mode == SEND)
   {
	  DisplaySLog ();
   }
   else
   { 
	  DisplayRLog ();
   }
	
   return 0;
}

int 
PrevPage (
 void)
{
   if (page_no > 0)
   {
      page_no--;
   }

   if (mode == SEND)
   { 
	  DisplaySLog ();  
   }
   else
   {
	  DisplayRLog ();
   }

   return 0;
}

void 
GetReceiveLog (
 int m)
{
    char   path [256];
    FILE   *fp;
    char   temp [81];
    int    i, j, x;

	strcpy (path,getenv ("PROG_PATH"));
	sprintf (path,"%-s/BIN/LOG/POS/srvdTrans%03d.log",
			path,
			posterm_rec.pos_term_no);
  
    box (0,3,79,15);

	fp = fopen (path,"r");

	if (fp)
	{
		memset (temp,0,81);
        
		i = 0;
        j = 0;
        x = 0;

        while (!feof (fp))
		{
           for (i=0; i < 81; i++)
		   {
               temp [i] = getc (fp);
			   if ( temp [i] == '\n' || feof (fp))
			   {
				  temp [i] = '\0';
				  if (j >= page_no*PAGE_LEN )
				  {
				     print_at (4+x,1, temp);
				     memset (temp,0,81);
					 x++;
					 row_displayed = x;
				  }
			   }
		   }

		   j++;

		   if (x == 14)
		   {
			   break;
		   }
		}
        
		mode = RECEIVE;
        if (m == 0)
		{
           run_menu (mmenuLog,"[Receive Log]",20);
		}
	}
	else
	{
	     print_at (4,2,"Receive log file error");
	}
 
}

int 
ReceiveLog (
 void)
{
   clear ();
   
   screen ();

   GetReceiveLog (0);
   
   return 0;
}

int 
DataStatus (
 void)
{
   return 0;
}

int 
Up (
 void)
{
   if (cur_row > TOP_ROW)
   {
      GetRecord (0);
	  cur_row--;
   }
   else
   {
	  putchar (BELL);  
   }
   
   GetRecord (1);

   return 0;
}

int 
Down (
 void)
{
   if (cur_row <  act_data + 3) /* (act_data - TOP_ROW)) */
   {
	  GetRecord (0);
	  cur_row++;
   }
   else
   {
	  putchar (BELL);
   }

   GetRecord (1);

   return 0;
}

int 
Next (
 void)
{
	if (act_data == 15)
	{
		start_data_row += 16;
		Refresh ();
	}
	else
	{
		putchar (BELL);
	}

	return 0;
}

int 
Prev (
 void)
{
	if (start_data_row != 0)
	{
		start_data_row -= 16;
		Refresh ();
	}
	else
	{
		putchar (BELL);
	}

	return 0;
}

void 
DisplayTerminals (
 void)
{
    int row = TOP_ROW,
    	i = 0,
		cc,
		r = 0;

	posterm_rec.pos_term_no = 0;

	cc = find_rec (posterm,&posterm_rec,GTEQ,"r");
    act_data = 0;

    while (!cc)
	{
		if (r >= start_data_row)
		{
			pos_terminals [r] = posterm_rec.pos_term_no;

	        print_at (row,2,"%-d",posterm_rec.pos_term_no);

			print_at (row,12,GetPosStatus(posterm_rec.pos_term_no));

			print_at (row,39,posterm_rec.last_user);

   	    	strcpy (err_str, DateToString (posterm_rec.last_login));
   	    	print_at (row,53, err_str);
			print_at (row,59, ttoa (posterm_rec.last_login,"HH:MM"));
        	strcpy (err_str, DateToString(posterm_rec.last_logoff)); 
        	print_at (row,67,err_str);
			print_at (row,73, ttoa (posterm_rec.last_logoff,"HH:MM"));
			row++;
			act_data++;

        }

		cc = find_rec (posterm,&posterm_rec,NEXT,"r");
        
		r++;

		i++;

		if (row > (18 /* - TOP_ROW */))
		{
		   break;
		}

	    /* row++;  */
	}

	cur_row = TOP_ROW;
}

void 
GetRecord (
 int rv)
{
   int 		cc;
   char	 	buf [80];

   posterm_rec.pos_term_no = pos_terminals [cur_row-TOP_ROW+start_data_row];
   
   cc = find_rec (posterm,&posterm_rec,EQUAL,"r");

   if (!cc)
   {
	  sprintf 
	  (
			buf,
			"%-9d %-26.26s%15.15s%5.5s %5.5s   %5.5s %5.5s",
			posterm_rec.pos_term_no,
			GetPosStatus (posterm_rec.pos_term_no),
			posterm_rec.last_user,
			DateToString (posterm_rec.last_login),
			ttoa (posterm_rec.last_login, "HH:MM"),
			DateToString (posterm_rec.last_logoff),
			ttoa (posterm_rec.last_logoff,"HH:MM")
		);

      rv_pr (buf,2,cur_row,rv);
      
	  print_at (23,11,GetCompany ());
   }
}

char *
GetPosStatus (
 int pos)
{
   static char status [20];
   char pos_online [40],
		pos_recv [40],
		pos_send [40];
   FILE *fpo,
		*fps = (FILE *) 0,
		*fpr = (FILE *) 0;
  
   sprintf (pos_online,"/tmp/PosStatus/Pos%03d.online",pos);
   sprintf (pos_recv,"/tmp/PosStatus/Pos%03d.recv",pos);
   sprintf (pos_send,"/tmp/PosStatus/Pos%03d.send",pos);
  
   fpo = fopen (pos_online,"w");
   if (fpo)
   {
       fpr = fopen (pos_recv,"w");
       if (fpr)
       {
		   strcpy (status,"Online-receiving");
           fps = fopen (pos_send,"w");
           if (fps)
           {
			  strcat (status,"/sending");
           }
       }
	   else
	   {
           fps = fopen (pos_send,"w");
           if (fps)
           {
			  strcpy (status,"Online-sending");
           }
		   else
		   {
			  strcpy (status,"Online");
		   }
	   }
   }
   else
   {
	   strcpy (status,"Offline");
   }
  
   fclose (fpo); fclose (fps); fclose (fpr);

   return status;
}

char *
GetCompany (
 void)
{
	static char name [60];

    strcpy (comr_rec.co_no,posterm_rec.co_no); 
    if (!find_rec (comr,&comr_rec,EQUAL,"r"))
	{
        sprintf (name,"%-2.2s-%-40.40s",comr_rec.co_no,comr_rec.co_name);		
	}
	else
	{
        sprintf (name,"%-2.2s-%-40.40s","","INVALID COMPANY");		
	}
    return name; 
}

void 
open_db (
 void)
{
   abc_dbopen (data);

   open_rec (posterm,posterm_list,posterm_fields_no,"pos_term_no");
   open_rec (comr,comr_list,comr_fields_no,"comr_co_no");

}

void 
close_db (
 void)
{
   abc_fclose (comr);
   abc_fclose (posterm);
 
   abc_fclose (data);

}

/**eof**/
