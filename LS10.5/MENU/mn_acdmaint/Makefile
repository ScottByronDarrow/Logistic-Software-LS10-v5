#
# makefile for mn_acdmaint.
#
#	Generated - on : 18/10/91
#	          - at : 12:10:37 PM
#	          - by : cam
#

SHELL   = /bin/sh

OBJS = acdmaint.o

TARGET = $(DESTDIR)/mn_acdmaint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
