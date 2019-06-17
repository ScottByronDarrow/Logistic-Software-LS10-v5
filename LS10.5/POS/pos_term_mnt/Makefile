#
# Makefile for pos_term_mnt
#
SHELL	=	/bin/sh

OBJS	=	_term_mnt.o

TARGET	=	$(DESTDIR)/pos_term_mnt

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
