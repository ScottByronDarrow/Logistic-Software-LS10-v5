#
# Makefile for as_move_prn
#
SHELL	=	/bin/sh

OBJS	=	move_prn.o

TARGET	=	$(DESTDIR)/as_move_prn

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
