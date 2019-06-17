#
# makefile for psl_dbstat.
#
#	Generated - on : 06/10/93
#	          - at : 03:43:52 PM
#	          - by : scott
#

SHELL   = /bin/sh

OBJS = psl_dbstat.o

TARGET = $(DESTDIR)/psl_dbstat

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
