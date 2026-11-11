Q9: instanceof(var, Class)
Stage: Stage 8
Files changed: tnode.h, classtable.h, classtable.c, expr.l, expr.y, tnode.c
Test file(s): test_q09.txt
Expected: 1 1 0 0 100

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what was changed relative to your original stage8/ex1.
