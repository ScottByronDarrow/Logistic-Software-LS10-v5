#
# Makefile for tr_trzm_mnt
#
SHELL	=	/bin/sh

OBJS	=	trzm_mnt.o

TARGET	=	$(DESTDIR)/tr_trzm_mnt

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
