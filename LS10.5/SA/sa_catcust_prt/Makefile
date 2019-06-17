#
# Makefile for sa_catcust_prt
#
SHELL	=	/bin/sh

OBJS	=	catcust_prt.o

TARGET	=	$(DESTDIR)/sa_catcust_prt

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
