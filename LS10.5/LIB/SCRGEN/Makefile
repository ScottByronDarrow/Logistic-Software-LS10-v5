#
# Make Script for std libscrgen.a
#
SHELL	=	/bin/sh
LIBNAME	=	ScrGen

CDEF	=	-D${MACH_TYPE}
CINC	=	-I../../INCLUDE -I${INFORMIXDIR}/include

CFLAGS	=	-s -O3 ${CDEF} ${CINC} \
			-DDEBUG \
			-DOS_VER="\"`uname -r`\"" \
			-DVERSION="\"${VERSION}\"" \
			-DLIBNAME="\"$(LIBNAME)\""
LIB		=	../libscrgen$(VERSION).a

SRCS	=	heading.c \
			pslscr.c \
			screen_set.c \
			edit_end_eall.c \
			use_window.c \
			win_func.c \
			win_func2.c \
			win_func3.c \
			win_func4.c \
			win_func5.c

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
