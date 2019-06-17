#
# Makefile for so_comm_upd
#
SHELL	=	/bin/sh

OBJS	=	comm_upd.o

TARGET	=	$(DESTDIR)/so_comm_upd

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
