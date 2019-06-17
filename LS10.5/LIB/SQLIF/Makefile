#
# Makefile for SQL db/if
#
SHELL	=	/bin/sh
LIBNAME	=	SQL-db/if
VERSION	=	9.10
VMINOR	=	1
VFIX	=	f

CDEF	=	-D${MACH_TYPE}
CINC	=	-I../../INCLUDE \
			-I$(INFORMIXDIR)/incl/esql

CFLAGS	=	-g ${CDEF} ${CINC} \
			-DDEBUG \
			-DOS_VER="\"`uname -r`\"" \
			-DVERSION="\"$(VERSION).$(VMINOR).$(VFIX)\"" \
			-DLIBNAME="\"$(LIBNAME)\""
LIB		=	../libdbif$(VERSION).a

include	$(BASE_PATH)/ver.etc/etc/Rules.esql

SRCS	=	dbif.c		dbopen.c	dbfind.c \
			dict.c		getrec.c	lock.c \
			idx.c		tblnode.c \
			update.c	view.c		utils.c

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
