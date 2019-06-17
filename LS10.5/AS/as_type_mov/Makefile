#
# Makefile for as_type_mov
#
SHELL	=	/bin/sh

OBJS	=	type_mov.o

TARGET	=	$(DESTDIR)/as_type_mov

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
