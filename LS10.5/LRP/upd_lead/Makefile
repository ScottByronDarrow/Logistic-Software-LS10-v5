#
# Makefile for upd_lead
#
SHELL	=	/bin/sh

OBJS	=	upd_lead.o

TARGET	=	$(DESTDIR)/upd_lead

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
