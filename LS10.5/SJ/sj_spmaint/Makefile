#
# makefile for sj_spmaint.
#
#	Generated - on : 16/11/89
#	          - at : 07:09:56 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_spmaint.o

TARGET = $(DESTDIR)/sj_spmaint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
