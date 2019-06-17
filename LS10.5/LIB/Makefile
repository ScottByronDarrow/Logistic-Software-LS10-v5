#	$Id: Makefile,v 5.1 2002/07/17 09:31:30 scott Exp $
#
#	Makefile for library build.
#
SHELL		=	/bin/sh
BASEDIRS	=	BASE CISAM-DBF SCRGEN DSO.Xtra
OPTDIRS		=	ORACLE7-DBIF

base:
	for d in ${BASEDIRS} ; \
	do \
		(cd $$d; ${MAKE}); \
	done

all:	base
	for d in ${OPTDIRS} ; \
	do \
		(cd $$d; ${MAKE}); \
	done

oracle:
	cd ORACLE7-DBIF; ${MAKE}

clean:
	-for d in ${BASEDIRS} ${OPTDIRS} ; \
	do \
		(cd $$d; cp /dev/null depend; touch 01010000 depend; ${MAKE} clean); \
	done
	rm -f *.a
