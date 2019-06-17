#
# Makefile for pm_prjt_inp
#
SHELL	=	/bin/sh

OBJS	=	prjt_inp.o

TARGET	=	$(DESTDIR)/pm_prjt_inp

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
