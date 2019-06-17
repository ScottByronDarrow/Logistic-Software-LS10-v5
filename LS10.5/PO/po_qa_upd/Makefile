#
# Makefile for po_qa_upd
#
SHELL	=	/bin/sh

OBJS	=	po_qa_upd.o

TARGET	=	$(DESTDIR)/po_qa_upd

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
