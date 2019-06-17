#=====================================================================|
#  Copyright (C) 1999 - 1999 Logistic Software Limited.               |
#=====================================================================|
# $Id: Makefile,v 5.0 2002/05/08 01:30:07 scott Exp $
#  Program Name  : (Makefile)
#  Program Desc  : (Makefile for ORACLE 8i dbif)
#---------------------------------------------------------------------|
# $Log: Makefile,v $
# Revision 5.0  2002/05/08 01:30:07  scott
# CVS administration
#
# Revision 1.2  2002/03/11 11:08:42  cha
# Added files and code checked.
#
# Revision 1.1  2002/02/05 02:39:48  kaarlo
# Initial check-in for ORACLE8i porting.
#======================================================================

LIB		=   ../libora8idbif${VERSION}.a

SHELL	=	/bin/sh

CINC	=	-I${PROG_PATH}/INCLUDE \
			-I$(ORACLE_HOME)/rdbms/demo \
			-I$(ORACLE_HOME)/rdbms/public
CFLAGS	=	-g -Wall ${CINC}

SRCS	=	dbif.c \
			alias.c columns.c delete.c errors.c \
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
