#
# makefile for sk_cusreass.
#
#	Generated - on : 04/11/93
#	          - at : 10:32:47 AM
#	          - by : simon
#

SHELL   = /bin/sh

OBJS = cusreass.o

TARGET = $(DESTDIR)/sk_cusreass

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
