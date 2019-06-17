#
# Makefile for tr_trzt_mnt
#
SHELL	=	/bin/sh

OBJS	=	trzt_mnt.o

TARGET	=	$(DESTDIR)/tr_trzt_mnt

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
