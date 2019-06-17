#
# Makefile for fe_exp_rep
#
SHELL	=	/bin/sh

OBJS	=	fe_exp_rep.o

TARGET	=	$(DESTDIR)/fe_exp_rep

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)
