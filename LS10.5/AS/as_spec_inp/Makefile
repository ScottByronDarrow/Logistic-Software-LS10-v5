#
# Makefile for as_spec_inp
#
SHELL	=	/bin/sh

OBJS	=	spec_inp.o

TARGET	=	$(DESTDIR)/as_spec_inp

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
