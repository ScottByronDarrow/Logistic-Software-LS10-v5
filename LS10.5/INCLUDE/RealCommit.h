#ifndef	REAL_COMMIT_H
#define	REAL_COMMIT_H
/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( RealCommit.c )                                   |
|  Program Desc  : ( Include file for calculation of real-time      ) |
|                  ( committed stock.                               ) |
|---------------------------------------------------------------------|
|  Access files  :  soic,     ,     ,     ,     ,     ,     ,         |
|  Database      : (    )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A)                                              |
|---------------------------------------------------------------------|
|  Date Written  : (09/05/94)      | Author       : Campbell Mander.  |
|---------------------------------------------------------------------|
|  Date Modified : (  /  /  )      | Modified  by :                   |
|                                                                     |
|  Comments      :                                                    |
|  (  /  /  )    :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/

	/*===============================
	| Real-Time Item Committal File |
	===============================*/
	struct dbview soic_list [] =
	{
		{"soic_pid"},
		{"soic_line"},
		{"soic_hhbr_hash"},
		{"soic_hhcc_hash"},
		{"soic_qty"},
		{"soic_program"},
		{"soic_op_id"},
		{"soic_time_create"},
		{"soic_date_create"},
		{"soic_status"}
	};

	int	soic_no_fields = 10;

	struct tag_soicRecord
	{
		long	pid;
		int		line;
		long	hhbr_hash;
		long	hhcc_hash;
		float	qty;
		char	program [21];
		char	op_id [15];
		long	time_create;
		long	date_create;
		char	status [2];
	} soic_rec;

	char	*soic = "soic";

/*-----------------------------------------------------------
| Function    : RealTimeCommitted ()                        |
| Description : Calculates the actual real-time committed   |
|               quantity of stock for a warehouse (or all   |
|               warehouses).  Data is taken from the soic   |
|               file which is updated real-time by a        |
|               number of programs including so_input etc   |
| Parameters  : hhbrHash - link to inventory master (inmr)  |
|               hhccHash - link to warehouse master (ccmr)  |
|                        - If hhccHash is 0L then calculate |
|                          for all warehouses.              |
| Returns     : A float containing the actual quantity      |
|               committed at this time.                     |
-----------------------------------------------------------*/
float RealTimeCommitted (long	hhbrHash, long	hhccHash)
{
	float	commQty;	/* Accumulator of committed quantity */

	/*---------------------------------
	| Initialise calculated quantity. |
	---------------------------------*/
	commQty = 0.00;

	/*---------------------------------------
	| Read soic records.  If hhccHash is 0L |
	| then read for all warehouses.         |
	---------------------------------------*/
	strcpy (soic_rec.status, "A");
	soic_rec.hhbr_hash = hhbrHash;
	soic_rec.hhcc_hash = hhccHash;
	cc = find_rec (soic, &soic_rec, GTEQ, "r");
	while (!cc &&
		   soic_rec.status[0] == 'A' &&
		   soic_rec.hhbr_hash == hhbrHash &&
		   (hhccHash == 0L || 
			(hhccHash != 0L && soic_rec.hhcc_hash == hhccHash)))
	{
		commQty += soic_rec.qty;

		cc = find_rec (soic, &soic_rec, NEXT, "r");
	}

	return (commQty);
}

#endif	/* REAL_COMMIT_H */
