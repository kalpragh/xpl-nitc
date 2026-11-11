Q1: Modulo operator %
Stage: Stage 5
Files changed: expr.y, tnode.c
Test file(s): test_q01.txt
Expected: run with input: 17 5  ->  2 6

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what was changed relative to your original stage8/ex1.
