#
# makefile for sj_ivrpr.i.
#
#	Generated - on : 16/11/89
#	          - at : 07:03:15 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_ivrpr.i.o

TARGET = $(DESTDIR)/sj_ivrpr.t
NTARGET = $(DESTDIR)/sj_ivrpr.i

$(NTARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
	-mv $(TARGET) $(NTARGET)

#$(OBJS) : $(INC)/*.[hc]
