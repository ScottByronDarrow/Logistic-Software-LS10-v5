#
# Makefile for as_ser_inp
#
SHELL	=	/bin/sh

OBJS	=	as_ser_inp.o

TARGET	=	$(DESTDIR)/as_ser_inp

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
