#
# makefile for psl_patch.
#
#	Generated - on : 07/11/91
#	          - at : 12:33:02 PM
#	          - by : trev
#

SHELL   = /bin/sh

OBJS = psl_patch.o

TARGET = $(DESTDIR)/psl_patch

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
