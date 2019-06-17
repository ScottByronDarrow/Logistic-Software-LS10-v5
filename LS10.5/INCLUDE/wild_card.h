#ifndef	IWILD_CARD
#define	IWILD_CARD
typedef	struct
{
	char	*piece;
	int	s_byte,
		e_byte,
		len;
} BIT_TAB, *BIT_PTR;

#define	MAX_BITS	10
#define	BIT_PIECE	bit_ptr->piece
#define	BIT_START	bit_ptr->s_byte
#define	BIT_END		bit_ptr->e_byte
#define	BIT_LEN		bit_ptr->len

#ifndef TRUE
#	define	TRUE	1
#	define	FALSE	0
#endif	

#endif	
