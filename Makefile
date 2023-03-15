EXEC_NAME = compiler

all : $(EXEC_NAME)

$(EXEC_NAME) : added.o lex.yy.c exp.tab.c
	gcc -c lex.yy.c
	gcc -c exp.tab.c
	gcc -o compiler added.o lex.yy.o exp.tab.o
	rm -rf added.o exp.tab.o lex.yy.o

added.o : added.c
	gcc -c added.c -std=c99

lex.yy.c : exp.l
	lex exp.l

exp.tab.c : exp.y
	bison -d exp.y

clean:
	rm -rf lex.yy.c exp.tab.c exp.tab.h compiler added.o exp.tab.o lex.yy.o
