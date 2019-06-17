#
# Makefile for ol_dbmaint
#
SHELL	=	/bin/sh

OBJS	=	ol_dbmaint.o

TARGET	=	$(DESTDIR)/ol_dbmaint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
