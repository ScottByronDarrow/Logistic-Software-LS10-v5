#ifndef	AUDIT_LIB
#define	AUDIT_LIB

struct	AUD_HDR
{
	char	op_id		[8];
	char	prog_name	[15];
	char	action		[1];
	char	date_time	[4];
	char	termno		[2];
	char	recno		[4];
};

#define	aud_hdlen	(sizeof (struct AUD_HDR))

#endif	/* AUDIT_LIB	*/
