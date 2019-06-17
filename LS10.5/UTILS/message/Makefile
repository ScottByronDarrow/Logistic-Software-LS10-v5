#
# Makefile for message
#
SHELL	=	/bin/sh

OBJS	=	message.o

TARGET	=	$(DESTDIR)/message

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB) -lcurses

