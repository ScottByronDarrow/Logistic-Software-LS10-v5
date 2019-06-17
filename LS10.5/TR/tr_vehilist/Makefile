#
# Makefile for tr_vehilist
#
SHELL	=	/bin/sh

OBJS	=	vehilist.o

TARGET	=	$(DESTDIR)/tr_vehilist

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
