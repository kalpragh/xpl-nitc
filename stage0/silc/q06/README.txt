Q6: final methods
Stage: Stage 8
Files changed: classtable.h, classtable.c, expr.l, expr.y
Test file(s): test_q06.txt, test_q06_error.txt
Expected: 1 20 ; error file -> cannot override final

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what was changed relative to your original stage8/ex1.
