SHELL	=	/bin/sh

LEX		=	flex

TARGET	=	$(DESTDIR)/buildsql
OBJS	=	buildsql.o diff.o lexer.o parser.o tbldef.o

$(TARGET)	:	$(OBJS)
	$(CC) $(CFLAGS) \
		$(OBJS) \
		-o $(TARGET) \
		$(OBJ) \
		$(LIB)

buildsql.o	:	commdefs.h diff.h tbldef.h
diff.o		:	commdefs.h diff.h tbldef.h
lexer.o		:	commdefs.h tbldef.h y.tab.h
parser.o	:	commdefs.h tbldef.h
tbldef.o	:	commdefs.h tbldef.h

y.tab.h		:	parser.y
	$(YACC) -d parser.y
	rm -f y.tab.c
