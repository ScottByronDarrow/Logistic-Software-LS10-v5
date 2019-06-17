/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( check_inuv.c )                                   |
|  Program Desc  : (                                                ) |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (20/11/1997)    | Author      : Scott B Darrow     |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+*/
#include	<std_decs.h>

extern	int	cc;

static	const	char	*inuv	=	"_inuv_check_inuv";

	/*========================================+
	 | Inventory Unit of Measure Volume file. |
	 ========================================*/
#define	INUV_NO_FIELDS	3

	static	struct dbview	inuv_list [INUV_NO_FIELDS] =
	{
		{"inuv_hhbr_hash"},
		{"inuv_hhum_hash"},
		{"inuv_volume"}
	};

	struct inuvRecord
	{
		long	hhbr_hash;
		long	hhum_hash;
		float	volume;
	};

	int		ValidItemUom (long, long);
	void	AddINUV (long, long);

	struct	inuvRecord	inuvRec;


int
ValidItemUom
(
	long	hhbrHash,
	long	hhumHash
)
{
	abc_alias (inuv, "inuv");
	open_rec (inuv, inuv_list, INUV_NO_FIELDS, "inuv_id_no");

	inuvRec.hhbr_hash	=	hhbrHash;
	inuvRec.hhum_hash	=	hhumHash;
	cc = find_rec (inuv, &inuvRec, COMPARISON, "r");
	if (cc) 
	{
		abc_fclose (inuv);
		return (EXIT_FAILURE);
	}
	abc_fclose (inuv);
	return (EXIT_SUCCESS);
}

void
AddINUV
(
	long	hhbrHash,
	long	hhumHash
)
{
	open_rec (inuv, inuv_list, INUV_NO_FIELDS, "inuv_id_no");

	inuvRec.hhbr_hash	=	hhbrHash;
	inuvRec.hhum_hash	=	hhumHash;
	cc = find_rec (inuv, &inuvRec, COMPARISON, "r");
	if (cc) 
	{
		inuvRec.volume	=	0.00;
		cc = abc_add (inuv, &inuvRec);
		if (cc)
			file_err (cc, "inuv", "DBADD");

	}
	abc_fclose (inuv);
}
