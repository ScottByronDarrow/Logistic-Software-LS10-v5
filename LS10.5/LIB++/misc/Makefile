#	$Id: Makefile,v 5.0 2001/06/19 08:19:06 cha Exp $
#
#	Misc library
#

include ../config

#################################################################
LIB			=	../libmisc.a

INC			=	-I../../INC++
DEF			=	-D${MACH_TYPE}
C++FLAGS	=	${INC} ${DEF} ${C++DBG} ${NDEBUG}
CFLAGS		=	${INC} ${DEF} ${CDBG} ${NDEBUG}

CSRCS		=	clip.c liberr.c padout.c wildmat.c
C++SRCS		=	ConfFile.C CoProcess.C \
				Date.C DebugLog.C \
				Money.C Number.C String.C

COBJS		=	${CSRCS:.c=.o}
C++OBJS		=	${C++SRCS:.C=.o}

SRCS		=	${CSRCS} ${C++SRCS}
OBJS		=	${COBJS} ${C++OBJS}

all:	depend ${LIB}

${LIB}:	${OBJS}
	${AR} rv $@ $?
	${AR} rv ../${LIBMAIN} $?

depend:	${SRCS}
	w4 ${INC} ${DEF} ${SRCS} > $@

clean:
	cp /dev/null depend
	-touch 01010000 depend
	rm -f *.o

include	depend
