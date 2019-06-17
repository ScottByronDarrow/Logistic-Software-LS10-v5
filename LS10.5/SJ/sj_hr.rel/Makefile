#
# makefile for sj_hr.rel.
#
#	Generated - on : 16/11/89
#	          - at : 06:58:48 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_hr.rel.o

TARGET = $(DESTDIR)/sj_hr.rel

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
