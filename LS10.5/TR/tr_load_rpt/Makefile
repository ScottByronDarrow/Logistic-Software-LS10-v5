#
# Makefile for tr_load_rpt
#
SHELL	=	/bin/sh

OBJS	=	load_rpt.o

TARGET	=	$(DESTDIR)/tr_load_rpt

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
