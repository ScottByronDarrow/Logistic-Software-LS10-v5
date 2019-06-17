#
# makefile for sj_dcmaint.
#
#	Generated - on : 16/11/89
#	          - at : 06:56:05 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_dcmaint.o

TARGET = $(DESTDIR)/sj_dcmaint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
