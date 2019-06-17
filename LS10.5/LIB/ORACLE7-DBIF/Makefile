#	$Id: Makefile,v 5.2 2001/11/29 02:39:22 cha Exp $
#
#	Makefile for ORACLE 7 dbif
#
################################################################################
#	$Log: Makefile,v $
#	Revision 5.2  2001/11/29 02:39:22  cha
#	Updated to make sure viewtype is compatible with Informix.
#	
#	Revision 5.1  2001/06/21 08:01:01  cha
#	Updated to handle correctly the Money datatype.
#	
#	Revision 5.0  2001/06/19 07:10:27  cha
#	LS10-5.0 New Release as of 19 JUNE 2001
#	
#	Revision 4.1  2001/04/06 02:09:52  cha
#	Updated to check in changes made to the Oracle DBIF Library
#	
#	Revision 1.1  2000/11/20 06:11:52  jason
#	Initial update.
#	
#	Revision 2.1  2000/07/26 10:09:55  raymund
#	Furnished missing functionalities. Use SQL for row locking.
#	
#	Revision 2.0  2000/07/15 07:33:50  gerry
#	Forced Revision No. Start to 2.0 Rel-15072000
#	
#	Revision 1.5  2000/07/13 11:08:44  raymund
#	16-bit reversed CRC hardware emulation algorithm for row locking.
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

LIB		=   ../libora7dbif${VERSION}.a

SHELL	=	/bin/sh

CINC	=	-I${PROG_PATH}/INCLUDE -I${INFORMIXDIR}/include
CFLAGS	=	-g -Wall ${CINC}


SRCS	=	dbif.c \
			alias.c attrs.c columns.c delete.c errors.c \
			gen-access.c \
			indexes.c insert.c \
			query.c \
			table.c types.c \
			locks.c crc.c \
			update.c utils.c
OBJS	=	${SRCS:.c=.o}

all:	depend ${LIB}

${LIB}	:	${OBJS}
	ar srv $@ $?

depend	:	${SRCS}
	w4 ${CINC} ${SRCS} > $@

clean	:
	cp /dev/null depend
	-touch 01010000 depend
	rm -f *.o ${LIB}


include depend
