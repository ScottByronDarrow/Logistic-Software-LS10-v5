#
# Makefile for pm_prj_dtl
#
SHELL	=	/bin/sh

OBJS	=	pm_prj_dtl.o

TARGET	=	$(DESTDIR)/pm_prj_dtl

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
