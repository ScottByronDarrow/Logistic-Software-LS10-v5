#
# Makefile for IDS db/if
#
SHELL	=	/bin/sh
LIBNAME	=	SQL-db/if
VERSION	= 	LS105	
VMINOR	=	1
VFIX	=	f

CDEF	=	-D${MACH_TYPE} -DDBLADE_API
CINC	=	-I../../INCLUDE \
			-I$(INFORMIXDIR)/incl/esql    \
			-I$(INFORMIXDIR)/incl/public

CFLAGS	=	-g ${CDEF} ${CINC} \
			-DDEBUG \
			-DOS_VER="\"`uname -r`\"" \
			-DVERSION="\"$(VERSION).$(VMINOR).$(VFIX)\"" \
			-DLIBNAME="\"$(LIBNAME)\""
LIB		=	../libIDSdbifLS10.5.a

include	$(BASE_PATH)/ver.etc/etc/Rules.esql

SRCS	=	dbif.c	error.c table.c alias.c column.c index.c utils.c \
		locks.c crc.c query.c data.c insert.c update.c delete.c genaccess.c

OBJS	=	${SRCS:.c=.o}

all:	depend ${LIB}

${LIB}:	${OBJS}
	${AR} rv $@ $?

depend:	${SRCS}
	w4 ${CINC} ${CDEF} ${SRCS} > $@

clean:
	cp /dev/null depend
	-touch 01010000 depend
	rm -f *.o

include depend
