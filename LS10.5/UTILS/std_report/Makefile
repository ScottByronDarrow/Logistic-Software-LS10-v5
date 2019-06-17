#
# Makefile for std_report
#
SHELL	=	/bin/sh

OBJS	=	std_report.o

TARGET	=	$(DESTDIR)/std_report

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $@ \
		$(OBJ) \
		$(LIB)
