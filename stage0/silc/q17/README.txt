Q17: && and || operators
Stage: Stage 3-5
Files changed: expr.l, expr.y, tnode.h, tnode.c
Test file(s): test_q17.txt
Expected: input 3 4 -> 1 11 20 ; input 1 0 -> 0 11 21

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what was changed relative to your original stage8/ex1.
