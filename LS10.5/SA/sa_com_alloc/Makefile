#
# Makefile for sa_com_alloc
#
SHELL	=	/bin/sh

OBJS	=	com_alloc.o

TARGET	=	$(DESTDIR)/sa_com_alloc

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
