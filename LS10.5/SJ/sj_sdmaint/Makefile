#
# makefile for sj_sdmaint.
#
#	Generated - on : 16/11/89
#	          - at : 07:09:31 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_sdmaint.o

TARGET = $(DESTDIR)/sj_sdmaint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
