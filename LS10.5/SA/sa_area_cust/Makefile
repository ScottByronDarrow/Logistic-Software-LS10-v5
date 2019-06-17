#
# Makefile for sa_area_cust
#
SHELL	=	/bin/sh

OBJS	=	area_cust.o

TARGET	=	$(DESTDIR)/sa_area_cust

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
