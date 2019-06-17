#
# Makefile for sk_mbaldel
#
SHELL	=	/bin/sh

OBJS	=	sk_mbaldel.o

TARGET	=	$(DESTDIR)/sk_mbaldel

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
