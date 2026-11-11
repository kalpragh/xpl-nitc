Q4: public fields in classes
Stage: Stage 7
Files changed: typetable.h, expr.l, expr.y, classtable.c, tnode.c
Test file(s): test_q04.txt, test_q04_error.txt
Expected: 12 7 ; error file -> private field error

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what was changed relative to your original stage8/ex1.
