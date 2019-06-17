#
# Makefile for as_type_inp
#
SHELL	=	/bin/sh

OBJS	=	type_inp.o

TARGET	=	$(DESTDIR)/as_type_inp

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
