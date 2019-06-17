/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: pc_qcreat.c,v 5.3 2001/11/07 05:53:27 scott Exp $
|  Program Name  : (pc_qcreat.c) 
|  Program Desc  : (Production Control fifo Queue Creation)
|---------------------------------------------------------------------|
|  Date Written  : 11/02/92        | Author       : Trevor van Bremen |
|---------------------------------------------------------------------|
| $Log: pc_qcreat.c,v $
| Revision 5.3  2001/11/07 05:53:27  scott
| Updated to add app.schema
| Updated to clean code
| Updated to remove gets and replace with scanf
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_qcreat.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_qcreat/pc_qcreat.c,v 5.3 2001/11/07 05:53:27 scott Exp $";

#include	<pslscr.h>

#include	"schema"

struct commRecord	comm_rec;
struct ineiRecord	inei_rec;
struct pclnRecord	pcln_rec;
struct pcrqRecord	pcrq_rec;
struct pcwoRecord	pcwo_rec;

	char	*data	= "data";

typedef	struct	_pcln_list
{
	struct	_pcln_list	*_next;
	struct	_pcln_list	*_prev;
	struct	pclnRecord	pcln_rec;
} PCLN_LIST;

#define	PCLN_NULL	(PCLN_LIST *) NULL

PCLN_LIST	*pcln_head = PCLN_NULL;
PCLN_LIST	*pcln_curr = PCLN_NULL;
PCLN_LIST	*pcln_tail = PCLN_NULL;
PCLN_LIST	*pcln_free = PCLN_NULL;

/*
 * function prototypes.
 */
