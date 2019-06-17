/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( alt_hash.c     )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (05.12.94)      | Author      : Jonathan Chen      |
|---------------------------------------------------------------------|
|  Date Modified : (  .  .  )      | Modified by :                    |
|                                                                     |
|  Comments                                                           |
|  (05.12.94) : Cut from conv_qty.c                                   |
|                                                                     |
=====================================================================*/
long
alt_hash (
 long	hhbr_hash,
 long	hhsi_hash)
{
	return (hhsi_hash ? hhsi_hash : hhbr_hash);
}
