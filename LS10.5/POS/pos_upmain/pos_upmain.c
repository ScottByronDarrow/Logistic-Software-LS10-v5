/*=============================================================================
|  Copyright (C) 1996 - 1999 SOFTWARE ENGINEERING LIMITED.                    |
|=============================================================================|
| Program name    :  ( pos_upmain.c              )                            |
| Program desc    :  ( POS pipe load program     )                            |
|-----------------------------------------------------------------------------|
| Author          :  Primo O. Esteria          : Date written  Sept 2, 1998   |
|-----------------------------------------------------------------------------|
| $Log: pos_upmain.c,v $
| Revision 5.2  2001/08/09 09:50:26  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:36:56  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:30  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:43  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:18  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:51  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.6  2000/02/18 02:22:16  scott
| Updated to fix small compile warings errors found when compiled under Linux.
|
| Revision 1.5  1999/11/19 06:27:43  scott
| Updated for warning errors.
|
| Revision 1.4  1999/10/16 01:49:20  scott
| Updated from ansi
|
| Revision 1.3  1999/06/18 02:05:28  scott
| Updated for log.
|
=============================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: pos_upmain.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/POS/pos_upmain/pos_upmain.c,v 5.2 2001/08/09 09:50:26 scott Exp $";

#ifdef	LINUX
#define	_XOPEN_SOURCE	500	/* Don't even THINK of asking */
#endif	/* LINUX */

#include <pslscr.h>

#include <alarm_time.h> 
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include <std_decs.h>

#include <messages.h>

#define                  MAX_DELAY      2 

struct msg_header        mess_head;

char                     com_no[3],
                         bra_no[3],
                         war_no[3];
struct               
{
   int pos_no;
   char filename[11];
   long hash;
} posdt_recx [65536];  /* this may be too little */ 

FILE             *SendLog;
int              terminal_no,
                 port_no;

char             *posterm = "posterm",
                 *posdtup = "posdtup";

struct dbview posterm_list [] =
{
   { "pos_no" },
   { "ip_address" },
   { "co_no"},
   { "br_no"},
   { "wh_no"}

};

int posterm_fields_no = 5;

struct  
{
   int    pos_no;
   char   ip_address[16];
   char   co_no[3];
   char   br_no[3];
   char   wh_no[3];
} posterm_rec;

struct
{
   int   pos_no;
   char  ip_address[16];
   char  co_no[3];
   char  br_no[3];
   char  wh_no[3];

} terminals[MAX_TERMINALS];

struct dbview posdtup_list [] = 
{
   { "pos_no" },
   { "file_name" },
   { "record_hash" },
   { "action" }
};

int posdtup_fields_no = 4;

struct
{
   int pos_no;
   char file_name [11];
   long record_hash;
   char action [2];

} posdtup_rec;

/* ================================ */

int    main (int, char *argc []);
int    InitiateConnection (int , char *);
void   open_db (void);
void   close_db (void);
void   InitTerminals (void);
void   ChildExit (int);
int    lockHandle;
void   SetSignals (void);
void   EndProcess (int);
void   daemonize (void);

int 
main(
 int  argc, 
 char *argv [])
{
     int mainLock;

	 SetSignals ();
 
	 daemonize ();

     mainLock = open ("/tmp/PosStatus/POSlock",O_RDWR);
     
     if (lockf (mainLock, F_TEST, 0) == 0)
     {
         lockf (mainLock, F_LOCK, 0);  
     }
     else
     {
         printf ("\npos_main, and pos_upload already running");
         close (mainLock);
         return 0; 
     }
      
     open_db ();
     
     InitTerminals ();

     close_db ();

     close (mainLock);     

	 return EXIT_SUCCESS;
}   


void
daemonize (void)
{
	pid_t pid;

	umask (0);

	if ((pid = fork ()) < 0)
	{
		printf ("\nCannot fork");
	}
	else if (pid != 0)
	{
		exit (0);
	}

	setsid ();


	sigset (SIGHUP, SIG_IGN);
	if ((pid = fork ()) < 0)
	{
        printf ("\nCannot fork");
	}
	else if (pid !=0)
	{
		exit (0);
	}

	chdir ("/");
}

void
SetSignals (void)
{
    signal (SIGTERM, EndProcess);
	signal (SIGHUP,  EndProcess);
	signal (SIGUSR1, EndProcess);
	signal (SIGCHLD, SIG_IGN);     
}

void
EndProcess (
 int sig)
 {
	 signal (sig, SIG_IGN);
	 signal (sig, EndProcess);
}

void 
InitTerminals (
 void)
{
   	   int  cc,
 	        i  = 0,
    	    num_terminals;

        char buf2 [20],
             buf3 [20],
             temp [20];
   
   posterm_rec.pos_no = 0;

   cc = find_rec (posterm,&posterm_rec,GTEQ,"r");

   while (!cc)
   {
          terminals [i].pos_no = posterm_rec.pos_no;
          strcpy (terminals [i].ip_address, posterm_rec.ip_address);
          strcpy (terminals [i].co_no,      posterm_rec.co_no);
          strcpy (terminals [i].br_no,      posterm_rec.br_no);
          strcpy (terminals [i].wh_no,      posterm_rec.wh_no);
      
          i++;
      
          if (i == MAX_TERMINALS)
          {
                 break;
          }

          cc = find_rec (posterm,&posterm_rec,NEXT,"r");
   }

   num_terminals = i;

   /* spawn child process for each terminal */

   while (1)
   {
        for (i = 0; i < num_terminals; i++)
        {
            /* Check to see if our status file is locked 
               if locked, a process devoted to this terminal
               is running
            */
            sprintf (temp, "/tmp/PosStatus/POSlock%d", terminals [i].pos_no);
            lockHandle = open (temp, O_RDWR);
            if (lockf (lockHandle, F_TEST, 0) == 0)
            { 
               	 {
                   	  if (fork () == 0)
                   	  {
                            sprintf (buf2, "%d", terminals [i].pos_no);
                         	sprintf (buf3, "%s", terminals [i].ip_address);
                         	execlp ("pos_upload",
                                 "pos_upload",
                                 buf2,
                                 terminals [i].ip_address,
                                 terminals [i].co_no,
                                 terminals [i].br_no,
                                 terminals [i].wh_no,
                                 (char *)0);
							exit (0);
               	      }
                   	  else
                      {
						 sleep (sleepTime);
                      }
                 }
			 }

             close (lockHandle);
       	} 
            
		time_out (); 

   }   /* while */
}

void 
open_db (
 void)
{
   abc_dbopen ("data");
   open_rec (posterm,posterm_list,posterm_fields_no,"pos_no") ;
}

void 
close_db (
 void)
{
   abc_fclose (posterm);
   abc_fclose ("data");
}


int 
InitiateConnection(
 int   terminal,
 char  *ipad)
{
    return 0;
}

void
ChildExit (
 int sig)
{
    printf ("\nWhich ID is this %d", getpid ());
}

/**eof**/
