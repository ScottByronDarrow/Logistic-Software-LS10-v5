#
#  $Id: Makefile,v 5.3 2001/11/14 00:44:25 scott Exp $
#  Make Script for DSOXtra - LINUX OS Only
#
################################################################################
#	$Log: Makefile,v $
#	Revision 5.3  2001/11/14 00:44:25  scott
#	Updated for compile errors on DSO
#	
#	Revision 5.2  2001/09/11 23:29:45  scott
#	Updated from Scott machine - 12th Sep 2001
#	
#	Revision 5.1  2001/07/25 03:25:24  scott
#	Updated for LS10.5
#	
#	Revision 5.0  2001/06/19 07:08:59  cha
#	LS10-5.0 New Release as of 19 JUNE 2001
#	
#	Revision 4.0  2001/03/09 02:27:58  scott
#	LS10-4.0 New Release as at 10th March 2001
#	
#	Revision 2.1  2000/10/07 08:38:31  gerry
#	Initial check-in for misssing Makefile in DSO.Xtra
#	
#	

SHELL	= /bin/sh
LIBNAME	=	LogisticSoftware

CDEF	=	-D${MACH_TYPE}
CINC	=	-I../../INCLUDE -I${INFORMIXDIR}/include

CFLAGS	= -s -Wall -O3 ${CDEF} ${CINC} \
#		-DDEBUG \
		-DVERSION="\"${VERSION}\"" \
		-DOS_VER="\"`uname -r`\"" \
		-DLIBNAME="\"$(LIBNAME)\""


LIB	=	../libDSOXtra$(VERSION).a

SRCS	=	DSOXtra.c	DSO_Base.c	DSO_Scrgen.c \
		DSO_Scrgen2.c	

OBJS	=	${SRCS:.c=.o}

all:	depend ${LIB}

${LIB}:	${OBJS}
	${AR} srv $@ $?

depend:	${SRCS}
	w4 ${CINC} ${CDEF} ${SRCS} > $@

clean:
	cp /dev/null depend
	-touch 01010000 depend
	rm -f *.o

include depend
