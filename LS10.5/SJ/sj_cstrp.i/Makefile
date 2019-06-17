#
# makefile for sj_cstrp.i.
#
#	Generated - on : 16/11/89
#	          - at : 06:55:19 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_cstrp.i.o

TARGET = $(DESTDIR)/sj_cstrp.t
NTARGET = $(DESTDIR)/sj_cstrp.i

$(NTARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
	-mv $(TARGET) $(NTARGET)

#$(OBJS) : $(INC)/*.[hc]
