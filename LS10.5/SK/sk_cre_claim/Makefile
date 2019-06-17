#
# makefile for sk_cre_claim.
#
#	Generated - on : 10/11/93
#	          - at : 10:40:24 AM
#	          - by : simon
#

SHELL   = /bin/sh

OBJS = cre_claim.o

TARGET = $(DESTDIR)/sk_cre_claim
ATARGET = $(DESTDIR)/sk_cclaimrprt

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

	-ln -f $(TARGET) $(ATARGET)

#$(OBJS) : $(INC)/*.[hc]
