#
# makefile for sa_bgmaint.i.
#
#	Generated - on : 23/08/89
#	          - at : 11:15:45 PM
#	          - by : scott
#

SHELL   = /bin/sh

OBJS = bgmaint.i.o

TARGET = $(DESTDIR)/sa_bgmaint.t
NTARGET = $(DESTDIR)/sa_bgmaint.i

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
	-mv $(TARGET) $(NTARGET)

#$(OBJS) : $(INC)/*.[hc]
