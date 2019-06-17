#
#	Makefile to compile the subsystem
#
SHELL = /bin/sh

all :
	MODULEDIR=`pwd`; \
	DESTDIR=$$PROG_PATH/BIN/`basename $$MODULEDIR`; export DESTDIR; \
	if [ ! -d $$DESTDIR ] ; \
	then \
		mkdir -p $$DESTDIR; \
	fi; \
	for d in * ; \
	do \
		if [ -f $$d/Makefile ] ; \
		then \
			(cd $$d; ${MAKE}) ; \
		fi \
	done
