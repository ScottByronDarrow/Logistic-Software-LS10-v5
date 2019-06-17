/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( find_cuit.h   )                                  |
|  Program Desc  : ( Find Debtor Specific Item record.            )   |
|---------------------------------------------------------------------|
|  Access files  :  cuit,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius   | Date Written  : 13/09/93         |
|---------------------------------------------------------------------|
|  Date Modified : (13/09/93)      | Modified  by  : Dirk Heinsius.   |
|                                                                     |
|  Comments      :                                                    |
|  (13/09/93)    : HGP 9500 Create find_cuit.h                        |
|                :                                                    |
=====================================================================*/
int	find_cuit (long, long, char *);

int
find_cuit (long hhbr_hash, long hhcu_hash, char *dbtr_codes_ok)
{
	if (hhcu_hash == 0L)
		return (1);

	if (dbtr_codes_ok[0] != 'Y')
		return (1);

	abc_selfield (cuit, "cuit_id_no");
	cuit_rec.hhcu_hash = hhcu_hash;
	cuit_rec.hhbr_hash = hhbr_hash;
	if (!find_rec (cuit, &cuit_rec, EQUAL, "r"))
		return (0);

	return (1);
}
