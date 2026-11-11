lex expr.l
yacc -d expr.y
gcc y.tab.c lex.yy.c tnode.c symboltable.c typetable.c classtable.c -o expr
lex label.l
gcc lex.yy.c -o label
./expr test_q01.txt
./label output.xsm final.xsm
cd ../../..
./xsm -l xsm_dev/library.lib -e stage0/silc/q01/final.xsm