#
# makefile for sk_nst_sel.
#

SHELL   = /bin/sh

OBJS = sk_nst_sel.o

TARGET = $(DESTDIR)/sk_nst_sel
DTARGET = $(DESTDIR)/sk_nst_vsel

all : $(TARGET) $(DTARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

$(DTARGET) :	${TARGET}
	-ln -f $(TARGET) $(DTARGET)
