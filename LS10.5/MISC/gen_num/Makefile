#
# Makefile for gen_num
#
SHELL	=	/bin/sh

OBJS	=	gen_num.o

TARGET	=	$(DESTDIR)/gen_num

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
