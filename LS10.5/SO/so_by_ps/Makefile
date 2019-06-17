#
# Makefile for so_by_ps
#
SHELL	=	/bin/sh

OBJS	=	so_by_ps.o

TARGET	=	$(DESTDIR)/so_by_ps

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
