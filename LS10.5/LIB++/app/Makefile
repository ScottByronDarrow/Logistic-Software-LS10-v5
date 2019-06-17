#	$Id: Makefile,v 5.0 2001/06/19 08:16:40 cha Exp $
#
#	Screen Interface library
#

include ../config

#############################################################################
LIB			=	../libapp.a

C++INC		=	-I../../INC++
C++DEF		=	-D${MACH_TYPE}
C++FLAGS	=	${C++DBG} ${C++INC} ${C++DEF} ${NDEBUG}

SRCS		=	Environ.C FormatP.C LpNumber.C UnixEnv.C \
                LogisticEnv.C LogisticEnvFile.C WorkFile.C
OBJS		=	${SRCS:.C=.o}

all:	depend ${LIB}

${LIB}:	${OBJS}
	${AR} rv $@ $?
	${AR} rv ../${LIBMAIN} $?

depend:	${SRCS}
	w4 ${C++INC} ${C++DEF} ${SRCS} > $@

clean:
	cp /dev/null depend
	-touch 01010000 depend
	rm -f *.o

include	depend
