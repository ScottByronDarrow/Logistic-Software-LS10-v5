#
# makefile for mn_menumaint.
#
#	Generated - on : 18/10/91
#	          - at : 12:13:40 PM
#	          - by : cam
#

SHELL   = /bin/sh

OBJS = menumaint.o

TARGET = $(DESTDIR)/mn_menumaint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
