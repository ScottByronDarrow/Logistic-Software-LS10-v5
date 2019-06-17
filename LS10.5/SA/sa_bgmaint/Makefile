#
# makefile for sa_bgmaint.
#
#	Generated - on : 23/08/89
#	          - at : 11:14:35 PM
#	          - by : scott
#

SHELL   = /bin/sh

OBJS = sa_bgmaint.o

TARGET = $(DESTDIR)/sa_bgmaint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
