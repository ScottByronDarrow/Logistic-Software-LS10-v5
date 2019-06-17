#
#  Make Script for libdbif
#  If Audits are required than set -DAUDIT 
#  If Log on file locks required set -DLOG
#
SHELL	=	/bin/sh
LIBNAME	=	CISAM-db/if

CDEF	=	-D${MACH_TYPE}
CINC	=	-I../../INCLUDE -I${INFORMIXDIR}/include

CFLAGS	=	-s -O3 ${CDEF} ${CINC} \
			-DDEBUG \
			-DOS_VER="\"`uname -r`\"" \
			-DVERSION="\"${VERSION}\"" \
			-DLIBNAME="\"$(LIBNAME)\""
LIB		=	../libdbif${VERSION}.a

SRCS	=	attrs.c \
			dbadd.c			dbalias.c \
			dbif.c			dbaudit.c		dbdelete.c \
			dbfield.c		dbfile.c		dbfind.c \
			dbinit.c		dblock.c \
			dbncomposite.c	dbselect.c		dbselfield.c \
			dbstructview.c	dbunlock.c		dbupdate.c \
			dbutils.c \
			errlog.c \
			gen-access.c	tstamp.c

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
