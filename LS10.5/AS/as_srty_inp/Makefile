#
# Makefile for as_srty_inp
#
SHELL	=	/bin/sh

OBJS	=	srty_inp.o

TARGET	=	$(DESTDIR)/as_srty_inp

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
