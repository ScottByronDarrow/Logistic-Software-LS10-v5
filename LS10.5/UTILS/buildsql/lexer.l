%{
/*
 *	Lexical analysiser for Pinnacle Schema files
 *
 *	Note:	this must be compiled with 'flex' due to the use of
 *			exclusive states
 *******************************************************************************
 *	$Log: lexer.l,v $
 *	Revision 5.0  2001/06/19 08:22:29  robert
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.0  2001/03/09 02:43:32  scott
 *	LS10-4.0 New Release as at 10th March 2001
 *	
 *	Revision 3.0  2000/10/10 12:24:00  gerry
 *	Revision No. 3 Start
 *	<after Rel-10102000>
 *	
 *	Revision 2.0  2000/07/15 09:15:02  gerry
 *	Forced Revision No Start 2.0 Rel-15072000
 *	
 *	Revision 1.5  1999/11/16 20:58:25  jonc
 *	Added -a and -A options from version 10
 *	
 *	Revision 1.4  1999/10/26 22:53:30  jonc
 *	Updated to use the generic system-catalog interface.
 *	
 */
#include	<string.h>

#include	<dbio.h>

#include	"commdefs.h"
#include	"tbldef.h"
#include	"y.tab.h"

#define		MAX_NAME_LEN	18

extern int	line_no;
extern char	cur_fname [], tbldesc [];

%}

W			[ \t]
D			[0-9]
A			[_a-zA-Z]

%x	COMMENT DESC
%%
\{						BEGIN COMMENT;	/* Enter comment mode */
<COMMENT>\}				BEGIN INITIAL;	/* Exit comment mode */
<COMMENT>\n				line_no++;		/* line counter must be incremented */
<COMMENT>.				;				/* ignore comments */

<COMMENT>"Schema Desc  : ("	BEGIN DESC;	/* Enter table description mode */

<DESC>[^\)\n]+			strcpy (tbldesc, yytext);
<DESC>\n				|
<DESC>\)				BEGIN COMMENT;	/* Fall back to comment mode */

file					return (TK_FILE);
end						return (TK_END);

field					return (TK_FIELD);
type					return (TK_TYPE);

index					return (TK_INDEX);
dups					return (TK_DUPS);
primary					return (TK_PRIMARY);

comp					|
composite				return (TK_COMPO);
char					return (TK_CHAR);
double					return (TK_DOUBLE);
date					|
edate					return (TK_DATE);
float					return (TK_FLOAT);
int						|
integer					return (TK_INT);
long					return (TK_LONG);
money					return (TK_MONEY);
serial					return (TK_SERIAL);

\n						line_no++;		/* increment line counter */

{A}+({A}|{D})*			{
							yylval.sval = strdup (yytext);

							if (yyleng > MAX_NAME_LEN)
							{
								warning ("Name \"%s\" truncated\n",
									yylval.sval);
								yylval.sval [MAX_NAME_LEN] = '\0';
							}
							return (TK_NAME);
						}
{D}+					{
							yylval.ival = atoi (yytext);
							return (TK_NUMBER);
						}
{W}+					;	/* ignore whitespace */

\,						return (TK_COMMA);
.						return (TK_BAD);

<<EOF>>					{
							/* Need to reset state for possible next file
							 */
							BEGIN INITIAL;
							return (0);
						}
%%
void
yyerror (
 char	*s)
{
	fprintf (stderr,
		"ERR: %s Line %d : %s at '%s'\n",
		cur_fname, line_no, s, yytext);
}

/*	Local yywrap
 */
int
yywrap ()
{
	return (1);		/* quit at EOF */
}
