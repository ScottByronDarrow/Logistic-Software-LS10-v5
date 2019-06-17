#
# makefile for sj_srmaint.
#
#	Generated - on : 16/11/89
#	          - at : 07:10:21 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_srmaint.o

TARGET = $(DESTDIR)/sj_srmaint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
