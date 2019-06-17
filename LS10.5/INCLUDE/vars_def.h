#ifndef	VARS_DEF_H
#define	VARS_DEF_H
struct	var
{
	int	scn;		/* The Screen Number			*/
	int	stype;		/* linear or tabular screen		*/
	char	*label;		/* Programmer label for field (unique)	*/
	int	row;		/* Screen row for input/display		*/
	int	col;		/* Screen column for input/display	*/
	int	type;		/* Informix compatiable type def.	*/
	char	*mask;		/* This is the input/print mask :	*/
				/* Mask characters can be :		*/
				/*	A - Alpha-numeric.		*/
				/*	U - Upper case alpha-numeric.	*/
				/*	L - Lower case alpha-numeric.	*/
				/*	N - Numeric.			*/
				/*	D - date.			*/
	char	*pmask;		/* Mask for print displaying (prog gen'd*/
	char	*fill;		/* The fill char either ' ' or '0' only.*/
	char	*deflt;		/* the default value if not 'required'.	*/
	char	*prmpt;		/* Prompt for writing of screen.	*/
	char	*comment;	/* Comment to be displayed		*/
	int	required;	/* Data Required in field can be :	*/
				/* YES	- Required			*/
				/* NO	- Not required			*/
				/* NE	- Required but no edit		*/
				/* NA	- No entry or editing		*/
				/* NI	- No entry but can edit		*/
				/* ND	- No display of field		*/
	int	autonext;	/* Auto line feed on field full		*/
	int	just;		/* Justify entry to the LEFT or RIGHT.	*/
	char	*lowval;	/* LOWER limit or valid chars		*/
	char	*highval;	/* UPPER value limit if field not "".	*/
	char	*tcptr;
};
#endif	
