/*
 *  Copyright (C) 1999 - 2000 LogisticSoftware
 *
 * $Id: InumGen.c,v 5.1 2001/11/29 01:25:40 scott Exp $
 *
 *
 * $Log: InumGen.c,v $
 * Revision 5.1  2001/11/29 01:25:40  scott
 * Updated to add warning message.
 *
 * Revision 5.0  2001/06/19 06:59:12  cha
 * LS10-5.0 New Release as of 19 JUNE 2001
 *
 * Revision 4.0  2001/03/09 00:52:34  scott
 * LS10-4.0 New Release as at 10th March 2001
 *
 * Revision 3.2  2000/10/17 01:20:23  scott
 * Updated for inuv error.
 *
 * Revision 3.1  2000/10/16 09:19:30  scott
 * Updated from testing.
 *
 * Revision 3.0  2000/10/12 13:34:17  gerry
 * Revision No. 3 Start
 * <after Rel-10102000>
 *
 * Revision 2.1  2000/10/09 02:06:38  scott
 * New function to generate unit of measure.
 *
 *
 *	
 */

#define	TRUE	1
#define	FALSE	0

long 	GenerateUOM (const char	*, const char *, float, long);

#include	<std_decs.h>

static const char
	*inum	= "_inum_gen",
	*inuv	= "_inuv_gen";

	/*======+
	 | inuv |
	 +======*/
#define	INUV_NO_FIELDS	2

	static struct dbview inuv_list [INUV_NO_FIELDS] =
	{
		{"inuv_hhbr_hash"},
		{"inuv_hhum_hash"},
	};

	struct inuvRecord
	{
		long	hhbr_hash;
		long	hhum_hash;
	};
	/*=================================+
	 | Inventory Unit of Measure File. |
	 +=================================*/
#define	INUM_NO_FIELDS	5

	static	struct dbview	inum_list [INUM_NO_FIELDS] =
	{
		{"inum_uom_group"},
		{"inum_hhum_hash"},
		{"inum_uom"},
		{"inum_desc"},
		{"inum_cnv_fct"}
	};

	struct inumRecord
	{
		char	uom_group [21];
		long	hhum_hash;
		char	uom [5];
		char	desc [41];
		float	cnv_fct;
	};
 
static	struct inumRecord	inumRec;

static void		TableSetup (void),
				TableTearDown (void);

static	char 	*CheckExistence (const	char *, float);

/*================================================================
| Generate a UOM using base UOM and the conversion factor input. |
================================================================*/
long	
GenerateUOM (
	const 	char *uomGroup,
	const 	char *uomCode,
	float	uomConversion,
	long	hhbrHash)
{
	int		conversion	=	0,
			err			=	0;
	char	workCode [5];
	char	*sptr;
	int		i;

	struct inuvRecord	inuvRec;

	TableSetup ();

	conversion	=	(int)	uomConversion;
	if ((float) conversion != (float) uomConversion)
	{
		clear_mess ();
		i = prmptmsg ("UOM conversion would generate a fraction, Accept (Y/N) ","YyNn",0,23);
		if (i == 'N' || i == 'n')
			return (-1L);
	}
	sprintf (workCode, "%04d", conversion);
	
	sptr = CheckExistence (uomGroup, uomConversion);
	if (sptr != (char *)0)
		sprintf (workCode, "%-4.4s", sptr);

	/*
		Clear structures.
	*/
	memset (&inumRec, 0, sizeof (struct inumRecord));
	memset (&inuvRec, 0, sizeof (struct inuvRecord));

		
	strcpy (inumRec.uom_group, 	uomGroup);
	strcpy (inumRec.uom, 		workCode);
	if ((err = find_rec (inum, &inumRec, COMPARISON, "r")))
	{
		sprintf (inumRec.desc, "Base UOM %4.4s / Package UOM %4.4s",
					uomCode, inumRec.uom);
		inumRec.cnv_fct		=	uomConversion;

		if ((err = abc_add (inum, &inumRec)))
			file_err (err, inum, "DBADD");

		if ((err = find_rec (inum, &inumRec, COMPARISON, "r")))
			file_err (err, inum, "DBADD");
	}
	inuvRec.hhbr_hash	=	hhbrHash;
	inuvRec.hhum_hash	=	inumRec.hhum_hash;
	if ((err = find_rec (inuv, &inuvRec, COMPARISON, "r")))
	{
		if ((err = abc_add (inuv, &inuvRec)))
			file_err (err, inuv, "DBADD");
	}
	TableTearDown ();
	return inumRec.hhum_hash;
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

		abc_alias (inum, "inum");
		abc_alias (inuv, "inuv");
	}

	open_rec (inuv, inuv_list, INUV_NO_FIELDS, "inuv_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_id_no2");
}

static void
TableTearDown (void)
{
	abc_fclose (inuv);
	abc_fclose (inum);
}

static	char	*
CheckExistence (
	const	char *uomGroup,
	float	uomConversion)
{
	int	err;

	memset (&inumRec, 0, sizeof (struct inumRecord));

	strcpy (inumRec.uom_group, 	uomGroup);
	strcpy (inumRec.uom, 		"    ");
	err = find_rec (inum, &inumRec, GTEQ, "r");
	while (!err && !strcmp (inumRec.uom_group, 	uomGroup))
	{
		if (inumRec.cnv_fct	== uomConversion)
			return (inumRec.uom);

		err = find_rec (inum, &inumRec, NEXT, "r");
	}
	return ((char *) 0);
}
