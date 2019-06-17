#
# Makefile for fa_regprt
#
SHELL	=	/bin/sh

OBJS	=	fa_regprt.o

TARGET	=	$(DESTDIR)/fa_regprt

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
