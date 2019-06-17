#
# makefile for mn_store.
#
#	Generated - on : 23/08/89
#	          - at : 11:18:30 PM
#	          - by : scott
#

SHELL   = /bin/sh

OBJS = mn_store.o

MN_STORE = $(DESTDIR)/mn_store
MN_CONVERT = $(DESTDIR)/mn_convert
MN_CREATE = $(DESTDIR)/mn_create
MN_STOREALL = $(DESTDIR)/mn_storeall
MN_CONV_ALL = $(DESTDIR)/mn_conv_all
MN_CREATEALL = $(DESTDIR)/mn_createall

all : $(MN_STORE) $(MN_CONVERT) $(MN_CREATE) $(MN_STOREALL) $(MN_CONV_ALL) $(MN_CREATEALL)

$(MN_STORE) : $(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(MN_STORE) \
		$(OBJ) \
		$(LIB)

$(MN_CONVERT) : $(MN_STORE)
	-ln -f $(MN_STORE) $@
$(MN_CREATE) : $(MN_STORE)
	-ln -f $(MN_STORE) $@
$(MN_STOREALL) : $(MN_STORE)
	-ln -f $(MN_STORE) $@
$(MN_CONV_ALL) : $(MN_STORE)
	-ln -f $(MN_STORE) $@
$(MN_CREATEALL) : $(MN_STORE)
	-ln -f $(MN_STORE) $@

#$(OBJS) : $(INC)/*.[hc]
