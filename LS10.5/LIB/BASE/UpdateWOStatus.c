/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: UpdateWOStatus.c,v 5.0 2001/06/19 06:59:13 cha Exp $
|  Program Name  : (UpdateWOStatus.c)  
|  Program Desc  : (Function to update works order status automatic.)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 28th Mar 2001    |
|---------------------------------------------------------------------|
| $Log: UpdateWOStatus.c,v $
| Revision 5.0  2001/06/19 06:59:13  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.3  2001/04/28 23:39:46  cha
| Updated from testing
|
| Revision 4.2  2001/04/02 04:08:48  scott
| Updated to usage of zero_pad with new string greater than existing string.
|
| Revision 4.1  2001/03/28 07:09:39  scott
| Updated to add new routine that Updates works order status.
| Used as automatic version of program pc_stat_mnt
|
=====================================================================*/
#include	<std_decs.h>
#include	<UpdateWOStatus.h>

	/*=================================================
	| The Following are needed for branding Routines. |
	=================================================*/
	static	const	char 	*_pcwo	=	"_pcwo_UpdateWOStatus";
					
	struct	pcwoRecord	pcwoRec;

	/*======================================+
	 | Production Control Works Order File. |
	 +======================================*/
#define	PCWO_NO_FIELDS	8

	static	struct dbview	pcwo_list [PCWO_NO_FIELDS] =
	{
		{"pcwo_co_no"},
		{"pcwo_br_no"},
		{"pcwo_wh_no"},
		{"pcwo_order_no"},
		{"pcwo_hhwo_hash"},
		{"pcwo_order_status"},
		{"pcwo_batch_no"},
		{"pcwo_stat_flag"}
	};

	struct pcwoRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	wh_no [3];
		char	order_no [8];
		long	hhwo_hash;
		char	order_status [2];
		char	batch_no [11];
		char	stat_flag [2];
	};

	static void		TableSetup 		 (void),
					TableTeardown 	 (void);

/*============================================================================
| Function that allows works order status to be updated. 					 |
| Basically performs job of program pc_stat_mnt but automatically from call. |
============================================================================*/
int
UpdateWOStatus (
	char	*companyNo,
	char	*branchNo,
	char	*warehouseNo,
	char	*worksOrderNo,
	char	*status)
{
	int		updateError	=	-1,
			errNo		=	0;
	char	sourceStatus [2];
	char	workString [11];

	TableSetup ();

	sprintf (pcwoRec.co_no, 	"%-2.2s", companyNo);
	sprintf (pcwoRec.br_no, 	"%-2.2s", branchNo);
	sprintf (pcwoRec.wh_no, 	"%-2.2s", warehouseNo);
	sprintf (pcwoRec.order_no,  "%-7.7s", worksOrderNo);
	sprintf (sourceStatus,		"%-1.1s", status);
	errNo = find_rec (_pcwo, &pcwoRec, COMPARISON, "u");
	if (errNo)
	{
		abc_unlock (_pcwo);
		return (WO_NOTFOUND);
	}

	switch (sourceStatus [0])
	{
		case	'P':
			strcpy (pcwoRec.order_status, "F");
			strcpy (workString, pcwoRec.order_no);
			strcpy (pcwoRec.batch_no, zero_pad (workString, 10));
			updateError = abc_update (_pcwo, &pcwoRec);
				
			break;

		case	'F':
			strcpy (pcwoRec.order_status, "I");
			updateError = abc_update (_pcwo, &pcwoRec);
				
			break;

		case	'A':
			strcpy (pcwoRec.order_status, "R");
			updateError = abc_update (_pcwo, &pcwoRec);
			break;
	}

	TableTeardown	();

	if (updateError < 0)
		return (WO_ISTATUS);

	if (updateError)
		return (WO_NOTFOUND);

	return (WO_AOK);
}

static void
TableSetup (void)
{
	/*
	 *	Open all the necessary tables et al
	 */
	static int	done_this_before = FALSE;

	if (!done_this_before)
	{
		done_this_before = TRUE;

		abc_alias (_pcwo, "pcwo");
	}
	open_rec (_pcwo, pcwo_list, PCWO_NO_FIELDS, "pcwo_id_no");
}

static void
TableTeardown (void)
{
	abc_fclose (_pcwo);
}
