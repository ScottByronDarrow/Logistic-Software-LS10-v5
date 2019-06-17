#
# makefile for env_maint.
#
#	Generated - on : 23/08/89
#	          - at : 10:45:19 PM
#	          - by : scott
#

SHELL   = /bin/sh

OBJS = env_maint.o

TARGET = $(DESTDIR)/env_maint

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
