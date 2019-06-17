#
# Makefile for fa_depprt
#
SHELL	=	/bin/sh

OBJS	=	fa_depprt.o

TARGET	=	$(DESTDIR)/fa_depprt

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
