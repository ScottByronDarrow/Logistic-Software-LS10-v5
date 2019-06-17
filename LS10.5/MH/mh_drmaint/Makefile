#
# makefile for mh_drmaint.
#
#	Generated - on : 23/08/89
#	          - at : 11:09:09 PM
#	          - by : scott
#

SHELL   = /bin/sh

OBJS = mh_drmaint.o

TARGET = $(DESTDIR)/mh_drmaint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
