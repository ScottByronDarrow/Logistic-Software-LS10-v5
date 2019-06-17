/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : wild_card.c                                    |
|  Source Desc       : Wild card search routines.                     |
|                                                                     |
|  Library Routines  : wild_card()                                    |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|---------------------------------------------------------------------|
|  Date Modified     :   /  /     | Modified  by  :                   |
|                                                                     |
|  Comments          :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>
#include	<wild_card.h>

static	int	wild_char (int c);
static	int	find_bit (char *s_ptr, BIT_PTR bit_ptr);

static	char	*get_a_bit (char *s_ptr, BIT_PTR bit_tab, BIT_PTR bit_ptr, int len, int wild)
{
	BIT_PTR	tmp_ptr;
	char	*o_ptr;
	int	cnt = MAX_BITS;

	BIT_START = 0;
	o_ptr = s_ptr;

	while (wild_char (*s_ptr))
		*s_ptr++ = '\0';

	for (tmp_ptr = bit_tab; tmp_ptr < bit_ptr && cnt--; tmp_ptr++)
		BIT_START += strlen (tmp_ptr->piece);

	BIT_PIECE = s_ptr;
	while (*s_ptr && !wild_char (*s_ptr))
		s_ptr++;

	if (wild && !*s_ptr)
		BIT_START = len - ((s_ptr - 1) - o_ptr);

	return (s_ptr);
}

/*
.function
	Function	:	wild_break ()

	Description	:	Break search value into pieces.

	Notes		:	Wild_break breaks the search value into 
				pieces. There may be up to 10 pieces.
			
	Parameters	:	key_val	- Value to break up.
				bit_tab	- Table in which to place value.
				len	- Maximum possible length of value.
.end
*/
void
wild_break (char *key_val, BIT_PTR bit_tab, int len)
{
	char	*s_ptr;
	BIT_PTR	bit_ptr;
	int	wild;
	static	char	tmp_val [128];

	strcpy (tmp_val, key_val);
	s_ptr = tmp_val;
	wild = is_wild (key_val);
	
	bit_ptr = bit_tab;
	do
	{
		s_ptr = get_a_bit (s_ptr, bit_tab, bit_ptr++, len, wild);
	} while (*s_ptr);

	for (bit_ptr--; bit_ptr >= bit_tab; bit_ptr--)
	{
		BIT_LEN = wild ? strlen (BIT_PIECE) : len;
		BIT_END = wild ? len - 1: BIT_LEN - 1;
		len -= BIT_LEN;
	}
}

/*
.function
	Function	:	wild_check ()

	Description	:	Check string against search values.

	Notes		:	Wild_check compares the current search value
				against the pieces stored by wild_break.
			
	Parameters	:	s_ptr	- Pointer to value to be checked.
				bit_ptr	- Pointer to start of table of pieces.

	Returns		:	TRUE	- if value matches criteria.
				FALSE	- if value doesn't matche criteria.
.end
*/
int
wild_check (char *s_ptr, BIT_PTR bit_ptr)
{
	int	cnt = MAX_BITS;

	while (BIT_LEN && cnt--)
		if (!find_bit (s_ptr, bit_ptr++))
			return (FALSE);

	return (TRUE);
}

static	int
find_bit (char *s_ptr, BIT_PTR bit_ptr)
{
	for (s_ptr += BIT_START; *s_ptr; s_ptr++)
		if (!strncmp (s_ptr, BIT_PIECE, BIT_LEN))
			return (TRUE);

	return (FALSE);
}

/*
.function
	Function	:	is_wild ()

	Description	:	Check string for wild search characters.

	Notes		:	This routine is used to identify strings
				containing wild card characters.
			
	Parameters	:	s_ptr	- Pointer to value to be checked.

	Returns		:	TRUE	- if value is wild.
				FALSE	- if value isn't wild.
.end
*/
int
is_wild (char *s_ptr)
{
	while (*s_ptr)
		if (wild_char (*s_ptr++))
			return (TRUE);

	return (FALSE);
}

static	char *init_range (char *s_ptr, BIT_PTR bit_ptr, char i_char, int len)
{
	static	char	tmp_str [128];

	pin_bfill (tmp_str, i_char, len);
	tmp_str [len] = '\0';

	if (!*s_ptr || wild_char (*s_ptr))
		return (tmp_str);

	strncpy (tmp_str + BIT_START, BIT_PIECE, BIT_LEN);
	while ((++bit_ptr)->piece)
		strncpy (tmp_str + BIT_END - (BIT_LEN - 1), BIT_PIECE, BIT_LEN);

	return (tmp_str);
}

/*
.function
	Function	:	wild_beg ()

	Description	:	Return the first possible value in range.

	Notes		:	Wild_beg is used to set the starting position
				for wild card searches.
			
	Parameters	:	s_ptr	- Value to be compared.
				bit_ptr	- Table of search pieces.
				len	- Maximum possible length of value.

	Returns		:	Pointer to lowest possible value.
.end
*/
char	*wild_beg (char *s_ptr, BIT_PTR bit_ptr, int len)
{
	return (init_range (s_ptr, bit_ptr, ' ', len));
}

/*
.function
	Function	:	wild_end ()

	Description	:	Return the last possible value in range.

	Notes		:	Wild_beg is used to set the ending position
				for wild card searches.
			
	Parameters	:	s_ptr	- Value to be compared.
				bit_ptr	- Table of search pieces.
				len	- Maximum possible length of value.

	Returns		:	Pointer to highest possible value.
.end
*/
char	*wild_end (char *s_ptr, BIT_PTR bit_ptr, int len)
{
	return (init_range (s_ptr, bit_ptr, '~', len));
}

static	int
wild_char (int c)
{
	if (c == '*' /* || c == '?' */)
		return (c);

	return (FALSE);
}
