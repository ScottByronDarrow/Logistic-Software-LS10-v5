#
# makefile for ts_ld_sheet.
#
#	Generated - on : 16/06/92
#	          - at : 02:41:52 PM
#	          - by : cam
#

SHELL   = /bin/sh

OBJS = ld_sheet.o

TARGET = $(DESTDIR)/ts_ld_sheet

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

#$(OBJS) : $(INC)/*.[hc]
