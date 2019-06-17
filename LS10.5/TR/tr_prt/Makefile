#
# Makefile for tr_prt
#
SHELL	=	/bin/sh

OBJS	=	tr_prt.o

TARGET	=	$(DESTDIR)/tr_prt

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
