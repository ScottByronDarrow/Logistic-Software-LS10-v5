#	$Id: Makefile,v 5.1 2001/08/20 23:32:11 scott Exp $
#
#	Makefile for DB2UDB 6.1 dbif
#
################################################################################
#	$Log: Makefile,v $
#	Revision 5.1  2001/08/20 23:32:11  scott
#	Updated for development related to bullet proofing
#	
#	Revision 5.0  2001/06/19 07:08:20  cha
#	LS10-5.0 New Release as of 19 JUNE 2001
#	
#	Revision 4.0  2001/03/09 02:27:55  scott
#	LS10-4.0 New Release as at 10th March 2001
#	
#	Revision 1.3  2000/09/25 09:48:45  gerry
#	DB2 Release 2 - After major fixes
#	
#	Revision 1.4  1999/11/15 02:53:06  jonc
#	Added lock code. Requires `alvin' the lock-daemon to be running.
#	
#	Revision 1.3  1999/10/28 23:24:31  jonc
#	Added DbIfAttribute interface.
#	
#	Revision 1.2  1999/10/21 21:50:27  jonc
#	Added version-tag to library.
#	
#	Revision 1.1  1999/10/21 21:47:03  jonc
#	Alpha level checkin:
#		Done: database queries, updates.
#		Todo: date conversion, locking and error-handling.
#	

LIB		=	../libdb2dbif${VERSION}.a

SHELL	=	/bin/sh

CINC	=	-I../../INCLUDE -I${INFORMIXDIR}/include -I${DB2DIR}/include
CFLAGS	=	-g -Wall -O0 ${CINC}


SRCS	=	dbif.c \
			alias.c columns.c delete.c \
			errors.c \
			indexes.c insert.c \
			query.c \
			table.c types.c \
			update.c utils.c \
			samputil.c gen-access.c \
			log.c

OBJS	=	${SRCS:.c=.o}

all:	depend ${LIB}

${LIB}	:	${OBJS}
	${AR} srv $@ $?

depend	:	${SRCS}
	w4 ${CINC} ${SRCS} > $@

clean	:
	cp /dev/null depend
	-touch 01010000 depend
	rm -f *.o ${LIB}

include depend