void 		shutdown_prog 		(void);
void 		OpenDB 				(void);
void 		CloseDB 			(void);
void 		Process 			(void);
void 		LoadRouting 		(long);
PCLN_LIST 	*AllocatePcln 		(void);
void 		DeAllocPcln 		(PCLN_LIST *);
void 		FreePcln 			(void);
void 		CreatePcrq 			(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main(
 int  argc, 
 char *argv[])
{
	/*
	 * Setup required parameters
	 */
	init_scr ();

	OpenDB ();
	
	Process ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm( comm_list, COMM_NO_FIELDS, (char *) &comm_rec );
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (pcln,  pcln_list, PCLN_NO_FIELDS, "pcln_id_no");
	open_rec (pcrq,  pcrq_list, PCRQ_NO_FIELDS, "pcrq_id_no");
	open_rec (pcwo,  pcwo_list, PCWO_NO_FIELDS, "pcwo_hhwo_hash");
}

/*
 * Close data base files
 */
void
CloseDB (void)
{
	abc_fclose (inei);
	abc_fclose (pcln);
	abc_fclose (pcrq);
	abc_fclose (pcwo);
	abc_dbclose (data);
}

void
Process (void)
{
	while (scanf ("%ld", &pcwo_rec.hhwo_hash) != EOF)
	{
		cc = find_rec (pcwo, &pcwo_rec, EQUAL, "u");
		if (cc)
		{
			abc_unlock (pcwo);
			continue;
		}

		LoadRouting (pcwo_rec.hhwo_hash);

		CreatePcrq ();
	}
}

void
LoadRouting(
	long	hhwoHash)
{
	PCLN_LIST	*tmp_pcln;

	while (pcln_head != PCLN_NULL)
	{
		tmp_pcln = pcln_head->_next;
		DeAllocPcln (pcln_head);
		pcln_head = tmp_pcln;
	}
	pcln_rec.hhwo_hash	= hhwoHash;
	pcln_rec.seq_no 	= 0;
	pcln_rec.line_no 	= 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
	while (!cc && pcln_rec.hhwo_hash == hhwoHash)
	{
		tmp_pcln = AllocatePcln ();
		pin_bcopy
		( 
			(char *) &tmp_pcln->pcln_rec, 
			(char *) &pcln_rec, 
			sizeof (struct pclnRecord)
		);
		cc = find_rec (pcln, &pcln_rec, NEXT, "r");
		if (pcln_head == PCLN_NULL)
		{
			pcln_head = tmp_pcln;
			pcln_tail = tmp_pcln;
			pcln_head->_next = PCLN_NULL;
			pcln_head->_prev = PCLN_NULL;
			continue;
		}
		tmp_pcln->_next = PCLN_NULL;
		tmp_pcln->_prev = pcln_tail;
		pcln_tail->_next = tmp_pcln;
		pcln_tail = tmp_pcln;
	}
}

/*
 * Allocate a buffer from either the free-list or from malloc. 
 * If malloc fails 100 times in a row then sys_err out of the program.	
 */
PCLN_LIST *
AllocatePcln (void)
{
	PCLN_LIST	*tmp_pcln;
	int	i = 0;

	tmp_pcln = pcln_free;
	if (tmp_pcln != PCLN_NULL)
		pcln_free = pcln_free->_next;
	else
	{
		while (i < 100)
		{
			tmp_pcln = (PCLN_LIST *) malloc ((unsigned) sizeof (PCLN_LIST));
			if (tmp_pcln != PCLN_NULL)
				break;
			i++;
			sleep (sleepTime);
		}
		if (tmp_pcln == PCLN_NULL)
			sys_err ("Error in AllocatePcln During (MALLOC)", 12, PNAME);
	}

	return (tmp_pcln);
}

/*
 * Deallocate the passed buffer	by moving it to the free-list
 */
void
DeAllocPcln(
	PCLN_LIST *pcln_ptr)
{
	pcln_ptr->_next = pcln_free;
	pcln_free = pcln_ptr;
}

/*
 * Free all buffers that we have malloced.
 */
void
FreePcln (void)
{
	PCLN_LIST	*pcln_ptr;

	while (pcln_head != PCLN_NULL)
	{
		pcln_ptr = pcln_head->_next;
		free (pcln_head);
		pcln_head = pcln_ptr;
	}
	pcln_tail = pcln_head;

	while (pcln_free != PCLN_NULL)
	{
		pcln_ptr = pcln_free->_next;
		free (pcln_free);
		pcln_free = pcln_ptr;
	}
}

void
CreatePcrq (void)
{
	PCLN_LIST	*fwd_pcln = pcln_head,
				*bwd_pcln = pcln_tail;

	inei_rec.hhbr_hash = pcwo_rec.hhbr_hash;
	strcpy (inei_rec.est_no, pcwo_rec.br_no);
	cc = find_rec (inei, &inei_rec, EQUAL, "r");
	if (cc)
		return;

	while (fwd_pcln != PCLN_NULL)
	{
	    pcrq_rec.hhrs_hash	= fwd_pcln->pcln_rec.hhrs_hash;
	    pcrq_rec.qty_rsrc	= fwd_pcln->pcln_rec.qty_rsrc;
	    pcrq_rec.hhwo_hash	= pcwo_rec.hhwo_hash;
	    strcpy (pcrq_rec.prod_class, inei_rec.prod_class);
	    pcrq_rec.priority	= pcwo_rec.priority;
	    pcrq_rec.seq_no		= fwd_pcln->pcln_rec.seq_no;
	    pcrq_rec.line_no	= fwd_pcln->pcln_rec.line_no;
	    pcrq_rec.last_date	= 0L;
	    pcrq_rec.last_time 	= 0L;
	    pcrq_rec.est_date	= 0L;
	    pcrq_rec.est_time	= 0L;
	    pcrq_rec.est_setup	= fwd_pcln->pcln_rec.setup;
	    pcrq_rec.est_run	= fwd_pcln->pcln_rec.run;
	    pcrq_rec.est_clean	= fwd_pcln->pcln_rec.clean;
	    pcrq_rec.act_date	= 0L;
	    pcrq_rec.act_time	= 0L;
	    pcrq_rec.act_setup	= 0L;
	    pcrq_rec.act_run	= 0L;
	    pcrq_rec.act_clean	= 0L;
	    strcpy (pcrq_rec.can_split, fwd_pcln->pcln_rec.can_split);
	    strcpy (pcrq_rec.firm_sched, "N");
	    strcpy (pcrq_rec.stat_flag, "E");

	    cc = abc_add (pcrq, &pcrq_rec);
	    if (cc)
		file_err (cc, pcrq, "DBADD");

	    fwd_pcln = fwd_pcln->_next;
	    bwd_pcln = bwd_pcln->_prev;
	}

	strcpy (pcwo_rec.order_status, "A");
	cc = abc_update (pcwo, &pcwo_rec);
	if (cc)
		file_err (cc, pcwo, "DBUPDATE");
}
