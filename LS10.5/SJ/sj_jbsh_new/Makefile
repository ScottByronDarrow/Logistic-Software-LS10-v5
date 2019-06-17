#
# makefile for sj_jbsh_new.
#
#	Generated - on : 16/11/89
#	          - at : 07:03:38 PM
#	          - by : scott
#

SHELL   = /bin/sh
GET     = get -r$(VER_NUM)

OBJS = jbsh_new.o

TARGET = $(DESTDIR)/sj_jbsh_new

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
