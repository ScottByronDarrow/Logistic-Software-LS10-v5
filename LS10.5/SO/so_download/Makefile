#
# Makefile for so_download
#
SHELL	=	/bin/sh

OBJS	=	download.o

TARGET	=	$(DESTDIR)/so_download

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
