#
# makefile for chain.
#

SHELL   = /bin/sh

OBJS = chain.o

TARGET1 = $(DESTDIR)/chain
TARGET2 = $(DESTDIR)/lp_chain
TARGET3 = $(DESTDIR)/su_chain

all	:	$(TARGET1) $(TARGET2) $(TARGET3)

$(TARGET1) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)

$(TARGET2) :	$(TARGET1)
	ln -f $(TARGET1) $@

$(TARGET3) :	$(TARGET1)
	cp $(TARGET1) $@
	echo "Required: chown root $@; chmod u+s $@"
