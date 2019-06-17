#
# Makefile for gen_scnfile
#
SHELL	=	/bin/sh

OBJS	=	_scnfile.o

TARGET	=	$(DESTDIR)/gen_scnfile

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
