#
# Makefile for tr_truck_list
#
SHELL	=	/bin/sh

OBJS	=	truck_list.o

TARGET	=	$(DESTDIR)/tr_truck_list

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
