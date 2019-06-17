#
# Makefile for ml_input
#
SHELL	=	/bin/sh

OBJS	=	ml_input.o

TARGET	=	$(DESTDIR)/ml_input
PTARGET	=	$(DESTDIR)/ml_pinput

all : $(TARGET) $(PTARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)

$(PTARGET) : $(TARGET)
	ln -f $(TARGET) $@
