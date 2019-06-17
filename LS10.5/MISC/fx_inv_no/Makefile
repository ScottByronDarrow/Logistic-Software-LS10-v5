#
# Makefile for fx_inv_no
#
SHELL	=	/bin/sh

OBJS	=	fx_inv_no.o

TARGET	=	$(DESTDIR)/fx_inv_no

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
