#
# makefile for sj_dpsel.i.
#
#	Generated - on : 16/11/89
#	          - at : 06:57:29 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = sj_dpsel.i.o

TARGET = $(DESTDIR)/sj_dpsel.t
NTARGET = $(DESTDIR)/sj_dpsel.i

$(NTARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
	-mv $(TARGET) $(NTARGET)

#$(OBJS) : $(INC)/*.[hc]
