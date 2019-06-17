#
# makefile for sj_ispurge.
#
#	Generated - on : 28/12/89
#	          - at : 05:06:12 PM
#	          - by : choo
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_ispurge.o

TARGET = $(DESTDIR)/sj_ispurge

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
