#
# Makefile for pm_act_maint
#
SHELL	=	/bin/sh

OBJS	=	act_maint.o

TARGET	=	$(DESTDIR)/pm_act_maint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
