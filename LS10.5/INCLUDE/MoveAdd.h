int		MoveOpen	=	FALSE;
int		moveUser	=	FALSE;
char	*moveCurrentUserName;

/*=================================================
| Routine to add inventory movements to work file |
=================================================*/
static	int
MoveAdd
 (
	char 	*moveCoNo, 
	char	*moveBrNo, 
	char	*moveWhNo,
	long  	moveHhbrHash, 
	long	moveHhccHash, 
	long	moveHhumHash, 
	long	moveDate,
	int   	moveType,
	char 	*moveBatchNo, 
	char	*MoveClass,
	char	*MoveCategory ,
	char	*moveRef1, 
	char	*moveRef2, 
	float 	moveQuantity, 
	double 	moveSale, 
	double	moveCost 
)
{
	if (!MoveOpen)
		open_rec ("move", move_list, MOVE_NO_FIELDS, "move_move_hash");
		
	if (moveUser == FALSE)
	{
		moveCurrentUserName = getenv ("LOGNAME");
		moveUser = TRUE;
	}

	sprintf (move_rec.co_no,    "%-2.2s",   moveCoNo);
	sprintf (move_rec.br_no,    "%-2.2s",   moveBrNo);
	sprintf (move_rec.wh_no,    "%-2.2s",   moveWhNo);
	sprintf (move_rec.batch_no, "%-7.7s",   moveBatchNo);
	sprintf (move_rec.move_class,"%-1.1s",   MoveClass);
	sprintf (move_rec.category, "%-11.11s", MoveCategory);
	sprintf (move_rec.ref1,     "%-15.15s", moveRef1);
	sprintf (move_rec.ref2,     "%-15.15s", moveRef2);
	sprintf (move_rec.op_id,    "%-14.14s", moveCurrentUserName);
	strcpy  (move_rec.time_create, TimeHHMMSS ());
	move_rec.hhbr_hash 	 = moveHhbrHash;
	move_rec.hhcc_hash 	 = moveHhccHash;
	move_rec.hhum_hash 	 = moveHhumHash;
	move_rec.date_tran 	 = moveDate;
	move_rec.type_tran 	 = moveType;
	move_rec.qty 		 = moveQuantity;
	move_rec.cost_price  = moveCost;
	move_rec.sale_price  = moveSale;
	move_rec.date_create = TodaysDate ();
	cc = abc_add ("move", &move_rec);
	if (cc)
		file_err (cc, "move", "DBADD");
	
	if (!MoveOpen)
		abc_fclose ("move");

	return (cc);
}
