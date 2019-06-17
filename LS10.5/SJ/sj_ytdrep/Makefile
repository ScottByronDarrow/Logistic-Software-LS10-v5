#
# makefile for sj_ytdrep.
#
#	Generated - on : 16/11/89
#	          - at : 07:12:25 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_ytdrep.o

TARGET = $(DESTDIR)/sj_ytdrep

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
