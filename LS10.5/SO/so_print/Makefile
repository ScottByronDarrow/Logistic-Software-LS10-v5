#
# Makefile for so_print
#
SHELL	=	/bin/sh

OBJS	=	so_print.o

TARGET	=	$(DESTDIR)/so_print

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
