#
# Makefile for pm_pq_maint
#
SHELL	=	/bin/sh

OBJS	=	pq_maint.o

TARGET	=	$(DESTDIR)/pm_pq_maint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
