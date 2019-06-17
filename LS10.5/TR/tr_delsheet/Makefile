#
# Makefile for tr_delsheet
#
SHELL	=	/bin/sh

OBJS	=	delsheet.o

TARGET	=	$(DESTDIR)/tr_delsheet

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
