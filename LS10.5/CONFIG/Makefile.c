#	$Id: Makefile.c,v 5.0 2002/05/08 01:39:31 scott Exp $
#
#	Standard application Makefile template
#
################################################################################
#	$Log: Makefile.c,v $
#	Revision 5.0  2002/05/08 01:39:31  scott
#	CVS administration
#	
#	Revision 1.3  2001/08/21 00:12:55  scott
#	Updated for development related to bullet proofing
#	
#	Revision 1.1  2001/01/09 04:46:12  scott
#	Checked in files from CONFIG directory to cvs
#	
APP		=	application-name
ALTS	=	alternate-name
SRCS	=	app.c app1.c app2.c

include ${BASE_PATH}/${VERSION}/CONFIG/Make.CBase
