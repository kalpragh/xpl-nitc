set -e
lex expr.l
yacc -d expr.y
gcc -w -o compiler y.tab.c lex.yy.c tnode.c symboltable.c typetable.c classtable.c
lex label.l
gcc -w -o label lex.yy.c
lex expr.l
