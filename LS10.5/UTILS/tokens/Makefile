#
# makefile for tokens.
#
#	Generated - on : 02/11/93
#	          - at : 03:42:48 PM
#	          - by : jonc
#

SHELL   = /bin/sh

OBJS = tokens.o

TARGET = $(DESTDIR)/tokens

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
