#
# Makefile for pm_proj_con
#
SHELL	=	/bin/sh

OBJS	=	proj_con.o

TARGET	=	$(DESTDIR)/pm_proj_con

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
