#
# Makefile for pm_del_proj
#
SHELL	=	/bin/sh

OBJS	=	del_proj.o

TARGET	=	$(DESTDIR)/pm_del_proj

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
