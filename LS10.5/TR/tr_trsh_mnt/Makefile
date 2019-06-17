#
# Makefile for tr_trsh_mnt
#
SHELL	=	/bin/sh

OBJS	=	trsh_mnt.o

TARGET	=	$(DESTDIR)/tr_trsh_mnt

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
