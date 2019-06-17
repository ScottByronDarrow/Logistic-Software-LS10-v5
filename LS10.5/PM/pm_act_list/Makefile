#
# Makefile for pm_act_list
#
SHELL	=	/bin/sh

OBJS	=	act_list.o

TARGET	=	$(DESTDIR)/pm_act_list

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
