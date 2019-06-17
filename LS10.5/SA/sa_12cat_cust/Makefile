#
# Makefile for sa_12cat_cust
#
SHELL	=	/bin/sh

OBJS	=	12cat_cust.o

TARGET	=	$(DESTDIR)/sa_12cat_cust

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
