#ifndef	CCHARTYPE

#define	CCHARTYPE	100
#define	CSHORTTYPE	101
#define	CINTTYPE	102
#define	CLONGTYPE	103
#define	CFLOSTTYPE	104
#define	CDOUBLETYPE	105
#define	CDECIMALTYPE	107
#define	CFIXCHARTYPE	108
#define	CSTRINGTYPE	109
#define	CDATETYPE	110
#define	CMONEYTYPE	111

#define	USERCOL(x)	((x))

#define	SQLCHAR		0
#define	SQLSMINT	1
#define	SQLINT		2
#define	SQLFLOAT	3
#define	SQLSMFLOAT	4
#define	SQLDECIMAL	5
#define	SQLSERIAL	6
#define	SQLDATE		7
#define	SQLMONEY	8
#define	SQLNULL		9
#define	SQLTYPE		0xF
#define	SQLNONULL	0400

#define	SQLHOST		01000

#define	SIZCHAR		1
#define	SIZSMINT	2
#define	SIZINT		4
#define	SIZFLOAT	(sizeof(double))
#define	SIZSMFLOAT	(sizeof(float))
#define	SIZDECIMAL	17
#define	SIZSERIAL	4
#define	SIZDATE		4
#define	SIZMONEY	17

#define	ISDECTYPE(t)	(((t)&SQLTYPE)==SQLDECIMAL || ((t)&SQLTYPE==SQLMONEY))

#define	DEFDECIMAL	9
#define	DEFMONEY	9
#define	DATENULL	-2147483648L

#define	SYSPUBLIC	"public"

#define	INTNULL		-32768
#define	LONGNULL	(-2147483647L -1L)

#endif
