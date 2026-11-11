Q5: super.method(args)
Stage: Stage 8
Files changed: tnode.h, expr.l, expr.y, tnode.c
Test file(s): test_q05.txt
Expected: 13 43 13

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what was changed relative to your original stage8/ex1.
