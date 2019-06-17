#	$Id: Makefile,v 5.0 2001/06/19 08:17:30 cha Exp $
#
#	Database Interface library
#
#		CISAM version
#
include	../config

############################# No config below this line ###################
LIB			=	../libcisam-if.a

C++INC		=	-I../../INC++ \
				-I${INFORMIXDIR}/include -I${INFORMIXDIR}/incl/esql
C++DEF		=	-D${MACH_TYPE} -DINFORMIXVERSION=${INFORMIXVN}
C++FLAGS	=	${C++DBG} ${C++INC} ${C++DEF} ${NDEBUG}

SRCS		=	ColInfo.C Constraint.C Index.C Database.C Table.C Query.C
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
