#
# Makefile for pm_prjcon_list
#
SHELL	=	/bin/sh

OBJS	=	prjcon_list.o

TARGET	=	$(DESTDIR)/pm_prjcon_list

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
