#
#	Makefile for system
#
SHELL	=	/bin/sh

DIRS	=	app cisam-if misc

all:
	for i in ${DIRS} ; \
	do \
		(cd $$i; ${MAKE}); \
	done

#
#	With releases, recompile everything with
#	the NDEBUG flag set
#
rls:	clean
	for i in ${DIRS} ; \
	do \
		(cd $$i; ${MAKE} "NDEBUG=-DNDEBUG"); \
	done

clean:
	-for i in ${DIRS} ; \
	do \
		(cd $$i; rm -f depend; touch 01010000 depend; ${MAKE} clean); \
	done
	rm -f *.a
