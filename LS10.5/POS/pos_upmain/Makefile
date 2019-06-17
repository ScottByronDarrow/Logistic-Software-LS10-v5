#
# Makefile for pos_upmain
#
SHELL	=	/bin/sh

OBJS	=	pos_upmain.o

TARGET	=	$(DESTDIR)/pos_upmain

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
