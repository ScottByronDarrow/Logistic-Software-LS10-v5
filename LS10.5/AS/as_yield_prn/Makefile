#
# Makefile for as_yield_prn
#
SHELL	=	/bin/sh

OBJS	=	yield_prn.o

TARGET	=	$(DESTDIR)/as_yield_prn

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
