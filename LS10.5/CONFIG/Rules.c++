#
#	Rules for building C++ files
#	for systems with make(1) that don't know about them.
#

.C.o:
	$(C++) $(C++FLAGS) -c $<

.C.a:
	$(C++) $(C++FLAGS) -c $<
	$(AR) $(ARFLAGS) $@ $(<F:.C=.o)
	-rm -f $(<F:.C=.o)

.C:
	$(C++) $(C++FLAGS) -o $@ $< $(LDFLAGS)

.SUFFIXES:	.C
