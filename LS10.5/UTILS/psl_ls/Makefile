#
# makefile for psl_ls.
#
#	Generated - on : 03/01/91
#	          - at : 09:28:10 AM
#	          - by : scott
#

SHELL   = /bin/sh

OBJS = psl_ls.o

TARGET = $(DESTDIR)/psl_ls

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
