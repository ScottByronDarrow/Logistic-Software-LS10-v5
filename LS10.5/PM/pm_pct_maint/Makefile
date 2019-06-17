#
# Makefile for pm_pct_maint
#
SHELL	=	/bin/sh

OBJS	=	pct_maint.o

TARGET	=	$(DESTDIR)/pm_pct_maint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
