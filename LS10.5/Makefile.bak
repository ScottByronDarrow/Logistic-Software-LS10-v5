#
#	Makefile to compile the entire system
#
#
SHELL	=	/bin/sh

#
#	UTILS needs to be compiled first
#
all:
	for i in ?? LRP POS UTILS MENU MISC WEB ; \
	do \
		if [ -f $$i/Makefile ] ; \
		then \
  			(cd $$i; rm -f */*.o >/dev/null) ; \
  			(cd $$i; rm -f */schema >/dev/null) ; \
  			(cd $$i; rm -f */depend >/dev/null) ; \
			(cd $$i; ${MAKE}) ; \
		fi \
	done
