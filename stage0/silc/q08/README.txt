Q8: for loop (continue runs the update)
Stage: Stage 5-6
Files changed: expr.l, expr.y, tnode.c
Test file(s): test_q08.txt
Expected: 13 6

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what was changed relative to your original stage8/ex1.
