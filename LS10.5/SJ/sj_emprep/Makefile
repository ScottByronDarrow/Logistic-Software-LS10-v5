#
# makefile for sj_emprep.
#
#	Generated - on : 16/11/89
#	          - at : 06:58:08 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_emprep.o

TARGET = $(DESTDIR)/sj_emprep

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
